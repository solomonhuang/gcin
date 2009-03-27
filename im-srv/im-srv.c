#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <X11/Xatom.h>
#include <sys/stat.h>
#include <gcin.h>
#include "gcin-protocol.h"
#include "im-srv.h"

int im_sockfd, im_tcp_sockfd;
void get_gcin_im_srv_sock_path(char *outstr, int outstrN);
void process_client_req(int fd);
Server_IP_port srv_ip_port;
static Window prop_win;
static Atom addr_atom;


static void cb_read_gcin_client_data(gpointer data, int source, GdkInputCondition condition)
{
  int fd=GPOINTER_TO_INT(data);

  process_client_req(fd);
}

Atom get_gcin_addr_atom(Display *dpy);

static void gen_passwd_idx()
{
  srv_ip_port.passwd.seed = (rand() >> 1) % __GCIN_PASSWD_N_;

  Server_IP_port tsrv_ip_port = srv_ip_port;

  to_gcin_endian_4(&srv_ip_port.passwd.seed);
  XChangeProperty(dpy, prop_win , addr_atom, XA_STRING, 8,
     PropModeReplace, (char *)&tsrv_ip_port, sizeof(srv_ip_port));

  XSync(GDK_DISPLAY(), FALSE);
}

static void cb_new_gcin_client(gpointer data, int source, GdkInputCondition condition)
{
  Connection_type type=(Connection_type) GPOINTER_TO_INT(data);

  dbg("im-srv: cb_new_gcin_client %s\n", type==Connection_type_unix ? "unix":"tcp");

  int newsockfd, clilen;

  if (type==Connection_type_unix) {
    struct sockaddr_un cli_addr;

    bzero(&cli_addr, sizeof(cli_addr));
    clilen=0;
    newsockfd = accept(im_sockfd,(struct sockaddr *) & cli_addr, &clilen);
  } else {
    struct sockaddr_in cli_addr;

    bzero(&cli_addr, sizeof(cli_addr));
    clilen=sizeof(cli_addr);
    newsockfd = accept(im_tcp_sockfd,(struct sockaddr *) & cli_addr, &clilen);
  }

  if (newsockfd < 0) {
    perror("accept");
    return;
  }

  if (newsockfd >= gcin_clientsN) {
    gcin_clients = trealloc(gcin_clients, GCIN_ENT, newsockfd+1);
    gcin_clientsN = newsockfd;
  }

  bzero(&gcin_clients[newsockfd], sizeof(gcin_clients[0]));

  gcin_clients[newsockfd].tag = gdk_input_add(newsockfd, GDK_INPUT_READ, cb_read_gcin_client_data,
              GINT_TO_POINTER(newsockfd));

  if (type==Connection_type_tcp) {
    gcin_clients[newsockfd].seed = srv_ip_port.passwd.seed;
    gen_passwd_idx();
  }

  gcin_clients[newsockfd].type = type;
}

static int get_ip_address(u_int *ip)
{
  char hostname[64];

  if (gethostname(hostname, sizeof(hostname)) < 0) {
    perror("cannot get hostname\n");
    return -1;
  }

  dbg("hostname %s\n", hostname);
  struct hostent *hent;

  if (!(hent=gethostbyname(hostname))) {
    dbg("cannot call gethostbyname to get IP address");
    return -1;
  }

  memcpy(ip, hent->h_addr_list[0], hent->h_length);
  return 0;
}


void init_gcin_im_serv(Window win)
{
  struct sockadd_un;
  struct sockaddr_un serv_addr;
  int servlen;

  prop_win = win;

  // unix socket
  bzero(&serv_addr,sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  char sock_path[128];
  get_gcin_im_srv_sock_path(sock_path, sizeof(sock_path));
  strcpy(serv_addr.sun_path, sock_path);
  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

  dbg("-- %s\n",serv_addr.sun_path);
  struct stat st;

  if (!stat(serv_addr.sun_path, &st)) {
    if (unlink(serv_addr.sun_path) < 0) {
      char tt[512];
      snprintf(tt, sizeof(tt), "unlink error %s", serv_addr.sun_path);
      perror(tt);
    }
  }


  if ((im_sockfd = socket(AF_UNIX,SOCK_STREAM,0)) < 0) {
    perror("cannot open unix socket");
    exit(-1);
  }

  if (bind(im_sockfd, (struct sockaddr *) &serv_addr, servlen) < 0) {
    perror("cannot bind");
    exit(-1);
  }

  listen(im_sockfd,2);

  dbg("im_sockfd:%d\n", im_sockfd);

  gdk_input_add(im_sockfd, GDK_INPUT_READ, cb_new_gcin_client,
                GINT_TO_POINTER(Connection_type_unix));

  addr_atom = get_gcin_addr_atom(GDK_DISPLAY());
  XSetSelectionOwner(dpy, addr_atom, win, CurrentTime);

  if (!gcin_remote_client) {
    dbg("connection via TCP is disabled\n");
    return;
  }

#if 1
  // tcp socket

  if (get_ip_address(&srv_ip_port.ip) < 0)
    return;


  if ((im_tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("cannot open tcp socket");
    exit(-1);
  }

  struct sockaddr_in serv_addr_tcp;
  u_short port;

  for(port=9999; port < 20000; port++) {
    // tcp socket
    bzero(&serv_addr_tcp, sizeof(serv_addr_tcp));
    serv_addr_tcp.sin_family = AF_INET;
    serv_addr_tcp.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr_tcp.sin_port = htons(port);

    if (bind(im_tcp_sockfd, (struct sockaddr *) &serv_addr_tcp, sizeof(serv_addr_tcp)) == 0)
      break;
  }

  u_char *myip = (char *)&srv_ip_port.ip;
  srv_ip_port.port = serv_addr_tcp.sin_port;
  dbg("server port bind to %d.%d.%d.%d:%d\n", myip[0], myip[1], myip[2], myip[3], port);

  time_t t;

  srand(time(&t));

  int i;
  for(i=0; i < __GCIN_PASSWD_N_; i++) {
    srv_ip_port.passwd.passwd[i] = (rand()>>2) & 0xff;
  }

  if (listen(im_tcp_sockfd, 5) < 0) {
    perror("cannot listen: ");
  }

  gen_passwd_idx();

  gdk_input_add(im_tcp_sockfd, GDK_INPUT_READ, cb_new_gcin_client,
                GINT_TO_POINTER(Connection_type_tcp));
#endif

//  printf("im_sockfd: %d\n",im_sockfd);
}
