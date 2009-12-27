#if UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <string.h>
#include "gcin.h"
#include "gcin-protocol.h"
#include "gcin-im-client.h"
#include "im-srv.h"
#include <gtk/gtk.h>

#define DBG 0

static int myread(int fd, void *buf, int bufN)
{
#if UNIX
  return read(fd, buf, bufN);
#else
  return recv(fd, (char *)buf, bufN, 0);
#endif
}


GCIN_ENT *gcin_clients;
int gcin_clientsN;
extern GCIN_PASSWD my_passwd;

gboolean ProcessKeyPress(KeySym keysym, u_int kev_state);
int gcin_FocusIn(ClientState *cs);
int gcin_FocusOut(ClientState *cs);
void update_in_win_pos();
void hide_in_win(ClientState *cs);
void init_state_chinese(ClientState *cs);
void clear_output_buffer();
void flush_edit_buffer();
int gcin_get_preedit(ClientState *cs, char *str, GCIN_PREEDIT_ATTR attr[], int *cursor);
void gcin_reset();
void dbg_time(char *fmt,...);

extern char *output_buffer;
extern int output_bufferN;

int write_enc(int fd, void *p, int n)
{
#if WIN32
  int r = send(fd, (char *)p, n, 0);
  if (r < 0)
    perror("write_enc");
  return r;
#else
  if (!fd)
    return 0;

  unsigned char *tmp = (unsigned char *)malloc(n);
  memcpy(tmp, p, n);
  if (gcin_clients[fd].type == Connection_type_tcp) {
    __gcin_enc_mem(tmp, n, &srv_ip_port.passwd, &gcin_clients[fd].seed);
  }
  int r =  write(fd, tmp, n);

#if DBG
  if (r < 0)
    perror("write_enc");
#endif

  free(tmp);

  return r;
#endif
}

#ifdef __cplusplus
extern "C" void gdk_input_remove	  (gint		     tag);
#endif

static void shutdown_client(int fd)
{
//  dbg("client shutdown rn %d\n", rn);
  gdk_input_remove(gcin_clients[fd].tag);

  if (gcin_clients[fd].cs == current_CS) {
    hide_in_win(current_CS);
    current_CS = NULL;
  }

  free(gcin_clients[fd].cs);
  gcin_clients[fd].cs = NULL;
#if UNIX
  close(fd);
#else
  closesocket(fd);
#endif
}

gboolean ProcessKeyRelease(KeySym keysym, u_int kev_state);
void message_cb(char *message);

