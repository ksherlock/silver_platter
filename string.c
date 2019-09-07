#pragma noroot
#pragma lint - 1
#pragma optimize - 1
#pragma debug 0x8000

#include <intmath.h>
#include <memory.h>
#include <types.h>

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "pointer.h"
#include "server.h"
#include "xstring.h"

#include "headers.h"
#include "methods.h"

extern int scan_header(const char *);
extern int scan_method(const char *);
extern int scan_cgi(const char *);

// checks for
// <name>?asingle
// <name>?html and
// ._<name>
//
// the filename is updated and the moreFlags value is returned.
//
Word ScanCGI(GSString255Ptr g, struct qEntry *q) {
  Word i;
  Word c;
  Word len = g->length;

  for (i = len - 1; i; i--) {
    c = g->text[i];
    if (c == '/')
      break;
    if (c == '?') {
      g->text[i] = 0;
      g->length = i;
      int type = scan_cgi(g->text + i + 1);
      if (type) {
        q->moreFlags = type;
        return 0;
      }
      return 422;
    }
  }
  // check for ._<name>
  // at this point, i will be 0 or a pointer to the /.
  if (len - i < 4)
    return 0;

  if ((g->text[i + 1] == '.') && (g->text[i + 2] == '_')) {
    len -= 2;
    for (i++; i < len; i++) {
      g->text[i] = g->text[i + 2];
    }
    g->text[i] = 0; // NULL terminate for convenience.
    g->length = len;
    q->moreFlags = CGI_APPLEDOUBLE;
  }

  return 0;
}

