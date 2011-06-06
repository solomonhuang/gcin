#include <chewing/chewing.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "gcin.h"

#define GCIN_CHEWING_CONFIG "/.gcin/config/chewing_conf.dat"

// TODO:
//     the hbox/label could be moved to local func
static GtkWidget *gcin_chewing_window = NULL;
static GtkWidget *vbox_top = NULL;
static GtkWidget *hbox_cancel_ok = NULL;
static GtkWidget *button_cancel = NULL;
static GtkWidget *button_ok = NULL;

static GtkWidget *g_pHBoxCandPerPage = NULL;
static GtkWidget *g_pLabelCandPerPage = NULL;
static GtkAdjustment *g_pGtkAdj = NULL;
static GtkWidget *g_pSpinButtonCandPerPage = NULL;

static GtkWidget *g_pHBoxSpaceAsSelection = NULL;
static GtkWidget *g_pLabelSpaceAsSelection = NULL;
static GtkWidget *g_pCheckButtonSpaceAsSelection = NULL;

static GtkWidget *g_pHBoxEscCleanAllBuf = NULL;
static GtkWidget *g_pLabelEscCleanAllBuf = NULL;
static GtkWidget *g_pCheckButtonEscCleanAllBuf = NULL;

static GtkWidget *g_pHBoxAutoShiftCur = NULL;
static GtkWidget *g_pLabelAutoShiftCur = NULL;
static GtkWidget *g_pCheckButtonAutoShiftCur = NULL;

static GtkWidget *g_pHBoxAddPhraseForward = NULL;
static GtkWidget *g_pLabelAddPhraseForward = NULL;
static GtkWidget *g_pCheckButtonAddPhraseForward = NULL;

static ChewingConfigData g_chewingConfig;
static int g_nFd = -1;

static void
chewing_config_dump (void)
{
    int nIdx = 0;
    printf ("chewing config:\n");
    printf ("\tcandPerPage: %d\n", g_chewingConfig.candPerPage);
    printf ("\tmaxChiSymbolLen: %d\n", g_chewingConfig.maxChiSymbolLen);
    printf ("\tbAddPhraseForward: %d\n", g_chewingConfig.bAddPhraseForward);
    printf ("\tbSpaceAsSelection: %d\n", g_chewingConfig.bSpaceAsSelection);
    printf ("\tbEscCleanAllBuf: %d\n", g_chewingConfig.bEscCleanAllBuf);
    printf ("\tbAutoShiftCur: %d\n", g_chewingConfig.bAutoShiftCur);
    printf ("\tbEasySymbolInput: %d\n", g_chewingConfig.bEasySymbolInput);
    printf ("\tbPhraseChoiceRearward: %d\n", g_chewingConfig.bPhraseChoiceRearward);
    printf ("\thsuSelKeyType: %d\n", g_chewingConfig.hsuSelKeyType);
    printf ("\tselKey: ");
    for (nIdx = 0; nIdx < MAX_SELKEY; nIdx++)
        printf ("%c ", g_chewingConfig.selKey[nIdx]);
    printf ("\n");
}

static void
chewing_config_default_set (void)
{
    int nDefaultSelKey[MAX_SELKEY] = {'a', 's', 'd', 'f',
                                      'g', 'h', 'j', 'k',
                                      'l', ';'};

    g_chewingConfig.candPerPage           = 10;
    g_chewingConfig.maxChiSymbolLen       = 16;
    g_chewingConfig.bAddPhraseForward     = 1;
    g_chewingConfig.bSpaceAsSelection     = 1;
    g_chewingConfig.bEscCleanAllBuf       = 0;
    g_chewingConfig.bAutoShiftCur         = 1;
    g_chewingConfig.bEasySymbolInput      = 0;
    g_chewingConfig.bPhraseChoiceRearward = 1;
    g_chewingConfig.hsuSelKeyType         = 0;
    memcpy (&g_chewingConfig.selKey,
            &nDefaultSelKey,
            sizeof (g_chewingConfig.selKey));
}

