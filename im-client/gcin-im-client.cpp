#if UNIX
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#ifndef _XSERVER64
#define _XSERVER64
#endif
#else
#include <windows.h>
#include <winsock.h>
#endif
#include "os-dep.h"
#include "gcin.h"
#include "gcin-protocol.h"
#include "gcin-im-client.h"
#define DBG 0
static int flags_backup;

#if UNIX
Atom get_gcin_sockpath_atom(Display *dpy);
static void save_old_sigaction_single(int signo, struct sigaction *act)
{
  sigaction(signo, NULL, act);

  if (act->sa_handler != SIG_IGN) {
    signal(signo, SIG_IGN);
  }
}

static void restore_old_sigaction_single(int signo, struct sigaction *act)
{
  if (act->sa_handler != SIG_IGN)
    signal(signo, act->sa_handler);
}
char *get_gcin_im_srv_sock_path();
Atom get_gcin_addr_atom(Display *dpy);
#endif

#if WIN32
bool init_winsock()
{
    WORD wVersionRequested;
    WSADATA wsaData;

/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        dbg("WSAStartup failed with error: %d\n", err);
		wVersionRequested = 0;
        return FALSE;
    }

	return TRUE;
}
#endif

#if WIN32
bool sys_end_session;
#endif


static GCIN_client_handle *gcin_im_client_reopen(GCIN_client_handle *gcin_ch, Display *dpy)
{
#if WIN32
  char current_exec[80];
  if (GetModuleFileNameA(NULL, current_exec, sizeof(current_exec))) {
	  if (strstr(current_exec, "\\gcin.exe"))
		  return NULL;
  }
  int retried;
#endif

//  dbg("gcin_im_client_reopen\n");
  int dbg_msg = getenv("GCIN_CONNECT_MSG_ON") != NULL;
  int sockfd=0;
  int servlen;
  char *addr;
  Server_IP_port srv_ip_port;
  int tcp = FALSE;
  GCIN_client_handle *handle;
  u_char *pp;
  int rstatus;

//  dbg("gcin_im_client_reopen\n");
#if UNIX
  if (!dpy) {
    dbg("null disp %d\n", gcin_ch->fd);
    goto next;
  }

  Atom gcin_addr_atom = get_gcin_addr_atom(dpy);
  Window gcin_win = None;


#define MAX_TRY 3
  int loop;
  for(loop=0; loop < MAX_TRY; loop++) {
    if ((gcin_addr_atom && (gcin_win=XGetSelectionOwner(dpy, gcin_addr_atom))!=None)
        || getenv("GCIN_IM_CLIENT_NO_AUTO_EXEC"))
      break;
    static time_t exec_time;

    if (time(NULL) - exec_time > 1 /* && count < 5 */) {
      time(&exec_time);
      dbg("XGetSelectionOwner: old version of gcin or gcin is not running ??\n");
      static char execbin[]=GCIN_BIN_DIR"/gcin";
      dbg("... try to start a new gcin server %s\n", execbin);

      int pid;

      if ((pid=fork())==0) {
        putenv("GCIN_DAEMON=");
        execl(execbin, "gcin", NULL);
      } else {
        int status;
        // gcin will daemon()
        waitpid(pid, &status, 0);
      }
    }
  }

  if (loop == MAX_TRY || gcin_win == None) {
    goto next;
  }

  Atom actual_type;
  int actual_format;
  u_long nitems,bytes_after;
  char *message_sock = NULL;
  Atom gcin_sockpath_atom = get_gcin_sockpath_atom(dpy);

//  printf("gcin_sockpath_atom %d\n", gcin_sockpath_atom);

  if (!gcin_sockpath_atom || XGetWindowProperty(dpy, gcin_win, gcin_sockpath_atom, 0, 64,
     False, AnyPropertyType, &actual_type, &actual_format,
     &nitems,&bytes_after,(u_char **)&message_sock) != Success) {
#if DBG || 1
    dbg("XGetWindowProperty 2: old version of gcin or gcin is not running ??\n");
#endif
    goto next;
  }

  Server_sock_path srv_sock_path;
  srv_sock_path.sock_path[0] = 0;
  if (message_sock) {
    memcpy(&srv_sock_path, message_sock, sizeof(srv_sock_path));
    XFree(message_sock);
  } else
    goto next;

  struct sockaddr_un serv_addr;
  bzero((char *) &serv_addr,sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  char sock_path[128];

  if (srv_sock_path.sock_path[0]) {
    strcpy(sock_path, srv_sock_path.sock_path);
  }
  else {
    get_gcin_im_srv_sock_path(sock_path, sizeof(sock_path));
  }

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

  char *message;

tcp:
  message = NULL;

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
#endif // UNIX

  struct sockaddr_in in_serv_addr;
  bzero((char *) &in_serv_addr, sizeof(in_serv_addr));

  in_serv_addr.sin_family = AF_INET;
#if WIN32
  retried=0;
restart:
  HWND serverWnd = FindWindowA(GCIN_WIN_NAME, NULL);
  if (!serverWnd) {
    if (retried < 10) {
	  if (!retried)
        win32exec("gcin.exe");

      Sleep(1000);
	  retried++;
      goto restart;
	} else {
      dbg("exec not found ?\n");
	}

	  MessageBoxA(NULL, "cannot find window", NULL, MB_OK);
	  goto next;
  }

  dbg("serverwnd %x\n", serverWnd);

  int port = SendMessageA(serverWnd, GCIN_PORT_MESSAGE, 0, 0);

  dbg("port %d\n", port);

  in_serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  in_serv_addr.sin_port = htons(port);
#else
  in_serv_addr.sin_addr.s_addr = srv_ip_port.ip;
  in_serv_addr.sin_port = srv_ip_port.port;
#endif
  servlen = sizeof(in_serv_addr);

#if WIN32
  bool inited_winsock=false;
retry_socket:
#endif
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
#if WIN32
    if (inited_winsock)
#endif
	{
      perror("cannot open socket");
      goto next;
	}
#if WIN32
	if (!init_winsock())
	  goto next;
    inited_winsock = true;
	goto retry_socket;
#endif
  }

  dbg("sock %d\n", sockfd);

  if (connect(sockfd, (struct sockaddr *)&in_serv_addr, servlen) < 0) {
#if WIN32
    dbg("err %d\n", WSAGetLastError());
#endif
	dbg("gcin_im_client_open cannot open: ") ;
    perror("");
#if UNIX
    close(sockfd);
#else
	closesocket(sockfd);
#endif
    sockfd = 0;
	goto next;
  }

#if WIN32
  if (dbg_msg || 1)
    dbg("gcin client connected to server\n");
#else
  pp = (u_char *)&srv_ip_port.ip;
  if (dbg_msg)
    dbg("gcin client connected to server %d.%d.%d.%d:%d\n",
        pp[0], pp[1], pp[2], pp[3], ntohs(srv_ip_port.port));
#endif

  tcp = TRUE;

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

#if UNIX
    if (tcp) {
      if (!handle->passwd)
        handle->passwd = malloc(sizeof(GCIN_PASSWD));
      memcpy(handle->passwd, &srv_ip_port.passwd, sizeof(srv_ip_port.passwd));
    } else {
      if (handle->passwd) {
        free(handle->passwd); handle->passwd = NULL;
      }
    }
#endif
  }

  if (handle->fd)  {
    if (BITON(handle->flag, FLAG_GCIN_client_handle_has_focus))
      gcin_im_client_focus_in(handle);

    gcin_im_client_set_flags(handle, flags_backup, &rstatus);
  }

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
  if (!handle)
	  return;
  if (handle->fd > 0)
