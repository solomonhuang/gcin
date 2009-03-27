typedef struct {
  int fd;
  Window client_win;	/* client window */
  u_int	input_style;	/* input style */
  XPoint spot_location; /* spot location */
} GCIN_client_handle;

GCIN_client_handle *gcin_im_client_open();
