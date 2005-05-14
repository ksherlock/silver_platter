// process an error message.

#pragma noroot
#pragma lint -1
#pragma optimize -1

#include <Memory.h>
#include <Resources.h>

#include <tcpip.h>
#include <timetool.h>

#include "server.h"
#include "config.h"

extern int orca_sprintf(char *, const char *, ...);

#define is_info(e)		(e >= 100 && e < 200)
#define is_success(e)		(e >= 200 && e < 300)
#define is_redirect(e)		(e >= 300 && e < 400)
#define is_client_error(e)	(e >= 400 && e < 500)
#define is_server_error(e)	(e >= 500 && e < 600)

#define is_error(e)		(e >= 400 && e < 600)
           

static char tiBuffer[38];

static char defaultErr[] =
"<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\r"
"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" "
"\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\r"
"<html xmlns=\"http://www.w3.org/1999/xhtml\">\r"
"<head>\r"
"<title>500 Internal Server Error</title>\r"
"</head>\r"
"<body>\r"
"<h1>Internal Server Error</h1>\r"
"<p>The server encountered an internal error and was unable "
"to complete your request.</p>"
"</body>\r"
"</html>\r";

Word ProcessError(Word error, struct qEntry *q)
{
Word i;
Handle h;
Word ipid;
Word len;
char *cp;

  ipid = q->ipid;

  q->flags &= ~FLAG_KA;

  h = LoadResource(rTextBlock, error);
  if (_toolErr)
  {
    h = NULL;
    len = sizeof(defaultErr) -1;
    error = 500;
    cp = defaultErr;
  }
  else
  {
    len = GetHandleSize(h);
    HLock(h);
    cp = *h;
  }

  if (q->version >= 0x0100)
  {
    SendHeader(q, error, len, NULL, "text/html", false);

    if (error == 405 || error == 501)
    {
      #undef xstr
      #define xstr "Allow: GET, HEAD, PUT\r\n"
      TCPIPWriteTCP(ipid, xstr, sizeof(xstr) - 1, false, false);
    }

    TCPIPWriteTCP(ipid, "\r\n", 2, false, false);
  }

  #undef MIN
  #define MIN(a,b) (a) < (b) ? (a) : (b)

  if (q->command != CMD_HEAD)
  {
    if (q->flags & FLAG_CHUNKED)
    {
      int i = orca_sprintf(buffer, "%x\r\n", len);
      TCPIPWriteTCP(ipid, buffer, i, false, false);
    }

    do
    {
      Word i = MIN(len, fMTU);

      TCPIPWriteTCP(ipid, cp, i, false, false);
      TCPIPPoll();
              
      cp += i;
      len -= i;
    } while (len);

    //TCPIPWriteTCP(ipid, cp, len, false, false);

    if (q->flags & FLAG_CHUNKED)
      TCPIPWriteTCP(ipid, "\r\n0\r\n\r\n", 7, false, false);

  }
  if (h) ReleaseResource(3, rTextBlock, error);
  
  q->state = STATE_CLOSE;

  return error;
}
