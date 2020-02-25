#pragma lint - 1
#pragma optimize - 1
#pragma noroot
#pragma debug 0x8000

#include <MiscTool.h>
#include <gsos.h>
#include <tcpip.h>

#include <stdarg.h>
#include <stdio.h>

#include "timetool.h"

#include "config.h"
#include "server.h"

#define B(x) x->length, x->text
#define PRIB ".*s"

static char tiBuffer[38];
static char buffer16[16];

extern Word logfd;

int fdprintf(word refNum, const char *fmt, ...) {

  static char buffer[256];
  IORecGS dcb;
  int size;
  va_list ap;

  va_start(ap, fmt);
  size = vsnprintf(buffer, sizeof(buffer), fmt, ap);
  va_end(ap);
  if (size < 0 || size >= sizeof(buffer) - 1)
    return -1;

  dcb.pCount = 4;
  dcb.refNum = refNum;
  dcb.dataBuffer = buffer;
  dcb.requestCount = size;
  WriteGS(&dcb);
  return size;
}

#define SZ(x) (sizeof(x) / sizeof(x[0]))
static const char *status_name(unsigned status) {

  static const char *_100[] = {
    "Continue", "Switching Protocols", "Processing", "Early Hints"
  };
  static const char *_200[] =  {
    "OK", "Created", "Accepted", "Non-Authoritative Information",
    "No Content", "Reset Content", "Partial Content", "Multi-Status",
    "Already Reported"
    /* [226] "IM Used" missing */
  };

  static const char *_300[] =  {
    "Multiple Choices", "Moved Permanently", "Found", "See Other",
    "Not Modified", "Use Proxy", "Switch Proxy", "Temporary Redirect",
    "Permanent Redirect"
  };

  static const char *_400[] =  {
    "Bad Request", "Unauthorized", "Payment Required", "Forbidden",
    "Not Found", "Method Not Allowed", "Not Acceptable", "Proxy Authentication Required",
    "Request Timeout", "Conflict", "Gone", "Length Required", 
    "Precondition Failed", "Payload Too Large", "URI Too Long", "Unsupported Media Type",
    "Range Not Satisfiable", "Expectation Failed", "I'm A Teapot", "",
    "", "Misdirected Request", "Unprocessable Entity", "Locked",
    "Failed Dependency", "Too Early", "Upgrade Required", "",
    "Precondition Required", "Too Many Requests", "", "Request Header Fields Too Large"
    /* [451] "Unavailable For Legal Reasons" missing */
  };

  static const char *_500[] =  {
    "Internal Server Error", "Not Implemented", "Bad Gateway", "Service Unavailable",
    "Gateway Timeout", "HTTP Version Not Supported", "Variant Also Negotiates", "Insufficient Storage",
    "Loop Detected", "", "Not Extended", "Network Authentication Required"

  };

  if (status >= 100 && status < 100 + SZ(_100)) return _100[status - 100];
  if (status >= 200 && status < 200 + SZ(_200)) return _200[status - 200];
  if (status >= 300 && status < 300 + SZ(_300)) return _300[status - 300];
  if (status >= 400 && status < 400 + SZ(_400)) return _400[status - 400];
  if (status >= 500 && status < 500 + SZ(_500)) return _500[status - 500];

  return "";
}

extern void tiTimeRec2GMTString(const TimeRec *, char *);

void SendHeader(struct qEntry *q, Word status, LongWord size,
                const TimeRec *modTime, const char *mimeString,
                const char *extra, word extralen) {

  // must be broken out to prevent \x from grabbing too much.
  /* clang-format off */
  static const char months[] = "\x03" "Jan" "\x03" "Feb" "\x03" "Mar"
                               "\x03" "Apr" "\x03" "May" "\x03" "Jun"
                               "\x03" "Jul" "\x03" "Aug" "\x03" "Sep"
                               "\x03" "Oct" "\x03" "Nov" "\x03" "Dec";
  /* clang-format on */


  static char buffer[128];

  Word ipid = q->ipid;
  Word flags = q->flags;
  Word i;

  if (fLog && logfd) {

    static char buffer6[6];
    static tiPrefRec ti;

    GSString255Ptr req = q->request ? q->request : (GSString255Ptr) "\x00\x00";
    TimeRec tr;

    TCPIPConvertIPToASCII(q->ip, buffer16, 0);

    ti.pCount = 1;
    tiGetTimePrefs(&ti);
    tiOffset2TimeZoneString(buffer6, ti.secOffset, 0);

    tr = ReadTimeHex();

    if (size != -1)
      fdprintf(logfd,
               "%b - - [%02u/%b/%04u:%02u:%02u:%02u %b] \"%" PRIB "\" %u %lu\r",
               buffer16, tr.day + 1, months + (tr.month << 2), 1900 + tr.year,
               tr.hour, tr.minute, tr.second, buffer6, B(req), status, size);
    else
      fdprintf(logfd,
               "%b - - [%02d/%b/%04u:%02u:%02u:%02u %b] \"%" PRIB "\" %u -\r",
               buffer16, tr.day + 1, months + (tr.month << 2), 1900 + tr.year,
               tr.hour, tr.minute, tr.second, buffer6, B(req), status);
  }

  if (q->version >= 0x0100) {
    i = sprintf(buffer, "HTTP/1.1 %u %s\r\n", status, status_name(status));
    TCPIPWriteTCP(ipid, buffer, i, false, false);

    tiToday2GMTString(tiBuffer);
    i = sprintf(buffer, "Date: %b\r\n", tiBuffer);
    TCPIPWriteTCP(ipid, buffer, i, false, false);

#undef xstr
#define xstr "Server: SilverPlatter/1.2 (IIgs)\r\n"
    TCPIPWriteTCP(ipid, xstr, sizeof(xstr) - 1, false, false);

    if (modTime) {
      tiTimeRec2GMTString(modTime, tiBuffer);
      i = sprintf(buffer, "Last-Modified: %b\r\n", tiBuffer);
      TCPIPWriteTCP(ipid, buffer, i, false, false);
    }

    if (flags & FLAG_CHUNKED) {
#undef xstr
#define xstr "Transfer-Encoding: chunked\r\n"
      TCPIPWriteTCP(ipid, xstr, sizeof(xstr) - 1, false, false);
    } else if (size != -1) {
      i = sprintf(buffer, "Content-Length: %lu\r\n", (LongWord)size);
      TCPIPWriteTCP(ipid, buffer, i, false, false);
    }

    if (flags & FLAG_KA) {
#undef xstr
#define xstr "Connection: keep-alive\r\n"
      TCPIPWriteTCP(ipid, xstr, sizeof(xstr) - 1, false, false);
#undef xstr
#define xstr "Keep-Alive: timeout=30\r\n"
      TCPIPWriteTCP(ipid, xstr, sizeof(xstr) - 1, false, false);
    } else {
#undef xstr
#define xstr "Connection: close\r\n"
      TCPIPWriteTCP(ipid, xstr, sizeof(xstr) - 1, false, false);
    }

    if (status == 206) {
      i = sprintf(buffer, "Content-Range: bytes %lu-%lu/%lu\r\n", q->range[0],
                  q->range[1], q->contentLength);
      TCPIPWriteTCP(ipid, buffer, i, false, false);
    }

    if (mimeString) {
      i = sprintf(buffer, "Content-Type: %s\r\n", mimeString);
      TCPIPWriteTCP(ipid, buffer, i, false, false);
    }
    //
    if (extralen)
      TCPIPWriteTCP(ipid, extra, extralen, false, false);
    TCPIPWriteTCP(ipid, "\r\n", 2, false, false);
  }
}
