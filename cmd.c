#pragma noroot
#pragma lint -1


#include <memory.h>
#include <misctool.h>
#include <tcpip.h>

#include <string.h>
#include <ctype.h>

#include <kstring.h>


struct qEntry q[16];

Word fUsed = 0;
Word fActive = 0;

extern Word MyID;

// normalize the name
void ParseName(const char *str, struct qEntry *q);
{
char c;
char *cp;
int i;
Handle h;
Word delta;
Word len;

  // 1 get the length, allocate a new handle.
  while (isspace(*str++)) ;
  for (i = 0; str[i] && !isspace(str[i]; i++)  ;

  h = NewHandle(i + 3, MyID, attrFixed | attrLocked, 0);
  cp = *h;

  // copy the string over
  // convert : --> /
  // strip non-ascii chars
  // convert %xx --> character
  i = 2;
  while (c = *str++)
  {
    if (isspace(c)) break;

    if (c == ':') c = '/';
    if (c == '%')
    {
      char t;
      t = *str++;
      if (!isxdigit(t)) break;
      t = isalpha(t) ? _toupper(t) - 'A' + 10 : t - '0';
      c = t << 4;

      t = *str++;
      if (!isxdigit(t)) break;
      t = isalpha(t) ? _toupper(t) - 'A' + 10 : t - '0';
      c |= t;

    }

    //if (c & 0x80) continue;

    cp[i++] = c;
  }

  cp[i + 1] = 0;
  len = i - 2;


  ((GSString255Ptr)cp).length = len;
  // todo - strip leading /, ExpandPathGS

  // check for HTTP version
  // HTTP-Version = "HTTP" "/" 1*DIGIT "." 1*DIGIT

  while (isspace(*str++)) ;
  if (stricmp("HTTP/", str) == 0)
  {
  Word v;
  Word j;

    str += 5;
    j = 0;
    // j = j * 10 + c - '0'
    while (isdigit(c = *str)) j = (j << 3) + (j << 1) + c - '0';
    v = j << 8;
    if (*str == '.')
    {
      str++;
      j = 0;
      while (isdigit(c = *str)) j = (j << 3) + (j << 1) + c - '0';
      v |= j;
    }
    q->version = v;
  }
  else q->version = 0x0009;

  q->pathname = h;
}

void ProcessQ(void)
{
static char buffer[4096 + 2];
static srBuff srBuffer;
static rlrBuff rlrBuffer;

Word i;
Word mask;
Word terr;

  mask = 1;
  if (fUsed) for (mask = 1, i = 0; mask; mask <<= 1, i++)
  {
    Longword tick;
    Word ipid;

    if (fUsed & mask == 0) continue;

    TCPIPPoll();
    ipid = q[i].ipid;
    TCPIPStatusTCP(ipid, &srBuffer);
    tick = GetTick();

    switch(q[i].state)
    {
    // wait for the connection to establish.
    case STATE_ESTABLISH:
      if (srBuffer.srState == TCPSESTABLISHED)
        q[i].state = STATE_READ;
      else if (tick > q[i].tick)
      {
          TCPIPClose(ipid);
          fUsed &= (~mask);
          break;
      }

    // we're waiting to read...
    case STATE_READ:
      {
      Word j;
        terr = TCPIPReadLineTCP(ipid, "\p\r\n", 0,
			(Ref)buffer, 4096, &rlrBuffer);

        if (terr)
        {
          CloseTCPIP(ipid);
          if (q[i].pathname) DisposeHandle(q[i].pathname);
          fActive--;
          fUsed &= (~mask);
          break;
        }
        j = rlrBuffer.rlrBuffCount;
        if (j > 0)
        {
          buffer[j + 1] = 0; //
          if (stricmp("GET ", buffer) == 0)
          {
            q[i].command = CMD_GET;
            ParseName(buffer + 4, &q[i]);
          }
          else if (stricmp("HEAD ", buffer) == 0)
          {
            q[i].command = CMD_HEAD;
            ParseName(buffer + 5, &q[i]);
          }
          else if (stricmp("Connection: keep-alive", buffer) == 0)
            q[i].flags |= FLAG_KA;
          break;
        }
        // end of options, open the file, prepare to send.
        if (rlrBuffer.rlrIsDataFlag)
        {

          q[i].state = STATE_WRITE;
        }

      }

    // write the file data.
    case STATE_WRITE:
      {
      static IORecGS ReadDCB;
	ReadDCB.pCount = 5;
	ReadDCB.refNum = q[i].fd;
        ReadDCB.dataBuffer = buffer;
	ReadDCB.requestCount = 4096;
	ReadGS(&ReadDCB);
        if (_toolErr == 0)
        {
          TCPIPWriteTCP(ipid, buffer, ReadDCB.transferCount, false, false);
          break;
        }
        q[i].state = STATE_CLOSE;
        q[i].tick = GetTick() + 300 * 60;
	if (_toolErr != eofEncountered)
        {
        }
        if (q[i].pathname) DisposeHandle(q[i].pathname);
        if (q[i].fd)
        {
          Word CloseDCB[2];
          CloseDCB[0] = 1;
          CloseDCB[1] = q[i].fd;

          CloseGS(CloseDCB);
        }
        q[i].fd = 0;
        q[i].pathname = NULL;
      }

    // wait for data to be sent.  If not keep-alive, kill it
    case STATE_CLOSE:
      if (srBuffer.srSndQueued == 0)
      {
        if (q[i].flag & FLAG_KA)
        {
          q[i].state = STATE_READ;
          q[i].tick = GetTick() + 300 * 60;
        }
        else
        {
          TCPIPClose(ipid);
          fUsed &= (~mask);
          fActive--;
          break;
	}
      }
    } // switch (q[i].state])
  } // if (fUsed)

  // now check for any incoming connections
  if (fUsed != 0xffff)
  {
    Word child;

    child = TCPIPAcceptTCP(Ipid, 0);
    if (_toolErr) return;

    for (i = 0, mask = 1; fUsed & mask; mask <<= 1, i++) ;

    memzero(q[i], sizeof(struct qEntry);

    q[i].ipid = child;
    TCPIPStatusTCP(child, &srBuffer);
    if (srBuffer.srState == TCPSESTABLISHED)
      q[i].state = STATE_READ;
    else q[i].state = STATE_ESTABLISH;

    q[i].tick = GetTick + 300 * 60;

    fUsed |= mask;
    fActive++;
  }
}
