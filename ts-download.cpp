#if UNIX
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
typedef int SOCKET;
#define closesocket(a) close(a)
#else
#include <WinSock2.h>
#include <ws2tcpip.h>
#define write(a,b,c) send(a,b,c,0)
#define read(a,b,c) recv(a,b,c,0)
#endif

#include "gcin.h"
#include "pho.h"
#include "config.h"
#if GCIN_i18n_message
#include <libintl.h>
#endif
#include "lang.h"
#include "tsin.h"
#include "gtab.h"
#include <gdk/gdkkeysyms.h>
#if GTK_CHECK_VERSION(2,90,7)
#include <gdk/gdkkeysyms-compat.h>
#endif
#include "ts-share.h"
#include "gcin-conf.h"

extern int tsN;
void load_tsin_at_ts_idx(int ts_row, char *len, usecount_t *usecount, void *pho, u_char *ch);


int connect_ts_share_svr();
extern char downloaded_file_src[];
void write_tsin_src(FILE *fw, char len, phokey_t *pho, char *s);

void ts_download()
{
  int sock = connect_ts_share_svr();
  int i;

  REQ_HEAD head;
  bzero(&head, sizeof(head));
  head.cmd = REQ_DOWNLOAD;
  write(sock, (char *)&head, sizeof(head));

  REQ_DOWNLOAD_S req;
  bzero(&req, sizeof(req));
  strcpy(req.tag, tsin32_f);

static char DL_CONF[]="ts-share-download-time";

  req.last_dl_time = get_gcin_conf_int(DL_CONF, 0);
 // req.last_dl_time = 0;

  write(sock, (char*)&req, sizeof(req));

  REQ_DOWNLOAD_REPLY_S rep;
  read(sock, (char*)&rep, sizeof(rep));

  FILE *fw;

  if ((fw=fopen(downloaded_file_src,"a"))==NULL)
    p_err("cannot create %s:%s", downloaded_file_src, sys_err_strA());

  int N=0;
  for(;;) {
    char len=0;
    read(sock, &len, sizeof(len));
    if (len<=0)
      break;
    phokey_t pho[128];
    read(sock, (char*)pho, len * sizeof(phokey_t));
    char slen=0;
    read(sock, &slen, sizeof(slen));
    char s[256];
    read(sock, s, slen);
    s[slen]=0;
    dbg("%s\n", s);
#if 1
    save_phrase_to_db(pho, s, len, 0);
#endif

    write_tsin_src(fw, len, pho, s);

    N++;
  }

  dbg("N:%d\n", N);
  if (N)
    save_gcin_conf_int(DL_CONF, (int)rep.this_dl_time);

  fclose(fw);

  closesocket(sock);
}
