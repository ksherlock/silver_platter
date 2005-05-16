#pragma noroot
#pragma lint -1
#pragma optimize -1
#pragma debug 0x8000

#include <gsos.h>

#include "server.h"
#include "globals.h"



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
	
  CreateDCB.pCount = 4;
  CreateDCB.pathname = q->fullpath;
  CreateDCB.fileType = 0x000f;
  CreateDCB.auxType = 0;
  CreateDCB.access = 0xe3;
  
  CreateGS(&CreateDCB);
  
  if (_toolErr) return ProcessError(q, 409);
  
  SendHeader(q, 201, 0, NULL, NULL, true);
  
  q->state = STATE_CLOSE;
  return 201;
}
