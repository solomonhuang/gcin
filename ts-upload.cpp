#if UNIX
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
typedef int SOCKET;
#define closesocket(a) close(a)
#else
#include <WinSock2.h>
#include <ws2tcpip.h>
#define write(a,b,c) send(a,b,c,0);
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

extern int tsN;
void load_tsin_at_ts_idx(int ts_row, char *len, usecount_t *usecount, void *pho, u_char *ch);

#if WIN32
char *err_strA(DWORD dw);
char *sock_err_strA()
{
	return err_strA(WSAGetLastError());
}
#endif

int connect_ts_share_svr()
{
    SOCKET ConnectSocket = -1;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    int iResult;

    bzero( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    char port_s[8];
    sprintf(port_s, "%d", TS_SHARE_SERVER_PORT);

    iResult = getaddrinfo(TS_SHARE_SERVER, port_s, &hints, &result);
    if ( iResult != 0 ) {
#if UNIX
        p_err("getaddrinfo failed: %s\n", sys_err_strA());
#else
		p_err("getaddrinfo failed: %s\n", sock_err_strA());
#endif
    }

    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket < 0) {
            dbg("Error at socket(): %s\n", sys_err_strA());
            continue;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult < 0) {
            closesocket(ConnectSocket);
            ConnectSocket = -1;
            continue;
        }
        break;
    }

    if (ConnectSocket <= 0) {
      p_err("cannot connect to %s:%d", TS_SHARE_SERVER, TS_SHARE_SERVER_PORT, sys_err_strA());
    }

    freeaddrinfo(result);
    return ConnectSocket;
}

extern char contributed_file_src[];

void write_tsin_src(FILE *fw, char len, phokey_t *pho, char *s)
{
  fprintf(fw, "%s", s);
  int j;
  for(j=0;j<len; j++)
    fprintf(fw, " %s", phokey_to_str2(pho[j], TRUE));
  fprintf(fw, " 0\n");
}

void ts_upload()
{
  int sock = connect_ts_share_svr();
  int i;

  REQ_HEAD head;
  bzero(&head, sizeof(head));
  head.cmd = REQ_CONTRIBUTE;
  write(sock, (char *)&head, sizeof(head));

  REQ_CONTRIBUTE_S req;
  bzero(&req, sizeof(req));
  strcpy(req.tag, tsin32_f);
  write(sock, (char *)&req, sizeof(req));

  dbg("tsN:%d\n", tsN);

  FILE *fw;

  if ((fw=fopen(contributed_file_src, "a"))==NULL)
    p_err("cannot write %s", contributed_file_src);

  for(i=0;i<tsN;i++) {
    phokey_t pho[MAX_PHRASE_LEN];
    char s[MAX_PHRASE_LEN * CH_SZ + 1];
    char len, slen;
    usecount_t usecount;
    load_tsin_at_ts_idx(i, &len, &usecount, pho, (u_char *)s);
    slen = strlen(s);

    write(sock, &len, sizeof(len));
    write(sock, (char *)pho, len * sizeof(phokey_t));
    write(sock, &slen, sizeof(slen));
    write(sock, s, slen);

    write_tsin_src(fw, len, pho, s);
  }

  fclose(fw);

  char end_mark=0;
  write(sock, &end_mark, sizeof(end_mark));
  closesocket(sock);
}
