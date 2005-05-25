#pragma lint -1
#pragma optimize -1
#pragma noroot

#include <tcpip.h>
#include <timetool.h>
#include <MiscTool.h>


#include "server.h"
#include "config.h"

extern int orca_sprintf(char *, const char *, ...);
extern int fdprintf(word, const char *, ...);

static char tiBuffer[38];
static char buffer16[16];

extern Word logfd;

extern void tiTimeRec2GMTString(const TimeRec *, char *);

void SendHeader(struct qEntry *q, Word status, LongWord size,
  const TimeRec * modTime, const char *mimeString, const char *extra, word extralen)
{
static char buffer[128];

Word ipid = q->ipid;
Word flags = q->flags;
Word i;


  if (fLog && logfd)
  {
  // must be broken out to prevent \x from grabbing too much.
  static char months[] = "\x03" "Jan" "\x03" "Feb" "\x03" "Mar"
                         "\x03" "Apr" "\x03" "May" "\x03" "Jun"
                         "\x03" "Jul" "\x03" "Aug" "\x03" "Sep"
                         "\x03" "Oct" "\x03" "Nov" "\x03" "Dec";

  static char buffer6[6];
  static tiPrefRec ti;

  char *req = q->request ? (char *)q->request : "\x00\x00";
  TimeRec tr;


    TCPIPConvertIPToASCII(q->ip, buffer16, 0);

    ti.pCount = 1;
    tiGetTimePrefs(&ti);
    tiOffset2TimeZoneString(buffer6, ti.secOffset, 0);

    tr = ReadTimeHex();



    if (size != -1)
      fdprintf(logfd, "%b - - [%02u/%b/%04u:%02u:%02u:%02u %b] \"%B\" %u %lu\r",
        buffer16,
        tr.day + 1, months + (tr.month << 2), 1900 + tr.year,
        tr.hour, tr.minute, tr.second, buffer6,
        req, status, size);
    else
      fdprintf(logfd, "%b - - [%02d/%b/%04u:%02u:%02u:%02u %b] \"%B\" %u -\r",
        buffer16,
        tr.day + 1, months + (tr.month << 2), 1900 + tr.year,
        tr.hour, tr.minute, tr.second, buffer6,
        req, status);

  }


  if (q->version >= 0x0100)
  {
    i = orca_sprintf(buffer, "HTTP/1.1 %u\r\n", status);
    TCPIPWriteTCP(ipid, buffer, i, false, false);

    tiToday2GMTString(tiBuffer);
    i = orca_sprintf(buffer, "Date: %b\r\n", tiBuffer);
    TCPIPWriteTCP(ipid, buffer, i, false, false);

    #undef xstr
    #define xstr "Server: SilverPlatter/1.1 (IIgs)\r\n"
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
    if (extralen) 
      TCPIPWriteTCP(ipid, extra, extralen, false, false);                                    
    TCPIPWriteTCP(ipid, "\r\n", 2, false, false);
  }
}
