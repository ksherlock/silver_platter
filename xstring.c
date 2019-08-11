#pragma lint -1
#pragma optimize -1
#pragma noroot
#pragma debug 0x8000

#include <ctype.h>

/* these assume the string length is 16-bit */

int xstrncasecmp(const char *a, const char *b, unsigned l) {
  unsigned i;

  for(i = 0; i < l; ++i) {
    unsigned aa = a[i];
    unsigned bb = b[i];
    if (!(aa | bb)) return 0;
    aa = tolower(aa);
    bb = tolower(bb);
    if (aa == bb) continue;
    return (int)aa - (int)bb;
  }
  return 0;
}


int xstrcasecmp(const char *a, const char *b) {
	unsigned i;

	for (i = 0; ; ++i) {
	    unsigned aa = a[i];
	    unsigned bb = b[i];
	    if (!(aa | bb)) return 0;
	    aa = tolower(aa);
	    bb = tolower(bb);
	    if (aa == bb) continue;
	    return (int)aa - (int)bb;
	}
}