void process_client_req(int fd)
{
  GCIN_req req;
//  dbg("svr--> process_client_req %d\n", fd);
  int rn = myread(fd, &req, sizeof(req));

  if (rn <= 0) {
    shutdown_client(fd);
    return;
  }
#if UNIX
  if (gcin_clients[fd].type == Connection_type_tcp) {
    __gcin_enc_mem((u_char *)&req, sizeof(req), &srv_ip_port.passwd, &gcin_clients[fd].seed);
  }
#endif
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

    int new_cli = 0;
    if (!cs) {
      cs = gcin_clients[fd].cs = tzmalloc(ClientState, 1);
      new_cli = 1;
    }

    cs->client_win = req.client_win;
    cs->b_gcin_protocol = TRUE;
    cs->input_style = InputStyleOverSpot;

    if (gcin_init_im_enabled && new_cli) {
      current_CS = cs;
      init_state_chinese(cs);
    }
  }

  if (!cs)
    p_err("bad cs\n");

  if (req.req_no != GCIN_req_message) {
    cs->spot_location.x = req.spot_location.x;
    cs->spot_location.y = req.spot_location.y;
  }

  gboolean status;
  GCIN_reply reply;
  bzero(&reply, sizeof(reply));

  switch (req.req_no) {
    case GCIN_req_key_press:
    case GCIN_req_key_release:
      current_CS = cs;
#if DBG && 0
      {
        char tt[128];

        if (req.keyeve.key < 127) {
          sprintf(tt,"'%c'", req.keyeve.key);
        } else {
          strcpy(tt, XKeysymToString(req.keyeve.key));
        }

        dbg_time("GCIN_key_press  %x %s\n", cs, tt);
      }
#endif
      to_gcin_endian_4(&req.keyeve.key);
      to_gcin_endian_4(&req.keyeve.state);

//	  dbg("serv key eve %x %x predit:%d\n",req.keyeve.key, req.keyeve.state, cs->use_preedit);

#if DBG
	  char *typ;
      typ="press";
#endif
      if (req.req_no==GCIN_req_key_press)
        status = ProcessKeyPress(req.keyeve.key, req.keyeve.state);
      else {
        status = ProcessKeyRelease(req.keyeve.key, req.keyeve.state);
#if DBG
        typ="rele";
#endif
      }

      if (status)
        reply.flag |= GCIN_reply_key_processed;
#if DBG
      dbg("%s srv flag:%x status:%d len:%d %x %c\n",typ, reply.flag, status, output_bufferN, req.keyeve.key,req.keyeve.key & 0x7f);
#endif
      int datalen;
      datalen = reply.datalen =
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
    case GCIN_req_focus_out2:
      {
#if DBG
      dbg_time("GCIN_req_focus_out2  %x\n", cs);
#endif
      gcin_FocusOut(cs);
      flush_edit_buffer();

      GCIN_reply reply;
      bzero(&reply, sizeof(reply));

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
      }
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
#if DBG
        dbg("********* raise * window\n");
#endif
        if (!gcin_pop_up_win)
          cs->b_raise_window = TRUE;
      }

	  if (req.flag & FLAG_GCIN_client_handle_use_preedit)
        cs->use_preedit = TRUE;

      int rflags;
      rflags = 0;
      if (gcin_pop_up_win)
        rflags = FLAG_GCIN_srv_ret_status_use_pop_up;

      write_enc(fd, &rflags, sizeof(rflags));
      break;
    case GCIN_req_get_preedit:
      {
#if DBG
      dbg("svr GCIN_req_get_preedit %x\n", cs);
#endif
      char str[GCIN_PREEDIT_MAX_STR];
      GCIN_PREEDIT_ATTR attr[GCIN_PREEDIT_ATTR_MAX_N];
      int cursor;
      int attrN = gcin_get_preedit(cs, str, attr, &cursor);
      if (gcin_edit_display&(GCIN_EDIT_DISPLAY_BOTH|GCIN_EDIT_DISPLAY_OVER_THE_SPOT))
        cursor=0;
      if (gcin_edit_display&GCIN_EDIT_DISPLAY_OVER_THE_SPOT) {
        attrN=0;
        str[0]=0;
      }
      int len = strlen(str)+1; // including \0
      write_enc(fd, &len, sizeof(len));
      write_enc(fd, str, len);
//      dbg("attrN:%d\n", attrN);
      write_enc(fd, &attrN, sizeof(attrN));
      if (attrN > 0)
        write_enc(fd, attr, sizeof(GCIN_PREEDIT_ATTR)*attrN);
      write_enc(fd, &cursor, sizeof(cursor));
//      dbg("uuuuuuuuuuuuuuuuu len:%d %d cursor:%d\n", len, attrN, cursor);
      }
      break;
    case GCIN_req_reset:
      gcin_reset();
      break;
    case GCIN_req_message:
      {
//        dbg("GCIN_req_message\n");
        short len=0;
        int rn = myread(fd, &len, sizeof(len));
        // only unix socket, no decrypt
        char buf[512];
        // message should include '\0'
        if (len > 0 && len < sizeof(buf)) {
          myread(fd, buf, len);
          message_cb(buf);
        }
      }
      break;
    default:
      dbg_time("Invalid request %x from:", req.req_no);

      struct sockaddr_in addr;
      socklen_t len=sizeof(addr);
      bzero(&addr, sizeof(addr));

      if (!getpeername(fd, (struct sockaddr *)&addr, &len)) {
        dbg("%s\n", inet_ntoa(addr.sin_addr));
      } else {
        perror("getpeername\n");
      }

      shutdown_client(fd);
      break;
  }
}



void close_all_clients()
{
  int i;
  for(i=3; i <= gcin_clientsN; i++)
    if (gcin_clients[i].tag)
#if UNIX
      close(i);
#else
      closesocket(i);
#endif
}
