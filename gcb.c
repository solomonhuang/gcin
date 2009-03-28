#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <stdio.h>
#include <time.h>
#include <X11/Xatom.h>
#include "gcin.h"
#include "gcin-conf.h"


static GtkWidget *mainwin;
static GtkClipboard *pclipboard;
static GtkWidget **buttonArr;
static gchar **buttonStr;
static int buttonArrN=3;
static int maxButtonStrlen=9;
// static GdkAtom atom_cutbuffer0;
static char geomstr[5];
static GtkWidget *snoop_button;
static GtkTooltips *button_bar_tips;
static GtkWidget *hist_window;
static gchar **hist_strArr;
static int hist_strArrN=10;
static GtkWidget **hist_buttonArr;

static void del_nl(char *tmpstr)
{
   int i;

   for(i=0;tmpstr[i];i++) {
     if (tmpstr[i]=='\n')
       tmpstr[i]=' ';
   }
}

static update_hist_button()
{
  int i;

  for(i=0;i<hist_strArrN;i++) {
    char tstr[16];

    if (!hist_strArr[i])
      continue;

    snprintf(tstr,sizeof(tstr),"%s",hist_strArr[i]);

    del_nl(tstr);
    gtk_button_set_label(GTK_BUTTON(hist_buttonArr[i]),tstr);
#if 0
    gtk_tooltips_set_tip (GTK_TOOLTIPS (button_bar_tips),hist_buttonArr[i],
      hist_strArr[i],NULL);
#endif
  }
}

static void show_hist_window()
{
  gtk_window_parse_geometry(GTK_WINDOW(hist_window),geomstr);

  update_hist_button();

  gtk_window_resize(GTK_WINDOW(hist_window), 40, 100);
  gtk_widget_show (hist_window);
  gtk_window_present(GTK_WINDOW(hist_window));
}

void set_win_title(const gchar *text)
{
   char titlestr[34];

   snprintf(titlestr,sizeof(titlestr),"%s",text);

   gtk_window_set_title (GTK_WINDOW (mainwin),titlestr);
}


/* Signal handler called when the selections owner returns the data */
static void selection_received(GtkClipboard *pclip, const gchar *text, gpointer data)
{
   char *tmpstr;
   GtkWidget *button = (GtkWidget *)data;
   int i;
   int textlen;

   if (!text || !text[0])
     return;

   for(i=0;i<buttonArrN;i++) {
     if (buttonStr[i] && !strcmp(buttonStr[i],text))
       return;
   }

   tmpstr=g_malloc(maxButtonStrlen+1);
#if 0
   // strncpy doesn't work as expected on Mandrake 9.0
   strncpy(tmpstr,text,maxButtonStrlen);
#else
   textlen=strlen(text);
   if (textlen > maxButtonStrlen)
     textlen = maxButtonStrlen;
   memcpy(tmpstr,text,textlen);
   tmpstr[textlen]=0;
#endif

   del_nl(tmpstr);

   for(i=0;i<buttonArrN;i++) {
     if (buttonArr[i]==button) {
       if (buttonStr[i])
         g_free(buttonStr[i]);
       buttonStr[i]=g_strdup(text);
     }
   }

   gtk_button_set_label(GTK_BUTTON(button),tmpstr);


   set_win_title(text);

   gtk_tooltips_set_tip (GTK_TOOLTIPS (button_bar_tips), button, text,NULL);

   g_free(tmpstr);

   gtk_window_resize(GTK_WINDOW(mainwin), 100, 24);

   // remove the duplicate item if any
   for(i=0;i< hist_strArrN; i++) {
     if (!hist_strArr[i])
       continue;
     if (strcmp(hist_strArr[i],text))
       continue;

     g_free(hist_strArr[i]);

     memmove(&hist_strArr[i],&hist_strArr[i+1],
             sizeof(hist_strArr[0])*(hist_strArrN- i - 1));

     hist_strArr[hist_strArrN-1]=NULL;
     break;
   }

   g_free(hist_strArr[hist_strArrN-1]);
   memmove(&hist_strArr[1],&hist_strArr[0],
           sizeof(hist_strArr[0])*(hist_strArrN-1));

   hist_strArr[0]=g_strdup(text);

   update_hist_button();
}


