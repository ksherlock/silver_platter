#pragma noroot
#pragma lint -1
#pragma optimize -1

#include <control.h>
#include <event.h>
#include <gsos.h>
#include <intmath.h>
#include <memory.h>
#include <resources.h>
#include <stdfile.h>
#include <window.h>

#include <string.h>

#include "config.h"
#include "rez.h"
#include "toolbox.h"

extern int orca_sprintf(char *, const char *, ...);

Word fAbort;
Word fJail;
Word fPort;
Word fDir;
Word fTeach;
Word fMTU;

Word fAS;


Word fLog;
GSString255Ptr fLogDir;
Handle fLogDirH;

GSString255Ptr fRoot;
Handle fRootH;


static Word fd;


static char *NameAbort = "\pFast Close";
static char *NameDir = "\pVirtual Directories";
static char *NameJail = "\pJail";
static char *NamePort = "\pPort";
static char *NameRoot = "\pRoot Directory";
static char *NameTeach ="\pTeach Ascii";
static char *NameMTU = "\pMTU";
static char *NameLog = "\pLog";
static char *NameLogDir = "\pLog Directory";
static char *NameAS = "\pAppleSingle";



Handle RMLoadNamedResource2(Word rType, const char *name, LongWord *rID)
{
Word fileID;
Handle h;
LongWord id;

  id = RMFindNamedResource (rType, name, &fileID);
  if (_toolErr) return NULL;
  h = LoadResource(rType, id);
  if (_toolErr) return NULL;
  *rID = id;
  return h;
}

void SetConfigValue(Word rType, const char *name, const void *value, Word size)
{
Handle h;
LongWord rID;


  h = RMLoadNamedResource2(rType, name, &rID);
  if (!_toolErr)
  {
    HUnlock(h);
    SetHandleSize(size, h);
    if (!_toolErr)
    {
      PtrToHand(value, h, size);
      ReleaseResource(3, rType, rID);
      MarkResourceChange(1, rType, rID);
      return;
    }
    else
    {
      ReleaseResource(3, rType, rID);
      RMSetResourceName (rType, rID, "");
      RemoveResource (rType, rID);
    }
  }

  h = NewHandle(size, GetCurResourceApp(), attrLocked, NULL);
  if (_toolErr) return;

  PtrToHand(value, h, size);

  rID = UniqueResourceID(0, rType);
  AddResource(h, 0, rType, rID);
  if (_toolErr)
  {
    /* if (size) */ DisposeHandle(h);
  }
  else
  {
    RMSetResourceName(rType, rID, name);
  }
}

/*
 * load default values.
 * return 0 on failure, anything else on success.
 */
//
Word LoadConfig(Word MyID)
{
Word t;
Word oFile;
Word oDepth;

LongWord rID;
Handle h;

#undef xstr
#define xstr "*:System:Preferences:SilverPlatter"
static GSString255 filePath = {sizeof(xstr) - 1, xstr};

#undef xstr
#define xstr "*:System:Preferences"
static GSString32 folderPath = {sizeof(xstr) - 1, xstr};

static FileInfoRecGS InfoDCB = {12, (GSString255Ptr)&filePath};
static CreateRecGS CreateDCB = {4, (GSString255Ptr)&folderPath, 0xe3, 0x0f, 0};

// default prefs
fAbort = true;
fDir = true;
fJail = false;
fTeach = false;
fPort = 80;
fMTU = 512;

fLog = false;
fLogDir = NULL;
fLogDirH = NULL;


fRoot = NULL;
fRootH = NULL;

// 1 - check if file exists
// 2 - if no, create the folder and file
// 3 - load up the data
// 4 - if data doesn't exist, store a default value.

  fd = 0;

  GetFileInfoGS(&InfoDCB);
  t = _toolErr;
  if (_toolErr == pathNotFound)
  {
    CreateGS(&CreateDCB);
    if (!_toolErr) t = fileNotFound;
  }
  if (t == fileNotFound) // file doesn't exist, create
  {
    CreateResourceFile(0,0x5A,0,(Pointer)&filePath);
    if (_toolErr) return 0;
  }
  else if (t) return 0;

  fd = OpenResourceFile(readWriteEnable, NULL, (Pointer)&filePath);
  if (_toolErr) return 0;

  // make sure we're the only resource file...
  oFile = GetCurResourceFile();
  SetCurResourceFile(fd);
  oDepth = SetResourceFileDepth(1);


  h = RMLoadNamedResource2(1, (Ptr)NameAbort, &rID);
  if (!_toolErr)
  {
    HLock(h);
    fAbort = **(Word **)h;
    ReleaseResource(3, 1, rID);
  }

  h = RMLoadNamedResource2(1, (Ptr)NamePort, &rID);
  if (!_toolErr)
  {
    HLock(h);
    fPort = **(Word **)h;
    ReleaseResource(3, 1, rID);
  }

  h = RMLoadNamedResource2(1, (Ptr)NameDir, &rID);
  if (!_toolErr)
  {
    HLock(h);
    fDir = **(Word **)h;
    ReleaseResource(3, 1, rID);
  }

  h = RMLoadNamedResource2(1, (Ptr)NameJail, &rID);
  if (!_toolErr)
  {
    HLock(h);
    fJail = **(Word **)h;
    ReleaseResource(3, 1, rID);
  }

  h = RMLoadNamedResource2(1, (Ptr)NameTeach, &rID);
  if (!_toolErr)
  {
    HLock(h);
    fTeach = **(Word **)h;
    ReleaseResource(3, 1, rID);
  }

  h = RMLoadNamedResource2(1, (Ptr)NameMTU, &rID);
  if (!_toolErr)
  {
    HLock(h);
    fMTU = **(Word **)h;
    ReleaseResource(3, 1, rID);
  }

  h = RMLoadNamedResource2(rC1InputString, (Ptr)NameRoot, &rID);
  if (!_toolErr)
  {
    DetachResource(rC1InputString, rID);
    HLock(h);
    fRootH = h;
    fRoot = (GSString255Ptr)*h;
  }
  else fJail = 0;


  // restore old resource file...

  SetCurResourceFile(oFile);
  SetResourceFileDepth(oDepth);

  return 1;
}


