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


extern int orca_sprintf(char *, const char *, ...);
extern void InsertString(Word, const char *);


#define DEBUG 1

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


  CreateDCB.pCount = 5;
  CreateDCB.pathname = path;
  CreateDCB.access = 0xc3;
  CreateDCB.fileType = 0x000f;
  CreateDCB.auxType = 0;
  CreateDCB.storageType = directoryFile;

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
      if (InfoDCB.storageType != directoryFile) break;
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
Word create = false;
Word err;
SetPositionRecGS eofDCB;
Word res = (q->moreFlags == CGI_APPLEDOUBLE);


  q->flags &= ~FLAG_KA;

  // error out if PUT is not allowed.
  if (fPut == 0)
    return ProcessError(403,q);

  // check/create directory tree.

  if (fPutMkdir)
  {
    MakeDirs(q->fullpath);
  }


  // if the file exists and fOverwrite == false,
  // error out unless the fork is 0.

  InfoDCB.pCount = 12;
  InfoDCB.pathname =q->fullpath;
  InfoDCB.optionList = NULL;
  GetFileInfoGS(&InfoDCB);

  if (_toolErr)
  {
    if (_toolErr == fileNotFound)
    {
      CreateDCB.pCount = 4;
      CreateDCB.pathname = q->fullpath;
      CreateDCB.fileType = q->flags & FLAG_TEXT ? 4 : 6;
      CreateDCB.auxType = 0;
      CreateDCB.access = 0xc3;
      if (res)
      {
        CreateDCB.storageType = extendedFile;
        CreateDCB.pCount = 5;
      }
      CreateGS(&CreateDCB);
      create = true;
      if (_toolErr)
      {
	#ifdef DEBUG
	  InsertString(
            orca_sprintf(buffer, "CreateGS(%B): $%04x\r", q->fullpath, _toolErr),
            buffer);
	#endif
      }
    }
  }
  if (_toolErr) return ProcessError(501, q);
   

  else if (!fPutOverwrite)
  {
    LongWord eof;
    eof = res ? InfoDCB.resourceEOF : InfoDCB.eof;
    if (eof != 0) return ProcessError(403, q);
  }

  // if writing to a resoruce fork, we may need to create the fork.
  if (res && !create && (InfoDCB.storageType != extendedFile))
  {
    CreateDCB.pCount = 5;
    CreateDCB.pathname = q->fullpath;
    CreateDCB.fileType = 0;
    CreateDCB.auxType = 0;
    CreateDCB.access = 0;
    CreateDCB.storageType = 0x8000 | extendedFile;
    CreateGS(&CreateDCB);
    if (_toolErr)
    {
      #ifdef DEBUG
	InsertString(
          orca_sprintf(buffer, "CreateGS(%B): $%04x\r", q->fullpath, _toolErr),
          buffer);
      #endif
      return ProcessError(501, q);
    }
    create = true;
  }


  OpenDCB.pCount = 15;
  OpenDCB.pathname = q->fullpath;
  OpenDCB.requestAccess = writeEnable;
  OpenDCB.resourceNumber = res ? 1 : 0;
  OpenDCB.optionList = NULL;
  OpenGS(&OpenDCB);
  if (_toolErr)
  {
    #ifdef DEBUG
      InsertString(
        orca_sprintf(buffer, "OpenGS(%B): $%04x\r", q->fullpath, _toolErr),
        buffer);
    #endif
    return ProcessError(501, q);
  }

  q->fd = OpenDCB.refNum;

  // if overwriting, we may need to truncate.
  if (!create)
  {
    eofDCB.pCount = 3;
    eofDCB.refNum = OpenDCB.refNum;
    eofDCB.base = startPlus;
    eofDCB.displacement = 0;
    SetEOFGS(&eofDCB);
    if (_toolErr)
    {
      #ifdef DEBUG
	InsertString(
          orca_sprintf(buffer, "SetEOFGS(%B): $%04x\r", q->fullpath, _toolErr),
          buffer);
      #endif
      return ProcessError(501, q);
    }
  }

  SendHeader(q, create ? 201 : 204, 0, NULL, NULL, NULL, 0);

  q->state = STATE_PUT;
  return 204;
}
