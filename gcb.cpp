#include <stdlib.h>
#include "os-dep.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <time.h>
#include "gcin.h"
#include "gcin-conf.h"

static GtkWidget *mainwin;
static GtkClipboard *pclipboard, *pclipboard_prim;
static GtkWidget **buttonArr;
static gchar **buttonStr;
static int maxButtonStrlen=9;
// static GdkAtom atom_cutbuffer0;
static char geomstr[5];
static GtkWidget *snoop_button;
#if !GTK_CHECK_VERSION(2,12,0)
static GtkTooltips *button_bar_tips;
#endif
static GtkWidget *hist_window;
static gchar **hist_strArr;
static GtkWidget **hist_buttonArr;
static void del_nl(char *tmpstr)
{
   int i;

   for(i=0;tmpstr[i];i++) {
     if (tmpstr[i]=='\n')
       tmpstr[i]=' ';
   }
}

void utf8ncpy(char *t, int tsize, char *s)
{
   int tlen = 0;
   char *p=s;
   int slen = strlen(s);
   while (*p) {
     char sz = utf8_sz(p);
     if (tlen + sz > slen || tlen+sz >= tsize-1) // incomplete utf8 char
       break;
     memcpy(t + tlen, p, sz);
     tlen += sz;
     p+=sz;
   }
   t[tlen]=0;
}

static void update_hist_button()
{
  int i;

  for(i=0;i<gcb_history_n;i++) {
    char tstr[16];

    if (!hist_strArr[i])
      continue;

    utf8ncpy(tstr, sizeof(tstr), hist_strArr[i]);

    del_nl(tstr);
    gtk_button_set_label(GTK_BUTTON(hist_buttonArr[i]),tstr);
#if 0
#if GTK_CHECK_VERSION(2,12,0)
    gtk_widget_set_tooltip_text (hist_buttonArr[i], hist_strArr[i]);
#else
    gtk_tooltips_set_tip (GTK_TOOLTIPS (button_bar_tips),hist_buttonArr[i],
      hist_strArr[i],NULL);
#endif
#endif
  }
}

static void show_hist_window()
{
  gtk_window_parse_geometry(GTK_WINDOW(hist_window),geomstr);

  update_hist_button();

  gtk_window_resize(GTK_WINDOW(hist_window), 1, 1);
  gtk_widget_show (hist_window);
  gtk_window_present(GTK_WINDOW(hist_window));
}

void set_win_title(const gchar *text)
{
   char titlestr[34];
   utf8ncpy(titlestr, sizeof(titlestr), (char *)text);
   gtk_window_set_title (GTK_WINDOW (mainwin),titlestr);
}

/* Signal handler called when the selections owner returns the data */
void disp_gcb_selection(const gchar *text)
{
  // dbg("selection_received '%s'\n", text);
  char *tmpstr;
  GtkWidget *button = snoop_button;
  int i;
  int textlen;
  static char bombom[]="\xef\xbb\xbf\xef\xbb\xbf";

  if (!text || !text[0])
     return;

  textlen=strlen(text);
  if (textlen==1 && text[0] <= ' ')
    return;
  // google chrome
  if (!strcmp(text,bombom))
    return;

#if 0
  dbg("textlen %d\n", textlen);
  for(i=0; i < textlen;i++)
    dbg("%x ", (unsigned char)text[i]);
  dbg("\n");
#endif

  if (!buttonArr)
    return;


  for(i=0;i<gcb_button_n;i++) {
    if (buttonStr[i] && !strcmp(buttonStr[i],text))
      return;
  }

   tmpstr=(char *)g_malloc(maxButtonStrlen);
   utf8ncpy(tmpstr, maxButtonStrlen, (char *)text);

   del_nl(tmpstr);

   for(i=0;i<gcb_button_n;i++) {
     if (buttonArr[i]==button) {
       if (buttonStr[i])
         g_free(buttonStr[i]);
       buttonStr[i]=g_strdup(text);
     }
   }

   gtk_button_set_label(GTK_BUTTON(button),tmpstr);

   set_win_title(text);

#if GTK_CHECK_VERSION(2,12,0)
   gtk_widget_set_tooltip_text (button, text);
#else
   gtk_tooltips_set_tip (GTK_TOOLTIPS (button_bar_tips), button, text,NULL);
#endif

   g_free(tmpstr);

   gtk_window_resize(GTK_WINDOW(mainwin), 100, 24);

   // remove the duplicate item if any
   for(i=0;i< gcb_history_n; i++) {
     if (!hist_strArr[i])
       continue;
     int len = strlen(hist_strArr[i]);
     if (strncmp(hist_strArr[i], text, len))
       continue;

     g_free(hist_strArr[i]);

     memmove(&hist_strArr[i],&hist_strArr[i+1],
             sizeof(hist_strArr[0])*(gcb_history_n - i - 1));

     hist_strArr[gcb_history_n-1]=NULL;
     break;
   }

   g_free(hist_strArr[gcb_history_n-1]);
   memmove(&hist_strArr[1],&hist_strArr[0],
           sizeof(hist_strArr[0])*(gcb_history_n-1));

   hist_strArr[0]=g_strdup(text);

   update_hist_button();
}


