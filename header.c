#pragma lint -1
#pragma optimize -1
#pragma noroot

#include <tcpip.h>
#include <timetool.h>

#include <kstring.h>

#include "server.h"

extern int orca_sprintf(char *, const char *, ...);

static char tiBuffer[38];

extern void tiTimeRec2GMTString(const TimeRec *, char *);

void SendHeader(struct qEntry *q, Word status, LongWord size,
  const TimeRec * modTime, const char *mimeString, Boolean term)
{
Word ipid = q->ipid;
Word flags = q->flags;
Word i;

  if (q->version >= 0x0100)
  {
    i = orca_sprintf(buffer, "HTTP/1.1 %u\r\n", status);
    TCPIPWriteTCP(ipid, buffer, i, false, false);

    tiToday2GMTString(tiBuffer);
    if (!_toolErr)
    {
      i = orca_sprintf(buffer, "Date: %b\r\n", tiBuffer);
      TCPIPWriteTCP(ipid, buffer, i, false, false);
    }

    #undef xstr
    #define xstr "Server: SilverPlatter/1.0 (IIgs)\r\n"
    TCPIPWriteTCP(ipid, xstr, sizeof(xstr) - 1, false, false);

    if (modTime)
    {
      tiTimeRec2GMTString(modTime, tiBuffer);
      i = orca_sprintf(buffer, "Last-Modified: %b\r\n", tiBuffer);
      TCPIPWriteTCP(ipid, buffer, i, false, false);
    }

    if (flags & FLAG_CHUNKED)
    {
      #undef xstr
      #define xstr "Transfer-Encoding: chunked\r\n"
      TCPIPWriteTCP(ipid, xstr, sizeof(xstr) - 1, false, false);
    }
    else if (size != -1)
    {
      i = orca_sprintf(buffer, "Content-Length: %lu\r\n", (LongWord)size);
      TCPIPWriteTCP(ipid, buffer, i, false, false);
    }

    if (flags & FLAG_KA)
    {
      #undef xstr
      #define xstr "Connection: Keep-Alive\r\n"
      TCPIPWriteTCP(ipid, xstr, sizeof(xstr) - 1, false, false);
      #undef xstr
      #define xstr "Keep-Alive: timeout=30\r\n"
      TCPIPWriteTCP(ipid, xstr, sizeof(xstr) - 1, false, false);
    }                                     
    else
    {
      #undef xstr
      #define xstr "Connection: close\r\n"
      TCPIPWriteTCP(ipid, xstr, sizeof(xstr) - 1, false, false);
    }

    if (mimeString)
    {
      i = orca_sprintf(buffer, "Content-Type: %s\r\n", mimeString);
      TCPIPWriteTCP(ipid, buffer, i, false, false);
    }
    //                                    
    if (term) TCPIPWriteTCP(ipid, "\r\n", 2, false, false);
  }
}
