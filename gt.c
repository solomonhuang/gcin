#include "gcin.h"
#include "gtab.h"
#include "gcin-conf.h"
#include "gcin-endian.h"
#include "pho.h"
#include "tsin.h"
#include "tsin-parse.h"



main()
{
  char *dat0[]={"海"};
  char *dat1[]={"小","水"};
  char *dat2[]={"測試"};

  insert_gbuf_cursor(dat0, sizeof(dat0)/sizeof(dat0[0]));
  insert_gbuf_cursor(dat1, sizeof(dat1)/sizeof(dat1[0]));
  insert_gbuf_cursor(dat2, sizeof(dat2)/sizeof(dat2[0]));

  dump_gbuf();
#if 1
  gtab_parse();
#endif
}
