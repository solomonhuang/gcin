#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#ifndef _XSERVER64
#define _XSERVER64
#endif
#include "gcin.h"
#include "gcin-protocol.h"
#include "gcin-im-client.h"

static void save_old_sigaction_single(int signo, struct sigaction *act)
{
  sigaction(signo, NULL, act);
  if (act->sa_handler != SIG_IGN)
    signal(signo, SIG_IGN);
}

static void restore_old_sigaction_single(int signo, struct sigaction *act)
{
  if (act->sa_handler != SIG_IGN)
    signal(signo, act->sa_handler);
}

char *get_gcin_im_srv_sock_path();

Atom get_gcin_addr_atom(Display *dpy);

static GCIN_client_handle *gcin_im_client_reopen(GCIN_client_handle *gcin_ch, Display *dpy)
{
//  dbg("gcin_im_client_reopen\n");
  int dbg_msg = getenv("GCIN_CONNECT_MSG_ON") != NULL;
  int sockfd=0;
  int servlen;
  char *addr;
  Server_IP_port srv_ip_port;
  int tcp = FALSE;

//  dbg("gcin_im_client_reopen\n");

  if (!dpy) {
    dbg("null disp\n");
    goto next;
  }


  Atom gcin_addr_atom = get_gcin_addr_atom(dpy);
  Window gcin_win = None;


#define MAX_TRY 3
  int loop;
  for(loop=0; loop < MAX_TRY; loop++) {
    if ((gcin_addr_atom && (gcin_win=XGetSelectionOwner(dpy, gcin_addr_atom))!=None)
        || getenv("GCIN_IM_CLEINT_NO_AUTO_EXEC"))
      break;
    static time_t exec_time;

    if (time(NULL) - exec_time > 1 /* && count < 5 */) {
      time(&exec_time);
      dbg("XGetSelectionOwner: old version of gcin or gcin is not running ??\n");
      static char execbin[]=GCIN_BIN_DIR"/gcin";
      dbg("... try to start a new gcin server %s\n", execbin);

      int pid;
      struct sigaction ori_act;
      save_old_sigaction_single(SIGCHLD, &ori_act);

      if ((pid=fork())==0) {
#if 	FREEBSD
        setpgid(0, getpid());
#else
        setpgrp();
#endif
        execl(execbin, "gcin", NULL);
      } else {
        sleep(1);
      }

      restore_old_sigaction_single(SIGCHLD, &ori_act);
    }
  }

  if (loop == MAX_TRY || gcin_win == None) {
    goto next;
  }

  struct sockaddr_un serv_addr;
  bzero((char *) &serv_addr,sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  char sock_path[128];

  get_gcin_im_srv_sock_path(sock_path, sizeof(sock_path));
  addr = sock_path;
  strcpy(serv_addr.sun_path, sock_path);
#ifdef SUN_LEN
  servlen = SUN_LEN(&serv_addr);
#else
  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
#endif

  if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("cannot open socket");
    goto tcp;
  }

  if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0) {
    close(sockfd);
    sockfd = 0;
    goto tcp;
  }

  if (dbg_msg)
    dbg("connected to unix socket addr %s\n", sock_path);
  goto next;

  struct sockaddr_in in_serv_addr;
  Atom actual_type;
  int actual_format;
  u_long nitems,bytes_after;
  char *message = NULL;

tcp:
  if (!gcin_addr_atom || XGetWindowProperty(dpy, gcin_win, gcin_addr_atom, 0, 64,
     False, AnyPropertyType, &actual_type, &actual_format,
     &nitems,&bytes_after,(u_char **)&message) != Success) {
#if DBG || 1
    dbg("XGetWindowProperty: old version of gcin or gcin is not running ??\n");
#endif
    goto next;
  }


  if (message) {
    memcpy(&srv_ip_port, message, sizeof(srv_ip_port));
    XFree(message);
  } else
    goto next;

//  dbg("im server tcp port %d\n", ntohs(srv_ip_port.port));

  bzero((char *) &in_serv_addr, sizeof(in_serv_addr));

  in_serv_addr.sin_family = AF_INET;
  in_serv_addr.sin_addr.s_addr = srv_ip_port.ip;
  in_serv_addr.sin_port = srv_ip_port.port;
  servlen = sizeof(in_serv_addr);

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("cannot open socket");
    goto next;
  }

  if (connect(sockfd, (struct sockaddr *)&in_serv_addr, servlen) < 0) {
    dbg("gcin_im_client_open cannot open") ;
    perror("");
    close(sockfd);
    sockfd = 0;
  }

  u_char *pp = (u_char *)&srv_ip_port.ip;
  if (dbg_msg)
    dbg("gcin client connected to server %d.%d.%d.%d:%d\n",
        pp[0], pp[1], pp[2], pp[3], ntohs(srv_ip_port.port));

  tcp = TRUE;

  GCIN_client_handle *handle;

