/*
 * handle background server tasks and such.
 */

#pragma noroot
#pragma lint -1
#pragma optimize -1
#pragma debug 0x8000

#include <types.h>
#include <control.h>
#include <gsos.h>
#include <IntMath.h>
#include <memory.h>
#include <MiscTool.h>
#include <tcpip.h>

#include <string.h>
#include <ctype.h>

//#include <dfa.h>

#include "httpnda.h"
#include "server.h"
#include "config.h"
#include "rez.h"
#include "toolbox.h"

#include "kmalloc.h"

extern int orca_sprintf(char *, const char *, ...);

#undef MIN
#define MIN(a,b) (a) < (b) ? (a) : (b)


#define DEBUG 1


// dfa table for methods
//extern Word methods[];

Word fActive;
Word fUsed;

Word logfd;


extern WindowPtr MyWindow;


struct qEntry queue[16];
char buffer[4096];


// appends h1 onto h2

void AppendHandle(Handle h1, Handle h2)
{
longword s1, s2;

  if (!h1 || !h2) return;

  s1 = GetHandleSize(h1);
  s2 = GetHandleSize(h2);

  if (s2)
  {
    HUnlock(h1);
    SetHandleSize(s1 + s2, h1);
    if (_toolErr) return;
    HLock(h1);
    HandToPtr(h2, *h1 + s1, s2);
  }
}

// returns a handle,
// 0 == blank line
// -1 == for no line
// bit 31 set == dispose the buffer.
// line may be delimited by \r, \n, \r\n, or \n\r... <sigh>

Handle GetLine(Handle h)
{
Word hsize;
char c;
Word i;
char *cp;
Handle ret;
Word length;


  if (!h) return (Handle)-1;

  hsize = GetHandleSize(h);
  if (!hsize) return (Handle)-1;

  HLock(h);
  cp = *h;
  i = 0;
  length = 0;

  while (i < hsize)
  {
    c = *cp++;
    i++;

    if (c == '\r')
    {
      length = i;
      if (i < hsize && *cp == '\n')
      {
        i++;
        cp++;
      }
      break;
    }
    if (c == '\n')
    {
      length = i;
      if (i < hsize && *cp == '\r')
      {
        i++;
        cp++;
      }
      break; 
    }
  }
  if (length == 0) return (Handle)-1;

  if (length == 1) // actually a blank line
  {
    ret = (Handle)0;
  }
  else
  {
    ret = NewHandle(length, MyID | 0x0d00, attrLocked, 0);
    if (!_toolErr)
    {
    char *cp;
      HandToHand(h, ret, length - 1);

      cp = *ret;
      cp[length - 1] = 0; // null terminate for convenience.
    }
  }

  if (hsize == i) ret = (Handle)(0x80000000 | (LongWord)ret);
  else
  {
    PtrToHand(cp, h, hsize - i);
    SetHandleSize(hsize - i, h);
  }
  return ret;
}


// check for ?asingle, ?html and remove from the string.
Word ScanCGI(GSString255Ptr g)
{
Word i;
char c;

  for (i = g->length; ; )
  {
    c = g->text[--i];
    if (c == '/') break;
    if (c == '?')
    {
      // g->text is null-terminated, so strcmp is ok
      if (!strcmp("?applesingle", &g->text[i]))
      {
        g->text[i] = 0;
        g->length = i;
        return CGI_APPLESINGLE;
      }
      if (!strcmp("?html", &g->text[i]))
      {
        g->text[i] = 0;
        g->length = i;
        return CGI_HTML;
      }
    } // c == '?'
  }
  return 0;
}



