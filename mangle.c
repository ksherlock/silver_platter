#pragma noroot
#pragma optimize - 1
#pragma lint - 1
#pragma debug 0x8000

#include "pointer.h"
#include <ctype.h>

// returns:
//-1 on error
// 0 if no mangling needed
// valid handle otherwise.

// rfc 1738:  everything except
// a-zA-Z$-_.+!*'(),/?:@=&
// must be converted to %xx entities
// i also convert & for xhtml compatability.
GSString255Ptr EncodeURL(const GSString255 *gstr) {
  Word i, j;
  Word c;
  Word extra;
  GSString255Ptr dest;

  extra = 0;
  for (i = 0; i < gstr->length; i++) {
    c = gstr->text[i];
    if (isalnum(c)
    	|| c == '$'
    	|| c == '-'
    	|| c == '_'
    	|| c == '.'
    	|| c == '+'
    	|| c == '!'
    	|| c == '*'
    	|| c == '\''
    	|| c == '('
    	|| c == ')'
    	|| c == ','
    	|| c == '/'
		|| c == '?'
		|| c == ':'
		|| c == '@'
		|| c == '='
    	) continue;

    extra += 2;
  }
  if (!extra)
    return NULL;

  dest = (GSString255Ptr)NewPointer(gstr->length + 3 + extra);
  if (!dest)
    return NULL;

  i = j = 0;
  while (i < gstr->length) {
    c = gstr->text[i++];

    if (isalnum(c)
    	|| c == '$'
    	|| c == '-'
    	|| c == '_'
    	|| c == '.'
    	|| c == '+'
    	|| c == '!'
    	|| c == '*'
    	|| c == '\''
    	|| c == '('
    	|| c == ')'
    	|| c == ','
    	|| c == '/'
		|| c == '?'
		|| c == ':'
		|| c == '@'
		|| c == '='
		)
    {
      dest->text[j++] = c;
    } else {
      Word x;

      dest->text[j++] = '%';

      x = (c & 0xf0) >> 4;
      dest->text[j++] = "0123456789abcdef"[x];

      x = c & 0x0f;
      dest->text[j++] = "0123456789abcdef"[x];
    }
  }
  dest->text[j] = 0; // make a cstring
  dest->length = j;

  return dest;
}

#define x(utf, y)                                                              \
  { utf, sizeof(y) - 1, y }

