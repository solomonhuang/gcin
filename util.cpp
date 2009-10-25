#include "gcin.h"


#if UNIX
static FILE *out_fp;

void p_err(char *fmt,...)
{
  va_list args;

  va_start(args, fmt);
  fprintf(stderr,"gcin:");
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr,"\n");
#if DEBUG
  abort();
#else
  if (getenv("GCIN_ERR_COREDUMP"))
    abort();

  exit(-1);
#endif
}

static void init_out_fp()
{
  if (!out_fp) {
    if (getenv("GCIN_DBG_TMP") || 0) {
      char fname[64];
      sprintf(fname, "/tmp/gcindbg-%d-%d", getuid(), getpid());
      out_fp = fopen(fname, "w");
    }

    if (!out_fp)
      out_fp = stdout;
  }
}

#if !CLIENT_LIB
void dbg_time(char *fmt,...)
{
  va_list args;
  time_t t;

  init_out_fp();

  time(&t);
  struct tm *ltime = localtime(&t);
  dbg("%02d:%02d:%02d ", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);

  va_start(args, fmt);
  vfprintf(out_fp, fmt, args);
  fflush(out_fp);
  va_end(args);
}
#endif

void dbg(char *fmt,...)
{
  va_list args;

  init_out_fp();

  va_start(args, fmt);
  vfprintf(out_fp, fmt, args);
  fflush(out_fp);
  va_end(args);
}

#else
#include <share.h>
#include <io.h>
#include <strsafe.h>

#if _DEBUG
#define _DBG 1
#endif


#if _DBG
static FILE *dbgfp;
#endif

void dbg_time(char *fmt,...)
{
}

static void init_dbgfp()
{
#if _DBG
	if (!dbgfp) {
#if (!GCIN_IME || 1) && !CONSOLE_OFF
		AllocConsole();
		fclose(stdout);
		fclose(stderr);
		int fh = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), 0);
		_dup2(fh, 1);
		_dup2(fh, 2);
		_fdopen(1, "wt");
		_fdopen(2, "wt");
		fflush(stdout);
#endif
		char tt[512];
#if GCIN_IME
		sprintf(tt, "C:\\dbg\\ime%x", GetCurrentProcessId());
#elif GCIN_SVR
		sprintf(tt, "C:\\dbg\\svr%x", GetCurrentProcessId());
#else
		sprintf(tt, "C:\\dbg\\other%x", GetCurrentProcessId());
#endif
		dbgfp=_fsopen(tt, "wt",  _SH_DENYWR);
		setbuf(dbgfp, NULL);

		char exe[MAX_PATH];
		GetModuleFileNameA(NULL, exe, sizeof(exe));
		dbg("started %s\n", exe);
	}
#endif
}

int utf8_to_big5(char *in, char *out, int outN);


void dbg(char *format, ...) {
#if _DBG
	va_list ap;
	va_start(ap, format);

	init_dbgfp();

	char buf[1024];

	vsprintf_s(buf, sizeof(buf), format, ap);
	char bufb5[1024];
#if 1
	utf8_to_big5(buf, bufb5, sizeof(bufb5));
#else
	strcpy(bufb5, buf);
#endif

	fprintf(dbgfp, "%s", bufb5);
	printf("%s", bufb5);

	fflush(dbgfp);
	va_end(ap);
#endif
}

void p_err(char *format, ...) {

	va_list ap;
	va_start(ap, format);
#if _DBG
	init_dbgfp();
	vfprintf_s(dbgfp, format, ap);
	vprintf(format, ap);
	fflush(dbgfp);
#endif
	char tt[512];
	vsprintf_s(tt, sizeof(tt), format, ap);
	MessageBoxA(NULL, tt, NULL, MB_OK);
	exit(0);
	va_end(ap);

}

void ErrorExit(LPTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"),
        lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw);
}
#endif

void *zmalloc(int n)
{
  void *p =  malloc(n);
  bzero(p, n);
  return p;
}

#if GCIN_SVR
#if WIN32
#include <gdk/gdkwin32.h>
void win32_init_win(GtkWidget *win)
{
  HWND handle=(HWND)gdk_win32_drawable_get_handle(win->window);

  ShowWindow(handle, SW_HIDE);

  SetWindowLong(handle, GWL_EXSTYLE, WS_EX_NOACTIVATE|WS_EX_TOPMOST);
  SetWindowPos(handle, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
//  ShowWindow(handle, SW_SHOW);
}
#endif
#endif
