#pragma noroot
#pragma lint -1
#pragma optimize -1
#pragma debug 0x8000

#include <tcpip.h>
#include "server.h"
#include "config.h"


int orca_sprintf(char *, const char *, ...);

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
  		i = orca_sprintf(buffer, "%x\r\n", length);
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