void cb_selection_received(GtkClipboard *pclip, const gchar *text, gpointer data)
{
//  dbg("cb_selection_received %s\n", text);
  disp_gcb_selection(text);
}


void get_selection(GtkClipboard *pcli)
{
  gtk_clipboard_request_text(pcli, cb_selection_received,snoop_button);
}


void set_snoop_button(GtkWidget *widget)
{
  gtk_button_set_relief(GTK_BUTTON(widget),GTK_RELIEF_NONE);
  snoop_button = widget;
}


static void get_mouse_button( GtkWidget *widget,GdkEventButton *event, gpointer data)
{
  int i;
#if 0
  printf("b gtk_widget_get_events  %d  %x  %d\n",event->type,event->state,event->button);
#endif
  if (event->button == 3) {
    for(i=0;i<gcb_button_n;i++) {
      if (buttonArr[i]!=widget)
        gtk_button_set_relief(GTK_BUTTON(buttonArr[i]),GTK_RELIEF_NORMAL);
    }

    gtk_window_present(GTK_WINDOW(mainwin));
//    dbg("widget %x\n", widget);
#if GTK_CHECK_VERSION(2,18,0)
    gtk_widget_set_can_default(widget, TRUE);
    gtk_widget_grab_default(widget);
#endif
    set_snoop_button(widget);
  } else
  if (event->button == 2) {
    show_hist_window();
  } else
  if (event->button == 1) {
    gtk_window_present(GTK_WINDOW(mainwin));

    for(i=0;i<gcb_button_n;i++) {
      if (buttonArr[i]!=widget)
        continue;
      if (buttonStr[i]) {
        gtk_clipboard_set_text(pclipboard, buttonStr[i], -1);
        gtk_clipboard_set_text(pclipboard_prim, buttonStr[i], -1);
        set_win_title(buttonStr[i]);
      }
      break;
    }

  }
}

static void hist_get_mouse_button( GtkWidget *widget,GdkEventButton *event, gpointer data)
{
  int i;
#if 0
  printf("b gtk_widget_get_events  %d  %x  %d\n",event->type,event->state,event->button);
#endif
  if (event->button == 1) {
    for(i=0;i<gcb_history_n;i++) {
      if (hist_buttonArr[i]!=widget)
        continue;
      if (hist_strArr[i]) {
        gtk_clipboard_set_text(pclipboard, hist_strArr[i], -1);
        gtk_clipboard_set_text(pclipboard_prim, hist_strArr[i], -1);
      }
      break;
    }
  }

  gtk_widget_hide(hist_window);
}

gboolean delete_hist_win()
{
  gtk_widget_hide(hist_window);
  return TRUE;
}

gboolean  key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
#if 0
   if ((event->string && event->string[0]=='q') || event->keyval==GDK_Escape)
     do_exit();
#endif
  return TRUE;
}


gboolean  hist_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
   gtk_widget_hide(hist_window);
//   if ((event->string && event->string[0]=='q') || event->keyval==GDK_Escape)
//   printf("hist event->keyval:%x\n",event->keyval);
   return TRUE;
}

