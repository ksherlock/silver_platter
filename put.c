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
#include "globals.h"


//#pragma debug -1
// normalizes CR, LF, and CRLF to a CR.
// returns new length of data.
Word ConvertCRLF(char *data, Word length)
{
Word rlength = length;
Word i;
char c;

  for (i = 0; length; length--, i++)
  {
    c = data[i];
    // convert LF -> CR (unix standard).
    if (c == 0x0a)
    {
      data[i] = 0x0d;
      continue;
    }

    //
    if (c == 0x0d)
    {
      if (length > 1)
      {
        length--;
        i++;
        c = data[i];
        if (c == 0x0a) // convert CRLF to just CR
        {
          TCPIPPtrToPtr(data + i + 1 , data + i, length - 1);
          rlength--;
        }
      }
    }
  }

  return rlength;

}
#pragma debug 0x8000


Word NextSlash(const char *str)
{
char c;
Word i;

  for (i = 0; ; i++)
  {
    c = str[i];
    if (c == 0) return -1;
    if (c == '/') return i + 1;
  }
}

Word MakeDirs(GSString255Ptr path)
{
Word i;
Word j;
Word length;


  CreateDCB.pCount = 4;
  CreateDCB.pathname = path;
  CreateDCB.fileType = 0x000f;
  CreateDCB.auxType = 0;
  CreateDCB.access = 0xe3;

  InfoDCB.pCount = 4;
  InfoDCB.pathname = path;


  length = path->length;

  // skip past the /volume/

  i = NextSlash(path->text + 1);
  if (i == -1) return 0; // will error later.
  path->length = i;

  for(;;)
  {
    // skip past the /
    i++;

    i = NextSlash(path->text + i);
    if (i == -1) break;

    i += path->length;
    path->length = i;
    GetFileInfo(&InfoDCB);
    if (_toolErr == 0)
    {
      if (InfoDCB.fileType != 0x000f) break;
      continue;
    }

    if (_toolErr == fileNotFound)
    {
      CreateGS(&CreateDCB);
    }
    if (_toolErr) break;
  }

  path->length = length;

  return 0;
}


// file upload via PUT.
// header won't be sent until file is actually received/saved.

Word ProcessPut(struct qEntry *q)
{
Word i;

  q->flags &= ~FLAG_KA;

  if (fPut == 0)
    return ProcessError(403,q);

  // check/create directory tree.

  if (fPutMkdir)
  {
    MakeDirs(q->fullpath);
  }

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

  if (q->buffer)
  {
    char * cp;
    Word size;

    i = size = (Word)GetHandleSize(q->buffer);
    HLock(q->buffer);

    cp = *q->buffer;

    if (q->flags & FLAG_TEXT) i = ConvertCRLF(cp, i);


    IODCB.pCount = 4;
    IODCB.refNum = OpenDCB.refNum;
    IODCB.dataBuffer = *q->buffer;
    IODCB.requestCount = i;

    WriteGS(&IODCB);

    if (_toolErr) return ProcessError(500, q);

    DisposeHandle(q->buffer);
    q->buffer = NULL;
    if (q->filesize)
    {
      q->filesize -= size;
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