// scan for headers we recognize.
void ScanHeader(char *cp, struct qEntry *q)
{
char c;
int i;
Handle h;
GSString255Ptr host;

  #undef xstr
  #define xstr "Keep-Alive:"
  if (!strincmp(xstr, cp, sizeof(xstr) - 1))
  {
    q->flags |= FLAG_KA;
    return;
  }

  #undef xstr
  #define xstr "Connection: close"
  if (!strincmp(xstr, cp, sizeof(xstr) - 1))
  {
    q->flags &= ~FLAG_KA;
    return;
  }        

  #undef xstr
  #define xstr "Connection: keep-alive"
  if (!strincmp(xstr, cp, sizeof(xstr) - 1))
  {
    q->flags |= FLAG_KA;
    return;
  }

  #undef xstr
  #define xstr "Host:"
  if (!strincmp(xstr, cp, sizeof(xstr) - 1))
  {
    cp += sizeof(xstr) - 1;

    // move past any leading whitespace
    while (isspace(*cp)) cp++;
    if (!*cp) return;

    // find the length...
    i = 0;
    while (c = cp[i] && !isspace(c)) i++;
    if (!i) return;

    host = kmalloc(i + 3);
    if (host)
    {
      q->host = host;
      host->length = i;
      BlockMove(cp, host->text, i);
      host->text[i] = 0;
    }

    return;
  }

  // PUT - Content-Length & Content-Type
  if (q->command == CMD_PUT)
  {
    // Content-Length: <space> <digit>+
    #undef xstr
    #define xstr "Content-Length:"
    if (!strincmp(xstr, cp, sizeof(xstr) - 1))
    {
      cp += sizeof(xstr) - 1;

      // move past any leading whitespace
      while (isspace(*cp)) cp++;
      if (!*cp) return;

      i = 0;

      while (isdigit(cp[i])) i++;

      q->filesize = Dec2Long(cp, i, 0);

      return;
    }


    // set the FLAG_TEXT if the type is text/*
    #undef xstr
    #define xstr "Content-Type:"
    if (!strincmp(xstr, cp, sizeof(xstr) - 1))
    {

      cp += sizeof(xstr) - 1;

      // move past any leading whitespace
      while (isspace(*cp)) cp++;
      if (!*cp) return;
      if (!strincmp("text/", cp, 5)) q->flags |= FLAG_TEXT;

      return;
    }

  } // q->command == CMD_PUT

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

Word cmd = -1;


  q->command = -1;

  // format: <method> <space>+ <path> <space>+ (HTTP/\d.\d)?

  //match = MatchDFA(methods, cp, &q->command);
  //if (match) cp += match;

  if (!strincmp("GET ", cp, 4))
  {
    cmd = CMD_GET;
    cp += 4;
  }
  else if (!strincmp("HEAD ", cp, 5))
  {
    cmd = CMD_HEAD;
    cp += 5;
  }
  else if (!strincmp("OPTIONS ", cp, 8))
  {
    cmd = CMD_OPTIONS;
    cp += 8;
  }
  else if (!strincmp("POST ", cp, 5))
  {
    cmd = CMD_POST;
    cp += 5;
  }
  else if (!strincmp("PUT ", cp, 4))
  {
    cmd = CMD_PUT;
    cp += 4;
  }
  else if (!strincmp("DELETE ", cp, 7))
  {
    cmd = CMD_DELETE;
    cp += 7;
  }
  else if (!strincmp("TRACE ", cp, 6))
  {
    cmd = CMD_TRACE;
    cp += 6;
  }
  else if (!strincmp("CONNECT ", cp, 8))
  {
    cmd = CMD_CONNECT;
    cp += 8;
  }
                     
  else
  {
    // skip past the offending command.
    while ((c = *cp) && !isspace(c)) cp++;
    cmd = -1;
  }

  while (isspace(*cp)) cp++;

  // URI: should be /fully/qualified/path

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
    path = kmalloc(len + 3);
    if (!path) return;

    q->pathname = path;

    path->length = len;
    BlockMove(cp, path->text, len);
    path->text[len] = 0;

    cp += len;


    // now demangle %xx codes.
    // since the demanglesd size will always be less, we're safe.
    i = j = 0;
    while (c = path->text[i++])
    {
      if (c == '%' && isxdigit(path->text[i]) && isxdigit(path->text[i + 1]))
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

      fullpath = kmalloc(len + 3);

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
      GSString255Ptr fullpath;
      fullpath = kmalloc(len + 3);
      if (fullpath)
      {
        q->fullpath = fullpath;
        BlockMove((Pointer)path, (Pointer)fullpath, len + 3);
      }
    }
  } // if (len)

  q->version = 0x0009;  // 0.9
  q->command = cmd;

  if (!*cp) return;

  while (isspace(*cp)) cp++;

  // http version....
  if (!strincmp("HTTP/", cp, 5))
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