#if WIN32
    closesocket(handle->fd);
#else
    close(handle->fd);
#endif
#if UNIX
  free(handle->passwd);
#endif
  free(handle);
}

static int gen_req(GCIN_client_handle *handle, u_int req_no, GCIN_req *req)
{
#if WIN32
  if (req_no  & (GCIN_req_key_press|GCIN_req_key_release|GCIN_req_test_key_press|GCIN_req_test_key_release)) {
//    dbg("gen_req validate\n");
	validate_handle(handle);
  }
#else
  validate_handle(handle);
#endif

  if (!handle->fd)
    return 0;

  handle->seq++;

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

  return 1;
}

static void error_proc(GCIN_client_handle *handle, char *msg)
{
  if (!handle->fd)
    return;

  perror(msg);
#if WIN32
  closesocket(handle->fd);
#else
  close(handle->fd);
#endif
  handle->fd = 0;
#if WIN32
  Sleep(100);
#else
  usleep(100000);
#endif
}


#if WIN32
static int handle_read(GCIN_client_handle *handle, void *ptr, int n)
{
  int r;
  int fd = handle->fd;

  if (!fd)
    return 0;

  r = recv(fd, (char *)ptr, n, 0);

#if (DBG || 0)
  if (r < 0)
    perror("handle_read");
#endif

  return r;
}
#else
typedef struct {
  struct sigaction apipe;
} SAVE_ACT;
static void save_old_sigaction(SAVE_ACT *save_act)
{
  save_old_sigaction_single(SIGPIPE, &save_act->apipe);
}
static void restore_old_sigaction(SAVE_ACT *save_act)
{
  restore_old_sigaction_single(SIGPIPE, &save_act->apipe);
}
static int handle_read(GCIN_client_handle *handle, void *ptr, int n)
{
  int fd = handle->fd;

  if (!fd)
    return 0;

  SAVE_ACT save_act;
  save_old_sigaction(&save_act);
  int r = read(fd, ptr, n);

#if (DBG || 1)
  if (r < 0)
    perror("handle_read");
#endif

  restore_old_sigaction(&save_act);

  if (r<=0)
    return r;
  if (handle->passwd)
    __gcin_enc_mem((u_char *)ptr, n, handle->passwd, &handle->passwd->seed);
  return r;
}
#endif


