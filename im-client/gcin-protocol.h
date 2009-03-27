#include <X11/Xlib.h>

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

#define swap_ch(a, b) do { char t; t = *(a); *(a) = *(b); *(b) = t; } while (0)

#if __BYTE_ORDER == __BIG_ENDIAN
#warning "big endian"
#define to_gcin_endian_2(pp) do { char *p=(char *)pp;  swap_ch(p, p+1); } while (0)
#define to_gcin_endian_4(pp) do { char *p=(char *)pp;  swap_ch(p, p+3); swap_ch(p+1, p+2); } while (0)
#else
#define to_gcin_endian_2(pp) do { } while (0)
#define to_gcin_endian_4(pp) do { } while (0)
#endif
