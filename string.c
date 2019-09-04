#pragma noroot
#pragma lint -1
#pragma optimize -1
#pragma debug 0x8000

#include <types.h>
#include <intmath.h>
#include <memory.h>


#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

#include "dfa.h"

#include "server.h"
#include "config.h"
#include "pointer.h"
#include "xstring.h"

#include "headers.h"
#include "methods.h"

extern int scan_headers(const char *);
extern int scan_methods(const char *);
extern int scan_cgi(const char *);

// checks for 
// <name>?asingle
// <name>?html and
// ._<name>
//
// the filename is updated and the moreFlags value is returned.
//
Word ScanCGI(GSString255Ptr g)
{
Word i;
Word c;
Word len = g->length;

  for (i = len - 1; i; i--) {
    c = g->text[i];
    if (c == '/') break;
    if (c == '?') {
        g->text[i] = 0;
        g->length = i;
      int type = scan_cgi(g->text + i + 1);
      if (type) {
        return type;
      }
      return CGI_ERROR;
      }
  }
  // check for ._<name>
  // at this point, i will be 0 or a pointer to the /.
  if (len - i < 4) return;
  
  if ((g->text[i + 1] == '.') && (g->text[i + 2] == '_')) {
        len -= 2;
    for (i++ ; i < len; i++) {
                g->text[i] = g->text[i + 2];
        }
        g->text[i] = 0;  // NULL terminate for convenience.
        g->length = len;
        return CGI_APPLEDOUBLE; 
  }
  
  return 0;
}

/* cp2 is lowercase */
int cmptoken(const char *cp1, const char *cp2) {
  unsigned a,b;
  unsigned i;
  for (i = 0; ; ++i) {
    a = cp1[i];
    b = cp2[i];
    if (a == ',' || isspace(a)) a = 0;
    if (a == 0 && b == 0) return 1;
    if (!a || !b) return 0;
    a = tolower(a);
    if (a != b) return 0;
  }
}
static void ScanConnection(char *cp, struct qEntry *q) {
  /* process a connection header */
  /* connection: keep-alive */
  /* connection: close */
  /* connection: upgrade, TE, keep-alive */

  unsigned c;
  unsigned i;

  for (i = 0; ;) {

    do {
      c = cp[i++];
    } while (c == ' ' || isspace(c));
    if (!c) return;

    if (c == 'c' && cmptoken(cp, "close")) {
      q->flags &= ~FLAG_KA;
      return;
    }
    if (c == 'k' && cmptoken(cp, "keep-alive")) {
      q->flags |= FLAG_KA;
      return;
    }
  }
}

static unsigned ScanRange(char *cp, struct qEntry *q) {
  /* process a Range: header */
  /* bytes=-<suffix-length> */
  /* bytes=<range-start>- */
  /* bytes=<range-start>-<range-end> */

  /* more complicated expressions are possible but not supported */
  unsigned i;
  unsigned valid = 0;

  q->flags |= FLAG_RANGE;

  if (strncmp(cp, "bytes=", 6)) return 0;
  cp += 6;


  if (isdigit(*cp)) {
    for (i = 0; isdigit(cp[i]); ++i);
    q->range[0] = Dec2Long(cp, i, 0);
    if (_toolErr) return 0;
    valid |= FLAG_RANGE0;
    cp += i;
  }
  if (cp[0] != '-') {
    return 0;
    /* should send error 416 */
  }
  ++cp;
  if (isdigit(*cp)) {
    for (i = 0; isdigit(cp[i]); ++i);
    q->range[1] = Dec2Long(cp, i, 0);
    if (_toolErr) return 0;
    valid |= FLAG_RANGE1;
    cp += i;
  }
  if (!valid) return 0;

  for (i = 0; isspace(cp[i]); ++i);

  if (cp[i]) return 0;

  if (valid == (FLAG_RANGE0|FLAG_RANGE1)) {
    if (q->range[1] < q->range[0]) return 0;
  }

  q->flags |= valid;
  return 1;
}


static unsigned ScanContentLength(char *cp, struct qEntry *q) {

  unsigned i;

    for (i = 0; isdigit(cp[i]); ++i);
    q->contentlength = Dec2Long(cp, i, 0);
    if (_toolErr) return 0;

  for(; isspace(cp[i]); ++i);
  if (cp[i]) return 0;
  return 1;
}

