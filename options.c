#pragma lint -1
#pragma optimize -1
#pragma noroot
#pragma debug 0x8000

#include <tcpip.h>
#include "server.h"


Word ProcessOptions(struct qEntry *q)
{
Word ipid = q->ipid;


  #define xstr \
  "AcceptRanges: none\r\n" \
  "DAV: 1, 2\r\n" \
  "Allow: OPTIONS, GET, HEAD, PUT, PROPFIND, MKCOL\r\n"
  
  SendHeader(q, 200, 0, NULL, NULL, xstr, sizeof(xstr) - 1);

  q->state = STATE_CLOSE;
  return 200;
}
