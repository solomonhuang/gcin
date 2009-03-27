#ifndef GCIN_IM_CLIENT_H
#define GCIN_IM_CLIENT_H

struct GCIN_PASSWD;

typedef struct {
  int fd;               // <=0 ; connection is not established
  Window client_win;	/* client window */
  u_int	input_style;	/* input style */
  XPoint spot_location; /* spot location */
// below is private data, don't modify them.
  u_int flag;
  Display *disp;
  struct GCIN_PASSWD *passwd;
} GCIN_client_handle;

enum {
  FLAG_GCIN_client_handle_has_focus = 1
};

GCIN_client_handle *gcin_im_client_open(Display *disp);
void gcin_im_client_close(GCIN_client_handle *handle);
void gcin_im_client_focus_in(GCIN_client_handle *handle);
void gcin_im_client_focus_out(GCIN_client_handle *handle);
void gcin_im_client_focus_out(GCIN_client_handle *handle);
void gcin_im_client_set_window(GCIN_client_handle *handle, Window win);
void gcin_im_client_set_cursor_location(GCIN_client_handle *handle,
                                        int x, int y);
/*  rstr returns UTF-8 encoding string, you should use 'free()' to free the
    memory.

    reutrn boolean:
      FALSE : the key is rejected, should use client's own result(ASCII key).
      TRUE : the key is accepted, translated result is in rstr.
 */
int gcin_im_client_forward_key_press(GCIN_client_handle *handle,
                                          KeySym key, u_int state,
                                          char **rstr);
int gcin_im_client_forward_key_release(GCIN_client_handle *handle,
                                          KeySym key, u_int state,
                                          char **rstr);
#endif
