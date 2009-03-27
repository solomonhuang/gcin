#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <gcin.h>
#include "gcin-protocol.h"

ClientState **cl_stArr;
int cl_stArrN;

gboolean ProcessKeyPress(KeySym keysym, u_int kev_state);
int gcin_FocusIn(ClientState *cs);
int gcin_FocusOut(ClientState *cs);
extern char *output_buffer;
extern int output_bufferN;

void process_client_req(int fd)
{
  GCIN_req req;

  int rn = read(fd, &req, sizeof(req));

  if (rn <= 0) {
    dbg("rn: %d\n", rn);
  }

  to_gcin_endian_4(&req.req_no);
  to_gcin_endian_4(&req.flag);
  to_gcin_endian_2(&req.win_spot.x);
  to_gcin_endian_2(&req.win_spot.y);
  to_gcin_endian_4(&req.keyeve.key);
  to_gcin_endian_4(&req.keyeve.state);

  ClientState *cs;

  if (req.client_win == current_CS->client_win)
    cs = current_CS;
  else {
    int i;
    for(i=0; i < cl_stArrN; i++) {
      if (cl_stArr[i]->client_win == req.client_win)
        break;
    }

    if (i==cl_stArrN) {
      trealloc(cl_stArr, ClientState *, cl_stArrN+1);
      cl_stArr[cl_stArrN++] = tzmalloc(ClientState, 1);
      cs->client_win = req.client_win;
      cs->b_gcin_protocol = TRUE;
    }
  }

  cs->spot_location.x = req.spot_location.x;
  cs->spot_location.y = req.spot_location.y;

  gboolean status;
  GCIN_reply reply;
  bzero(&reply, sizeof(reply));

  switch (req.req_no) {
    case GCIN_req_key_press:
      status = ProcessKeyPress(req.keyeve.key, req.keyeve.state);
      if (status) {
        reply.flag |= GCIN_reply_key_processed;
      }

      reply.datalen = output_bufferN;
      to_gcin_endian_4(&reply.flag);
      to_gcin_endian_4(&reply.datalen);

      write(fd, &reply, sizeof(reply));

      if (reply.datalen)
        write(fd, output_buffer, output_bufferN);

      break;
    case GCIN_req_key_release:
      break;
    case GCIN_req_focus_in:
      gcin_FocusIn(cs);
      break;
    case GCIN_req_focus_out:
      gcin_FocusOut(cs);
      break;
    case GCIN_req_set_cursor_location:
      break;
  }
}