#if WIN32
static int handle_write(GCIN_client_handle *handle, void *ptr, int n)
{
  int r;
  char *tmp;
  int fd = handle->fd;

  if (!fd)
    return 0;

  tmp = (char *)malloc(n);
  memcpy(tmp, ptr, n);

  r =  send(fd, tmp, n, 0);
  free(tmp);
  return r;
}
#else
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
#if 1
  save_old_sigaction(&save_act);
#endif
  int r =  write(fd, tmp, n);
#if 1
  restore_old_sigaction(&save_act);
#endif
  free(tmp);

  return r;
}
#endif


void gcin_im_client_focus_in(GCIN_client_handle *handle)
{
  if (!handle)
	  return;

  GCIN_req req;
//  dbg("gcin_im_client_focus_in\n");
  handle->flag |= FLAG_GCIN_client_handle_has_focus;

  if (!gen_req(handle, GCIN_req_focus_in, &req))
    return;

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"gcin_im_client_focus_in error");
  }

  gcin_im_client_set_cursor_location(handle, handle->spot_location.x,
     handle->spot_location.y);
}


void gcin_im_client_focus_out(GCIN_client_handle *handle)
{
  if (!handle)
    return;

  GCIN_req req;
//  dbg("gcin_im_client_focus_out\n");
  handle->flag &= ~FLAG_GCIN_client_handle_has_focus;

  if (!gen_req(handle, GCIN_req_focus_out, &req))
    return;

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"gcin_im_client_focus_out error");
  }
}

#if UNIX
void gcin_im_client_focus_out2(GCIN_client_handle *handle, char **rstr)
{
  GCIN_req req;
  GCIN_reply reply;

  if (!handle)
    return;

#if DBG
  dbg("gcin_im_client_focus_out2\n");
#endif
  handle->flag &= ~FLAG_GCIN_client_handle_has_focus;
  *rstr = NULL;

  if (!gen_req(handle, GCIN_req_focus_out2, &req))
    return;

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"gcin_im_client_focus_out error");
  }

  bzero(&reply, sizeof(reply));
  if (handle_read(handle, &reply, sizeof(reply)) <=0) {
    error_proc(handle, "cannot read reply from gcin server");
    return;
  }

  to_gcin_endian_4(&reply.datalen);
  to_gcin_endian_4(&reply.flag);

  if (reply.datalen > 0) {
    *rstr = (char *)malloc(reply.datalen);
    if (handle_read(handle, *rstr, reply.datalen) <= 0) {
      free(*rstr); *rstr = NULL;
      error_proc(handle, "cannot read reply str from gcin server");
      return;
    }
  }

//  dbg("gcin_im_client_forward_key_event %x\n", reply.flag);

  return;
}
#endif

