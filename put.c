#pragma noroot
#pragma lint -1
#pragma optimize -1
#pragma debug 0x8000


#include <gsos.h>
#include <Memory.h>
#include <misctool.h>
#include <tcpip.h>

#include "server.h"
#include "config.h"

static CreateRecGS CreateDCB;
static OpenRecGS OpenDCB;
static NameRecGS DestroyDCB;
static IORecGS WriteDCB;

// file upload via PUT.
// header won't be sent until file is actually received/saved.

Word ProcessPut(struct qEntry *q)
{
Word i;

  if (fPut == 0)
    return ProcessError(403,q);

  // todo - check/create directory tree.

  //

  q->flags |= FLAG_CREATE;

  if (fPutOverwrite)
  {
    DestroyDCB.pCount = 1;
    DestroyDCB.pathname = q->fullpath;
    DestroyGS(&DestroyDCB);
    if (_toolErr == 0) q->flags &= ~FLAG_CREATE;
    else if (_toolErr != fileNotFound)
      return ProcessError(403,q);
  }


  CreateDCB.pCount = 4;
  CreateDCB.pathname = q->fullpath;
  CreateDCB.fileType = q->flags & FLAG_TEXT ? 4 : 6;
  CreateDCB.auxType = 0;
  CreateDCB.access = 0xe3;
  CreateGS(&CreateDCB);
  if (_toolErr) return ProcessError(501, q);

  OpenDCB.pCount = 15;
  OpenDCB.pathname = q->fullpath;
  OpenDCB.requestAccess = writeEnable;
  OpenDCB.resourceNumber = 0;
  OpenDCB.optionList = NULL;
  OpenGS(&OpenDCB);
  if (_toolErr) return ProcessError(501, q);

  q->fd = OpenDCB.refNum;


  // q->buffer may have file data... if so, write it here.

  // todo - verify handlesize <= filesize?

  if (q->buffer && (i = GetHandleSize(q->buffer)));
  {
    char *cp = *q->buffer;
    WriteDCB.pCount = 4;
    WriteDCB.refNum = OpenDCB.refNum;
    WriteDCB.dataBuffer = cp;
    WriteDCB.requestCount = i;

    WriteGS(&WriteDCB);

    if (_toolErr) return ProcessError(500,q);

    DisposeHandle(q->buffer);
    q->buffer = NULL;
    if (q->filesize)
    {
      q->filesize -= i;
      if (q->filesize <= 0)
      {
        SendHeader(q, q->flags & FLAG_CREATE ? 201 : 204 , 0, NULL, NULL, true);
        q->state = STATE_CLOSE;
        return 204;
      }
    }
  }


  q->state = STATE_PUT;
  return 204;

}
