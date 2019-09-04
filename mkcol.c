#pragma noroot
#pragma lint -1
#pragma optimize -1
#pragma debug 0x8000

segment "WebDAV    ";

#include <gsos.h>

#include "server.h"
#include "globals.h"
#include "config.h"
#include "http.h"


// create a colection (aka directory)
// return values:
// 201 - created
// 403 - forbidden
// 405 - method not allowed
// 409 - conflict (ie, parent directory does not exist)
// 415 - unsupported media
// 507 - insufficent storage
//
Word ProcessMkcol(struct qEntry *q)
{
	
  if (fWebDav == false)
    return ProcessError(HTTP_METHOD_NOT_ALLOWED, q);  

  if (q->moreFlags)
    return ProcessError(HTTP_UNPROCESSABLE_ENTITY, q);
	
  CreateDCB.pCount = 5;
  CreateDCB.pathname = q->fullpath;
  CreateDCB.access = 0xc3;
  CreateDCB.fileType = 0x000f;
  CreateDCB.auxType = 0;
  CreateDCB.storageType = directoryFile;
  
  CreateGS(&CreateDCB);
  
  if (_toolErr) return ProcessError(409, q);
  
  SendHeader(q, HTTP_CREATED, 0, NULL, NULL, NULL, 0);
  
  q->state = STATE_CLOSE;
  return HTTP_CREATED;
}
