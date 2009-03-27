#include <sys/socket.h>
#include <sys/un.h>
#include "gcin.h"
#include "gcin-im-client.h"
#include "gcin-protocol.h"

char *get_gcin_im_srv_sock_path();

GCIN_client_handle *gcin_im_client_open()
{
  printf("gcin_im_client_open\n");
  fflush(stdout);

  int sockfd, servlen;
  struct sockaddr_un serv_addr;

  bzero((char *) &serv_addr,sizeof(serv_addr));

  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, get_gcin_im_srv_sock_path());
  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

  if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("cannot open socket");
    exit(-1);
  }

  if (connect(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0) {
    close(sockfd);
    printf("gcin_im_client_open cannot open %s\n", get_gcin_im_srv_sock_path()) ;
  }

  GCIN_client_handle *handle = tzmalloc(GCIN_client_handle, 1);
  handle->fd = sockfd;

  return handle;
}


void gcin_im_client_close(GCIN_client_handle *handle)
{
  close(handle->fd);
  free(handle);
}


static void gen_req(GCIN_client_handle *handle, u_int req_no, GCIN_req *req)
{
  bzero(req, sizeof(req));
  req->req_no = req_no;
  req->client_win = handle->client_win;
  req->input_style = handle->input_style;
  req->spot_location = handle->spot_location;
}

// return TRUE if the key is accepted
gboolean gcin_im_client_forward_key_press(GCIN_client_handle *handle,
                                          KeySym key, u_int state,
                                          char **rstr)
{
  GCIN_req req;
  gen_req(handle, GCIN_req_key_press, &req);

  write(handle->fd, &req, sizeof(req));

  GCIN_reply reply;
  bzero(&reply, sizeof(reply));
  read(handle->fd, &reply, sizeof(reply));

  *rstr = NULL;

  if (reply.datalen > 0) {
    *rstr = malloc(reply.datalen);
    read(handle->fd, *rstr, reply.datalen);
  }

  return (reply.flag & GCIN_reply_key_processed);
}

void gcin_im_client_focus_in(GCIN_client_handle *handle)
{
  GCIN_req req;
  gen_req(handle, GCIN_req_focus_in, &req);
  write(handle->fd, &req, sizeof(req));
}

void gcin_im_client_focus_out(GCIN_client_handle *handle)
{
  GCIN_req req;
  gen_req(handle, GCIN_req_focus_out, &req);
  write(handle->fd, &req, sizeof(req));
}
