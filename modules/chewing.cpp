#include <chewing/chewing.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "gcin.h"
#include "pho.h"
#include "gst.h"
#include "im-client/gcin-im-client-attr.h"
#include "win1.h"
#include "gcin-module.h"
#include "gcin-module-cb.h"

#define MAX_SEG_NUM  128
#define GCIN_CHEWING_CONFIG "/.gcin/config/chewing_conf.dat"

typedef struct _SEGMENT {
    GtkWidget *label;
    unsigned char selidx, selN;
} SEG;

/*---------------------------------------------------------------------------*/
/*    call back funcs                                                        */
/*---------------------------------------------------------------------------*/
int module_init_win (GCIN_module_main_functions *pFuncs);
void module_get_win_geom (void);
int module_reset (void);
int module_get_preedit (char *pszStr, GCIN_PREEDIT_ATTR attr[],
                        int *pnCursor, int *pCompFlag);
gboolean module_feedkey (int nKeyVal, int nKeyState);
int module_feedkey_release (KeySym xkey, int nKbState);
void module_move_win(int x, int y);
void module_change_font_size (void);
void module_show_win (void);
void module_hide_win (void);
int module_win_visible (void);
int module_flush_input (void);

/*---------------------------------------------------------------------------*/
/*    local funcs                                                            */
/*---------------------------------------------------------------------------*/
static gboolean select_idx (int c);
static void prev_page (void);
static void next_page (void);
static gboolean chewing_initialize (void);
static gboolean is_empty (void);
static gboolean gcin_label_show (char *pszPho, int nPos);
static gboolean gcin_label_clear (int nCount);
static gboolean gcin_label_cand_show (char *pszWord, int nCount);
static gboolean gtk_pango_font_pixel_size_get (int *pnFontWidth, int *pnFontHeight);
static gboolean chewing_config_open (void);
static void chewing_config_load (void);
static void chewing_config_set (void);
static void chewing_config_dump (void);
void chewing_config_close (void);

/*---------------------------------------------------------------------------*/
/*    global vars                                                            */
/*---------------------------------------------------------------------------*/
static GCIN_module_main_functions g_gcinModMainFuncs;
static GtkWidget *g_pWinChewing      = NULL;
static ChewingContext *g_pChewingCtx = NULL;
static GtkWidget *g_pEvBoxChewing    = NULL;
static GtkWidget *g_pHBoxChewing     = NULL;
static SEG *g_pSeg = NULL;
static int g_nCurrentCursorPos = 0;
static ChewingConfigData g_chewingConfig;
static int g_nFd;
static gboolean g_bUseDefault = FALSE;

// FIXME: impl
static gboolean
select_idx (int c)
{
    return TRUE;
}

// FIXME: impl
static void
prev_page (void)
{
}

// FIXME: impl
static void
next_page (void)
{
}

static gboolean
gcin_label_show (char *pszPho, int nPos)
{
    char szTmp[128];

    if (!pszPho)
        return FALSE;

    memset (szTmp, 0x00, 128);

    sprintf (szTmp, "<span background=\"%s\">%s</span>",
             *g_gcinModMainFuncs.mf_tsin_cursor_color,
             pszPho);

    gtk_label_set_markup (GTK_LABEL (g_pSeg[nPos].label),
                          nPos != g_nCurrentCursorPos ? pszPho : szTmp);

    return TRUE;
}

static gboolean
gcin_label_clear (int nCount)
{
    while (nCount--)
        gtk_label_set_text (GTK_LABEL (g_pSeg[nCount].label), NULL);

    return TRUE;
}

static gboolean
gtk_pango_font_pixel_size_get (int *pnFontWidth, int *pnFontHeight)
{
    PangoLayout *pPangoLayout;
    PangoContext *pPangoContext;
    PangoFontDescription *pPangoFontDesc;
    int nFontWidth, nFontHeight;

    pPangoLayout = gtk_widget_create_pango_layout (g_pWinChewing, "中");
    //pPangoLayout = gtk_widget_create_pango_layout (
    //                   g_pWinChewing,
    //                   (char *)gtk_label_get_text (GTK_LABEL (g_pSeg[g_nCurrentCursorPos].label)));
    pPangoContext = gtk_widget_get_pango_context (g_pWinChewing);
    pPangoFontDesc = pango_context_get_font_description (pPangoContext);

    pango_layout_set_font_description (pPangoLayout, pPangoFontDesc);
    pango_layout_get_pixel_size (pPangoLayout, pnFontWidth, pnFontHeight);

    g_object_unref (pPangoLayout);
    return TRUE;
}