static int gcin_im_client_forward_key_event(GCIN_client_handle *handle,
                                          GCIN_req_t event_type,
                                          KeySym key, u_int state,
                                          char **rstr)
{
  GCIN_reply reply;
  GCIN_req req;

  *rstr = NULL;

  if (!gen_req(handle, event_type, &req))
    return 0;

  req.keyeve.key = key;
  to_gcin_endian_4(&req.keyeve.key);
  req.keyeve.state = state;
  to_gcin_endian_4(&req.keyeve.state);


  if (handle_write(handle, &req, sizeof(req)) <= 0) {
    error_proc(handle, "cannot write to gcin server");
    return FALSE;
  }

  bzero(&reply, sizeof(reply));
  if (handle_read(handle, &reply, sizeof(reply)) <=0) {
    error_proc(handle, "cannot read reply from gcin server");
    return FALSE;
  }

  to_gcin_endian_4(&reply.datalen);
  to_gcin_endian_4(&reply.flag);

  if (reply.datalen > 0) {
    *rstr = (char *)malloc(reply.datalen);
    if (handle_read(handle, *rstr, reply.datalen) <= 0) {
      free(*rstr); *rstr = NULL;
      error_proc(handle, "cannot read reply str from gcin server");
      return FALSE;
    }
  }

//  dbg("gcin_im_client_forward_key_event %x\n", reply.flag);

  return reply.flag;
}


// return TRUE if the key is accepted
int gcin_im_client_forward_key_press(GCIN_client_handle *handle,
                                          KeySym key, u_int state,
                                          char **rstr)
{
  int flag;
  if (!handle)
    return 0;
  // in case client didn't send focus in event
  if (!BITON(handle->flag, FLAG_GCIN_client_handle_has_focus)) {
    gcin_im_client_focus_in(handle);
    handle->flag |= FLAG_GCIN_client_handle_has_focus;
    gcin_im_client_set_cursor_location(handle, handle->spot_location.x,
       handle->spot_location.y);
  }

//  dbg("gcin_im_client_forward_key_press\n");
  flag = gcin_im_client_forward_key_event(
             handle, GCIN_req_key_press, key, state, rstr);

  return ((flag & GCIN_reply_key_processed) !=0);
}


// return TRUE if the key is accepted
int gcin_im_client_forward_key_release(GCIN_client_handle *handle,
                                          KeySym key, u_int state,
                                          char **rstr)
{
  int flag;
  if (!handle)
    return 0;
  handle->flag |= FLAG_GCIN_client_handle_has_focus;
//  dbg("gcin_im_client_forward_key_release\n");
  flag = gcin_im_client_forward_key_event(
             handle, GCIN_req_key_release, key, state, rstr);
  return ((flag & GCIN_reply_key_processed) !=0);
}


void gcin_im_client_set_cursor_location(GCIN_client_handle *handle, int x, int y)
{
  if (!handle)
	  return;

//  dbg("gcin_im_client_set_cursor_location %d   %d,%d\n", handle->flag, x, y);

  GCIN_req req;
  handle->spot_location.x = x;
  handle->spot_location.y = y;

  if (!BITON(handle->flag, FLAG_GCIN_client_handle_has_focus))
    return;

  if (!gen_req(handle, GCIN_req_set_cursor_location, &req))
    return;

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"gcin_im_client_set_cursor_location error");
  }
}

// in win32, if win is NULL, this means gcin_im_client_set_cursor_location(x,y) is screen position
void gcin_im_client_set_window(GCIN_client_handle *handle, Window win)
{
  if (!handle)
	  return;
//  dbg("gcin_im_client_set_window %x\n", win);
#if UNIX
  if (!win)
    return;
#endif
  handle->client_win = win;
}

void gcin_im_client_set_flags(GCIN_client_handle *handle, int flags, int *ret_flag)
{
  GCIN_req req;

  if (!handle)
    return;

  if (!gen_req(handle, GCIN_req_set_flags, &req))
    return;

  req.flag |= flags;

  flags_backup = req.flag;

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"gcin_im_client_set_flags error");
  }

  if (handle_read(handle, ret_flag, sizeof(int)) <= 0) {
    error_proc(handle, "cannot read reply str from gcin server");
  }
}


void gcin_im_client_clear_flags(GCIN_client_handle *handle, int flags, int *ret_flag)
{
  GCIN_req req;

  if (!handle)
    return;

  if (!gen_req(handle, GCIN_req_set_flags, &req))
    return;

  req.flag &= ~flags;

  flags_backup = req.flag;

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"gcin_im_client_set_flags error");
  }

  if (handle_read(handle, ret_flag, sizeof(int)) <= 0) {
    error_proc(handle, "cannot read reply str from gcin server");
  }
}


