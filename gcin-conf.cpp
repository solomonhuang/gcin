#include "os-dep.h"
#include "gcin.h"
#if UNIX
#include <dirent.h>
#include <X11/Xatom.h>
#else
#include <shellapi.h>
#include <shlobj.h>
#include <io.h>
#endif

#if !CLIENT_LIB
#if WIN32
char *TableDir;
void init_gcin_program_files();
#else
char *TableDir=GCIN_TABLE_DIR;
#endif

void init_TableDir()
{
  char *dname;
  if ((dname=getenv("GCIN_TABLE_DIR"))) {
    TableDir = dname;
    return;
  }

#if WIN32
  char tt[MAX_PATH];
  init_gcin_program_files();

  sprintf_s(tt, sizeof(tt), "%s\\table", gcin_program_files_path);
  TableDir=strdup(tt);
  dbg("%s\n", TableDir);
#endif
}


void get_gcin_dir(char *tt)
{
#if WIN32
  SHGetFolderPathA(NULL,CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, tt);
  strcat(tt,"\\gcin");
#else
    strcpy(tt,(char *)getenv("HOME"));
    strcat(tt,"/.gcin");
#endif
}


gboolean get_gcin_user_fname(char *name, char fname[])
{
  get_gcin_dir(fname);
#if UNIX
  strcat(strcat(fname,"/"),name);
  return access(fname, R_OK)==0;
#else
  strcat(strcat(fname,"\\"),name);
  return (_access(fname, 04) >=0);
#endif
//  dbg("get_gcin_user_fname %s %s\n", name, fname);
}

void get_gcin_conf_fname(char *name, char fname[])
{
  get_gcin_dir(fname);
#if UNIX
  strcat(strcat(fname,"/config/"),name);
#else
  strcat(strcat(fname,"\\config\\"),name);
#endif
}

void get_gcin_user_or_sys_fname(char *name, char fname[])
{
  if (!getenv("GCIN_TABLE_DIR")) {
    if (get_gcin_user_fname(name, fname))
      return;
  }

  get_sys_table_file_name(name, fname);
}

void get_gcin_conf_str(char *name, char **rstr, char *default_str)
{
  char fname[MAX_GCIN_STR];
  char out[256];

  if (*rstr)
    free(*rstr);

  get_gcin_conf_fname(name, fname);

  FILE *fp;

  if ((fp=fopen(fname, "rb")) == NULL) {
    *rstr = strdup(default_str);
    return;
  }

  myfgets(out, sizeof(out), fp);
  int len = strlen(out);
  if (len && out[len-1]=='\n')
    out[len-1] = 0;

  fclose(fp);

  *rstr = strdup(out);
}

void get_gcin_conf_fstr(char *name, char rstr[], char *default_str)
{
  char *tt = NULL;
  get_gcin_conf_str(name, &tt, default_str);
  strcpy(rstr, tt);
  free(tt);
}

int get_gcin_conf_int(char *name, int default_value)
{
  char tt[32];
  char default_value_str[MAX_GCIN_STR];

  sprintf(default_value_str, "%d", default_value);
  get_gcin_conf_fstr(name, tt, default_value_str);

  return atoi(tt);
}


void save_gcin_conf_str(char *name, char *str)
{
  FILE *fp;
  char fname[256];

  get_gcin_conf_fname(name, fname);

  if ((fp=fopen(fname,"wb"))==NULL) {
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

void get_sys_table_file_name(char *name, char *fname)
{
#if UNIX
  sprintf(fname, "%s/%s", TableDir, name);
#else
  sprintf(fname, "%s\\%s", TableDir, name);
#endif
}
#endif

#if UNIX
char *get_gcin_xim_name()
{
  char *xim_name;

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
#endif
