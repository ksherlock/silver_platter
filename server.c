/*
 * handle background server tasks and such.
 */

#pragma noroot
#pragma lint - 1
#pragma optimize - 1
#pragma debug 0x8000

#include <IntMath.h>
#include <MiscTool.h>
#include <control.h>
#include <gsos.h>
#include <memory.h>
#include <tcpip.h>
#include <types.h>

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "http.h"
#include "httpnda.h"
#include "rez.h"
#include "server.h"
#include "toolbox.h"

#include "globals.h"
#include "pointer.h"

#define B(x) x->length, x->text
#define PRIB ".*s"

extern Word ProcessPropfind(struct qEntry *q);
extern Word ProcessOptions(struct qEntry *q);
extern Word ProcessMkcol(struct qEntry *q);
extern Word ProcessLock(struct qEntry *q);
extern Word ProcessUnlock(struct qEntry *q);

extern Word ReadData(struct qEntry *, void *, Word);

extern Word ScanCGI(GSString255Ptr);
extern void ScanHeader(char *, struct qEntry *);
extern void ScanMethod(char *, struct qEntry *);

#define DEBUG 1

extern Word ConvertCRLF(char *, Word);

Word fActive;
Word fUsed;

Word logfd;

extern WindowPtr MyWindow;

struct qEntry queue[16];
char buffer[4096];

// appends h1 onto h2
void AppendHandle(Handle h1, Handle h2) {
  longword s1, s2;

  if (!h1 || !h2)
    return;

  s1 = GetHandleSize(h1);
  s2 = GetHandleSize(h2);

  if (s2) {
    HUnlock(h1);
    SetHandleSize(s1 + s2, h1);
    if (_toolErr)
      return;
    HLock(h1);
    HandToPtr(h2, *h1 + s1, s2);
  }
}

// returns a handle,
// 0 == blank line
// -1 == for no line
// bit 31 set == dispose the buffer.
// line may be delimited by \r, \n, \r\n, or \n\r... <sigh>
Handle GetLine(struct qEntry *q, Boolean *done) {
  Word hsize;
  unsigned c;
  Word i;
  char *cp;
  Handle ret;
  Word length;
  Handle h;

  h = q->buffer;

  if (!h)
    return NULL;

  hsize = GetHandleSize(h);
  if (!hsize)
    return NULL;

  HLock(h);
  cp = *h;
  i = 0;
  length = 0;

  while (i < hsize) {
    c = cp[i++];

    if (c == '\r') {
      length = i;
      if (i < hsize && cp[i] == '\n') {
        i++;
      }
      break;
    }
    if (c == '\n') {
      length = i;
      if (i < hsize && cp[i] == '\r') {
        i++;
      }
      break;
    }
  }

  if (length == 0)
    return NULL;

  /* length include the first \r or \n terminator */
  --length;
  if (length == 0) {
    ret = (Handle)0; // actually a blank line;
    *done = true;
  } else {
    ret = NewHandle(length, MyID | 0x0d00, attrLocked, 0);
    if (!_toolErr) {
      char *cp;
      HandToHand(h, ret, length);

      cp = *ret;
      cp[length] = 0; // null terminate for convenience.
    }
  }

  /* 
   * could be optimized to return the old handle
   * instead of allocating and copying over
   */
  if (hsize == i) {
    DisposeHandle(h);
    q->buffer = NULL;
  }
  else {
    PtrToHand(cp + i, h, hsize - i);
    SetHandleSize(hsize - i, h);
  }
  return ret;
}

void ReleaseQ(struct qEntry *q) {
  Word CloseDCB[2];

  if (q->buffer) {
    DisposeHandle(q->buffer);
    q->buffer = 0;
  }

  // new fangled pointers.
  if (q->request) {
    DisposePointer(q->request);
    q->request = NULL;
  }
  if (q->host) {
    DisposePointer(q->host);
    q->host = NULL;
  }

  if (q->pathname) {
    ReleasePointer(q->pathname);
    q->pathname = NULL;
  }

  if (q->fullpath) {
    ReleasePointer(q->fullpath);
    q->fullpath = NULL;
  }

  if (q->workHandle) {
    HUnlock(q->workHandle);
    SetHandleSize(0, q->workHandle);
  }

  if (q->fd) {
    CloseDCB[0] = 1;
    CloseDCB[1] = q->fd;
    CloseGS(CloseDCB);
  }
  if (q->rfd) {
    CloseDCB[0] = 1;
    CloseDCB[1] = q->rfd;
    CloseGS(CloseDCB);
  }

  q->state = 0;
  q->ipid = 0;
  q->fd = 0;
  q->rfd = 0;
  q->method = 0;
  q->version = 0;
  q->flags = 0;
  q->moreFlags = 0;
  q->tick = 0;
  q->contentlength = 0;
  q->depth = -1;
}

