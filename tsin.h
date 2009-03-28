extern int phcount;
extern int hashidx[];
extern int *phidx;
extern FILE *fph;

extern int *ts_gtab;   // number of array is phcount

typedef struct {
  phokey_t pho;
  char ch[CH_SZ];
  char och[CH_SZ];
  char ph1ch[CH_SZ]; // char selected by 1st pho
  u_char flag;
  char psta; // phrase start index
} CHPHO;

enum {
  FLAG_CHPHO_FIXED=1,    // user selected the char, so it should not be changed
  FLAG_CHPHO_PHRASE_HEAD=2,
  FLAG_CHPHO_PHRASE_VOID=4
};

extern CHPHO chpho[MAX_PH_BF_EXT];
extern int c_idx, c_len, ph_sta, ph_sta_last;

void extract_pho(int chpho_idx, int plen, phokey_t *pho);
gboolean tsin_seek(phokey_t *pho, int plen, int *r_sti, int *r_edi);
void load_tsin_entry(int idx, u_char *len, usecount_t *usecount, phokey_t *pho, u_char *ch);
gboolean check_fixed_mismatch(int chpho_idx, char *mtch, int plen);

typedef struct {
  char len, flag;
  u_char start;
  char str[MAX_PHRASE_LEN * CH_SZ + 1];
} TSIN_PARSE;

enum {
  FLAG_TSIN_PARSE_PHRASE = 1,
  FLAG_TSIN_PARSE_PARTIAL = 2, //partial phrase
};

void tsin_parse(TSIN_PARSE out[]);
