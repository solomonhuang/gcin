/*
	Copyright (C) 1995	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/
typedef u_short phokey_t;

typedef struct {
  char selkey[12];
  struct {
    char num, typ;
  } phokbm[128][3];  // for 26 keys pho, it may have up-to 3 pho char for a key
} PHOKBM;

extern PHOKBM phkbm;

typedef struct {
  u_char ch[CH_SZ];
  u_short count;
} PHO_ITEM;

typedef struct {
  phokey_t key;
  u_short start;
} PHO_IDX;

extern int start_idx, stop_idx;

#define MAX_PHRASE_LEN (16)
#define MAX_PHRASE_STR_LEN (MAX_PHRASE_LEN * CH_SZ + 1)
#define SELKEY (9)

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
int big5_pho_chars(char *big5, phokey_t *phkeys);

#define MAX_PH_BF (90)

#define MAX_PH_BF_EXT (MAX_PH_BF + MAX_PHRASE_LEN + 1)


#define TSIN_HASH_SHIFT 6