// FIXME: the pos of g_pSeg[].label is not correct
static gboolean
gcin_label_cand_show (char *pszWord, int nIdx)
{
    int nX, nY;
    int nFontWidth, nFontHeight;

    g_gcinModMainFuncs.mf_set_sele_text (chewing_cand_TotalChoice (g_pChewingCtx),
                                         nIdx,
                                         pszWord,
                                         -1);

    // find the position of the cand win
    g_gcinModMainFuncs.mf_get_widget_xy (g_pWinChewing,
                                         g_pSeg[g_nCurrentCursorPos].label,
                                         &nX, &nY);

    gtk_pango_font_pixel_size_get (&nFontWidth, &nFontHeight);
    nX += g_nCurrentCursorPos * nFontWidth;

    nY = g_gcinModMainFuncs.mf_gcin_edit_display_ap_only () ?
         *g_gcinModMainFuncs.mf_win_y :
         *g_gcinModMainFuncs.mf_win_y + *g_gcinModMainFuncs.mf_win_yl;

    g_gcinModMainFuncs.mf_disp_selections (nX, nY);

    return TRUE;
}

static gboolean
chewing_initialize (void)
{
    char *pszChewingHashDir;
    char *pszHome;

    pszHome = getenv ("HOME");
    if (!pszHome)
        pszHome = "";

    pszChewingHashDir = malloc (strlen (pszHome) + strlen ("/.chewing/") + 1);
    memset (pszChewingHashDir, 0x00, strlen (pszHome) + strlen ("/.chewing/") + 1);
    sprintf (pszChewingHashDir, "%s/.chewing", pszHome);

    if (chewing_Init (CHEWING_DATADIR, pszChewingHashDir) != 0)
    {
        free (pszChewingHashDir);
        return FALSE;
    }
    free (pszChewingHashDir);

    g_pChewingCtx = chewing_new ();
    if (!g_pChewingCtx)
        return FALSE;

    if (chewing_config_open ())
        chewing_config_load ();

    chewing_config_set ();

    chewing_config_close ();

    return TRUE;
}

// FIXME: impl
static gboolean
is_empty (void)
{
    if (!g_pChewingCtx)
        return FALSE;
    return chewing_zuin_Check (g_pChewingCtx) ? FALSE : TRUE;
}

int
module_init_win (GCIN_module_main_functions *pFuncs)
{
    GtkWidget *pErrDialog = NULL;
    int nIdx;
    int nSegSize = 0;

    if (!pFuncs)
        return FALSE;

    g_gcinModMainFuncs = *pFuncs;

    g_gcinModMainFuncs.mf_set_tsin_pho_mode ();
    g_gcinModMainFuncs.mf_set_win1_cb ((cb_selec_by_idx_t)select_idx,
                                        prev_page,
                                        next_page);

    if (g_pWinChewing)
        return TRUE;

    if (!chewing_initialize ())
    {
        pErrDialog = gtk_message_dialog_new (NULL,
                         GTK_DIALOG_MODAL,
                         GTK_MESSAGE_ERROR,
                         GTK_BUTTONS_CLOSE,
                         "chewing init failed");
        gtk_dialog_run (GTK_DIALOG (pErrDialog));
        gtk_widget_destroy (pErrDialog);
        return FALSE;
    }

    g_pWinChewing = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_has_resize_grip (GTK_WINDOW (g_pWinChewing), FALSE);

    gtk_window_set_default_size (GTK_WINDOW (g_pWinChewing), 32, 12);

    gtk_widget_realize (g_pWinChewing);
    g_gcinModMainFuncs.mf_set_no_focus (g_pWinChewing);

    g_pEvBoxChewing = gtk_event_box_new ();
    if (!g_pEvBoxChewing)
        return FALSE;
    gtk_container_add (GTK_CONTAINER (g_pWinChewing), g_pEvBoxChewing);

    g_pHBoxChewing = gtk_hbox_new (FALSE, 0);
    if (!g_pHBoxChewing)
        return FALSE;
    gtk_container_add (GTK_CONTAINER (g_pEvBoxChewing), g_pHBoxChewing);

    // TODO: not sure if we need the mouse CB or not...
    //        if we add the CB here, then it seems that we have to use gdk
    //        it seems not so important...? do it later
    //g_signal_connect (G_OBJECT (g_pEvBoxChewing), "button-press-event",
    //                  G_CALLBACK (mouse_button_callback), NULL);

    if (!g_pSeg)
    {
        nSegSize = sizeof (SEG) * MAX_SEG_NUM;
        g_pSeg = malloc (nSegSize);
        memset (g_pSeg, 0, nSegSize);
    }

    for (nIdx = 0; nIdx < MAX_SEG_NUM; nIdx++)
    {
        g_pSeg[nIdx].label = gtk_label_new (NULL);
        gtk_widget_show (g_pSeg[nIdx].label);
        gtk_box_pack_start (GTK_BOX (g_pHBoxChewing),
                            g_pSeg[nIdx].label,
                            FALSE,
                            FALSE,
                            0);
    }

    if (!g_gcinModMainFuncs.mf_phkbm->selkeyN)
        g_gcinModMainFuncs.mf_load_tab_pho_file();

    gtk_widget_show_all (g_pWinChewing);

    g_gcinModMainFuncs.mf_init_tsin_selection_win ();

    module_change_font_size ();

    module_hide_win ();

    return TRUE;
}