void UnloadConfig(void)
{
  if (fd) CloseResourceFile(fd);
  fd = 0;
  if (fRootH)
    DisposeHandle(fRootH);
  fRootH = NULL;
  fRoot = NULL;
}

void DoConfig(Word MyID)
{
static EventRecord event;
static char buffer[10];

Handle newPath;

WindowPtr win;
Word control;
Word done;
Word i;

  newPath = NULL;

  memset(&event, 0, sizeof(event));
  event.wmTaskMask = 0x001f0004;

  win = NewWindow2(NULL, NULL, NULL, NULL,
    refIsResource, (long)rConfigWindow, rWindParam1);

  CenterWindow(win);

  if (fRoot)
    SetCtlTextByID(win, CtrlPathStat, 2, (Ref)fRoot);
  else SetCtlTextByID(win, CtrlPathStat, 1, (Ref)"");

  SetCtlValueByID(fAbort, win, CtrlAbort);
  SetCtlValueByID(fDir, win, CtrlDir);
  SetCtlValueByID(fJail, win, CtrlJail);
  SetCtlValueByID(fTeach, win, CtrlTeach);

  i = orca_sprintf(buffer + 1, "%u", fPort);
  buffer[0] = i;
  SetLETextByID(win, CtrlPort, (StringPtr)buffer);

  ShowWindow(win);

  for (done = false; !done; )
  {
    control = (Word)DoModalWindow(&event, NULL, NULL, NULL, 0x0008);
    switch(control)
    {
    case 0:
      if ( (event.what == keyDownEvt) || (event.what == autoKeyEvt)
        && event.modifiers & appleKey 
        && (Word)event.message == '.')
        done = true;
      break;
    case CtrlCancel:
    case CtrlOk:
      done = true;
      break;
    case CtrlBrowse:
      {
      SFReplyRec2 myReply;

        myReply.nameRefDesc = refIsNewHandle;
        myReply.pathRefDesc = refIsNewHandle;

        SFGetFolder2(20, 20, refIsPointer,
          (Ref)"\pSelect Home Directory", &myReply);
        if (myReply.good)
        {
          Word size;
          char *cp;
          Word i;
          GSString255Ptr gstr;

          if (newPath) DisposeHandle(newPath);

          newPath = (Handle)myReply.pathRef;
          HLock(newPath);
          cp = *newPath;

          // includes trailing ':', so -1 to remove that.

          size = ((ResultBuf255 *)cp)->bufString.length + 2 - 1;
          BlockMove(cp + 2, cp, size);
          SetHandleSize(size, newPath);

          size -= 2;
          gstr = (GSString255Ptr)cp;
          gstr->length = size;

          while (size)
          {
            size--;
            if (gstr->text[size] == ':') gstr->text[size] = '/';
          }
          SetCtlTextByID(win, CtrlPathStat, 2, (Ref)cp);

          DisposeHandle((Handle)myReply.nameRef);
          SetCtlValueByID(1, win, CtrlJail);
        }
      }
      break; // case CtrlBrowse
                        
    } // switch(control)
  }
  // save and such
  if (control == CtrlOk)
  {
  Word oFile;
  Word oDepth;
  LongWord rID;
  Word FileID;
  Handle h;


    fAbort = GetCtlValueByID(win, CtrlAbort);
    fDir = GetCtlValueByID(win, CtrlDir);
    fJail = GetCtlValueByID(win, CtrlJail);
    fTeach = GetCtlValueByID(win, CtrlTeach);

    GetLETextByID(win, CtrlPort, (StringPtr)buffer);
    fPort = Dec2Int(buffer + 1, buffer[0], 0);
    if (_toolErr || !fPort) fPort = 80;


    oFile = GetCurResourceFile();
    SetCurResourceFile(fd);
    oDepth = SetResourceFileDepth(1);


    if (newPath)
    {
      if (fRootH) DisposeHandle(fRootH);

      SetConfigValue(rC1InputString, NameRoot, *newPath, GetHandleSize(newPath));
      fRootH = newPath;
      fRoot = (GSString255Ptr)*newPath;
    }
    if (!fRoot || fRoot->length == 0) fJail = false;

    SetConfigValue(1, NameAbort, &fAbort, sizeof(Word));
    SetConfigValue(1, NameDir, &fDir, sizeof(Word));
    SetConfigValue(1, NameJail, &fJail, sizeof(Word));
    SetConfigValue(1, NameTeach, &fTeach, sizeof(Word));
    SetConfigValue(1, NamePort, &fPort, sizeof(Word));
    SetConfigValue(1, NameMTU, &fMTU, sizeof(Word));

    UpdateResourceFile(fd);

    SetCurResourceFile(oFile);
    SetResourceFileDepth(oDepth);
  }

  CloseWindow(win);                             
}
