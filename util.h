void p_err(char *fmt,...);
#if DEBUG
void __gcin_dbg_(char *fmt,...);
#define dbg(...) __gcin_dbg_(__VA_ARGS__)
#else
#define dbg(...)
#endif
