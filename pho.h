/*
	Copyright (C) 1995	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/
typedef u_short phokey_t;

typedef struct {
  char selkey[12];
  int selkeyN;
  struct {
    char num, typ;
  } phokbm[128][3];  // for 26 keys pho, it may have up-to 3 pho char for a key
} PHOKBM;

extern PHOKBM phkbm;

typedef struct {
  u_char ch[CH_SZ];
  short count;
} PHO_ITEM;

typedef struct {
  phokey_t key;
  u_short start;
} PHO_IDX;

extern int start_idx, stop_idx;

#define MAX_PHRASE_LEN (16)
#define MAX_PHRASE_STR_LEN (MAX_PHRASE_LEN * CH_SZ + 1)

#define Min(a,b) ((a) < (b) ? (a):(b))

#define TSIN_HASH_N (256)

extern char phofname[128];
extern u_short idxnum_pho;
extern PHO_IDX idx_pho[];
extern int ch_pho_ofs;
extern PHO_ITEM *ch_pho;
extern int ch_phoN;

void pho_load();
extern char *pho_chars[];
char *phokey_to_str(phokey_t kk);
int utf8_pho_keys(char *big5, phokey_t *phkeys);
void prph(phokey_t kk);
phokey_t pho2key(char typ_pho[]);
gboolean save_phrase_to_db(phokey_t *phkeys, char *utf8str, int len, int usecount);
int lookup(u_char *s);

#define MAX_PH_BF (90)

#define MAX_PH_BF_EXT (MAX_PH_BF + MAX_PHRASE_LEN + 1)


#define TSIN_HASH_SHIFT 6