static gboolean
chewing_config_open (void)
{
    char *pszChewingConfig;
    char *pszHome;

    pszHome = getenv ("HOME");
    if (!pszHome)
        pszHome = "";

    pszChewingConfig = malloc (strlen (pszHome) + strlen (GCIN_CHEWING_CONFIG) + 1);
    memset (pszChewingConfig, 0x00, strlen (pszHome) + strlen (GCIN_CHEWING_CONFIG) + 1);
    sprintf (pszChewingConfig, "%s%s", pszHome, GCIN_CHEWING_CONFIG);

    g_nFd = open (pszChewingConfig,
                  O_RDWR  | O_CREAT,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    free (pszChewingConfig);

    return (g_nFd == -1 ? FALSE : TRUE);
}

static void
chewing_config_close (void)
{
    if (g_nFd != -1)
        close (g_nFd);
    g_nFd = -1;
}

static gboolean
chewing_config_load (void)
{
    int nReadSize;

    if (!chewing_config_open ())
        return FALSE;

    nReadSize = read (g_nFd, &g_chewingConfig, sizeof (g_chewingConfig));
    if (nReadSize == 0)
        chewing_config_default_set ();
    else if (nReadSize != sizeof (g_chewingConfig))
        return FALSE;

    chewing_config_close ();

    return TRUE;
}

static gboolean
chewing_config_save (void)
{
    int nWriteSize;

    if (!chewing_config_open ())
        return FALSE;

    g_chewingConfig.candPerPage =
        (int)gtk_spin_button_get_value (GTK_SPIN_BUTTON (g_pSpinButtonCandPerPage));
    if (g_chewingConfig.candPerPage > MAX_SELKEY)
        g_chewingConfig.candPerPage = MAX_SELKEY;

    g_chewingConfig.bSpaceAsSelection =
        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (g_pCheckButtonSpaceAsSelection));

    g_chewingConfig.bEscCleanAllBuf =
        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (g_pCheckButtonEscCleanAllBuf));

    g_chewingConfig.bAutoShiftCur =
        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (g_pCheckButtonAutoShiftCur));

    g_chewingConfig.bAddPhraseForward =
        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (g_pCheckButtonAddPhraseForward));

    nWriteSize = write (g_nFd, &g_chewingConfig, sizeof (g_chewingConfig));
    if (nWriteSize != sizeof (g_chewingConfig))
        return FALSE;

    chewing_config_close ();

    return TRUE;
}

static gboolean
cb_close_window (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    chewing_config_close ();

    gtk_widget_destroy (gcin_chewing_window);
    gcin_chewing_window = NULL;
    return TRUE;
}

static gboolean
cb_update_setting (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    chewing_config_save ();
    chewing_config_close ();

    gtk_widget_destroy (gcin_chewing_window);
    gcin_chewing_window = NULL;
    return TRUE;
}

