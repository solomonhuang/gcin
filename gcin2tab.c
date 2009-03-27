
/*
	Copyright (C) 1995	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <string.h>
#include "gcin.h"
#include "gtab.h"


FILE *fr, *fw;
int lineno;


char *skip_spc(char *s)
{
	while ((*s==' ' || *s=='\t') && *s) s++;
	return s;
}

char *to_spc(char *s)
{
	while (*s!=' ' && *s!='\t' && *s) s++;
	return s;
}

void del_nl_spc(char *s)
{
  char *t;

  int len=strlen(s);
  if (!*s) return;

  t=s+len-1;

  while (*t=='\n' || *t==' ' || *t=='\t' && t > s)
    t--;

  *(t+1)=0;
}


void get_line(u_char *tt)
{
  while (!feof(fr)) {
    fgets(tt,128,fr);
    lineno++;

    if (tt[0]=='#')
      continue;
    else
      break;
  }
}

void cmd_arg(u_char *s, u_char **cmd, u_char **arg)
{
  char *t;

  get_line(s);

  if (!*s) {
    *cmd=*arg=s;
    return;
  }

  s=skip_spc(s);
  t=to_spc(s);
  *cmd=s;
  if (!(*t)) {
    *arg=t;
    return;
  }

  *t=0;
  t++;

  t=skip_spc(t);
  del_nl_spc(t);

  *arg=t;
}

int sequ(char *s, char *t)
{
  return (!strcmp(s,t));
}

typedef struct {
	u_long key;
	u_char ch[CH_SZ];
	u_short oseq;
} ITEM2;

#define MAX_K (40000)
ITEM2 itar[MAX_K], itmp[MAX_K];
ITEM itout[MAX_K], lastit;

int qcmp2(const void *aa, const void *bb)
{
  ITEM2 *a = (ITEM2 *)aa, *b = (ITEM2 *) bb;

  char tt[CH_SZ+1];
  if (a->key > b->key) return 1;
  if (a->key < b->key) return -1;
  return memcmp(a->ch ,b->ch, CH_SZ);
}

int qcmp(const void *aa, const void *bb)
{
  ITEM2 *a = (ITEM2 *)aa, *b = (ITEM2 *) bb;

  if (a->key > b->key) return 1;
  if (a->key < b->key) return -1;
  return (int)a->oseq - (int)b->oseq;
}


#define mtolower(ch) (ch>='A'&&ch<='Z'?ch+0x20:ch)

char kno[128];

main(int argc, char **argv)
{
  int i;
  char fname[64];
  char fname_cin[64];
  char fname_tab[64];
  char fname_sel1st[64];
  char tt[128];
  u_char *cmd, *arg;
  struct TableHead th;
  int KeyNum;
  char kname[128][CH_SZ];
  char keymap[64];
  int chno,cpcount;
  u_short idx1[256], last_ser;
  char def1[256];
  int quick_def;
  int sel1st_def;
  int phridx[12000], phr_cou=0;
  char phrbuf[32768];
  int prbf_cou=0;

  dbg("-- gcin2tab encoding UTF-8 --\n");
  dbg("--- please use iconv -f big5 -t utf-8 if your file is in big5 encoding\n");

  if (argc<=1) {
          printf("Enter table file name [.cin] : ");
          scanf("%s", fname);
  } else strcpy(fname,argv[1]);


  if (!strcmp(fname, "-v") || !strcmp(fname, "--version")) {
    dbg("gcin2tab for gcin " GCIN_VERSION "\n");
    exit(0);
  }


  char *p;

  if(p=strchr(fname, '.'))
    *p = 0;

  strcpy(fname_cin,fname);
  strcpy(fname_tab,fname);
  strcat(fname_cin,".cin");
  strcat(fname_tab,".gtab");
  strcpy(fname_sel1st,fname_tab);
  strcat(fname_sel1st,".sel1st");

  if ((fr=fopen(fname_cin,"r"))==NULL)
          p_err("Cannot open %s\n", fname_cin);
  bzero(&th,sizeof(th));
  bzero(kno,sizeof(kno));
  bzero(keymap,sizeof(keymap));
  bzero(itar,sizeof(itar));
  bzero(itout,sizeof(itout));

  cmd_arg(tt, &cmd, &arg);
  if (sequ(cmd, "%gen_inp\n")) {
    printf("skip gen_inp\n");
    cmd_arg(tt, &cmd, &arg);
  }

  if (!sequ(cmd,"%ename") || !(*arg) )
    p_err("%d:  %%ename english_name  expected", lineno);
  arg[15]=0;
  strcpy(th.ename,arg);

  cmd_arg(tt, &cmd, &arg);
  if (!(sequ(cmd,"%prompt") || sequ(cmd,"%cname")) || !(*arg) )
    p_err("%d:  %%prompt prompt_name  expected", lineno);
  strcpy(th.cname, arg);
  printf("cname %s\n", th.cname);

  cmd_arg(tt,&cmd, &arg);
  if (!sequ(cmd,"%selkey") || !(*arg) )
    p_err("%d:  %%selkey select_key_list expected", lineno);
  strcpy(th.selkey,arg);

  cmd_arg(tt,&cmd, &arg);
  if (!sequ(cmd,"%dupsel") || !(*arg) ) {
    th.M_DUP_SEL = 10;
  }
  else {
    th.M_DUP_SEL=atoi(arg);
    cmd_arg(tt,&cmd, &arg);
  }

  if (!sequ(cmd,"%keyname") || !sequ(arg,"begin"))
    p_err("%d:  %%keyname begin   expected", lineno);

  for(KeyNum=0;;) {
    char k;

    cmd_arg(tt,&cmd, &arg);
    if (sequ(cmd,"%keyname")) break;
    k=mtolower(cmd[0]);
    if (kno[k])
      p_err("%d:  key %c is already used",lineno, k);

    kno[k]=++KeyNum;
    keymap[KeyNum]=k;
    bchcpy(&kname[KeyNum][0], arg);
  }

  keymap[0]=kname[0][0]=kname[0][1]=' ';
  KeyNum++;
  th.KeyS=KeyNum;    /* include space */

  cmd_arg(tt,&cmd, &arg);
  if (sequ(cmd,"%quick") && sequ(arg,"begin")) {
    dbg(".. quick keys defined\n");
    for(quick_def=0;;) {
      char k;
      int len;

      cmd_arg(tt,&cmd, &arg);
      if (sequ(cmd,"%quick")) break;
      k=kno[mtolower(cmd[0])]-1;
      len=strlen(arg);

      for(i=0; i<len; i+=CH_SZ)
        bchcpy(th.qkeys.quick1[k][i/CH_SZ], &arg[i]);

      quick_def++;
    }

//    cmd_arg(tt,&cmd, &arg);
  }

  puts("char def");
  chno=0;
  while (!feof(fr)) {
    int len;
    u_long kk;
    int k;

    cmd_arg(tt,&cmd,&arg);
    if (!cmd[0] || !arg[0])
      continue;
    if (cmd[0]=='%')
      continue;

    len=strlen(cmd);

    if (len > th.MaxPress)
      th.MaxPress=len;

    if (len > 5)
      p_err("%d:  only <= 5 keys is allowed", lineno);

    kk=0;
    for(i=0;i<len;i++) {
      k=kno[mtolower(cmd[i])];
      kk|=k<<(24-i*6);
    }

    memcpy(&itar[chno].key, &kk, 4);

    if ((len=strlen(arg)) <= CH_SZ || !(arg[0] & 0x80)) {
      char out[CH_SZ+1];

      bzero(out, sizeof(out));
      memcpy(out, arg, len);
      bchcpy(itar[chno].ch, out);
//      printf("uuu %x %c%c\n", kk, out[0], out[1]);
    } else {
      itar[chno].ch[0]=phr_cou>>8;
      itar[chno].ch[1]=phr_cou&0xff;
      phridx[phr_cou++]=prbf_cou;
      strcpy(&phrbuf[prbf_cou],arg);
//      printf("phrase:%d  len:%d'%s'\n", phr_cou, len, arg);
      prbf_cou+=len;
    }

    itar[chno].oseq=chno;
    chno++;
  }
  fclose(fr);


  qsort(itar, chno,sizeof(ITEM2), qcmp2);

  bzero(&lastit,sizeof(ITEM));
  cpcount=0;

  for(i=0;i<chno;i++) {
    if (memcmp(&itar[i], &lastit, sizeof(ITEM))) {
      memcpy(&itmp[cpcount++],&itar[i],sizeof(ITEM2));
      memcpy(&lastit,&itar[i],sizeof(ITEM));
    }
  }

  chno=cpcount;
  th.DefC=chno;
  qsort(itmp,chno,sizeof(ITEM2),qcmp);

  for(i=0;i<chno;i++) {
    memcpy(&itout[i],&itmp[i],sizeof(ITEM));
  }

  bzero(def1,sizeof(def1));
  bzero(idx1,sizeof(idx1));
  for(i=0;i<chno;i++) {
    char tt[3];
    int kk=(CONVT(itout[i].key)>>24) & 0x3f;
    if (!def1[kk]) {
      idx1[kk]=(u_short)i;
      def1[kk]=1;
    }
  }

  idx1[KeyNum]=chno;
  for(i=KeyNum-1;i>0;i--)
          if (!def1[i]) idx1[i]=idx1[i+1];

  if ((fw=fopen(fname_tab,"w"))==NULL) {
    p_err("Cannot create");
  }

  printf("Defined Char:%d\n", chno);
  fwrite(&th,1,sizeof(th),fw);
  fwrite(keymap, 1, KeyNum, fw);
  fwrite(kname, CH_SZ, KeyNum, fw);
  fwrite(idx1,2, KeyNum+1,fw);
  fwrite(itout,sizeof(ITEM),chno, fw);

  if (phr_cou) {
    phridx[phr_cou++]=prbf_cou;
    printf("phrase count:%d\n", phr_cou);
    fwrite(&phr_cou, sizeof(int), 1, fw);
    fwrite(phridx, sizeof(int), phr_cou, fw);
    fwrite(phrbuf,1,prbf_cou,fw);
  }

  fclose(fw);
}
