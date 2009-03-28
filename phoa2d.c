#include "gcin.h"
#include "pho.h"

#define MAX_CHS (35000)

typedef struct {
  u_short key;
  u_char ch[CH_SZ];
  short count;
  int oseq;
} PHITEM;

PHITEM items[MAX_CHS];
int itemsN;

PHO_ITEM pho_items[MAX_CHS];
int pho_itemsN=0;


void p_err(char *fmt,...)
{
  va_list args;

  va_start(args, fmt);
  fprintf(stderr,"gcin:");
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr,"\n");
  exit(-1);
}

void dbg(char *fmt,...)
{
  va_list args;

  va_start(args, fmt);
  vprintf(fmt, args);
  fflush(stdout);
  va_end(args);
}


int qcmp_key(const void *aa, const void *bb)
{
  PHITEM *a=(PHITEM *)aa;
  PHITEM *b=(PHITEM *)bb;

  int d;
  if ((d=a->key - b->key))
    return a->key - b->key;

  if ((d = b->count - a->count))
    return d;

  return a->oseq - b->oseq;
}


void send_gcin_message(Display *dpy, char *s);

int main(int argc, char **argv)
{
  char *fname = "pho.tab.src";
  FILE *fp;
  char s[64];
  gboolean reload = getenv("GCIN_NO_RELOAD")==NULL;

  if (reload)
    gtk_init(&argc, &argv);

  if (argc > 1)
    fname = argv[1];

  if ((fp=fopen(fname,"r"))==NULL)
    p_err("cannot open %s\n", fname);


  while (!feof(fp)) {
    s[0]=0;
    fgets(s,sizeof(s),fp);
    int len=strlen(s);

    if (s[len-1]=='\n')
      s[--len]=0;

    if (len==0)
      continue;

    phokey_t kk=0;
    u_char *p = s;

    while (*p && *p!=' ') {
      kk |= lookup(p);

      p += utf8_sz(p);
    }

    items[itemsN].key = kk;

    p++;

    int u8len = u8cpy(items[itemsN].ch, p);

    p+= u8len + 1; // skip ch & space

    items[itemsN].count = atoi(p);
    items[itemsN].oseq = itemsN;

    itemsN++;
  }

  fclose(fp);

  qsort(items, itemsN, sizeof(PHITEM), qcmp_key);

  int i;

  PHO_IDX pho_idx[3000];
  u_short pho_idxN=0;

  for(i=0; i < itemsN; ) {
    phokey_t key = items[i].key;
    pho_idx[pho_idxN].key = key;
    pho_idx[pho_idxN].start = i;
    pho_idxN++;

    int j;

    for (j=i+1; j < itemsN && items[j].key == key; j++);

    int l;
    for(l=i; l<j; l++) {
      bchcpy(pho_items[pho_itemsN].ch, items[l].ch);
      pho_items[pho_itemsN].count = items[l].count;
      pho_itemsN++;
    }

    i = j;
  }

  char *tp = strstr(fname, ".src");
  if (!tp)
    p_err("file name should be *.tab.src");

  *tp=0;

  char *fname_out = fname;

  if ((fp=fopen(fname_out,"w"))==NULL)
    p_err("cannot create %s\n", fname_out);

  fwrite("PH",1,2,fp);
//  dbg("pho_itemsN:%d  pho_idxN:%d\n", pho_itemsN, pho_idxN);
  fwrite(&pho_idxN, sizeof(u_short), 1, fp);
#if 0
  fclose(fp); exit(0);
#endif
  fwrite(pho_idx, sizeof(PHO_IDX), pho_idxN, fp);
  fwrite(pho_items, sizeof(PHO_ITEM), pho_itemsN, fp);

  fclose(fp);

  if (reload)
    send_gcin_message(GDK_DISPLAY(), "reload");

  return 0;
}