void ReleaseQ(struct qEntry *q)
{
Word CloseDCB[2];

  if (q->buffer)
  {
    DisposeHandle(q->buffer);
    q->buffer = 0;
  }

  // new fangled pointers.
  if (q->request)
  {
    kfree(q->request);
    q->request = NULL;
  }
  if (q->host)
  {
    kfree(q->host);
    q->host = NULL;
  }

  if (q->pathname)
  {
    kfree(q->pathname);
    q->pathname = NULL;
  }

  if (q->fullpath)
  {
    kfree(q->fullpath);
    q->fullpath = NULL;
  }

  if (q->workHandle)
  {
    HUnlock(q->workHandle);
    SetHandleSize(0, q->workHandle);
  }


  if (q->fd)                       
  {
    CloseDCB[0] = 1;
    CloseDCB[1] = q->fd;
    CloseGS(CloseDCB);
  }
  if (q->rfd)                       
  {
    CloseDCB[0] = 1;
    CloseDCB[1] = q->rfd;
    CloseGS(CloseDCB);
  }

  q->state = 0;
  q->ipid = 0;
  q->fd = 0;
  q->rfd = 0;
  q->command = 0;
  q->version = 0;
  q->flags = 0;
  q->moreFlags = 0;
  q->tick = 0;
  q->filesize = 0;
}    

extern Word CreateLog(void);

Word StartServer(void)
{
  fActive = 0;
  fUsed = 0;

  kmstartup(MyID | 0x0f00);

  memset(queue, 0, sizeof(queue));

  Ipid = TCPIPLogin(MyID, 0, 0, 0, 64);
#if DEBUG
  if (_toolErr)
      InsertString(
        orca_sprintf(buffer, "TCPIPLogin -- %x\r", _toolErr),
        buffer);
#endif

  TCPIPSetSourcePort(Ipid, fPort);
#if DEBUG
  if (_toolErr)
      InsertString(
        orca_sprintf(buffer, "TCPIPSetSourcePort -- %x\r", _toolErr),
        buffer);
#endif
                                    
  TCPIPListenTCP(Ipid);
#if DEBUG
  if (_toolErr)
      InsertString(
        orca_sprintf(buffer, "TCPIPListenTCP -- %x\r", _toolErr),
        buffer);
#endif
                                          
  FlagHTTP = true;

  logfd = CreateLog();


  #undef xstr
  #define xstr "Server started\r"
  InsertString(sizeof(xstr) - 1, xstr);

  if (MyWindow) SetCtlTextByID(MyWindow, CtrlCount, 1, (Ref)"0");
  if (MyWindow) SetCtlTextByID(MyWindow, CtrlCount, 1, (Ref)"0");

  return true;
}

Word StopServer(void)
{
Word mask;
struct qEntry *q;
Word CloseDCB[2];

  if (logfd)
  {
    CloseDCB[0] = 1;
    CloseDCB[1] = logfd;
    CloseGS(CloseDCB);
  }

  // abort any open connections
  if (fUsed) for (mask = 1, q = queue; mask; mask <<= 1, q++)
  {
    if (fUsed & mask == 0) continue; // nobody home.
    TCPIPAbortTCP(q->ipid);
    TCPIPLogout(q->ipid);
    ReleaseQ(q);


    if (q->buffer)
    {
      DisposeHandle(q->buffer);
      q->buffer = NULL;
    }
    if (q->workHandle)
    {
      DisposeHandle(q->workHandle);
      q->workHandle = NULL;
    }

  }
  fUsed = 0;
  fActive = 0;

  TCPIPCloseTCP(Ipid);
  TCPIPLogout(Ipid);

  FlagHTTP = false;
  Ipid = 0;

  kmshutdown();


  #undef xstr
  #define xstr "Server stopped\r"
  InsertString(sizeof(xstr) - 1, xstr);

  if (MyWindow) SetCtlValueByID(0, MyWindow, CtrlTherm);
  if (MyWindow) SetCtlTextByID(MyWindow, CtrlCount, 1, (Ref)"");
      
  return true;
}

