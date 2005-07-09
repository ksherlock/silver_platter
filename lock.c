#pragma noroot
#pragma lint -1
#pragma optimize -1
#pragma debug 0x8000

#include <types.h>

#include "server.h"
#include "config.h"

int orca_sprintf(char *, const char *, ...);

static Word lock = 0;

// pretend to lock a resource.
Word ProcessLock(struct qEntry *q)
{
Word len;
GSString255Ptr host;

	if (fWebDav == false)
	{
		return ProcessError(405, q);
	}

	host = q->host;
	if (host == NULL) host = (GSString255Ptr)"\x09\x00" "localhost";

	len = orca_sprintf(buffer,
		"<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n"
		"<D:prop xmlns:D=\"DAV:\">\r\n"
		  "<D:lockdiscovery>\r\n"
		    "<D:activelock>\r\n"
		      "<D:locktype><D:write/></D:locktype>\r\n"
		      "<D:lockscope><D:exclusive/></D:lockscope>\r\n"
		      "<D:depth>Infinity</D:depth>\r\n"
		      "<D:owner>\r\n"
		        "<D:href>http://%B%B</D:href>\r\n"
		      "</D:owner>\r\n"
		      "<D:timeout>Infinite</D:timeout>\r\n"
		      "<D:locktoken>\r\n"
		        "<D:href>opaquelocktoken:%u\</D:href>\r\n"
		      "</D:locktoken>\r\n"
		    "</D:activelock>\r\n"
		  "</D:lockdiscovery>\r\n"
		"</D:prop>\r\n",
		
		host,
		q->pathname,
		lock++);
		
	SendHeader(q, 200, len, NULL, "text/xml", NULL, 0);
	
	WriteData(q, buffer, len);
	WriteData(q, NULL, 0);
	
	q->state = STATE_CLOSE;
	return 200;
}


// pretend to unlock a resource.
Word ProcessUnlock(struct qEntry *q)
{
	
	if (fWebDav == false)
	{
		return ProcessError(405, q);
	}	
	
  SendHeader(q, 204, 0, NULL, NULL, NULL, 0);
  
  q->state = STATE_CLOSE;
  return 204;
}