void get_selection()
{
  gtk_clipboard_request_text(pclipboard,selection_received,snoop_button);
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
    for(i=0;i<buttonArrN;i++) {
      if (buttonArr[i]!=widget)
        gtk_button_set_relief(GTK_BUTTON(buttonArr[i]),GTK_RELIEF_NORMAL);
    }

    gtk_window_present(GTK_WINDOW(mainwin));
    set_snoop_button(widget);
  } else
  if (event->button == 2) {
    show_hist_window();
  } else
  if (event->button == 1) {
    gtk_window_present(GTK_WINDOW(mainwin));

    for(i=0;i<buttonArrN;i++) {
      if (buttonArr[i]!=widget)
        continue;
      if (buttonStr[i]) {
        gtk_clipboard_set_text(pclipboard, buttonStr[i], -1);
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
    for(i=0;i<hist_strArrN;i++) {
      if (hist_buttonArr[i]!=widget)
        continue;
      if (hist_strArr[i]) {
        gtk_clipboard_set_text(pclipboard, hist_strArr[i], -1);

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

#if 0
static void free_mem()
{
  int i;

  for(i=0;i<buttonArrN;i++)
    g_free(buttonStr[i]);

  g_free(buttonArr);
  g_free(buttonStr);

  for(i=0;i<hist_strArrN;i++)
    g_free(hist_strArr[i]);
  g_free(hist_strArr);
  g_free(hist_buttonArr);
}
#endif

gboolean  key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
#if 0
   if ((event->string && event->string[0]=='q') || event->keyval==GDK_Escape)
     do_exit();
   return TRUE;
#endif
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
  int winx,winy,i;

  if (event->direction!=GDK_SCROLL_DOWN)
    return TRUE;

  show_hist_window();
}

gboolean hist_focus_out_callback(GtkWidget *widget, GdkEventFocus *event,
                                 gpointer user_data)
{
   gtk_widget_hide(hist_window);
   return TRUE;
}

gboolean timeout_periodic_clipboard_fetch()
{
  get_selection();
  return TRUE;
}

void gcb_main()
{

  GtkWidget *hbox,*vbox;
  GtkWidget *proxy_invisible;
  int i;

  if (mainwin)
    gtk_widget_destroy(mainwin);
#if 0
  if (button_bar_tips)
    gtk_widget_destroy(button_bar_tips);
#endif
  if (hist_window)
    gtk_widget_destroy(hist_window);

  if (!gcb_position)
    return;

  printf("gcb_position:%d\n", gcb_position);

  static char geo[][2]={{0,0},{'+','-'},{'+','+'},{'-','-'},{'-','+'}};
  sprintf(geomstr, "%c%d%c%d",
  geo[gcb_position][0], gcb_position_x, geo[gcb_position][1], gcb_position_y);
//  puts(geomstr);

  if (!buttonArr) {
    buttonArr=g_malloc(buttonArrN * sizeof(GtkWidget *));
    buttonStr=g_malloc0(buttonArrN * sizeof(gchar *));
  }

  if (!hist_strArr) {
    hist_strArr=g_malloc0(hist_strArrN * sizeof(gchar *));
    hist_buttonArr=g_malloc(hist_strArrN * sizeof(GtkWidget *));
  }

  mainwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  hist_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  GtkWidget *win_icon=gtk_image_new_from_file(GCIN_ICON_DIR "/gcb.png");
  gtk_window_set_icon(GTK_WINDOW(mainwin),
     gtk_image_get_pixbuf(GTK_IMAGE(win_icon)));
  gtk_window_set_icon(GTK_WINDOW(hist_window),
     gtk_image_get_pixbuf(GTK_IMAGE(win_icon)));

  // Under gnome 2.0, the mainwin is not fixed if decorated, annoying
  gtk_window_set_decorated(GTK_WINDOW(hist_window),FALSE);
  gtk_window_set_title (GTK_WINDOW (hist_window),"gcb history");

  gtk_window_set_title (GTK_WINDOW(mainwin), "gcb: gtk copy-paste buffer");
  gtk_window_stick(GTK_WINDOW(mainwin));

//  g_signal_connect(G_OBJECT (mainwin),"destroy", G_CALLBACK (do_exit), NULL);
  g_signal_connect(G_OBJECT (hist_window),"delete_event",
    G_CALLBACK (delete_hist_win), NULL);
  g_signal_connect(G_OBJECT (hist_window),"focus-out-event",
    G_CALLBACK (hist_focus_out_callback), NULL);

  button_bar_tips = gtk_tooltips_new ();

  hbox = gtk_hbox_new (FALSE, 1);
  vbox = gtk_vbox_new (FALSE, 1);

  gtk_window_parse_geometry(GTK_WINDOW(mainwin),geomstr);

  for(i=0;i<buttonArrN;i++) {
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


  for(i=0;i<hist_strArrN;i++) {
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

  gtk_container_add (GTK_CONTAINER(mainwin), hbox);
  gtk_container_add (GTK_CONTAINER(hist_window), vbox);

  gtk_widget_show (hbox);
  gtk_widget_show (vbox);
  gtk_widget_show (mainwin);
#if 0
  gdk_input_set_extension_events(mainwin->window, GDK_EXTENSION_EVENTS_ALL,
                                 GDK_EXTENSION_EVENTS_ALL);
#endif
  pclipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);

  set_snoop_button(buttonArr[0]);
  get_selection();
#if 0
  gdk_window_set_events (GDK_ROOT_PARENT (), GDK_PROPERTY_CHANGE_MASK);
  gdk_window_add_filter (GDK_ROOT_PARENT (), property_change_event, NULL);
#endif
  gtk_container_set_border_width(GTK_CONTAINER(hbox),0);
  gtk_container_set_border_width(GTK_CONTAINER(mainwin),0);

  gtk_window_set_decorated(GTK_WINDOW(mainwin),FALSE);

  gtk_window_parse_geometry(GTK_WINDOW(mainwin),geomstr);

  gtk_timeout_add(3000, timeout_periodic_clipboard_fetch, NULL);
}
