#pragma noroot
#pragma lint - 1
#pragma optimize - 1
#pragma debug 0x8000

segment "WebDAV    ";

#include <types.h>

#include <stdio.h>

#include "config.h"
#include "http.h"
#include "server.h"

#define B(x) x->length, x->text
#define PRIB ".*s"

static Word lock = 0;

// pretend to lock a resource.
Word ProcessLock(struct qEntry *q) {
  Word len;
  GSString255Ptr host;

  if (fWebDav == false)
    return ProcessError(HTTP_METHOD_NOT_ALLOWED, q);

  if (q->moreFlags)
    return ProcessError(HTTP_UNPROCESSABLE_ENTITY, q);

  host = q->host;
  if (host == NULL)
    host = (GSString255Ptr) "\x09\x00"
                            "localhost";

  len = sprintf(buffer,
                "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n"
                "<D:prop xmlns:D=\"DAV:\">\r\n"
                "<D:lockdiscovery>\r\n"
                "<D:activelock>\r\n"
                "<D:locktype><D:write/></D:locktype>\r\n"
                "<D:lockscope><D:exclusive/></D:lockscope>\r\n"
                "<D:depth>Infinity</D:depth>\r\n"
                "<D:owner>\r\n"
                "<D:href>http://%" PRIB "%" PRIB "</D:href>\r\n"
                "</D:owner>\r\n"
                "<D:timeout>Infinite</D:timeout>\r\n"
                "<D:locktoken>\r\n"
                "<D:href>opaquelocktoken:%u</D:href>\r\n"
                "</D:locktoken>\r\n"
                "</D:activelock>\r\n"
                "</D:lockdiscovery>\r\n"
                "</D:prop>\r\n",

                B(host), B(q->pathname), lock++);

  SendHeader(q, HTTP_OK, len, NULL, "text/xml", NULL, 0);

  WriteData(q, buffer, len);
  WriteData(q, NULL, 0);

  q->state = STATE_CLOSE;
  return HTTP_OK;
}

// pretend to unlock a resource.
Word ProcessUnlock(struct qEntry *q) {
  if (fWebDav == false)
    return ProcessError(HTTP_METHOD_NOT_ALLOWED, q);

  if (q->moreFlags)
    return ProcessError(HTTP_UNPROCESSABLE_ENTITY, q);

  SendHeader(q, HTTP_NO_CONTENT, 0, NULL, NULL, NULL, 0);

  q->state = STATE_CLOSE;
  return HTTP_NO_CONTENT;
}
