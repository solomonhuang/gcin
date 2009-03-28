#include "gcin.h"
#include <dirent.h>
#include <X11/Xatom.h>

char *TableDir=GCIN_TABLE_DIR;

void init_TableDir()
{
  char *dname;

  if ((dname=getenv("GCIN_TABLE_DIR"))) {
    TableDir = dname;
  }
}


void get_gcin_dir(char *tt)
{
    strcpy(tt,(char *)getenv("HOME"));
    strcat(tt,"/.gcin");
}


void get_gcin_user_fname(char *name, char fname[])
{
  get_gcin_dir(fname);
  strcat(strcat(fname,"/"),name);
}

void get_gcin_conf_fname(char *name, char fname[])
{
  get_gcin_dir(fname);
  strcat(strcat(fname,"/config/"),name);
}

void get_gcin_conf_str(char *name, char rstr[], char *default_str)
{
  char fname[MAX_GCIN_STR];

  get_gcin_conf_fname(name, fname);

  FILE *fp;

  if ((fp=fopen(fname, "r")) == NULL) {
    strcpy(rstr, default_str);
    return;
  }

  fgets(rstr, MAX_GCIN_STR, fp);

  int len = strlen(rstr);

  if (len && rstr[len-1]=='\n')
    rstr[len-1] = 0;

  fclose(fp);
}


int get_gcin_conf_int(char *name, int default_value)
{
  char tt[MAX_GCIN_STR];
  char default_value_str[MAX_GCIN_STR];

  sprintf(default_value_str, "%d", default_value);
  get_gcin_conf_str(name, tt, default_value_str);

  return atoi(tt);
}


void save_gcin_conf_str(char *name, char *str)
{
  FILE *fp;
  char fname[256];

  get_gcin_conf_fname(name, fname);

  if ((fp=fopen(fname,"w"))==NULL) {
    p_err("cannot create %s", fname);
  }

  fprintf(fp, "%s", str);
  fclose(fp);
}


void save_gcin_conf_int(char *name, int val)
{
  char tt[16];

  sprintf(tt, "%d", val);
  save_gcin_conf_str(name, tt);
}


char *get_gcin_xim_name()
{
  char *xim_name;

  if ((xim_name=getenv("GCIN_XIM"))) {
//    dbg("GCIN_XIM is set with %s\n", xim_name);
    return xim_name;
  }
  if ((xim_name=getenv("XMODIFIERS"))) {
    static char find[] = "@im=";
    static char sstr[32];
    char *p = strstr(xim_name, find);

    p += strlen(find);
    strncpy(sstr, p, sizeof(sstr));
    sstr[sizeof(sstr) - 1]=0;

    if ((p=strchr(sstr, '.')))
      *p=0;

//    dbg("Try to use name from XMODIFIERS=@im=%s\n", sstr);
    return sstr;
  }

  return "gcin";
}

Atom get_gcin_atom(Display *dpy)
{
  char *xim_name = get_gcin_xim_name();
  char tt[128];

  snprintf(tt, sizeof(tt), "GCIN_ATOM_%s", xim_name);

  Atom atom = XInternAtom(dpy, tt, False);

  return atom;
}


void get_sys_table_file_name(char *name, char *fname)
{
  sprintf(fname, "%s/%s", TableDir, name);
}
