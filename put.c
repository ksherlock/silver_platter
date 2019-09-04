#pragma noroot
#pragma lint - 1
#pragma optimize - 1
#pragma debug 0x8000

#include <Memory.h>
#include <gsos.h>
#include <misctool.h>
#include <tcpip.h>

#include <stdio.h>

#include "config.h"
#include "globals.h"
#include "server.h"

#define B(x) x->length, x->text
#define PRIB ".*s"

extern void InsertString(Word, const char *);

#define DEBUG 1

Word NextSlash(const char *str) {
  char c;
  Word i;

  for (i = 0;; i++) {
    c = str[i];
    if (c == 0)
      return -1;
    if (c == '/')
      return i + 1;
  }
}

Word MakeDirs(GSString255Ptr path) {
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
  if (i == -1)
    return 0; // will error later.
  path->length = i;

  for (;;) {
    // skip past the /
    i++;

    i = NextSlash(path->text + i);
    if (i == -1)
      break;

    i += path->length;
    path->length = i;
    GetFileInfo(&InfoDCB);
    if (_toolErr == 0) {
      if (InfoDCB.storageType != directoryFile)
        break;
      continue;
    }

    if (_toolErr == fileNotFound) {
      CreateGS(&CreateDCB);
    }
    if (_toolErr)
      break;
  }

  path->length = length;

  return 0;
}

// file upload via PUT.
// header won't be sent until file is actually received/saved.

/* todo - support for Ranges header */

Word ProcessPut(struct qEntry *q) {
  Word i;
  Word create = false;
  Word err;
  SetPositionRecGS eofDCB;
  Word res = (q->moreFlags == CGI_APPLEDOUBLE);

  q->flags &= ~FLAG_KA;

  // error out if PUT is not allowed.
  if (fPut == 0)
    return ProcessError(403, q);

  if (q->flags & FLAG_RANGE) {
    return ProcessError(501, q);
  }

  /* todo -- CGI_RESOURCE support to write the resource fork */
  if (q->moreFlags) {
    return ProcessError(422, q);
  }

  // check/create directory tree.

  if (fPutMkdir) {
    MakeDirs(q->fullpath);
  }

  // if the file exists and fOverwrite == false,
  // error out unless the fork is 0.

  InfoDCB.pCount = 12;
  InfoDCB.pathname = q->fullpath;
  InfoDCB.optionList = NULL;
  GetFileInfoGS(&InfoDCB);

  if (err = _toolErr) {
    if (err == fileNotFound) {
      CreateDCB.pCount = 4;
      CreateDCB.pathname = q->fullpath;
      CreateDCB.fileType = q->flags & FLAG_TEXT ? 4 : 6;
      CreateDCB.auxType = 0;
      CreateDCB.access = 0xc3;
      if (res) {
        CreateDCB.storageType = extendedFile;
        CreateDCB.pCount = 5;
      }
      CreateGS(&CreateDCB);
      create = true;
      if (err = _toolErr) {
#ifdef DEBUG
        InsertString(sprintf(buffer, "CreateGS(%" PRIB "): $%04x\r",
                             B(q->fullpath), err),
                     buffer);
#endif
      }
    } else {
#ifdef DEBUG
      InsertString(sprintf(buffer, "GetFileInfoGS(%" PRIB "): $%04x\r",
                           B(q->fullpath), err),
                   buffer);
#endif
    }
  }
  if (err)
    return ProcessError(500, q);

  if (!fPutOverwrite) {
    LongWord eof;
    eof = res ? InfoDCB.resourceEOF : InfoDCB.eof;
    if (eof != 0)
      return ProcessError(403, q);
  }

  // if writing to a resource fork, we may need to create the fork.
  if (res && !create && (InfoDCB.storageType != extendedFile)) {
    CreateDCB.pCount = 5;
    CreateDCB.pathname = q->fullpath;
    CreateDCB.fileType = 0;
    CreateDCB.auxType = 0;
    CreateDCB.access = 0;
    CreateDCB.storageType = 0x8000 | extendedFile;
    CreateGS(&CreateDCB);
    if (_toolErr) {
#ifdef DEBUG
      InsertString(sprintf(buffer, "CreateGS(%" PRIB "): $%04x\r",
                           B(q->fullpath), _toolErr),
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
  if (_toolErr) {
#ifdef DEBUG
    InsertString(
        sprintf(buffer, "OpenGS(%" PRIB "): $%04x\r", B(q->fullpath), _toolErr),
        buffer);
#endif
    return ProcessError(500, q);
  }

  q->fd = OpenDCB.refNum;

  // if overwriting, we may need to truncate.
  if (!create) {
    eofDCB.pCount = 3;
    eofDCB.refNum = OpenDCB.refNum;
    eofDCB.base = startPlus;
    eofDCB.displacement = 0;
    SetEOFGS(&eofDCB);
    if (_toolErr) {
#ifdef DEBUG
      InsertString(sprintf(buffer, "SetEOFGS(%" PRIB "): $%04x\r",
                           B(q->fullpath), _toolErr),
                   buffer);
#endif
      return ProcessError(500, q);
    }
  }

  SendHeader(q, create ? 201 : 204, 0, NULL, NULL, NULL, 0);

  q->state = STATE_PUT;
  return 204;
}
