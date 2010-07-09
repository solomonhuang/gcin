extern int phcount;
extern int hashidx[];
//extern int *phidx;
//extern FILE *fph;

typedef struct CHPHO {
  phokey_t pho;
  char ch[CH_SZ];
//  char och[CH_SZ];
  u_char flag;
  char psta; // phrase start index
} CHPHO;

enum {
  FLAG_CHPHO_FIXED=1,    // user selected the char, so it should not be changed
  FLAG_CHPHO_PHRASE_HEAD=2,
  FLAG_CHPHO_PHRASE_USER_HEAD=4,
  FLAG_CHPHO_PHRASE_VOID=8,
  FLAG_CHPHO_PHRASE_BODY=16,
};

void extract_pho(int chpho_idx, int plen, phokey_t *pho);
gboolean tsin_seek(void *pho, int plen, int *r_sti, int *r_edi);
void load_tsin_entry(int idx, char *len, usecount_t *usecount, void *pho, u_char *ch);
gboolean check_fixed_mismatch(int chpho_idx, char *mtch, int plen);
gboolean tsin_pho_mode();

#define TSIN_GTAB_KEY "!!!!gtab-keys"

typedef struct {
  char signature[32];
  int version, flag;
  int keybits, maxkey;
  char keymap[128];
} TSIN_GTAB_HEAD;

extern gboolean tsin_is_gtab;