static gboolean  gcb_button_scroll_event(GtkWidget *widget,GdkEventScroll *event, gpointer user_data)
{
  if (event->direction!=GDK_SCROLL_DOWN)
    return TRUE;

  show_hist_window();

  return FALSE;
}

gboolean hist_focus_out_callback(GtkWidget *widget, GdkEventFocus *event,
                                 gpointer user_data)
{
   gtk_widget_hide(hist_window);
   return TRUE;
}


static void cb_owner_change(GtkClipboard *clipboard, GdkEvent *event, gpointer ser_data)
{
//  dbg("cb_owner_change\n");
  get_selection(clipboard);
}


static void mouse_button_callback( GtkWidget *widget,GdkEventButton *event, gpointer data)
{
  show_hist_window();
}

static int old_gcb_position=-1, old_gcb_position_x=-1, old_gcb_position_y=-1, old_gcb_enabled=-1;
void sys_icon_fname(char *iconame, char fname[]);
void gcb_main()
{

  GtkWidget *hbox,*vbox;
  int i;
#if 1
  if (gcb_enabled==old_gcb_enabled && gcb_position==old_gcb_position && gcb_position_x==old_gcb_position_x
      && gcb_position_y==old_gcb_position_y)
    return;
#endif
  old_gcb_enabled = gcb_enabled;
  old_gcb_position = gcb_position;
  old_gcb_position_x = gcb_position_x;
  old_gcb_position_y = gcb_position_y;

  if (mainwin)
    gtk_widget_destroy(mainwin);
#if 0
  if (button_bar_tips)
    gtk_widget_destroy(button_bar_tips);
#endif
  if (hist_window)
    gtk_widget_destroy(hist_window);

  if (!gcb_enabled)
    return;

//  printf("gcb_position:%d\n", gcb_position);

  static char geo[][2]={{0,0},{'+','-'},{'+','+'},{'-','-'},{'-','+'}};
  sprintf(geomstr, "%c%d%c%d",
  geo[gcb_position][0], gcb_position_x, geo[gcb_position][1], gcb_position_y);
//  puts(geomstr);

  if (!buttonArr) {
    buttonArr=(GtkWidget**)g_malloc(gcb_button_n * sizeof(GtkWidget *));
    buttonStr=(gchar**)g_malloc0(gcb_button_n * sizeof(gchar *));
  }

  if (!hist_strArr) {
    hist_strArr=(gchar**)g_malloc0(gcb_history_n * sizeof(gchar *));
    hist_buttonArr=(GtkWidget**)g_malloc(gcb_history_n * sizeof(GtkWidget *));
  }

  mainwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_has_resize_grip(GTK_WINDOW(mainwin), FALSE);
  gtk_window_set_decorated(GTK_WINDOW(mainwin),FALSE);
  gtk_window_set_focus_on_map (GTK_WINDOW(mainwin), FALSE);

  hist_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_has_resize_grip(GTK_WINDOW(hist_window), FALSE);

  char icon_path[40];
  sys_icon_fname("gcb.png", icon_path);
  GtkWidget *win_icon=gtk_image_new_from_file(icon_path);
  gtk_window_set_icon(GTK_WINDOW(mainwin),
     gtk_image_get_pixbuf(GTK_IMAGE(win_icon)));
  gtk_window_set_icon(GTK_WINDOW(hist_window),
     gtk_image_get_pixbuf(GTK_IMAGE(win_icon)));

  // Under gnome 2.0, the mainwin is not fixed if decorated, annoying
  gtk_window_set_decorated(GTK_WINDOW(hist_window),FALSE);
  gtk_window_set_skip_pager_hint(GTK_WINDOW(hist_window),TRUE);
  gtk_window_set_skip_taskbar_hint(GTK_WINDOW(hist_window),TRUE);
  gtk_window_set_title (GTK_WINDOW (hist_window),"gcb history");

  gtk_window_set_title (GTK_WINDOW(mainwin), "gcb: gtk copy-paste buffer");
  gtk_window_stick(GTK_WINDOW(mainwin));

//  g_signal_connect(G_OBJECT (mainwin),"destroy", G_CALLBACK (do_exit), NULL);
  g_signal_connect(G_OBJECT (hist_window),"delete_event",
    G_CALLBACK (delete_hist_win), NULL);
  g_signal_connect(G_OBJECT (hist_window),"focus-out-event",
    G_CALLBACK (hist_focus_out_callback), NULL);

#if !GTK_CHECK_VERSION(2,12,0)
  button_bar_tips = gtk_tooltips_new ();
#endif

  hbox = gtk_hbox_new (FALSE, 1);
  gtk_container_add (GTK_CONTAINER(mainwin), hbox);

  gtk_window_parse_geometry(GTK_WINDOW(mainwin),geomstr);

  for(i=0;i<gcb_button_n;i++) {
    buttonArr[i] = gtk_button_new_with_label ("---");
//    gtk_container_set_border_width(GTK_CONTAINER(buttonArr[i]),0);
    gtk_box_pack_start (GTK_BOX(hbox), buttonArr[i], TRUE, TRUE, FALSE);
    gtk_widget_show (buttonArr[i]);
    g_signal_connect (G_OBJECT (buttonArr[i]), "button-press-event",
                      G_CALLBACK (get_mouse_button), (gpointer) buttonArr[i]);
#if 0
    g_signal_connect (G_OBJECT (buttonArr[i]), "key-press-event",
                      G_CALLBACK (key_press_event), NULL);
#endif
#if 1
    g_signal_connect (G_OBJECT (buttonArr[i]), "scroll-event",
                      G_CALLBACK (gcb_button_scroll_event), NULL);
#endif
  }

  vbox = gtk_vbox_new (FALSE, 1);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER(hist_window), vbox);

  for(i=0;i<gcb_history_n;i++) {
    hist_buttonArr[i] = gtk_button_new_with_label ("---");
    gtk_container_set_border_width(GTK_CONTAINER(hist_buttonArr[i]),0);
    gtk_box_pack_start (GTK_BOX(vbox), hist_buttonArr[i], TRUE, TRUE, FALSE);
    gtk_widget_show (hist_buttonArr[i]);
    g_signal_connect (G_OBJECT (hist_buttonArr[i]), "button-press-event",
                      G_CALLBACK (hist_get_mouse_button), (gpointer) hist_buttonArr[i]);
#if 1
    g_signal_connect (G_OBJECT (hist_buttonArr[i]), "key-press-event",
                      G_CALLBACK (hist_key_press_event), NULL);
#endif
  }

