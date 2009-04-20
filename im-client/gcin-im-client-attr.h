#define GCIN_PREEDIT_ATTR_FLAG_UNDERLINE 1
#define GCIN_PREEDIT_ATTR_FLAG_REVERSE 2
#define GCIN_PREEDIT_ATTR_MAX_N 64
#define GCIN_PREEDIT_MAX_STR 512

typedef struct {
  int flag;
  short ofs0, ofs1;   // ofs : bytes offset
} GCIN_PREEDIT_ATTR;