void module_setup_window_create ()
{
    if (!chewing_config_load ())
        return;

    if (gcin_chewing_window)
    {
        gtk_window_present (GTK_WINDOW (gcin_chewing_window));
        return;
    }

    gcin_chewing_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    /* main setup win setting */
    gtk_window_set_position (GTK_WINDOW (gcin_chewing_window),
                             GTK_WIN_POS_MOUSE);
    gtk_window_set_has_resize_grip (GTK_WINDOW (gcin_chewing_window), FALSE);

    g_signal_connect (G_OBJECT (gcin_chewing_window), "delete_event",
                      G_CALLBACK (cb_close_window),
                      NULL);

    gtk_window_set_title (GTK_WINDOW (gcin_chewing_window),
                          _(_L("gcin 新酷音設定")));
    gtk_container_set_border_width (GTK_CONTAINER (gcin_chewing_window), 1);

    vbox_top = gtk_vbox_new (FALSE, 3);
    gtk_container_add (GTK_CONTAINER (gcin_chewing_window), vbox_top);

    // cand per page
    g_pHBoxCandPerPage = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), g_pHBoxCandPerPage, TRUE, TRUE, 1);
    g_pLabelCandPerPage = gtk_label_new (_(_L("每頁候選字數")));
    gtk_box_pack_start (GTK_BOX (g_pHBoxCandPerPage), g_pLabelCandPerPage, TRUE, TRUE, 0);
    g_pGtkAdj = (GtkAdjustment *)gtk_adjustment_new (g_chewingConfig.candPerPage, 1, 10, 1.0, 1.0, 0.0);
    g_pSpinButtonCandPerPage = gtk_spin_button_new (g_pGtkAdj, 0, 0);
    gtk_box_pack_start (GTK_BOX (g_pHBoxCandPerPage), g_pSpinButtonCandPerPage, FALSE, FALSE, 0);

    // space as selection
    g_pHBoxSpaceAsSelection = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), g_pHBoxSpaceAsSelection, TRUE, TRUE, 1);
    g_pLabelSpaceAsSelection = gtk_label_new (_(_L("空白鍵選字")));
    gtk_box_pack_start (GTK_BOX (g_pHBoxSpaceAsSelection), g_pLabelSpaceAsSelection, TRUE, TRUE, 0);
    g_pCheckButtonSpaceAsSelection = gtk_check_button_new ();
    gtk_box_pack_start (GTK_BOX (g_pHBoxSpaceAsSelection), g_pCheckButtonSpaceAsSelection, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (g_pCheckButtonSpaceAsSelection), g_chewingConfig.bSpaceAsSelection);

    // esc clean buf
    g_pHBoxEscCleanAllBuf = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), g_pHBoxEscCleanAllBuf, TRUE, TRUE, 1);
    g_pLabelEscCleanAllBuf = gtk_label_new (_(_L("ESC 鍵清空緩衝區")));
    gtk_box_pack_start (GTK_BOX (g_pHBoxEscCleanAllBuf), g_pLabelEscCleanAllBuf, TRUE, TRUE, 0);
    g_pCheckButtonEscCleanAllBuf = gtk_check_button_new ();
    gtk_box_pack_start (GTK_BOX (g_pHBoxEscCleanAllBuf), g_pCheckButtonEscCleanAllBuf, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (g_pCheckButtonEscCleanAllBuf), g_chewingConfig.bEscCleanAllBuf);

    // auto shift cursor
    g_pHBoxAutoShiftCur = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), g_pHBoxAutoShiftCur, TRUE, TRUE, 1);
    g_pLabelAutoShiftCur = gtk_label_new (_(_L("選字完畢自動跳字")));
    gtk_box_pack_start (GTK_BOX (g_pHBoxAutoShiftCur), g_pLabelAutoShiftCur, TRUE, TRUE, 0);
    g_pCheckButtonAutoShiftCur = gtk_check_button_new ();
    gtk_box_pack_start (GTK_BOX (g_pHBoxAutoShiftCur), g_pCheckButtonAutoShiftCur, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (g_pCheckButtonAutoShiftCur), g_chewingConfig.bAutoShiftCur);

    // add phrase forward
    g_pHBoxAddPhraseForward = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), g_pHBoxAddPhraseForward, TRUE, TRUE, 1);
    g_pLabelAddPhraseForward = gtk_label_new (_(_L("向後加詞")));
    gtk_box_pack_start (GTK_BOX (g_pHBoxAddPhraseForward), g_pLabelAddPhraseForward, TRUE, TRUE, 0);
    g_pCheckButtonAddPhraseForward = gtk_check_button_new ();
    gtk_box_pack_start (GTK_BOX (g_pHBoxAddPhraseForward), g_pCheckButtonAddPhraseForward, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (g_pCheckButtonAddPhraseForward), g_chewingConfig.bAddPhraseForward);

    // cancel & ok buttons
    hbox_cancel_ok = gtk_hbox_new (FALSE, 10);
    gtk_box_pack_start (GTK_BOX (vbox_top), hbox_cancel_ok , FALSE, FALSE, 5);

    button_cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
    gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_cancel, TRUE, TRUE, 0);
    button_ok = gtk_button_new_from_stock (GTK_STOCK_OK);
    gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_ok, TRUE, TRUE, 5);

    g_signal_connect (G_OBJECT (button_cancel), "clicked",
                      G_CALLBACK (cb_close_window),
                      G_OBJECT (gcin_chewing_window));

    g_signal_connect (G_OBJECT (button_ok), "clicked",
                      G_CALLBACK (cb_update_setting),
                      G_OBJECT (gcin_chewing_window));

    gtk_widget_show_all (gcin_chewing_window);
}