int gcin_im_client_get_preedit(GCIN_client_handle *handle, char **str, GCIN_PREEDIT_ATTR att[], int *cursor
#if WIN32
				,int *sub_comp_len
#endif
			   )
{
  *str=NULL;
  if (!handle)
    return 0;

  int attN, tcursor, str_len;
#if DBG
  dbg("gcin_im_client_get_preedit\n");
#endif
  GCIN_req req;
  if (!gen_req(handle, GCIN_req_get_preedit, &req)) {
err_ret:
#if DBG
    dbg("aaaaaaaaaaaaa %x\n", str);
#endif
    if (cursor)
      *cursor=0;
    *str=strdup("");
    return 0;
  }

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"gcin_im_client_get_preedit error");
    goto err_ret;
  }

  str_len=-1; // str_len includes \0
  if (handle_read(handle, &str_len, sizeof(str_len))<=0)
    goto err_ret; // including \0

  *str = (char *)malloc(str_len);

  if (handle_read(handle, *str, str_len)<=0)
    goto err_ret;
#if DBG
  dbg("gcin_im_client_get_preedit len:%d '%s' \n", str_len, *str);
#endif
  attN = -1;
  if (handle_read(handle, &attN, sizeof(attN))<=0) {
    goto err_ret;
  }

//  dbg("attrN:%d\n", attN);

  if (attN>0 && handle_read(handle, att, sizeof(GCIN_PREEDIT_ATTR)*attN)<=0) {
    goto err_ret;
  }


  tcursor=0;
  if (handle_read(handle, &tcursor, sizeof(tcursor))<=0) {
    goto err_ret;
  }

  if (cursor)
    *cursor = tcursor;


#if WIN32
  int tsub_comp_len;
  tsub_comp_len=0;
  if (handle_read(handle, &tsub_comp_len, sizeof(tsub_comp_len))<=0) {
    goto err_ret;
  }
  if (sub_comp_len)
	*sub_comp_len = tsub_comp_len;
#endif

#if DBG
  dbg("jjjjjjjjj %d tcursor:%d\n", attN, tcursor);
#endif
  return attN;
}



void gcin_im_client_reset(GCIN_client_handle *handle)
{
  if (!handle)
	  return;

  GCIN_req req;
#if DBG
  dbg("gcin_im_client_reset\n");
#endif
  if (!gen_req(handle, GCIN_req_reset, &req))
    return;

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"gcin_im_client_reset error");
  }
}


void gcin_im_client_message(GCIN_client_handle *handle, char *message)
{
  GCIN_req req;
  short len;
#if DBG
  dbg("gcin_im_client_message\n");
#endif
  if (!gen_req(handle, GCIN_req_message, &req))
    return;

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"gcin_im_client_message error 1");
  }

  len = strlen(message)+1;
  if (handle_write(handle, &len, sizeof(len)) <=0) {
    error_proc(handle,"gcin_im_client_message error 2");
  }

  if (handle_write(handle, message, len) <=0) {
    error_proc(handle,"gcin_im_client_message error 2");
  }
}


#if TSF
bool gcin_im_client_key_eaten(GCIN_client_handle *handle, int press_release,
                                          KeySym key, u_int state)
{
  GCIN_reply reply;
  GCIN_req req;

  if (!gen_req(handle, press_release?GCIN_req_test_key_release:GCIN_req_test_key_press, &req))
    return 0;

  req.keyeve.key = key;
  to_gcin_endian_4(&req.keyeve.key);
  req.keyeve.state = state;
  to_gcin_endian_4(&req.keyeve.state);


  if (handle_write(handle, &req, sizeof(req)) <= 0) {
    error_proc(handle, "cannot write to gcin server");
    return FALSE;
  }

  bzero(&reply, sizeof(reply));
  if (handle_read(handle, &reply, sizeof(reply)) <=0) {
    error_proc(handle, "cannot read reply from gcin server");
    return FALSE;
  }

  to_gcin_endian_4(&reply.datalen);
  to_gcin_endian_4(&reply.flag);

  return (reply.flag & GCIN_reply_key_processed) > 0;
}
#endif