#if 1
  // need this because on win32 scoll is not recieved if win is not focused.
  gtk_box_pack_start (GTK_BOX (hbox), gtk_vseparator_new(), FALSE, FALSE, 0);
  GtkWidget *eve_arrow = gtk_event_box_new();
  gtk_event_box_set_visible_window (GTK_EVENT_BOX(eve_arrow), FALSE);
  gtk_box_pack_start (GTK_BOX(hbox), eve_arrow, FALSE, FALSE, FALSE);
    g_signal_connect(G_OBJECT(eve_arrow),"button-press-event", G_CALLBACK(mouse_button_callback), NULL);
  gtk_container_add(GTK_CONTAINER(eve_arrow), gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_IN));
#endif


  gtk_widget_show_all(hbox);
  gtk_widget_show (vbox);
  gtk_widget_show (mainwin);


  pclipboard_prim = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  pclipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

  set_snoop_button(buttonArr[0]);
  get_selection(pclipboard);
  get_selection(pclipboard_prim);
  gtk_container_set_border_width(GTK_CONTAINER(hbox),0);
  gtk_container_set_border_width(GTK_CONTAINER(mainwin),0);

  gtk_window_parse_geometry(GTK_WINDOW(mainwin),geomstr);
#if GTK_CHECK_VERSION(2,6,0)
  g_signal_connect(pclipboard, "owner-change", G_CALLBACK (cb_owner_change), NULL);
  g_signal_connect(pclipboard_prim, "owner-change", G_CALLBACK (cb_owner_change), NULL);
#endif
}