// FIXME: impl
void
module_get_win_geom (void)
{
    return;
}

// FIXME: chk
int
module_reset (void)
{
    if (!g_pWinChewing)
        return 0;
    return 1;
}

// FIXME: refine and chk
int
module_get_preedit (char *pszStr, GCIN_PREEDIT_ATTR gcinPreeditAttr[],
                    int *pnCursor, int *pCompFlag)
{
    char *pszTmpStr = NULL;
    int nIdx;
    int nLength;
    int nTotalLen = 0;
    int nAttr = 0;

    pszStr[0] = 0;
    *pnCursor = 0;
    gcinPreeditAttr[0].flag = GCIN_PREEDIT_ATTR_FLAG_UNDERLINE;
    gcinPreeditAttr[0].ofs0 = 0;

    if (chewing_buffer_Len (g_pChewingCtx))
        nAttr = 1;

    for (nIdx = 0; nIdx < chewing_buffer_Len (g_pChewingCtx); nIdx++)
    {
        pszTmpStr = (char *)gtk_label_get_text (GTK_LABEL (g_pSeg[nIdx].label));
        nLength = g_gcinModMainFuncs.mf_utf8_str_N (pszTmpStr);
        nTotalLen += nLength;

        if (nIdx < chewing_cursor_Current (g_pChewingCtx))
            *pnCursor += nLength;

#if 0
        if (nIdx == chewing_cursor_Current (g_pChewingCtx))
        {
            gcinPreeditAttr[1].ofs0 = *pnCursor;
            gcinPreeditAttr[1].ofs1 = *pnCursor + nLength;
            gcinPreeditAttr[1].flag = GCIN_PREEDIT_ATTR_FLAG_REVERSE;
            nAttr++;
        }
#endif

        strcat (pszStr, pszTmpStr);
    }

    gcinPreeditAttr[0].ofs1 = nTotalLen;

    pCompFlag = 0;

    return nAttr;
}