// close all connections waiting to close.
void ResetServer(void)
{
word mask;
struct qEntry *q;
Word ipid;

  // abort any open connections
  if (fUsed) for (mask = 1, q = queue; mask; mask <<= 1, q++)
  {
    if (fUsed & mask == 0) continue; // nobody home.

    if (q->state != STATE_LOGOUT) continue;
    ipid = q->ipid;
    TCPIPAbortTCP(ipid);
    TCPIPLogout(ipid);
    ReleaseQ(q);
    fActive--;
    fUsed &= (~mask);
  } 

  if (MyWindow) SetCtlValueByID(fActive, MyWindow, CtrlTherm);
  if (MyWindow)
  {
    static char buffer[6];
    orca_sprintf(buffer, "%u", fActive);
    SetCtlTextByID(MyWindow, CtrlCount, 1, (Ref)buffer);
  }

  #undef xstr
  #define xstr "Server reset\r"
  InsertString(sizeof(xstr) - 1, xstr);

}

void Server(void)
{
static srBuff srBuffer;
static rlrBuff rlrBuffer;
static rrBuff rrBuffer;

static char buffer16[16];

Word oldActive = fActive;

Word i;
Word mask;
Word terr;

struct qEntry *q;
Word oldPrefs;

  // don't prompt for missing disks...
  oldPrefs = DoSysPrefs(0xffff, 0x6000);

  TCPIPPoll(); 
  
  // process any pending requests.
  if (fUsed) for (mask = 1, q = queue; mask; mask <<= 1, q++)
  {
    Longword tick;
    Word ipid;

    if ((fUsed & mask) == 0) continue; // nobody home.

    TCPIPPoll();

    ipid = q->ipid;
    terr = TCPIPStatusTCP(ipid, &srBuffer);

    if (_toolErr)
    {
      ReleaseQ(q);
      fActive--;
      fUsed &= (~mask);
      continue;
    }

    if (srBuffer.srState == TCPSCLOSED)
    {
      #ifdef DEBUG
      i = orca_sprintf(buffer, "TCPIPLogout(%d)\r", ipid);
      InsertString(i, buffer);
      #endif

      TCPIPLogout(ipid);
      ReleaseQ(q);        
      fActive--;
      fUsed &= (~mask);
      continue;
    }

    // timedwait won't close immediately and may cause the tcp table to fill up.
    if (fAbort &&
      (srBuffer.srState == TCPSTIMEWAIT || srBuffer.srState == TCPSCLOSING))
    {
      #ifdef DEBUG
      i = orca_sprintf(buffer, "TCPIPAbort(%d)\r", ipid);
      InsertString(i, buffer);
      #endif

      TCPIPAbortTCP(ipid);                  
      TCPIPLogout(ipid);
      ReleaseQ(q);        
      fActive--;
      fUsed &= (~mask);
      continue;
    }                     

    tick = GetTick();

    switch(q->state)
    {
    // waiting for the connection to establish....
    case STATE_ESTABLISH:

      if (srBuffer.srState == TCPSESTABLISHED)
        q->state = STATE_READ;
      else if (tick > q->tick)
      {
        #ifdef DEBUG
        i = orca_sprintf(buffer, "TCPIPCloseTCP(%d) [establish timeout]\r", ipid);
        InsertString(i, buffer);
        #endif

        TCPIPCloseTCP(ipid);        
        ReleaseQ(q);
        q->ipid = ipid;
        q->state = STATE_LOGOUT;
      }
      if (q->state != STATE_READ) break;

    // reading the request string...
    case STATE_READ:
      {
      Word j;
      Word done;
      Handle h;

	if (tick > q->tick)
	{
          #ifdef DEBUG
          i = orca_sprintf(buffer, "TCPIPCloseTCP(%d) [read timeout]\r", ipid);
          InsertString(i, buffer);
          #endif

          TCPIPCloseTCP(ipid);
          ReleaseQ(q);
          q->ipid = ipid;
          q->state = STATE_LOGOUT;
          break;
	}

	// read any pending data...
	if (srBuffer.srRcvQueued)
	{
          terr = TCPIPReadTCP(ipid, 2, (Ref)0, srBuffer.srRcvQueued , &rrBuffer);
          if (!q->buffer) q->buffer = rrBuffer.rrBuffHandle;
          else
          {
            AppendHandle(q->buffer, rrBuffer.rrBuffHandle);
            DisposeHandle(rrBuffer.rrBuffHandle);
          }
	}
	// now try splitting it into a line.
        if (q->buffer)
        {
        Boolean done = false;
        char *cp;

          do
          {
            h = GetLine(q->buffer);

            if (h == (Handle)-1) break; // no line

            if (0x80000000 & (LongWord)h)  // dispose the buffer...
            {
              DisposeHandle(q->buffer);
              q->buffer = NULL;

              h = (Handle) (0x7fffffff & (LongWord)(h));
              done = true;
            }

            if (h == (Handle)0) break;  // blank line

            cp = *h;
            if (q->command == 0)
            {
              GSString255Ptr req;

              Word i = GetHandleSize(h);
              ScanMethod(cp, q);
              req = kmalloc(2 + i);

              if (req)
              {
                q->request = req;
                req->length = i - 1; // includes null terminator.
                HandToPtr(h, req->text, i);
              }

            }
            else ScanHeader(cp, q);


            DisposeHandle(h);
          } while (!done);

          // if h == 0, all headers received.
          if (h == (Handle)0)
          {
            q->ip = srBuffer.srDestIP;

            switch(q->command)
            {
            case CMD_GET:
            case CMD_HEAD:
              switch (q->moreFlags)
              {
              case CGI_APPLESINGLE:
                terr = AppleSingle(q);
                break;
              default:
                terr = ProcessFile(q);
              }
              break;

            case CMD_PUT:
              terr = ProcessPut(q);
              break;

            case 0xfff:
              terr = ProcessError(501, q);
              break;

            default:
              terr = ProcessError(405, q);
            }
          }
        }
      }
      break; // case: STATE_READ

    case STATE_WRITE:
    case STATE_ASINGLE_1:  // apple single, data fork
    case STATE_ASINGLE_2:  // apple single, resource fork
      {
      IORecGS ReadDCB;

        ReadDCB.pCount = 4;
        ReadDCB.refNum = q->state == STATE_ASINGLE_2 ? q->rfd : q->fd;
        ReadDCB.dataBuffer = buffer;
        ReadDCB.requestCount = 4096;
        ReadGS(&ReadDCB);
        if ((terr = _toolErr) == 0)
        {
        char *cp = buffer;
        Word total = (Word)ReadDCB.transferCount;

          if (q->flags & FLAG_CHUNKED)
          {
            int i = orca_sprintf(buffer16, "%x\r\n", total);
            TCPIPWriteTCP(ipid, buffer16, i, false, false);
          }

          //TCPIPWriteTCP(ipid, buffer, ReadDCB.transferCount, false, false);
	  do
	  {
	    Word i = MIN(total, fMTU);

	    TCPIPWriteTCP(ipid, cp, i, false, false);
            TCPIPPoll();
	    cp += i;
	    total -= i;
	  } while (total);


          if (q->flags & FLAG_CHUNKED)
            TCPIPWriteTCP(ipid, "\r\n",2, false, false);

          #ifdef DEBUG
          i = orca_sprintf(buffer, "TCPIPWriteTCP(%d) [%d bytes sent]\r",
            ipid, (Word)ReadDCB.transferCount);
          InsertString(i, buffer);
          #endif
        }                                   
        else
        {
          if (terr == eofEncountered)
          {
            // if apple single, now send the resource fork.
            if (q->state == STATE_ASINGLE_1 && q->rfd)
            {
              q->state = STATE_ASINGLE_2;
            }
            else
            {
              if (q->flags & FLAG_CHUNKED)
                TCPIPWriteTCP(ipid, "0\r\n\r\n", 5, false, false);
              q->state = STATE_CLOSE;
            }
          }
          else // read error - just close and be done with it.
          {
            #ifdef DEBUG
            i = orca_sprintf(buffer, "TCPIPCloseTCP(%d) [read error]\r", ipid);
            InsertString(i, buffer);
            #endif

            TCPIPCloseTCP(ipid);
            ReleaseQ(q);
            q->ipid = ipid;
            q->state = STATE_LOGOUT;
            break;
          }
        }
      }
      break;

      // read any incoming data, dump it to the file.
    case STATE_PUT:
      {
      IORecGS WriteDCB;

	// read any pending data...
	if (srBuffer.srRcvQueued)
	{
        Handle h = q->workHandle;

          terr = TCPIPReadTCP(ipid, 1, (Ref)h,
            srBuffer.srRcvQueued , &rrBuffer);

          HLock(h);
	  WriteDCB.pCount = 4;
	  WriteDCB.refNum = q->fd;
	  WriteDCB.dataBuffer = *h;
	  WriteDCB.requestCount = rrBuffer.rrBuffCount;

	  WriteGS(&WriteDCB);

	  if (_toolErr)
          {
            ProcessError(500,q);
            break;
          }

          if (q->filesize)
          {
            q->filesize -= rrBuffer.rrBuffCount;
            if (q->filesize <= 0)
            {
              SendHeader(q, q->flags & FLAG_CREATE ? 201 : 204 ,
                0, NULL, NULL, true);
              q->state = STATE_CLOSE;

            }
          } // q->filesize.
	}
      }
      break;
        
    case STATE_CLOSE:
      {
        if (srBuffer.srSndQueued == 0)
        {

          if (q->flags & FLAG_KA)
          {
            Handle h;
            h = q->buffer;
            q->buffer = NULL;
            ReleaseQ(q);
            // reset these values.
            q->ipid = ipid;
            q->buffer = h;
            q->state = STATE_READ;
            q->tick = GetTick() + 60 * 30; // 30 seconds.
          }
          else
          {
            #ifdef DEBUG
            i = orca_sprintf(buffer, "TCPIPCloseTCP(%d)\r", ipid);
            InsertString(i, buffer);
            #endif

            TCPIPCloseTCP(ipid);
            ReleaseQ(q);
            q->ipid = ipid;
            q->state = STATE_LOGOUT;
          }                      
        }
      }
      break; // STATE_CLOSE
      //case STATE_LOGOUT: /* handled at start */

    } // case (q->state)
  }
  // now check for any new ones.
  if (fUsed != 0xffff)
  {
    Word child;
    Word j;

    TCPIPPoll();
    child = TCPIPAcceptTCP(Ipid, 0);
    if (!_toolErr)
    {
    Word err;

      // find an open slot...
      for (mask = 1, q = queue; fUsed & mask; mask <<= 1, q++) ;

      //memzero(q, sizeof(struct qEntry));

      q->ipid = child;
      TCPIPStatusTCP(child, &srBuffer);
      if (srBuffer.srState == TCPSESTABLISHED)
	q->state = STATE_READ;
      else q->state = STATE_ESTABLISH;

      q->tick = GetTick() + 60 * 60;

      // allocate any handles if needed.
      err = 0;

      //if (!q->buffer) q->buffer = NewHandle(0, MyID | 0x0d00, attrNoSpec, 0);
      //err |= _toolErr;


      if (!q->workHandle) q->workHandle = NewHandle(0, MyID | 0x0d00, attrNoSpec, 0);
      err |= _toolErr;


      if (err)
      {
#if DEBUG
        InsertString(
          orca_sprintf(buffer, " NewHandle -- %x\r", err),
          buffer);
#endif
        TCPIPAbortTCP(child);
        TCPIPLogout(child);
        ReleaseQ(q);
      }
      else
      {
        fUsed |= mask;
        fActive++;
      }
    }
#if DEBUG
    else if (_toolErr != terrNOINCOMING)
    {
      InsertString(
        orca_sprintf(buffer, "TCPIPAcceptTCP -- %x\r", _toolErr),
        buffer);
    }
#endif
  }

  if (oldActive != fActive && MyWindow)
  {
    static char buffer[6];

    SetCtlValueByID(fActive, MyWindow, CtrlTherm);
    orca_sprintf(buffer, "%u", fActive);
    SetCtlTextByID(MyWindow, CtrlCount, 1, (Ref)buffer);
  }

  DoSysPrefs(0xffff, oldPrefs);
}
