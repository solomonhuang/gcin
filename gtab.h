#define MAX_CIN_PHR (40)

typedef struct {
  u_char key[4];   /* If I use u_long key, the struc size will be 8 */
  u_char ch[CH_SZ];
} ITEM;

typedef struct {
  u_char quick1[46][10][CH_SZ];
} QUICK_KEYS;

struct TableHead {
  char ename[16];         /* ascii name */
  char cname[16];         /* prompt */
  char selkey[12];        /* select keys */
  char endian;
  int KeyS;               /* number of keys needed */
  int MaxPress;           /* Max len of keystroke  ar30:4  changjei:5 */
  int M_DUP_SEL;          /* how many keys used to select */
  int DefC;               /* Defined characters */
  QUICK_KEYS qkeys;
};

u_long CONVT(char *s);
#define KeyBits (6)

#define MAX_GTAB_NUM_KEY 9