struct ConvTable {
  Word utf;
  Word length;
  char *text;
} mr2html[] = {
    // 0x80
  x(0xc4, "Auml"),
  x(0xc5, "Aring"),
  x(0xc7, "Ccedil"),
  x(0xc9, "Eacute"),
  x(0xd1, "Ntilde"),
  x(0xd6, "Ouml"),
  x(0xdc, "Uuml"),
  x(0xe1, "aacute"),
  x(0xe0, "agrave"),
  x(0xe2, "acirc"),
  x(0xe4, "auml"),
  x(0xe3, "atilde"),
  x(0xe5, "aring"),
  x(0xe7, "ccedil"),
  x(0xe9, "eacute"),
  x(0xe8, "egrave"),
    // 0x90
  x(0xea, "ecirc"),
  x(0xeb, "euml"),
  x(0xed, "iacute"),
  x(0xec, "igrave"),
  x(0xee, "icirc"),
  x(0xef, "iuml"),
  x(0xf1, "ntilde"),
  x(0xf3, "oacute"),
  x(0xf2, "ograve"),
  x(0xf4, "ocirc"),
  x(0xf6, "ouml"),
  x(0xf5, "otilde"),
  x(0xfa, "uacute"),
  x(0xf9, "ugrave"),
  x(0xfb, "ucirc"),
  x(0xfc, "uuml"),
    // 0xA0
  x(0x2020, "dagger"),
  x(0xb0, "deg"),
  x(0xa2, "cent"),
  x(0xa3, "pound"),
  x(0xa7, "sect"),
  x(0x2022, "bull"),
  x(0xb6, "para"),
  x(0xdf, "szlig"),
  x(0xae, "reg"),
  x(0xa9, "copy"),
  x(0x2122, "trade"),
  x(0xb4, "acute"),
  x(0xa8, "uml"),
  x(0x2260, "ne"),
  x(0xc6, "AElig"),
  x(0xd8, "Oslash"),
    // 0xB0
  x(0x221e, "infin"),
  x(0xb1, "plusmn"),
  x(0x2264, "le"),
  x(0x2265, "ge"),
  x(0xa5, "yen"),
  x(0xb5, "micro"),
  x(0x2202, "part"),
  x(0x2211, "sum"),
  x(0x220f, "product"),
  x(0x03c0, "pi"),
  x(0x222b, "int"),
  x(0xaa, "ordf"),
  x(0xba, "ordm"),
  x(0x03a9, "Omega"),
  x(0xe6, "aelig"),
  x(0xf8, "oslash"),
    // 0xC0
  x(0xbf, "iquest"),
  x(0xa1, "iexcl"),
  x(0xac, "not"),
  x(0x221a, "radic"),
  x(0x0192, "fnof"),
  x(0x2248, "asymp"),
  x(0x2206, "#x2206"),
  x(0xab, "laquo"),
  x(0xbb, "raquo"),
  x(0x2026, "hellip"),
  x(0xa0, "nbsp"),
  x(0xc0, "Agrave"),
  x(0xc3, "Atilde"),
  x(0xd5, "Otilde"),
  x(0x152, "OElig"),
    x(0x153, "oelig"),
    // 0xD0
  x(0x2013, "ndash"),
  x(0x2014, "mdash"),    
  x(0x201c, "ldquo"),    
  x(0x201d, "rdquo"),    
  x(0x2018, "lsquo"),    
  x(0x2019, "rsquo"),    
  x(0xf7, "divide"),    
  x(0x25ca, "loz"),    
  x(0xff, "yuml"),    
  x(0x0178, "Yuml"),    
  x(0x2044, "frasl"),    
  x(0x20ac, "euro"),    
  x(0x2039, "lsaquo"),    
  x(0x203a, "rsaquo"),    
  x(0xfb01, "#xfb01"),    
  x(0xfb02, "#xfb02"),
    // 0xE0
  x(0x2021, "Dagger"),
  x(0xb7, "middot"),
  x(0x201a, "sbquo"),
  x(0x201e, "bdquo"),
  x(0x2030, "permil"),
  x(0xc2, "Acirc"),
  x(0xca, "Ecirc"),
  x(0xc1, "Aacute"),
  x(0xcb, "Euml"),
  x(0xc8, "Egrave"),
  x(0xcd, "Iacute"),
  x(0xce, "Icirc"),
  x(0xcf, "Iuml"),
  x(0xcc, "Igrave"),
  x(0xd3, "Oacute"),
    x(0xd4, "Ocirc"),
    // 0xF0
    x(0xf8ff, "#xf8ff"), // apple logo
  x(0xd2, "Ograve"),
  x(0xda, "Uacute"),
  x(0xdb, "Ucirc"),
  x(0xd9, "Ugrave"),
  x(0x0131, "#x0131"),
  x(0x02c6, "circ"),
  x(0x02dc, "tilde"),
  x(0xaf, "macr"),
  x(0x02d8, "#x02d8"),
  x(0x02d9, "#x02d9"),
  x(0x2da, "#x02da"),
  x(0xb8, "cedil"),
  x(0x02dd, "#x02dd"),
  x(0x02db, "#x02db"),
  x(0x02c7, "#x02c7")
};        
#undef x

