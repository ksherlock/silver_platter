// process an error message.

#pragma noroot
#pragma lint -1
#pragma optimize -1
#pragma debug 0x8000

#include <Memory.h>
#include <Resources.h>

#include <gsos.h>
#include <tcpip.h>
#include <timetool.h>

#include "server.h"
#include "config.h"

#include "http.h"


extern int orca_sprintf(char *, const char *, ...);


           

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
char *extra;

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

  i = 0;
  extra = NULL;

  if ((error == 405) || (error == 501))
  {
    if (fWebDav)
    {
      #undef xstr
      #define xstr \
      "Allow: OPTIONS, GET, HEAD, PUT, PROPFIND, MKCOL, LOCK, UNLOCK\r\n"
      i = sizeof(xstr) -1;
      extra = xstr;
    }
    else
    {
      #undef xstr
      #define xstr \
      "Allow: OPTIONS, GET, HEAD, PUT\r\n"
      i = sizeof(xstr) -1;
      extra = xstr;
    }	
  }
  

  SendHeader(q, error, len, NULL, "text/html", extra, i);


  if (q->command != CMD_HEAD)
  {
  	WriteData(q, cp, len);
  	WriteData(q, NULL, 0);
  }
  if (h) ReleaseResource(3, rTextBlock, error);
  
  q->state = STATE_CLOSE;

  return error;
}



/* remap a gs/os error to a HTTP error */
Word RemapError(Word e)
{
Word err;

	switch(e)
	{
	case devNotFound:
	case pathNotFound:
	case volNotFound:
		err = RC_CONFLICT;
		break;
	case fileNotFound:
		err = RC_NOT_FOUND;
		break;
	case volumeFull:
	case volDirFull:
		err = RC_INSUFFICIENT_STORAGE;
		break;
	case invalidAccess:
		err = RC_FORBIDDEN;
		break;
	default:
		err = RC_INTERNAL_SERVER_ERROR;
	}
	
	return err;
}