next:

  if (!gcin_ch)
    handle = tzmalloc(GCIN_client_handle, 1);
  else {
    handle = gcin_ch;
  }

  if (sockfd < 0)
    sockfd = 0;

  if (sockfd > 0) {
    handle->fd = sockfd;

    if (tcp) {
      if (!handle->passwd)
        handle->passwd = malloc(sizeof(GCIN_PASSWD));
      memcpy(handle->passwd, &srv_ip_port.passwd, sizeof(srv_ip_port.passwd));
    } else {
      if (handle->passwd) {
        free(handle->passwd); handle->passwd = NULL;
      }
    }
  }

  if (handle->fd && BITON(handle->flag, FLAG_GCIN_client_handle_has_focus))
    gcin_im_client_focus_in(handle);

  return handle;
}


static void validate_handle(GCIN_client_handle *gcin_ch)
{
  if (gcin_ch->fd > 0)
    return;

  gcin_im_client_reopen(gcin_ch, gcin_ch->disp);
}


GCIN_client_handle *gcin_im_client_open(Display *disp)
{
//  dbg("gcin_im_client_open\n");
  GCIN_client_handle *handle = gcin_im_client_reopen(NULL,  disp);
  handle->disp = disp;
  return handle;
}

void gcin_im_client_close(GCIN_client_handle *handle)
{
  if (handle->fd > 0)
    close(handle->fd);

  free(handle->passwd);
  free(handle);
}



static void gen_req(GCIN_client_handle *handle, u_int req_no, GCIN_req *req)
{
  validate_handle(handle);

  bzero(req, sizeof(GCIN_req));

  req->req_no = req_no;
  to_gcin_endian_4(&req->req_no);

  req->client_win = handle->client_win;
  to_gcin_endian_4(&req->client_win);

  req->input_style = handle->input_style;
  to_gcin_endian_4(&req->input_style);

  req->spot_location.x = handle->spot_location.x;
  req->spot_location.y = handle->spot_location.y;
  to_gcin_endian_2(&req->spot_location.x);
  to_gcin_endian_2(&req->spot_location.y);
}

static void error_proc(GCIN_client_handle *handle, char *msg)
{
  if (!handle->fd)
    return;

  perror(msg);
  close(handle->fd);
  handle->fd = 0;
}

typedef struct {
  struct sigaction apipe;
  struct sigaction aio;
} SAVE_ACT;




static void save_old_sigaction(SAVE_ACT *save_act)
{
  save_old_sigaction_single(SIGPIPE, &save_act->apipe);
  save_old_sigaction_single(SIGIO, &save_act->aio);
}


static void restore_old_sigaction(SAVE_ACT *save_act)
{
  restore_old_sigaction_single(SIGPIPE, &save_act->apipe);
  restore_old_sigaction_single(SIGIO, &save_act->aio);
}

static int handle_read(GCIN_client_handle *handle, void *ptr, int n)
{
  int fd = handle->fd;

  if (!fd)
    return 0;

  SAVE_ACT save_act;

  save_old_sigaction(&save_act);
  int r = read(fd, ptr, n);
  restore_old_sigaction(&save_act);

  if (r<=0)
    return r;

  if (handle->passwd)
    __gcin_enc_mem((u_char *)ptr, n, handle->passwd, &handle->passwd->seed);

  return r;
}


static int handle_write(GCIN_client_handle *handle, void *ptr, int n)
{
  int fd = handle->fd;

  if (!fd)
    return 0;

  u_char *tmp = malloc(n);
  memcpy(tmp, ptr, n);

  if (handle->passwd)
    __gcin_enc_mem(tmp, n, handle->passwd, &handle->passwd->seed);


  SAVE_ACT save_act;

  save_old_sigaction(&save_act);
  int r =  write(fd, tmp, n);
  restore_old_sigaction(&save_act);

  free(tmp);

  return r;
}