extern Word CreateLog(void);

static void Listening(void) {
  static char ip[20];

  TCPIPConvertIPToASCII(TCPIPGetMyIPAddress(), ip, 0);
  InsertString(sprintf(buffer, "Listening on %b:%u\r", ip, fPort), buffer);
}

Word StartServer(void) {
  fActive = 0;
  fUsed = 0;

  PointerStartUp(MyID | 0x0f00);

  memset(queue, 0, sizeof(queue));

  Ipid = TCPIPLogin(MyID, 0, 0, 0, 64);

#if DEBUG
  if (_toolErr)
    InsertString(sprintf(buffer, "TCPIPLogin -- %x\r", _toolErr), buffer);
#endif

  TCPIPSetSourcePort(Ipid, fPort);

#if DEBUG
  if (_toolErr)
    InsertString(sprintf(buffer, "TCPIPSetSourcePort -- %x\r", _toolErr),
                 buffer);
#endif

  TCPIPListenTCP(Ipid);

#if DEBUG
  if (_toolErr)
    InsertString(sprintf(buffer, "TCPIPListenTCP -- %x\r", _toolErr), buffer);
#endif

  FlagHTTP = true;

  logfd = CreateLog();

#undef xstr
#define xstr "Server started\r"
  InsertString(sizeof(xstr) - 1, xstr);
  Listening();

  if (MyWindow)
    SetCtlTextByID(MyWindow, CtrlCount, 1, (Ref) "0");
  if (MyWindow)
    SetCtlTextByID(MyWindow, CtrlCount, 1, (Ref) "0");

  return true;
}

