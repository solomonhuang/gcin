#include <X11/Xlib.h>
#include "gcin-endian.h"

typedef enum {
  GCIN_req_key_press = 1,
  GCIN_req_key_release = 2,
  GCIN_req_focus_in = 4,
  GCIN_req_focus_out = 8,
  GCIN_req_set_cursor_location = 16,
} GCIN_req_t;

enum {
  GCIN_flag_has_key_release = 1,
};

typedef struct {
    KeySym key;
    u_int state;
} KeyEvent;

typedef struct {
  u_int req_no;  // to make the im server stateless, more is better
  Window client_win;
  u_int flag;
  u_int input_style;
  XPoint spot_location;

  union {
    KeyEvent keyeve;
    char dummy[32];   // for future expansion
  };
} GCIN_req;


enum {
  GCIN_reply_key_processed = 1,
};


typedef struct {
  u_int flag;
  u_int datalen;    // '\0' shoule be counted if data is string
} GCIN_reply;


#define __GCIN_PASSWD_N_ (31)

typedef struct GCIN_PASSWD {
  u_int seed;
  u_char passwd[__GCIN_PASSWD_N_];
} GCIN_PASSWD;


typedef struct {
  u_int ip;
  u_short port;
  GCIN_PASSWD passwd;
} Server_IP_port;


void __gcin_enc_mem(u_char *p, int n, GCIN_PASSWD *passwd, u_int *seed);

