#pragma lint -1
#pragma optimize -1
#pragma noroot
#pragma debug 0x8000

#include <tcpip.h>
#include "server.h"


Word ProcessOptions(struct qEntry *q)
{
Word ipid = q->ipid;

  SendHeader(q, 200, 0, NULL, NULL, false);


  #undef xstr
  #define xstr "DAV: 1\r\n"
  TCPIPWriteTCP(ipid, xstr, sizeof(xstr) - 1, false, false);

  #undef xstr
  #define xstr "Allow: OPTIONS, GET, HEAD, PUT, PROPFIND\r\n"
  TCPIPWriteTCP(ipid, xstr, sizeof(xstr) - 1, false, false);


  TCPIPWriteTCP(ipid, "\r\n", 2, false, false);


  q->state = STATE_CLOSE;
  return 200;
}
