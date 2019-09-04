#pragma lint - 1
#pragma optimize - 1
#pragma noroot
#pragma debug 0x8000

#include "config.h"
#include "server.h"
#include <tcpip.h>

Word ProcessOptions(struct qEntry *q) {
  Word ipid = q->ipid;

  if (fWebDav) {
#undef xstr
#define xstr                                                                   \
  "AcceptRanges: none\r\n"                                                     \
  "DAV: 1, 2\r\n"                                                              \
  "Allow: OPTIONS, GET, HEAD, PUT, PROPFIND, MKCOL, LOCK, UNLOCK\r\n"

    SendHeader(q, 200, 0, NULL, NULL, xstr, sizeof(xstr) - 1);
  } else {
#undef xstr
#define xstr                                                                   \
  "AcceptRanges: none\r\n"                                                     \
  "Allow: OPTIONS, GET, HEAD, PUT\r\n"

    SendHeader(q, 200, 0, NULL, NULL, xstr, sizeof(xstr) - 1);
  }

  q->state = STATE_CLOSE;
  return 200;
}