/* cp2 is lowercase */
int cmptoken(const char *cp1, const char *cp2) {
  unsigned a, b;
  unsigned i;
  for (i = 0;; ++i) {
    a = cp1[i];
    b = cp2[i];
    if (a == ',' || isspace(a))
      a = 0;
    if (a == 0 && b == 0)
      return 1;
    if (!a || !b)
      return 0;
    a = tolower(a);
    if (a != b)
      return 0;
  }
}
static void ScanConnection(char *cp, struct qEntry *q) {
  /* process a connection header */
  /* connection: keep-alive */
  /* connection: close */
  /* connection: upgrade, TE, keep-alive */

  unsigned c;
  unsigned i;

  for (i = 0;;) {

    do {
      c = cp[i++];
    } while (c == ' ' || isspace(c));
    if (!c)
      return;

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

  if (strncmp(cp, "bytes=", 6))
    return 416;
  cp += 6;

  if (isdigit(*cp)) {
    for (i = 0; isdigit(cp[i]); ++i)
      ;
    q->range[0] = Dec2Long(cp, i, 0);
    if (_toolErr)
      return 416;
    valid |= FLAG_RANGE0;
    cp += i;
  }
  if (cp[0] != '-') {
    return 416;
    /* should send error 416 */
  }
  ++cp;
  if (isdigit(*cp)) {
    for (i = 0; isdigit(cp[i]); ++i)
      ;
    q->range[1] = Dec2Long(cp, i, 0);
    if (_toolErr)
      return 416;
    valid |= FLAG_RANGE1;
    cp += i;
  }
  if (!valid)
    return 416;

  for (i = 0; isspace(cp[i]); ++i)
    ;

  if (cp[i])
    return 416;

  if (valid == (FLAG_RANGE0 | FLAG_RANGE1)) {
    if (q->range[1] < q->range[0])
      return 416;
  }

  q->flags |= valid;
  return 0;
}

static unsigned ScanContentLength(char *cp, struct qEntry *q) {

  unsigned i;

  for (i = 0; isdigit(cp[i]); ++i)
    ;
  q->contentLength = Dec2Long(cp, i, 0);
  if (_toolErr)
    return 400;

  for (; isspace(cp[i]); ++i)
    ;
  if (cp[i])
    return 400;
  return 0;
}

// scan for headers we recognize.
Word ScanHeader(char *cp, struct qEntry *q) {
  char c;
  unsigned i;
  Handle h;
  GSString255Ptr host;
  Word header;

  i = scan_header(cp);
  if (!i)
    return 0;
  cp += i & 0xff;
  i >>= 8;
  while (isspace(*cp))
    cp++;

  switch (i) {
  case HDR_KEEP_ALIVE: // Keep-Alive.
    q->flags |= FLAG_KA;
    break;

  case HDR_CONNECTION: // Connection:
    ScanConnection(cp, q);
    break;

  case HDR_HOST: // Host:
    if (!*cp)
      return 400;

    // find the length...
    i = 0;
    while ((c = cp[i]) && !isspace(c))
      i++;
    if (!i)
      return 400;

    host = NewPointer(i + 3);
    if (host) {
      q->host = host;
      host->length = i;
      BlockMove(cp, host->text, i);
      host->text[i] = 0;
    }
    break;

  case HDR_CONTENT_LENGTH: // Content-Length
    return ScanContentLength(cp, q);
    break;

  case HDR_CONTENT_TYPE: // Content-Type ... check if text/*
    if (!*cp)
      return 0;

    if (!xstrncasecmp("text/", cp, 5))
      q->flags |= FLAG_TEXT;
    break;

  case HDR_DEPTH: // Depth: 0, 1, or infinity.
    if (isdigit(c = *cp))
      q->depth = c - '0';
    else
      q->depth = -1;
    break;

  case HDR_RANGE: // Range:
    return ScanRange(cp, q);
    break;
  }

  return 0;
}

// scan a request for the request and http version.
Word ScanRequest(char *cp, struct qEntry *q) {
  unsigned c;
  unsigned len;
  GSString255Ptr path;
  unsigned i, j;
  unsigned method;
  unsigned err;

  // format: <method> <space>+ <path> <space>+ (HTTP/\d.\d)?

  method = scan_method(cp);
  if (method) {
    cp += method & 0xff;
    method >>= 8;
  } else {
    method = -1;
    return 405;
  }
  q->method = method;
  q->version = 0x0009;

  for (i = 0; isspace(cp[i]); ++i)
    ;
  cp += i;

  // URI: should be /fully/qualified/path
  // BUT '*' is valid for OPTIONS.

  c = *cp;
  if (c != '/' && c != '*') {
    return 400;
  }

  // first, find the length of the URI
  // then copy it to a new handle
  // then convert %xx to characters.

  // URI
  len = 0;
  while ((c = cp[len]) && !isspace(c))
    len++;

  if (*cp == '*') {
    if (method != CMD_OPTIONS || len != 1) {
      return 400;
    }
  } else {
    path = NewPointer(len + 3);
    if (!path) {
      return 400;
    }

    q->pathname = path;

    path->length = len;
    BlockMove(cp, path->text, len);
    path->text[len] = 0;

    cp += len;

    // now demangle %xx codes.
    // since the demangled size will always be less, we're safe.
    i = j = 0;
    while (c = path->text[i++]) {
      if ((c == '%') && isxdigit(path->text[i]) &&
          isxdigit(path->text[i + 1])) {
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

    err = ScanCGI(path, q);
    if (err) return err;

    // copy the path to the fullpath
    if (fJail) {
      Word len;
      GSString255Ptr fullpath;

      len = fRoot->length + path->length;

      fullpath = NewPointer(len + 3);

      if (fullpath) {
        q->fullpath = fullpath;

        BlockMove(fRoot->text, fullpath->text, fRoot->length);
        BlockMove(path->text, fullpath->text + fRoot->length, path->length);
        fullpath->length = len;
        fullpath->text[len] = 0;
      }
    } else {
      q->fullpath = path;
      RetainPointer(path);
    }
  }

  if (!*cp)
    return 0;

  for (i = 0; isspace(cp[i]); ++i)
    ;
  cp += i;

  // http version....
  if (!xstrncasecmp("HTTP/", cp, 5)) {
    int major, minor;
    major = 0;
    minor = 0;

    cp += 5;
    while (isdigit(c = *cp)) {
      major = major * 10 + c - '0';
      cp++;
    }
    if (*cp++ != '.')
      return 400;

    while (isdigit(c = *cp)) {
      minor = minor * 10 + c - '0';
      cp++;
    }
    q->version = (major << 8) | minor;
    if (q->version > 0x0100)
      q->flags |= FLAG_KA;
  }
  return 0;
}
