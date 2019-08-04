#pragma noroot
#pragma lint -1
#pragma optimize -1
#pragma debug 0x8000


#include <tcpip.h>
#include <memory.h>

#include <stdio.h>

#include "server.h"
#include "config.h"



#undef MIN
#define MIN(a,b) (a) < (b) ? (a) : (b)


void WriteData(struct qEntry *q, const char *data, word length)
{
	char buffer[8];
	Word i;
	Word ipid = q->ipid;
	char *cp;
	
  	if (q->flags & FLAG_CHUNKED)
  	{	
  		i = sprintf(buffer, "%x\r\n", length);
  		TCPIPWriteTCP(ipid, buffer, i, false, false);	
  	}
  	


  	if (cp && length) for (cp = data; length;)
  	{
  		i = MIN(length, fMTU);
  		TCPIPWriteTCP(ipid, cp, i, false, false);
  		TCPIPPoll();
  		
  		cp += i;
  		length -= i; 		
  	}
	if (q->flags & FLAG_CHUNKED)
	{
		  TCPIPWriteTCP(ipid, "\r\n", 2, false, false);
	}
}


// reads data and updates contentlength.
Word ReadData(struct qEntry *q, void *data, word req)
{
Handle h;
Word size;
Word i;
     
static srBuff srBuffer;
static rrBuff rrBuffer;


  if (req == 0) return 0;
  if (data == NULL) return 0;

  if (q->version > 0x0009)
  {
    if (q->contentlength == 0)
      return 0;
    req = MIN(req, q->contentlength);
  }

  // step one -- if buffer has data, read from that.

  h = q->buffer;
  if (h)
  {
    size = GetHandleSize(h);
    if (size)
    {
      i = MIN(req, size);

      HandToPtr(h, data, i);

      if (i == size)
      {
        DisposeHandle(h);
        q->buffer = NULL;
      }
      else
      {
        char *cp;
        //HLock(h); -- already locked.
        cp = *h;
        BlockMove(cp + i, cp, size - i);
        SetHandleSize(size - i, h);
      }

      if (q->version > 0x0009)
        q->contentlength -= i;
      return i;
    }
  }

  // must read incoming data.
  TCPIPStatusTCP(q->ipid, &srBuffer);
  if (_toolErr) return 0;

  if (srBuffer.srRcvQueued)
  {
    i = MIN(srBuffer.srRcvQueued, req);

    TCPIPReadTCP(q->ipid, 0, (Ref)data, i, &rrBuffer);
    if (_toolErr) return 0;

    i = (Word)rrBuffer.rrBuffCount;

    if (q->version > 0x0009)
      q->contentlength -= i;
    return i;
  }
  return 0;
}