gboolean
module_feedkey (int nKeyVal, int nKeyState)
{
    char *pszTmp         = NULL;
    char *pszChewingCand = NULL;
    int nZuinLen         = 0;
    char szWord[4];
    int nPhoIdx, nBufIdx;
    int nShiftModifier;
    int nTotalCandNum, nIdx;

    if (!g_pChewingCtx)
        return FALSE;

    nShiftModifier = (nKeyState & ShiftMask) > 0;

    memset (szWord, 0x00, 4);

    if (!g_gcinModMainFuncs.mf_tsin_pho_mode ())
        return FALSE;

    switch (nKeyVal)
    {
        case XK_space:
            chewing_handle_Space (g_pChewingCtx);
            break;

        case XK_Escape:
            chewing_handle_Esc (g_pChewingCtx);
            break;

        case XK_Return:
        case XK_KP_Enter:
            chewing_handle_Enter (g_pChewingCtx);
            break;

        case XK_Delete:
        case XK_KP_Delete:
            chewing_handle_Del (g_pChewingCtx);
            break;

        case XK_BackSpace:
            chewing_handle_Backspace (g_pChewingCtx);
            break;

        case XK_Up:
        case XK_KP_Up:
            chewing_handle_Up (g_pChewingCtx);
            break;

        case XK_Down:
        case XK_KP_Down:
            chewing_handle_Down (g_pChewingCtx);
            break;

        case XK_Left:
        case XK_KP_Left:
            chewing_handle_Left (g_pChewingCtx);
            break;

        case XK_Right:
        case XK_KP_Right:
            chewing_handle_Right (g_pChewingCtx);
            break;

#if 0
        case XK_Shift_L:
            chewing_handle_ShiftLeft (g_pChewingCtx);
            break;

        case XK_Shift_R:
            chewing_handle_ShiftRight (g_pChewingCtx);
            break;
#endif

        case XK_Tab:
            chewing_handle_Tab (g_pChewingCtx);
            break;

        default:
            if (nKeyVal > 32 && nKeyVal < 127)
                chewing_handle_Default (g_pChewingCtx, nKeyVal);
            break;
    }

    gcin_label_clear (MAX_SEG_NUM);
    g_nCurrentCursorPos = chewing_cursor_Current (g_pChewingCtx);

    if (g_nCurrentCursorPos < 0 || g_nCurrentCursorPos > MAX_SEG_NUM)
        return FALSE;

    // zuin symbols
    pszTmp = chewing_zuin_String (g_pChewingCtx, &nZuinLen);
    if (pszTmp)
    {
        for (nBufIdx = 0; nBufIdx < nZuinLen; nBufIdx++)
        {
            memcpy (szWord, pszTmp + nBufIdx * 3, 3);
            for (nPhoIdx = 0; nPhoIdx < 3; nPhoIdx++)
                if (strstr (g_gcinModMainFuncs.mf_pho_chars[nPhoIdx], szWord) != NULL)
                    gcin_label_show (szWord, nPhoIdx + chewing_buffer_Len (g_pChewingCtx) + 1);
        }
        free (pszTmp);
    }

    // check if the composing is valid or not
    if (chewing_buffer_Check (g_pChewingCtx))
    {
        g_gcinModMainFuncs.mf_hide_selections_win ();
        pszTmp = chewing_buffer_String (g_pChewingCtx);

        // init cand_no
        chewing_cand_Enumerate (g_pChewingCtx);

        g_gcinModMainFuncs.mf_clear_sele ();

        if (chewing_cand_TotalChoice (g_pChewingCtx))
        {
            nTotalCandNum = nIdx = 0;
            while (chewing_cand_hasNext (g_pChewingCtx))
            {
                pszChewingCand = chewing_cand_String (g_pChewingCtx);

                if (nIdx > chewing_get_candPerPage (g_pChewingCtx) - 1)
                    break;
                gcin_label_cand_show (pszChewingCand, nIdx++);
                free (pszChewingCand);
            }
        }

        for (nIdx = 0; nIdx < chewing_buffer_Len (g_pChewingCtx); nIdx++)
        {
            memcpy (szWord, pszTmp + (nIdx * 3), 3);
            gcin_label_show (szWord, nIdx);
        }

        free (pszTmp);
    }

    if (chewing_commit_Check (g_pChewingCtx))
    {
        pszTmp = chewing_commit_String (g_pChewingCtx);
        g_gcinModMainFuncs.mf_send_text (pszTmp);

        // FIXME: workaround for repeated commit
        //        it impacts the bEscCleanAllBuf setting!
        chewing_handle_Esc (g_pChewingCtx);
        free (pszTmp);
    }

    module_show_win ();

    return TRUE;
}

// FIXME: impl
int
module_feedkey_release (KeySym xkey, int nKbState)
{
    return 0;
}

void
module_move_win (int nX, int nY)
{
    gtk_window_get_size (GTK_WINDOW (g_pWinChewing),
                         g_gcinModMainFuncs.mf_win_xl,
                         g_gcinModMainFuncs.mf_win_yl);

    if (nX + *g_gcinModMainFuncs.mf_win_xl > *g_gcinModMainFuncs.mf_dpy_xl)
        nX = *g_gcinModMainFuncs.mf_dpy_xl - *g_gcinModMainFuncs.mf_win_xl;
    if (nX < 0)
        nX = 0;

    if (nY + *g_gcinModMainFuncs.mf_win_yl > *g_gcinModMainFuncs.mf_dpy_yl)
        nY = *g_gcinModMainFuncs.mf_dpy_yl - *g_gcinModMainFuncs.mf_win_yl;
    if (nY < 0)
        nY = 0;

    gtk_window_move (GTK_WINDOW(g_pWinChewing), nX, nY);

    *g_gcinModMainFuncs.mf_win_x = nX;
    *g_gcinModMainFuncs.mf_win_y = nY;

    g_gcinModMainFuncs.mf_move_win_sym ();
}

