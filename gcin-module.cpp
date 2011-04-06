#include "gcin.h"
#include "gtab.h"
#include "pho.h"
#include "tsin.h"
#include "gst.h"
#include "im-client/gcin-im-client-attr.h"
#include "gcin-module.h"

void show_win_sym(),hide_win_sym(),move_win_sym(),toggle_win_sym();
void init_tsin_selection_win(),disp_selections(),hide_selections_win();
void disp_arrow_up(),disp_arrow_down(), set_tsin_pho_mode();
void set_sele_text(int tN, int i, char *text, int len);
void tsin_set_eng_ch(int nmod);
void get_widget_xy(GtkWidget *win, GtkWidget *widget, int *rx, int *ry);
void get_win_size(GtkWidget *win, int *width, int *height);
void exec_gcin_setup();
void load_tab_pho_file();
void clear_sele();
extern gboolean force_show;

void init_GCIN_module_main_functions(GCIN_module_main_functions *func)
{
  func->mf_show_win_sym = show_win_sym;
  func->mf_hide_win_sym = hide_win_sym;
  func->mf_move_win_sym = move_win_sym;
  func->mf_toggle_win_sym = toggle_win_sym;

  func->mf_init_tsin_selection_win = init_tsin_selection_win;
  func->mf_clear_sele = clear_sele;
  func->mf_disp_selections = disp_selections;
  func->mf_hide_selections_win = hide_selections_win;
  func->mf_disp_arrow_up = disp_arrow_up;
  func->mf_disp_arrow_down = disp_arrow_down;
  func->mf_set_sele_text = set_sele_text;

  func->mf_tsin_set_eng_ch = tsin_set_eng_ch;
  func->mf_tsin_pho_mode = tsin_pho_mode;
  func->mf_set_tsin_pho_mode = set_tsin_pho_mode;

  func->mf_get_widget_xy = get_widget_xy;
  func->mf_get_win_size = get_win_size;
  func->mf_change_win_bg = change_win_bg;
  func->mf_set_label_font_size = set_label_font_size;
  func->mf_set_no_focus = set_no_focus;


  func->mf_current_time = current_time;

  func->mf_exec_gcin_setup = exec_gcin_setup;
  func->mf_gcin_edit_display_ap_only = gcin_edit_display_ap_only;
  func->mf_inmd_switch_popup_handler = inmd_switch_popup_handler;
  func->mf_load_tab_pho_file = load_tab_pho_file;
  func->mf_send_text = send_text;
  func->mf_utf8_str_N = utf8_str_N;

  func->mf_phkbm = &phkbm;
  func->mf_tss = &tss;
  func->mf_tsin_chinese_english_toggle_key = &tsin_chinese_english_toggle_key;

  func->mf_gcin_pop_up_win = &gcin_pop_up_win;
  func->mf_gcin_font_size = &gcin_font_size;
  func->mf_gcin_win_color_fg = &gcin_win_color_fg;
  func->mf_gcin_win_color_use = &gcin_win_color_use;
  func->mf_tsin_cursor_color = &tsin_cursor_color;
  func->mf_pho_selkey = &pho_selkey;
  func->mf_force_show = &force_show;
  func->mf_win_x = &win_x;
  func->mf_win_y = &win_y;
  func->mf_win_xl = &win_xl;
  func->mf_win_yl = &win_yl;
  func->mf_dpy_xl = &dpy_xl;
  func->mf_dpy_yl = &dpy_yl;
}

#if UNIX
#include <dlfcn.h>
#endif

GCIN_module_callback_functions *init_GCIN_module_callback_functions(char *sofile)
{
#if UNIX
  void *handle;
  char *error;

  if (!(handle = dlopen(sofile, RTLD_LAZY))) {
    if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
    }
    dbg("dlopen %s failed\n", sofile);
    return NULL;
  }
#else
  HINSTANCE *handle;
  LoadLibrary(sofile);
#define dlsym GetProcAddress
#endif

  GCIN_module_callback_functions st;
  *(void **) (&st.module_init_win) = dlsym(handle, "module_init_win");
  if (!st.module_init_win)
    p_err("module_init_win() not found in %s", sofile);

  *(void **) (&st.module_get_win_geom) = dlsym(handle, "module_get_win_geom");
  *(void **) (&st.module_reset) = dlsym(handle, "module_reset");
  *(void **) (&st.module_get_preedit) = dlsym(handle, "module_get_preedit");
  *(void **) (&st.module_feedkey) = dlsym(handle, "module_feedkey");
  *(void **) (&st.module_feedkey_release) = dlsym(handle, "module_feedkey_release");
  *(void **) (&st.module_move_win) = dlsym(handle, "module_move_win");
  *(void **) (&st.module_change_font_size) = dlsym(handle, "module_change_font_size");
  *(void **) (&st.module_show_win) = dlsym(handle, "module_show_win");
  *(void **) (&st.module_hide_win) = dlsym(handle, "module_hide_win");
  *(void **) (&st.module_win_visible) = dlsym(handle, "module_win_visible");
  *(void **) (&st.module_flush_input) = dlsym(handle, "module_flush_input");

  return tmemdup(&st, GCIN_module_callback_functions, 1);
}