void gcin_im_client_focus_in(GCIN_client_handle *handle)
{
//  dbg("gcin_im_client_focus_in\n");
  handle->flag |= FLAG_GCIN_client_handle_has_focus;

  GCIN_req req;
  gen_req(handle, GCIN_req_focus_in, &req);

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"gcin_im_client_focus_in error");
  }

  gcin_im_client_set_cursor_location(handle, handle->spot_location.x,
     handle->spot_location.y);
}

void gcin_im_client_focus_out(GCIN_client_handle *handle)
{
//  dbg("gcin_im_client_focus_out\n");
  handle->flag &= ~FLAG_GCIN_client_handle_has_focus;

  GCIN_req req;
  gen_req(handle, GCIN_req_focus_out, &req);

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"gcin_im_client_focus_out error");
  }
}


static int gcin_im_client_forward_key_event(GCIN_client_handle *handle,
                                          GCIN_req_t event_type,
                                          KeySym key, u_int state,
                                          char **rstr)
{
  *rstr = NULL;

  if (!handle->fd) {
//    dbg("gcin_im_client_forward_key_event fd is 0\n");
    return 0;
  }

  GCIN_req req;
  gen_req(handle, event_type, &req);
  req.keyeve.key = key;
  to_gcin_endian_4(&req.keyeve.key);
  req.keyeve.state = state;
  to_gcin_endian_4(&req.keyeve.state);


  if (handle_write(handle, &req, sizeof(req)) <= 0) {
    error_proc(handle, "cannot write to gcin server");
    return FALSE;
  }

  GCIN_reply reply;
  bzero(&reply, sizeof(reply));
  if (handle_read(handle, &reply, sizeof(reply)) <=0) {
    error_proc(handle, "cannot read reply from gcin server");
    return FALSE;
  }


  to_gcin_endian_4(&reply.datalen);
  to_gcin_endian_4(&reply.flag);

  if (reply.datalen > 0) {
    *rstr = malloc(reply.datalen);
    if (handle_read(handle, *rstr, reply.datalen) <= 0) {
      free(*rstr); *rstr = NULL;
      error_proc(handle, "cannot read reply str from gcin server");
      return FALSE;
    }
  }

//  dbg("gcin_im_client_forward_key_event %x\n", reply.flag);

  return ((reply.flag & GCIN_reply_key_processed) !=0 );
}


// return TRUE if the key is accepted
int gcin_im_client_forward_key_press(GCIN_client_handle *handle,
                                          KeySym key, u_int state,
                                          char **rstr)
{
  // in case client didn't send focus in event
  if (!BITON(handle->flag, FLAG_GCIN_client_handle_has_focus)) {
    gcin_im_client_focus_in(handle);
    handle->flag |= FLAG_GCIN_client_handle_has_focus;
    gcin_im_client_set_cursor_location(handle, handle->spot_location.x,
       handle->spot_location.y);
  }

//  dbg("gcin_im_client_forward_key_press\n");
  return gcin_im_client_forward_key_event(
             handle, GCIN_req_key_press, key, state, rstr);
}


// return TRUE if the key is accepted
int gcin_im_client_forward_key_release(GCIN_client_handle *handle,
                                          KeySym key, u_int state,
                                          char **rstr)
{
  handle->flag |= FLAG_GCIN_client_handle_has_focus;
//  dbg("gcin_im_client_forward_key_release\n");
  return gcin_im_client_forward_key_event(
             handle, GCIN_req_key_release, key, state, rstr);
}


void gcin_im_client_set_cursor_location(GCIN_client_handle *handle, int x, int y)
{
  handle->spot_location.x = x;
  handle->spot_location.y = y;
//  dbg("gcin_im_client_set_cursor_location %d   %d,%d\n", handle->flag, x, y);
  if (!BITON(handle->flag, FLAG_GCIN_client_handle_has_focus))
    return;

  GCIN_req req;
  gen_req(handle, GCIN_req_set_cursor_location, &req);

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"gcin_im_client_set_cursor_location error");
  }
}


void gcin_im_client_set_window(GCIN_client_handle *handle, Window win)
{
  if (!win) {
    dbg("gcin_im_client_set_window Invalid window");
    return;
  }
  handle->client_win = win;
}


void gcin_im_client_set_flags(GCIN_client_handle *handle, int flags)
{
  GCIN_req req;

  gen_req(handle, GCIN_req_set_flags, &req);
  req.flag |= flags;

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"gcin_im_client_set_flags error");
  }
}