void
module_change_font_size (void)
{
    GdkColor colorFG;
    GtkWidget *pLabel;
    int n;

    gdk_color_parse (*g_gcinModMainFuncs.mf_gcin_win_color_fg, &colorFG);
    g_gcinModMainFuncs.mf_change_win_bg (g_pWinChewing);
    g_gcinModMainFuncs.mf_change_win_bg (g_pEvBoxChewing);

    for (n = 0; n < MAX_SEG_NUM; n++)
    {
        pLabel = g_pSeg[n].label;
        set_label_font_size (pLabel, *g_gcinModMainFuncs.mf_gcin_font_size);

        if (*g_gcinModMainFuncs.mf_gcin_win_color_use)
            gtk_widget_modify_fg (pLabel, GTK_STATE_NORMAL, &colorFG);
    }
}

void
module_show_win (void)
{
    if (g_gcinModMainFuncs.mf_gcin_edit_display_ap_only ())
        return;

    gtk_window_resize (GTK_WINDOW (g_pWinChewing),
                       32 * (chewing_buffer_Check (g_pChewingCtx) + 1),
                       12);
    gtk_widget_show (g_pWinChewing);

    g_gcinModMainFuncs.mf_show_win_sym ();
}

void
module_hide_win (void)
{
    gtk_widget_hide (g_pWinChewing);
    g_gcinModMainFuncs.mf_hide_selections_win ();
    g_gcinModMainFuncs.mf_hide_win_sym ();
}

// FIXME: chk
int
module_win_visible (void)
{
    return GTK_WIDGET_VISIBLE (g_pWinChewing);
}

void
module_win_geom (void)
{
    if (!g_pWinChewing)
        return;

    gtk_window_get_position(GTK_WINDOW(g_pWinChewing),
                            g_gcinModMainFuncs.mf_win_x,
                            g_gcinModMainFuncs.mf_win_y);

    g_gcinModMainFuncs.mf_get_win_size(g_pWinChewing,
                                       g_gcinModMainFuncs.mf_win_xl,
                                       g_gcinModMainFuncs.mf_win_yl);
}

int
module_flush_input (void)
{
    char *pszTmp;

    if (chewing_commit_Check (g_pChewingCtx))
    {
        pszTmp = chewing_commit_String (g_pChewingCtx);
        g_gcinModMainFuncs.mf_send_text (pszTmp);
        free (pszTmp);
    }

    chewing_Reset (g_pChewingCtx);
    return 0;
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
                  O_RDONLY,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    free (pszChewingConfig);

    return (g_nFd == -1 ? !(g_bUseDefault = TRUE) : TRUE);
}

static void
chewing_config_load (void)
{
    int nReadSize;

    nReadSize = read (g_nFd, &g_chewingConfig, sizeof (g_chewingConfig));
    if (nReadSize == 0 || nReadSize != sizeof (g_chewingConfig))
        g_bUseDefault = TRUE;
}

// TODO: add kbtype into gcin_conf.dat
//       or use the gcin setting instead
static void
chewing_config_set (void)
{
    if (g_bUseDefault)
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

    chewing_set_KBType (g_pChewingCtx, chewing_KBStr2Num ("KB_DEFAULT"));
    chewing_set_selKey (g_pChewingCtx, g_chewingConfig.selKey, 10);
    chewing_set_candPerPage (g_pChewingCtx, g_chewingConfig.candPerPage);
    chewing_set_maxChiSymbolLen (g_pChewingCtx, g_chewingConfig.maxChiSymbolLen);
    chewing_set_addPhraseDirection (g_pChewingCtx, g_chewingConfig.bAddPhraseForward);
    chewing_set_spaceAsSelection (g_pChewingCtx, g_chewingConfig.bSpaceAsSelection);
    chewing_set_escCleanAllBuf (g_pChewingCtx, g_chewingConfig.bEscCleanAllBuf);
    chewing_set_autoShiftCur (g_pChewingCtx, g_chewingConfig.bAutoShiftCur);
    chewing_set_easySymbolInput (g_pChewingCtx, g_chewingConfig.bEasySymbolInput);
    chewing_set_phraseChoiceRearward (g_pChewingCtx, g_chewingConfig.bPhraseChoiceRearward);
    chewing_set_hsuSelKeyType (g_pChewingCtx, g_chewingConfig.hsuSelKeyType);
}

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

void
chewing_config_close (void)
{
    if (g_nFd != -1)
        close (g_nFd);
}