Word StopServer(void) {
  Word mask;
  struct qEntry *q;
  Word CloseDCB[2];

  if (logfd) {
    CloseDCB[0] = 1;
    CloseDCB[1] = logfd;
    CloseGS(CloseDCB);
  }

  // abort any open connections
  if (fUsed)
    for (mask = 1, q = queue; mask; mask <<= 1, q++) {
      if (fUsed & mask == 0)
        continue; // nobody home.
      TCPIPAbortTCP(q->ipid);
      TCPIPLogout(q->ipid);
      ReleaseQ(q);

      if (q->buffer) {
        DisposeHandle(q->buffer);
        q->buffer = NULL;
      }
      if (q->workHandle) {
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

#undef xstr
#define xstr "Server stopped\r"
  InsertString(sizeof(xstr) - 1, xstr);

  if (MyWindow)
    SetCtlValueByID(0, MyWindow, CtrlTherm);
  if (MyWindow)
    SetCtlTextByID(MyWindow, CtrlCount, 1, (Ref) "");

  return true;
}

// close all connections waiting to close.
void ResetServer(void) {
  word mask;
  struct qEntry *q;
  Word ipid;

  // abort any open connections
  if (fUsed)
    for (mask = 1, q = queue; mask; mask <<= 1, q++) {
      if (fUsed & mask == 0)
        continue; // nobody home.

      if (q->state != STATE_LOGOUT)
        continue;
      ipid = q->ipid;
      TCPIPAbortTCP(ipid);
      TCPIPLogout(ipid);
      ReleaseQ(q);
      fActive--;
      fUsed &= (~mask);
    }

  if (MyWindow) {
    static char buffer[6];
    sprintf(buffer, "%u", fActive);
    SetCtlTextByID(MyWindow, CtrlCount, 1, (Ref)buffer);

    SetCtlValueByID(fActive, MyWindow, CtrlTherm);
  }

#undef xstr
#define xstr "Server reset\r"
  InsertString(sizeof(xstr) - 1, xstr);
  Listening();
}

void Server(void) {
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
  if (fUsed)
    for (mask = 1, q = queue; mask; mask <<= 1, q++) {
      Longword tick;
      Word ipid;

      if ((fUsed & mask) == 0)
        continue; // nobody home.

      TCPIPPoll();

      ipid = q->ipid;
      terr = TCPIPStatusTCP(ipid, &srBuffer);

      if (_toolErr) {
        ReleaseQ(q);
        fActive--;
        fUsed &= (~mask);
        continue;
      }

      if (srBuffer.srState == TCPSCLOSED) {
#ifdef DEBUG
        i = sprintf(buffer, "TCPIPLogout(%d)\r", ipid);
        InsertString(i, buffer);
#endif

        TCPIPLogout(ipid);
        ReleaseQ(q);
        fActive--;
        fUsed &= (~mask);
        continue;
      }

      // timedwait won't close immediately and may cause the tcp table to fill
      // up.
      if (fAbort && (srBuffer.srState == TCPSTIMEWAIT ||
                     srBuffer.srState == TCPSCLOSING)) {
#ifdef DEBUG
        i = sprintf(buffer, "TCPIPAbort(%d)\r", ipid);
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

      switch (q->state) {
      // waiting for the connection to establish....
      case STATE_ESTABLISH:

        if (srBuffer.srState == TCPSESTABLISHED)
          q->state = STATE_READ;
        else if (tick > q->tick) {
#ifdef DEBUG
          i = sprintf(buffer, "TCPIPCloseTCP(%d) [establish timeout]\r", ipid);
          InsertString(i, buffer);
#endif

          TCPIPCloseTCP(ipid);
          ReleaseQ(q);
          q->ipid = ipid;
          q->state = STATE_LOGOUT;
        }
        if (q->state != STATE_READ)
          break;

      // reading the request string...
      case STATE_READ: {
        Word j;
        Word done;
        Handle h;

        if (tick > q->tick) {
#ifdef DEBUG
          i = sprintf(buffer, "TCPIPCloseTCP(%d) [read timeout]\r", ipid);
          InsertString(i, buffer);
#endif

          TCPIPCloseTCP(ipid);
          ReleaseQ(q);
          q->ipid = ipid;
          q->state = STATE_LOGOUT;
          break;
        }

        // read any pending data...
        if (srBuffer.srRcvQueued) {
          terr = TCPIPReadTCP(ipid, 2, (Ref)0, srBuffer.srRcvQueued, &rrBuffer);
          if (!q->buffer)
            q->buffer = rrBuffer.rrBuffHandle;
          else {
            AppendHandle(q->buffer, rrBuffer.rrBuffHandle);
            DisposeHandle(rrBuffer.rrBuffHandle);
          }
        }
        // now try splitting it into a line.
        if (q->buffer) {
          Boolean done = false;
          char *cp;

          do {
            h = GetLine(q, &done);
            if (!h) break;

            cp = *h;
            if (q->method == 0) {
              GSString255Ptr req;

              Word i = GetHandleSize(h);

              ScanMethod(cp, q);
              req = NewPointer(2 + i);

              if (req) {
                q->request = req;
                req->length = i - 1; // includes null terminator.
                HandToPtr(h, req->text, i);
              }
            } else {
              ScanHeader(cp, q);
            }

            DisposeHandle(h);
          } while (!done);

          // if h == 0, all headers received.
          if (h == (Handle)0) {
            q->ip = srBuffer.srDestIP;

            if (q->error) {
              terr = ProcessError(q->error, q);
              break;
            }
            if (q->moreFlags == CGI_ERROR) {
              terr = ProcessError(HTTP_UNPROCESSABLE_ENTITY, q);
              break;
            }

            switch (q->method) {
            case CMD_OPTIONS:
              terr = ProcessOptions(q);
              break;

            case CMD_HEAD:
            case CMD_GET:
              terr = ProcessFile(q);
              break;

            case CMD_PUT:
              terr = ProcessPut(q);
              break;

            case CMD_PROPFIND:
              terr = ProcessPropfind(q);
              break;

            case CMD_MKCOL:
              terr = ProcessMkcol(q);
              break;

            case CMD_LOCK:
              terr = ProcessLock(q);
              break;

            case CMD_UNLOCK:
              terr = ProcessUnlock(q);
              break;

            case CMD_PROPPATCH:
            case CMD_COPY:
            case CMD_MOVE:
            case 0xffff:
              terr = ProcessError(501, q);
              break;

            default:
              terr = ProcessError(405, q);
            }
          }
        }
      } break; // case: STATE_READ

      case STATE_WRITE:
      case STATE_ASINGLE_1: // apple single, data fork
      case STATE_ASINGLE_2: // apple single, resource fork
      {
        Word count;

        IODCB.pCount = 4;
        IODCB.refNum = q->state == STATE_ASINGLE_2 ? q->rfd : q->fd;
        IODCB.dataBuffer = buffer;
        IODCB.requestCount = 4096;

        if (q->contentlength) {
          /* range-based */
          if (q->contentlength < 4096)
            IODCB.requestCount = q->contentlength;
        }

        ReadGS(&IODCB);

        count = (Word)IODCB.transferCount;

        if ((terr = _toolErr) == 0) {
          // macbinary is padded to 128 bytes.
          if (q->moreFlags == CGI_MACBINARY) {
            i = count & 0x7f;

            if (i) {
              memset(buffer + count, 0, 128 - i);
              count += 128 - i;
            }
          }
          WriteData(q, buffer, count);

#ifdef DEBUG
          i = sprintf(buffer, "TCPIPWriteTCP(%d) [%d bytes sent]\r", ipid,
                      count);
          InsertString(i, buffer);
#endif

          if (q->contentlength) {
            /* range-based */
            q->contentlength -= count;
            if (!q->contentlength) {
              q->state = STATE_CLOSE;
            }
          }
        } else {
          if (terr == eofEncountered) {
            // if apple single, now send the resource fork.
            if (q->state == STATE_ASINGLE_1 && q->rfd) {
              q->state = STATE_ASINGLE_2;
            } else {
              WriteData(q, NULL, 0); // if chunked.
              q->contentlength = 0;
              q->state = STATE_CLOSE;
            }
          } else // read error - just close and be done with it.
          {
#ifdef DEBUG
            i = sprintf(buffer, "TCPIPCloseTCP(%d) [read error]\r", ipid);
            InsertString(i, buffer);
#endif

            TCPIPCloseTCP(ipid);
            ReleaseQ(q);
            q->ipid = ipid;
            q->state = STATE_LOGOUT;
            break;
          }
        }
      } break;

        // read any incoming data, dump it to the file.
      case STATE_PUT: {
        Word i;

        i = ReadData(q, buffer, 4096);
        if (i) {
          if (q->flags & FLAG_TEXT)
            i = ConvertCRLF(buffer, i);

          IODCB.pCount = 4;
          IODCB.refNum = q->fd;
          IODCB.dataBuffer = buffer;
          IODCB.requestCount = i;

          WriteGS(&IODCB);

          if (_toolErr) {
#ifdef DEBUG
            i = sprintf(buffer, "WriteGS(%" PRIB "): %04x\r", B(q->fullpath),
                        _toolErr);
            InsertString(i, buffer);
#endif
            ProcessError(500, q);
            break;
          }
        }
        if ((q->version > 0x0009) && (!q->contentlength)) {
          q->state = STATE_CLOSE;
        } // q->contentlength.
      } break;

      case STATE_CLOSE: {
        if (srBuffer.srSndQueued == 0) {
          if (q->flags & FLAG_KA) {
            Handle h;

            // if a content-length headers was sent, purge any remaining data.
            // since KA is only valid for HTTP 1.0+, no need to check version.

            if (q->contentlength) {
              ReadData(q, buffer, 4096);
              if (q->contentlength)
                break;
            }

            h = q->buffer;

            q->buffer = NULL;
            ReleaseQ(q);
            // reset these values.
            q->ipid = ipid;
            q->buffer = h;
            q->state = STATE_READ;
            q->tick = GetTick() + 60 * 30; // 30 seconds.
          } else {
#ifdef DEBUG
            i = sprintf(buffer, "TCPIPCloseTCP(%d)\r", ipid);
            InsertString(i, buffer);
#endif

            TCPIPCloseTCP(ipid);
            ReleaseQ(q);
            q->ipid = ipid;
            q->state = STATE_LOGOUT;
          }
        }
      } break; // STATE_CLOSE
        // case STATE_LOGOUT: /* handled at start */

      } // case (q->state)
    }
  // now check for any new ones.
  if (fUsed != 0xffff) {
    Word child;
    Word j;

    TCPIPPoll();
    child = TCPIPAcceptTCP(Ipid, 0);
    if (!_toolErr) {
      Word err;

      // find an open slot...
      for (mask = 1, q = queue; fUsed & mask; mask <<= 1, q++)
        ;

      q->ipid = child;
      TCPIPStatusTCP(child, &srBuffer);
      if (srBuffer.srState == TCPSESTABLISHED)
        q->state = STATE_READ;
      else
        q->state = STATE_ESTABLISH;

      q->tick = GetTick() + 60 * 60;
      q->depth = -1;

      // allocate any handles if needed.
      err = 0;

      // if (!q->buffer) q->buffer = NewHandle(0, MyID | 0x0d00, attrNoSpec, 0);
      // err |= _toolErr;

      if (!q->workHandle)
        q->workHandle = NewHandle(0, MyID | 0x0d00, attrNoSpec, 0);
      err |= _toolErr;

      if (err) {
#if DEBUG
        InsertString(sprintf(buffer, " NewHandle -- %x\r", err), buffer);
#endif
        TCPIPAbortTCP(child);
        TCPIPLogout(child);
        ReleaseQ(q);
      } else {
        fUsed |= mask;
        fActive++;
      }
    }
#if DEBUG
    else if (_toolErr != terrNOINCOMING) {
      InsertString(sprintf(buffer, "TCPIPAcceptTCP -- %x\r", _toolErr), buffer);
    }
#endif
  }

  if (oldActive != fActive && MyWindow) {
    static char buffer[6];

    SetCtlValueByID(fActive, MyWindow, CtrlTherm);
    sprintf(buffer, "%u", fActive);
    SetCtlTextByID(MyWindow, CtrlCount, 1, (Ref)buffer);
  }

  DoSysPrefs(0xffff, oldPrefs);
}
