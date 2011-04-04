// optional functions/data for gcin module to use, for anthy module
typedef struct {
  void (*mf_show_win_sym)();
  void (*mf_hide_win_sym)();
  void (*mf_move_win_sym)();
  void (*mf_toggle_win_sym)();

  // display in tsin's selection window
  void (*mf_init_tsin_selection_win)();
  void (*mf_disp_selections)();
  void (*mf_hide_selections_win)();
  void (*mf_disp_arrow_up)();
  void (*mf_disp_arrow_down)();
  void (*mf_set_sele_text)(int tN, int i, char *text, int len);

  void (*mf_tsin_set_eng_ch)(int nmod);

  void (*mf_get_widget_xy)(GtkWidget *win, GtkWidget *widget, int *rx, int *ry);
  void (*mf_get_win_size)(GtkWidget *win, int *width, int *height);

  gint64 (*mf_current_time)();

  void (*mf_exec_gcin_setup)();
  gboolean (*mf_gcin_edit_display_ap_only)();
  gint (*mf_inmd_switch_popup_handler)(GtkWidget *widget, GdkEvent *event);
  void (*mf_load_tab_pho_file)();

  PHOKBM *mf_phkbm;
  TSIN_ST *mf_tss;
} GCIN_module_main_functions;
