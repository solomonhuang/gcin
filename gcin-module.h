// optional functions/data for gcin module to use, please refer to anthy.cpp
#if UNIX && defined(__cplusplus)
extern "C" {
#endif
typedef struct {
  void (*mf_show_win_sym)();
  void (*mf_hide_win_sym)();
  void (*mf_move_win_sym)();
  void (*mf_toggle_win_sym)();

  // display in tsin's selection window
  void (*mf_init_tsin_selection_win)();
  void (*mf_clear_sele)();
  void (*mf_disp_selections)(int x, int y);
  void (*mf_hide_selections_win)();
  void (*mf_disp_arrow_up)();
  void (*mf_disp_arrow_down)();
  void (*mf_set_sele_text)(int selN, int sel_idx, char *text, int len);
  void (*mf_set_win1_cb)(cb_selec_by_idx_t selc_by_idx, cb_page_ud_t cb_page_up, cb_page_ud_t cb_page_down);

  void (*mf_tsin_set_eng_ch)(int nmod);
  void (*mf_set_tsin_pho_mode)();
  gboolean (*mf_tsin_pho_mode)();

  int (*mf_get_widget_xy)(GtkWidget *win, GtkWidget *widget, int *rx, int *ry);
  void (*mf_get_win_size)(GtkWidget *win, int *width, int *height);
  void (*mf_change_win_bg)(GtkWidget *win);
  void (*mf_set_label_font_size)(GtkWidget *label, int size);
  void (*mf_set_no_focus)(GtkWidget *win);

  gint64 (*mf_current_time)();

  void (*mf_exec_gcin_setup)();
  gboolean (*mf_gcin_edit_display_ap_only)();
  gint (*mf_inmd_switch_popup_handler)(GtkWidget *widget, GdkEvent *event);
  void (*mf_load_tab_pho_file)();
  int (*mf_utf8_str_N)(char *str);

  // call this function to return the string
  void (*mf_send_text)(char *str);

  PHOKBM *mf_phkbm;
  TSIN_ST *mf_tss;
  int *mf_tsin_chinese_english_toggle_key;

  int *mf_gcin_pop_up_win;
  int *mf_gcin_font_size, *mf_gcin_win_color_use;
  char **mf_gcin_win_color_fg, **mf_pho_selkey, **mf_tsin_cursor_color;
  gboolean *mf_force_show;
  int *mf_win_x, *mf_win_y, *mf_win_xl, *mf_win_yl, *mf_dpy_xl, *mf_dpy_yl;
} GCIN_module_main_functions;
#if UNIX && defined(__cplusplus)
}
#endif

typedef struct _GCIN_module_callback_functions {
  int (*module_init_win)(GCIN_module_main_functions *funcs);
  void (*module_get_win_geom)();
  int (*module_reset)();
  int (*module_get_preedit)(char *str, GCIN_PREEDIT_ATTR attr[], int *pcursor);
  gboolean (*module_feedkey)(int kv, int kvstate);
  int (*module_feedkey_release)(KeySym xkey, int kbstate);
  void (*module_move_win)(int x, int y);
  void (*module_change_font_size)();
  void (*module_show_win)();
  void (*module_hide_win)();
  int (*module_win_visible)();
  int (*module_flush_input)();
} GCIN_module_callback_functions;

void init_GCIN_module_main_functions(GCIN_module_main_functions *func);

#ifdef __cplusplus
extern "C" {
#endif
  int module_init_win(GCIN_module_main_functions *funcs);
  void module_get_win_geom();
  int module_reset();
  int module_get_preedit(char *str, GCIN_PREEDIT_ATTR attr[], int *pcursor);
  gboolean module_feedkey(int kv, int kvstate);
  int module_feedkey_release(KeySym xkey, int kbstate);
  void module_move_win(int x, int y);
  void module_change_font_size();
  void module_show_win();
  void module_hide_win();
  int module_win_visible();
  int module_flush_input();
#ifdef __cplusplus
}
#endif
