#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <string.h>
#include <gcin.h>
#include "gcin-protocol.h"
#include "gcin-im-client.h"
#include "im-srv.h"

#define DBG 0

GCIN_ENT *gcin_clients;
int gcin_clientsN;
extern GCIN_PASSWD my_passwd;

gboolean ProcessKeyPress(KeySym keysym, u_int kev_state);
int gcin_FocusIn(ClientState *cs);
int gcin_FocusOut(ClientState *cs);
void update_in_win_pos();

extern char *output_buffer;
extern int output_bufferN;

int write_enc(int fd, void *p, int n)
{

  if (!fd)
    return 0;

  char *tmp = malloc(n);
  memcpy(tmp, p, n);

  if (gcin_clients[fd].type == Connection_type_tcp) {
    __gcin_enc_mem(tmp, n, &srv_ip_port.passwd, &gcin_clients[fd].seed);
  }

  int r =  write(fd, tmp, n);
  free(tmp);

  return r;
}

static void shutdown_client(int fd)
{
//  dbg("client shutdown rn %d\n", rn);
  gdk_input_remove(gcin_clients[fd].tag);

  if (gcin_clients[fd].cs == current_CS)
    current_CS = NULL;

  free(gcin_clients[fd].cs);
  gcin_clients[fd].cs = NULL;

  close(fd);
}

gboolean ProcessKeyRelease(KeySym keysym, u_int kev_state);

void process_client_req(int fd)
{
  GCIN_req req;

//  printf("process_client_req %d\n", fd);

  int rn = read(fd, &req, sizeof(req));

  if (rn <= 0) {
    shutdown_client(fd);
    return;
  }

  if (gcin_clients[fd].type == Connection_type_tcp) {
    __gcin_enc_mem((u_char *)&req, sizeof(req), &srv_ip_port.passwd, &gcin_clients[fd].seed);
  }

  to_gcin_endian_4(&req.req_no);
  to_gcin_endian_4(&req.client_win);
  to_gcin_endian_4(&req.flag);
  to_gcin_endian_2(&req.spot_location.x);
  to_gcin_endian_2(&req.spot_location.y);

//  dbg("spot %d %d\n", req.spot_location.x, req.spot_location.y);

  ClientState *cs = NULL;

  if (current_CS && req.client_win == current_CS->client_win) {
    cs = current_CS;
  } else {
    cs = gcin_clients[fd].cs;

    if (!cs) {
      cs = gcin_clients[fd].cs = tzmalloc(ClientState, 1);
    }

    cs->client_win = req.client_win;
    cs->b_gcin_protocol = TRUE;
    cs->input_style = InputStyleOverSpot;
  }

  if (!cs)
    p_err("bad cs\n");

  cs->spot_location.x = req.spot_location.x;
  cs->spot_location.y = req.spot_location.y;

  gboolean status;
  GCIN_reply reply;
  bzero(&reply, sizeof(reply));

  switch (req.req_no) {
    case GCIN_req_key_press:
    case GCIN_req_key_release:
      current_CS = cs;
#if 0
      {
        char tt[128];

        if (req.keyeve.key < 127) {
          sprintf(tt,"'%c'", req.keyeve.key);
        } else {
          strcpy(tt, XKeysymToString(req.keyeve.key));
        }

//        dbg_time("GCIN_key_press  %x %s\n", cs, tt);
      }
#endif
      to_gcin_endian_4(&req.keyeve.key);
      to_gcin_endian_4(&req.keyeve.state);

//      dbg("serv key eve %x %x\n",req.keyeve.key, req.keyeve.state);

      if (req.req_no==GCIN_req_key_press)
        status = ProcessKeyPress(req.keyeve.key, req.keyeve.state);
      else {
        status = ProcessKeyRelease(req.keyeve.key, req.keyeve.state);
      }

      if (status) {
        reply.flag |= GCIN_reply_key_processed;
      }

      int datalen = reply.datalen =
        output_bufferN ? output_bufferN + 1 : 0; // include '\0'
      to_gcin_endian_4(&reply.flag);
      to_gcin_endian_4(&reply.datalen);
      write_enc(fd, &reply, sizeof(reply));

//      dbg("server reply.flag %x\n", reply.flag);

      if (output_bufferN) {
        write_enc(fd, output_buffer, datalen);
        clear_output_buffer();
      }

      break;
    case GCIN_req_focus_in:
#if DBG
      dbg_time("GCIN_req_focus_in  %x %d %d\n",cs, cs->spot_location.x, cs->spot_location.y);
#endif
      gcin_FocusIn(cs);
      break;
    case GCIN_req_focus_out:
#if DBG
      dbg_time("GCIN_req_focus_out  %x\n", cs);
#endif
      gcin_FocusOut(cs);
      break;
    case GCIN_req_set_cursor_location:
#if DBG
      dbg_time("set_cursor_location %x %d %d\n", cs,
         cs->spot_location.x, cs->spot_location.y);
#endif
      update_in_win_pos();
      break;
    case GCIN_req_set_flags:
      if (BITON(req.flag, FLAG_GCIN_client_handle_raise_window)) {
        cs->b_raise_window = TRUE;
      }
      break;
    default:
      dbg_time("Invalid request %x from:", req.req_no);

      struct sockaddr_in addr;
      int len=sizeof(addr);
      bzero(&addr, sizeof(addr));

      if (!getpeername(fd, (struct sockaddr *)&addr, &len)) {
        u_char addr_ch[4];
        memcpy(addr_ch, &addr.sin_addr.s_addr,sizeof(addr.sin_addr.s_addr));
        dbg("%d %d %d %d\n", addr_ch[0], addr_ch[1], addr_ch[2], addr_ch[3]);
      } else {
        perror("getpeername\n");
      }

      shutdown_client(fd);
      break;
  }
}
