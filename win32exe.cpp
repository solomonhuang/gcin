#include "os-dep.h"
#include <shellapi.h>
#include <stdio.h>
#include <shlobj.h>
#include "gcin.h"

char *gcin_program_files_path;
char *gcin_script_path;
bool get_reg_str(char *reg, char *out, DWORD outsz);

void init_gcin_program_files()
{
  char path[MAX_PATH];

  if (!get_reg_str("gcin_dir", path, MAX_PATH)) {
	SHGetFolderPathA(NULL, CSIDL_PROGRAM_FILES , NULL, SHGFP_TYPE_CURRENT, path);
	strcat(path,"\\gcin");
  }

  char tt[MAX_PATH];
  sprintf_s(tt, sizeof(tt), "GCIN_DIR=%s", path);
  _putenv(tt);

  gcin_program_files_path = strdup(path);
  strcat(path, "\\script");
  gcin_script_path = strdup(path);
  dbg("init_gcin_program_files script:%s\n", gcin_script_path);
}

void win32exec_para(char *s, char *para)
{
  char path[MAX_PATH];
  if (!gcin_program_files_path)
    init_gcin_program_files();

  sprintf_s(path, sizeof(path), "%s\\bin\\%s", gcin_program_files_path, s);

  dbg("win32exec_para %s %s\n", path, para);

  ShellExecuteA(NULL, "open", path, para, NULL, SW_SHOWNORMAL);
}


void win32exec(char *s)
{
  win32exec_para(s, NULL);
}

DWORD win32exec_create(char *s)
{
  PROCESS_INFORMATION procinfo;
  STARTUPINFOA si;

  char path[MAX_PATH];
  if (!gcin_program_files_path)
    init_gcin_program_files();

  sprintf_s(path, sizeof(path), "\"%s\\bin\\%s\"", gcin_program_files_path, s);

  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  ZeroMemory( &procinfo, sizeof(procinfo));

  if(!CreateProcessA(NULL, path, NULL,NULL,FALSE, CREATE_NO_WINDOW, NULL,NULL, &si, &procinfo)) {
#if _DEBUG
    dbg("cannot exec %s\n", path);
	return -1;
#endif
  }

  return procinfo.dwThreadId;
}

int win32exec_script(char *s, char *para)
{
  if (!gcin_program_files_path)
	init_gcin_program_files();

#if 0
  char tt[256];
  sprintf(tt, "%s\\%s", gcin_script_path, s);
  dbg("win32exec_script %s\n", tt);
  int r = (int)ShellExecuteA(NULL, "open", tt, para, NULL, SW_SHOWNORMAL);

  if (r > 32)
    return 0;
  else {
    dbg("win32exec_script %s erro -> %d\n", tt, r);
    return -1;
  }
#else
char cmd[256];
#if 0
  if (para)
    sprintf_s(cmd, sizeof(cmd), "%s\\cmd.exe \"%s\"\\%s %s", sdir, gcin_script_path, s, para);
  else
    sprintf_s(cmd, sizeof(cmd), "%s\\cmd.exe \"%s\\%s\"", sdir, gcin_script_path, s);
#else
  if (para)
    sprintf_s(cmd, sizeof(cmd), "\"%s\\%s\" \"%s\"", gcin_script_path, s, para);
  else
    sprintf_s(cmd, sizeof(cmd), "\"%s\\%s\"", gcin_script_path, s);
#endif

  dbg("cmd %s\n", cmd);

  PROCESS_INFORMATION procinfo;
  STARTUPINFOA si;

  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  ZeroMemory( &procinfo, sizeof(procinfo));

  if(!CreateProcessA(NULL, cmd, NULL,NULL,FALSE, CREATE_NO_WINDOW, NULL,NULL, &si, &procinfo)) {
#if _DEBUG
    dbg("cannot exec %s\n", cmd);
	return -1;
#endif
  }

  return 0;
#endif
}