//
// returns:
//-1 on error
// 0 if no conversion needed
// valid handle otherwise.
GSString255Ptr MacRoman2HTML(const GSString255 *gstr) {
  Word i, j;
  Word c;
  Word extra;
  GSString255Ptr dest;

  extra = 0;
  for (i = 0; i < gstr->length; i++) {
    c = gstr->text[i];
    if (c & 0x80) {
      // -1 since we're overwriting 1 char, +2 for &;
      extra += mr2html[c & 0x7f].length + 2 - 1;
    }
    // &amp;
    else if (c == '&')
      extra += 5 - 1;
    // &quot;
    else if (c == '"')
      extra += 6 - 1;
    // &apos;
    // else if (c == '\'') extra += 6 - 1;
    // &lt; &gt;
    else if (c == '>' || c == '<')
      extra += 4 - 1;
  }
  if (!extra)
    return NULL;

  dest = (GSString255Ptr)NewPointer(gstr->length + 3 + extra);
  if (!dest)
    return NULL;

  i = j = 0;
  while (i < gstr->length) {
    c = gstr->text[i++];
    if (c & 0x80) {
      Word l;
      char *cp;

      cp = mr2html[c & 0x7f].text;
      dest->text[j++] = '&';
      while (c = *cp++)
        dest->text[j++] = c;
      dest->text[j++] = ';';
    } else if (c == '&') {
      dest->text[j++] = '&';
      dest->text[j++] = 'a';
      dest->text[j++] = 'm';
      dest->text[j++] = 'p';
      dest->text[j++] = ';';
    } else if (c == '"') {
      dest->text[j++] = '&';
      dest->text[j++] = 'q';
      dest->text[j++] = 'u';
      dest->text[j++] = 'o';
      dest->text[j++] = 't';
      dest->text[j++] = ';';
    }
#if 0
    else if (c == '\'')
    {
      dest->text[j++] = '&';
      dest->text[j++] = 'a';
      dest->text[j++] = 'p';
      dest->text[j++] = 'o';
      dest->text[j++] = 's';
      dest->text[j++] = ';';
    }
#endif
    else if (c == '>') {
      dest->text[j++] = '&';
      dest->text[j++] = 'g';
      dest->text[j++] = 't';
      dest->text[j++] = ';';
    } else if (c == '<') {
      dest->text[j++] = '&';
      dest->text[j++] = 'l';
      dest->text[j++] = 't';
      dest->text[j++] = ';';
    }

    else
      dest->text[j++] = c;
  }
  dest->text[j] = 0; // make a cstring
  dest->length = j;

  return dest;
}

// Convert MacRoman --> utf-8
// returns:
//-1 on error
// 0 if no conversion needed
// valid handle otherwise.
GSString255Ptr MacRoman2UTF8(const GSString255 *gstr) {
  Word i, j;
  Word c;
  Word extra;
  GSString255Ptr dest;
  Word utf;

  extra = 0;
  for (i = 0; i < gstr->length; i++) {
    c = gstr->text[i];
    if (c & 0x80) {
      utf = mr2html[c & 0x7f].utf;
      if (utf > 0x0fff)
        extra += 3; //
      else
        extra += 2;
    }
  }
  if (!extra)
    return NULL;

  dest = (GSString255Ptr)NewPointer(gstr->length + 3 + extra);
  if (!dest)
    return NULL;

  i = j = 0;
  while (i < gstr->length) {
    c = gstr->text[i++];
    if (c & 0x80) {
      utf = mr2html[c & 0x7f].utf;
      if (utf > 0x0fff) {
        dest->text[j++] = 0xe0 | (utf >> 12);
        dest->text[j++] = 0x80 | ((utf >> 6) & 0x3f);
        dest->text[j++] = 0x80 | (utf & 0x3f);
      } else {
        dest->text[j++] = 0xc0 | (utf >> 6);
        dest->text[j++] = 0x80 | (utf & 0x3f);
      }
    } else
      dest->text[j++] = c;
  }
  dest->text[j] = 0; // make a cstring
  dest->length = j;

  return dest;
}
