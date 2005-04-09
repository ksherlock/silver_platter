#pragma noroot
#pragma optimize -1
#pragma lint -1

#include <Memory.h>

#include <ctype.h>


extern Word MyID;

// returns:
//-1 on error
//0 if no mangling needed
// valid handle otherwise.
Handle MangleName(const GSString255 *gstr)
{
Word i, j;
char c;
Word extra;
Handle h;
GSString255Ptr dest;

  extra = 0;
  for (i = 0; i < gstr->length; i++)
  {
    c = gstr->text[i];
    if (c == '%'
      || c == '"'
      || c == '\''
      || c == ' '
      || c == '>'
      || c == '<'
      || c == '?'
      || c & 0x80
      || !isprint(c)) extra += 2;
  }
  if (!extra) return NULL;

  h = NewHandle(gstr->length + 3 + extra, MyID | 0x0e00, attrLocked, NULL);
  if (_toolErr) return (Handle)0; //-1;

  dest = (GSString255Ptr)*h;

  i = j = 0;
  while ( i < gstr->length)
  {
    c = gstr->text[i++];
    if (c == '%'
      || c == '"'
      || c == '\''
      || c == ' '
      || c == '>'
      || c == '<'
      || c == '?'
      || c & 0x80
      || !isprint(c))
    {
      Word x;

      dest->text[j++] = '%';

      x = (c & 0xf0) >> 4;
      dest->text[j++] = "0123456789abcdef"[x];

      x = c & 0x0f;
      dest->text[j++] = "0123456789abcdef"[x];

    }
    else dest->text[j++] = c;
  }
  dest->text[j] = 0; // make a cstring
  dest->length = j;

  return h;
}


#define x(y) { sizeof(y) -1 , y }

struct ConvTable
{
  Word length;
  char *text;
} mr2html[] =
{
  //0x80
  x("Auml"),
  x("Aring"),
  x("Ccedil"),
  x("Eacute"),
  x("Ntilde"),
  x("Ouml"),
  x("Uuml"),
  x("aacute"),
  x("agrave"),
  x("acirc"),
  x("auml"),
  x("atilde"),
  x("aring"),
  x("ccedil"),
  x("eacute"),
  x("egrave"),
  //0x90
  x("ecirc"),
  x("euml"),
  x("iacute"),
  x("igrave"),
  x("icirc"),
  x("iuml"),
  x("ntilde"),
  x("oacute"),
  x("ograve"),
  x("ocirc"),
  x("ouml"),
  x("otilde"),
  x("uacute"),
  x("ugrave"),
  x("ucirc"),
  x("uuml"),
  //0xA0      
  x("dagger"),
  x("deg"),
  x("cent"),
  x("pound"),
  x("sect"),
  x("bull"),
  x("para"),
  x("szlig"),
  x("reg"),
  x("copy"),
  x("trade"),
  x("acute"),
  x("uml"),
  x("ne"),
  x("AElig"),
  x("Oslash"),
  //0xB0      
  x("infin"),
  x("plusmn"),
  x("le"),
  x("ge"),
  x("yen"),
  x("micro"),
  x("part"),
  x("sum"),
  x("product"),
  x("pi"),
  x("int"),
  x("ordf"),
  x("ordm"),
  x("Omega"),
  x("aelig"),
  x("oslash"),
  //0xC0
  x("iquest"),
  x("iexcl"),
  x("not"),
  x("radic"),
  x("fnof"),
  x("asymp"),
  x("#x2206"),
  x("laquo"),
  x("raquo"),
  x("hellip"),
  x("nbsp"),
  x("Agrave"),
  x("Atilde"),
  x("Otilde"),
  x("OElig"),
  x("oelig"),
  // 0xD0
  x("ndash"),
  x("mdash"),    
  x("ldquo"),    
  x("rdquo"),    
  x("lsquo"),    
  x("rsquo"),    
  x("divide"),    
  x("loz"),    
  x("yuml"),    
  x("Yuml"),    
  x("frasl"),    
  x("euro"),    
  x("lsaquo"),    
  x("rsaquo"),    
  x("#xfb01"),    
  x("#xfb02"),
  // 0xE0
  x("Dagger"),
  x("middot"),
  x("sbquo"),
  x("bdquo"),
  x("permil"),
  x("Acirc"),
  x("Ecirc"),
  x("Aacute"),
  x("Euml"),
  x("Egrave"),
  x("Iacute"),
  x("Icirc"),
  x("Iuml"),
  x("Igrave"),
  x("Oacute"),
  x("Ocirc"),
  // 0xF0
  x("#xf8ff"),  // apple logo
  x("Ograve"),
  x("Uacute"),
  x("Ucirc"),
  x("Ugrave"),
  x("#x0131"),
  x("circ"),
  x("tilde"),
  x("macr"),
  x("#x02d8"),
  x("#x02d9"),
  x("#x02da"),
  x("cedil"),
  x("#x02dd"),
  x("#x02db"),
  x("#x02c7")
};        
#undef x

//
// returns:
//-1 on error
//0 if no conversion needed
// valid handle otherwise.
Handle MacRoman2HTML(const GSString255 *gstr)
{
Word i, j;
char c;
Word extra;
Handle h;
GSString255Ptr dest;

  extra = 0;
  for (i = 0; i < gstr->length; i++)
  {
    c = gstr->text[i];
    if (c & 0x80)
    {
      // -1 since we're overwriting 1 char, +2 for &;
      extra += mr2html[c & 0x7f].length + 2 - 1;
    }
    // &amp;
    else if (c == '&') extra += 5 - 1;
    // &quot;
    else if (c == '"') extra += 6 - 1;
    // &apos;
    //else if (c == '\'') extra += 6 - 1;
    // &lt; &gt;
    else if (c == '>' || c == '<') extra += 4 - 1;
  }
  if (!extra) return NULL;

  h = NewHandle(gstr->length + 3 + extra, MyID | 0x0e00, attrLocked, NULL);
  if (_toolErr) return (Handle)0; //-1;

  dest = (GSString255Ptr)*h;

  i = j = 0;
  while ( i < gstr->length)
  {
    c = gstr->text[i++];
    if (c & 0x80)
    {
    Word l;
    char *cp;

      cp = mr2html[c & 0x7f].text;
      dest->text[j++] = '&';
      while (c = *cp++) dest->text[j++] = c;
      dest->text[j++] = ';';
    }
    else if (c == '&')
    {
      dest->text[j++] = '&';
      dest->text[j++] = 'a';
      dest->text[j++] = 'm';
      dest->text[j++] = 'p';
      dest->text[j++] = ';';
    }                       
    else if (c == '"')
    {
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
    else if (c == '>')
    {
      dest->text[j++] = '&';
      dest->text[j++] = 'g';
      dest->text[j++] = 't';
      dest->text[j++] = ';';
    }                       
    else if (c == '<')
    {
      dest->text[j++] = '&';
      dest->text[j++] = 'l';
      dest->text[j++] = 't';
      dest->text[j++] = ';';
    }                       

    else dest->text[j++] = c;
  }
  dest->text[j] = 0; // make a cstring
  dest->length = j;

  return h;
}