// scan for headers we recognize.
void ScanHeader(char *cp, struct qEntry *q)
{
char c;
unsigned i;
Handle h;
GSString255Ptr host;
Word header;


  i = scan_headers(cp);
  if (!i) return;
  cp += i & 0xff;
  i >>= 8;
  while (isspace(*cp)) cp++;

  switch(i)
  {
  case h_KeepAlive: // Keep-Alive.
    q->flags |= FLAG_KA;
    break;

  case h_Connection: // Connection:
    ScanConnection(cp, q);
    break;

  case h_Host: // Host:
    if (!*cp) return;

    // find the length...
    i = 0;
    while ((c = cp[i]) && !isspace(c)) i++;
    if (!i) return;

    host = NewPointer(i + 3);
    if (host)
    {
      q->host = host;
      host->length = i;
      BlockMove(cp, host->text, i);
      host->text[i] = 0;
    }
    break;

  case h_ContentLength:  //Content-Length
    if (!ScanContentLength(cp, q))
      q->error = 400;
    break;

  case h_ContentType: // Content-Type ... check if text/*
    if (!*cp) return;

    if (!xstrncasecmp("text/", cp, 5)) q->flags |= FLAG_TEXT;
    break;

  case h_Depth: // Depth: 0, 1, or infinity.
    if (isdigit(c = *cp)) q->depth = c - '0';
    else q->depth = -1;
    break;

  case h_Range: // Range:
    ScanRange(cp, q);
    break;
  }                     
}



// scan a request for the request and http version.
void ScanMethod(char *cp, struct qEntry *q)
{
char c;
int len;
Handle h;
GSString255Ptr path;
Word match;
int i, j;
unsigned cmd;



  // format: <method> <space>+ <path> <space>+ (HTTP/\d.\d)?

  cmd = scan_methods(cp);
  if (cmd) {
    cp += cmd & 0xff;
    cmd >>= 8;
  } else {
    // skip past the offending command.
    while ((c = *cp) && !isspace(c)) cp++;
    cmd = -1;
  }

  while (isspace(*cp)) cp++;

  // URI: should be /fully/qualified/path
  // TODO '*' is valid for OPTIONS.

  if (*cp != '/')
    return;

  // first, find the length of the URI
  // then copy it to a new handle
  // then convert %xx to characters.

  // URI
  len = 0;
  while ((c = cp[len]) && !isspace(c)) len++;

  if (len)
  {
    path = NewPointer(len + 3);
    if (!path) return;

    q->pathname = path;

    path->length = len;
    BlockMove(cp, path->text, len);
    path->text[len] = 0;

    cp += len;

    // now demangle %xx codes.
    // since the demangled size will always be less, we're safe.
    i = j = 0;
    while (c = path->text[i++])
    {
      if ((c == '%') && isxdigit(path->text[i]) && isxdigit(path->text[i + 1]))
      {
        Word a, b;
        c = path->text[i++];
        a = isdigit(c) ? c - '0' : _tolower(c) - 'a' + 10;

        c = path->text[i++];
        b = isdigit(c) ? c - '0' : _tolower(c) - 'a' + 10;
        c = a << 4 | b;
      }
      path->text[j++] = c;
    }
    path->text[j] = 0;
    path->length = j;

    q->moreFlags = ScanCGI(path);

    // copy the path to the fullpath
    if (fJail)
    {
      Word len;
      GSString255Ptr fullpath;

      len = fRoot->length + path->length;

      fullpath = NewPointer(len + 3);

      if (fullpath)
      {
        q->fullpath = fullpath;

        BlockMove(fRoot->text, fullpath->text, fRoot->length);
        BlockMove(path->text, fullpath->text + fRoot->length, path->length);
        fullpath->length = len;
        fullpath->text[len] = 0;
      }
    }
    else
    {
      q->fullpath = path;
      RetainPointer(path);
    }
  } // if (len)

  q->version = 0x0009;  // 0.9
  q->command = cmd;

  if (!*cp) return;

  while (isspace(*cp)) cp++;

  // http version....
  if (!xstrncasecmp("HTTP/", cp, 5))
  {
  int major, minor;
    major = 0;
    minor = 0;

    cp += 5;
    while (isdigit(c = *cp))
    {
      major = major * 10 + c - '0';
      cp++;
    }
    if (*cp++ != '.') return;

    while (isdigit(c = *cp))
    {
      minor = minor * 10 + c - '0';
      cp++;
    }
    q->version = (major << 8) | minor;
    if (q->version > 0x0100) q->flags |= FLAG_KA;
  }
}
