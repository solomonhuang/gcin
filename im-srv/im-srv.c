#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <gcin.h>
#include "gcin-protocol.h"

int im_sockfd;
char *get_gcin_im_srv_sock_path();

static void cb_read_gcin_client_data(gpointer data, int source, GdkInputCondition condition)
{
  int fd=GPOINTER_TO_INT(data);
}


static void cb_new_gcin_client(gpointer data, int source, GdkInputCondition condition)
{
  dbg("cb_new_gcin_client\n");

  int newsockfd,clilen;
  struct sockaddr_un cli_addr;

  bzero(&cli_addr, sizeof(cli_addr));
  clilen=0;
  newsockfd = accept(im_sockfd,(struct sockaddr *) & cli_addr, &clilen);
  if (newsockfd < 0) {
    perror("accept");
    return;
  }

//  read(newsockfd, premind,sizeof(REMINDER));
  gdk_input_add(im_sockfd, GDK_INPUT_READ, cb_read_gcin_client_data, GINT_TO_POINTER(newsockfd));
}


void init_gcin_im_serv()
{
  struct sockadd_un;
  struct sockaddr_un serv_addr;
  int servlen;

  bzero((char *) &serv_addr,sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, get_gcin_im_srv_sock_path());
  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

  dbg("-- %s\n",serv_addr.sun_path);
  unlink(serv_addr.sun_path);

  if ((im_sockfd = socket(AF_UNIX,SOCK_STREAM,0)) < 0) {
    perror("cannot open socket");
    exit(-1);
  }

  if (bind(im_sockfd, (struct sockaddr *) &serv_addr, servlen) < 0) {
    perror("cannot bind");
    exit(-1);
  }

  listen(im_sockfd,2);

  dbg("im_sockfd:%d\n", im_sockfd);

  gdk_input_add(im_sockfd, GDK_INPUT_READ, cb_new_gcin_client, NULL);
//  printf("im_sockfd: %d\n",im_sockfd);
}
