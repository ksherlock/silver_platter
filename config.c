#pragma noroot
#pragma lint -1
#pragma optimize -1
#pragma debug 0x8000

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

#define SCREEN_COUNT 5	// number of config screens.


Word fAbort;
Word fJail;
Word fPort;
Word fTeach;
Word fMTU;

Word fAppleSingle;
Word fAppleDouble;
Word fMacBinary;

// PUT
Word fPut;
Word fPutOverwrite;
Word fPutMkdir;	

// virtual directory options.
Word fDir;
Word fDirHidden;
Word fDirAppleShare;
Word fDirRemovable;

// Web DAV
Word fWebDav;

Word fLog;
GSString255Ptr fLogDir;
Handle fLogDirH;

GSString255Ptr fRoot;
Handle fRootH;


static Word fd;


static char *NameAbort = "\pFast Close";
static char *NameJail = "\pJail";
static char *NamePort = "\pPort";
static char *NameRoot = "\pRoot Directory";
static char *NameTeach ="\pTeach Ascii";
static char *NameMTU = "\pMTU";
static char *NameLog = "\pLog";
static char *NameLogDir = "\pLog Directory";
static char *NameAppleSingle = "\pAppleSingle";
static char *NameAppleDouble = "\pAppleDouble";
static char *NameMacBinary = "\pMacBinary";

static char *NameDir = "\pVirtual Directories";
static char *NameDirHidden = "\pShow Hidden Files";
static char *NameDirAppleShare = "\pShow AppleShare";
static char *NameDirRemovable = "\pShow Removable";


static char *NamePut ="\pPut";
static char *NamePutMkdir = "\pPut Mkdir";
static char *NamePutOverwrite = "\pPut Overwrite";

static char *NameWebDav = "\pWebDav";


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

Word LoadWord(const char *name, Word defval)
{
Handle h;
Word ret;
LongWord rID;


  h = RMLoadNamedResource2(1, (Ptr)name, &rID);
  if (_toolErr) return defval;

  HLock(h);
  ret = **(Word **)h;
  ReleaseResource(3, 1, rID);

  return ret;
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


  fLogDir = NULL;
  fLogDirH = NULL;

  fRoot = NULL;
  fRootH = NULL;


  fAbort = LoadWord(NameAbort, true);
  fPort = LoadWord(NamePort, 80);
  
  fAppleSingle = LoadWord(NameAppleSingle, 0);
  fAppleDouble = LoadWord(NameAppleDouble, 0);
  fMacBinary = LoadWord(NameMacBinary, 0);

  fJail = LoadWord(NameJail, false);
  fLog = LoadWord(NameLog, false);
  fTeach = LoadWord(NameTeach, false);


  fDir = LoadWord(NameDir, true);
  fDirHidden = LoadWord(NameDirHidden, false);
  fDirAppleShare = LoadWord(NameDirAppleShare, false);
  fDirRemovable = LoadWord(NameDirRemovable, false);


  fPut = LoadWord(NamePut, false);
  fPutMkdir = LoadWord(NamePutMkdir, false);
  fPutOverwrite = LoadWord(NamePutOverwrite, false);

  fMTU = LoadWord(NameMTU, 512);

  fWebDav = LoadWord(NameWebDav, false);
//

  h = RMLoadNamedResource2(rC1InputString, (Ptr)NameRoot, &rID);
  if (!_toolErr)
  {
    DetachResource(rC1InputString, rID);
    HLock(h);
    fRootH = h;
    fRoot = (GSString255Ptr)*h;
  }
  else fJail = 0;

  h = RMLoadNamedResource2(rC1InputString, (Ptr)NameLogDir, &rID);
  if (!_toolErr)
  {
    DetachResource(rC1InputString, rID);
    HLock(h);
    fLogDirH = h;
    fLogDir = (GSString255Ptr)*h;
  }
  else fLog = 0;



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
  if (fLogDirH)
    DisposeHandle(fLogDirH);

  fRootH = NULL;
  fRoot = NULL;
  fLogDirH = NULL;
  fLog = NULL;
}                        



static Handle screens[SCREEN_COUNT];
static Word current;

void LoadControls(WindowPtr win, Word value)
{
  static char buffer[10];

  CtlRecHndl *ctls;
  Word index;
  Word i;

  index = value - (Ctrl_PU_1);


  // check if actually changed.
  if (index == current) return;

  // hide the old controls.
  if (current != -1)
  {
    ctls = (CtlRecHndl *)*screens[current];
    while (*ctls)
    {
      HideControl(*ctls);
      ctls++;
    }
  }


  // check if loaded.
  if (screens[index] == NULL)
  {
  CtlRecHndl c;
  Handle h;

    h = LoadResource(rControlList, value);
    if (_toolErr) return;

    DetachResource(rControlList, value);

    HLock(h);
    screens[index] = h;

    ctls = (CtlRecHndl *)*h;
    while (*ctls)
    {
      c = NewControl2(win, refIsResource, (Ref)*ctls);
      InvalOneCtlByID(win, (Long)*ctls);
      *ctls = c;

      ctls++;

      // set initial values.
      switch(value)
      {
      // general
      case Ctrl_PU_1:

        SetCtlValueByID(fJail, win, CtrlJail);
        SetCtlValueByID(fTeach, win, CtrlTeach);
        SetCtlValueByID(fAbort, win, CtrlAbort);

        if (fRoot)
          SetCtlTextByID(win, CtrlHomeStat, 2, (Ref)fRoot);
        else SetCtlTextByID(win, CtrlHomeStat, 1, (Ref)"");

	i = orca_sprintf(buffer + 1, "%u", fPort);
	buffer[0] = i;
	SetLETextByID(win, CtrlPort, (StringPtr)buffer);

        break;

      // directory
      case Ctrl_PU_2:

        SetCtlValueByID(fDir, win, CtrlDir);
        SetCtlValueByID(fDirHidden, win, CtrlDirHidden);
        SetCtlValueByID(fDirAppleShare, win, CtrlDirAppleShare);
        SetCtlValueByID(fDirRemovable, win, CtrlDirRemovable);


        break;

      // Put
      case Ctrl_PU_3:

	SetCtlValueByID(fPut, win, CtrlPut);
	SetCtlValueByID(fPutMkdir, win, CtrlPutMkdir);
	SetCtlValueByID(fPutOverwrite, win, CtrlPutOverwrite);

        break;

      // logging.
      case Ctrl_PU_4:

	SetCtlValueByID(fLog, win, CtrlLog);

	if (fLogDir)
	  SetCtlTextByID(win, CtrlLogStat, 2, (Ref)fLogDir);
	else SetCtlTextByID(win, CtrlLogStat, 1, (Ref)"");

        break;
        
       // apple single/double/macbinary
       case Ctrl_PU_5:
         SetCtlValueByID(EncapNever + fAppleSingle, win, Ctrl_AS);
         SetCtlValueByID(EncapNever + fAppleDouble, win, Ctrl_AD);
         SetCtlValueByID(EncapNever + fMacBinary, win, Ctrl_MB);
         break;
      }
    }
  }
  else
  {
    ctls = (CtlRecHndl *)*screens[index];
    while (*ctls)
    {
      ShowControl(*ctls);
      ctls++;
    }
  }
  //InvalCtls(win);

  current = index;
}


// convert apple-. to esc.
void EventHook(EventRecord *event)
{
  switch (event->what)
  {
  case keyDownEvt:
  case autoKeyEvt:
    if (event->message == '.' && event->modifiers & appleKey)
    {
      event->modifiers = 0;
      event->message = 0x1b;
    }
    break;
  }
}

void DoConfig(Word MyID)
{
static EventRecord event;
static char buffer[10];

Handle newPath;
Handle newLog;

WindowPtr win;
volatile Word control;
Word done;
Word i;

  newPath = NULL;
  newLog = NULL;

  memset(&event, 0, sizeof(event));
  event.wmTaskMask = 0x001f0004;

  win = NewWindow2(NULL, NULL, NULL, NULL,
    refIsResource, (long)rConfigWindow, rWindParam1);

  CenterWindow(win);

  current = -1;
  for (i = 0; i < SCREEN_COUNT; i++)
  	screens[i] = NULL;


  LoadControls(win, Ctrl_PU_1);

  ShowWindow(win);

  for (done = false; !done; )
  {
    control = (Word)DoModalWindow(&event, NULL, (void *)EventHook, NULL, 0x0008);
    switch(control)
    {
    case CtrlCancel:
    case CtrlOk:
      done = true;
      break;

    // switch the visible controls.
    case CtrlPopUp:
      {
	Word value;
        value = GetCtlValueByID(win, CtrlPopUp);
        LoadControls(win, value);
      }
      break;

    case CtrlHomeBrowse:
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
          SetCtlTextByID(win, CtrlHomeStat, 2, (Ref)cp);

          DisposeHandle((Handle)myReply.nameRef);
          SetCtlValueByID(1, win, CtrlJail);
        }
      }
      break; // case CtrlHomeBrowse

    case CtrlLogBrowse:
      {
      SFReplyRec2 myReply;

        myReply.nameRefDesc = refIsNewHandle;
        myReply.pathRefDesc = refIsNewHandle;

        SFGetFolder2(20, 20, refIsPointer,
          (Ref)"\pSelect Log Directory", &myReply);
        if (myReply.good)
        {
          Word size;
          char *cp;
          Word i;
          GSString255Ptr gstr;

          if (newLog) DisposeHandle(newLog);

          newLog = (Handle)myReply.pathRef;
          HLock(newLog);
          cp = *newLog;

          // includes trailing ':', so -1 to remove that.

          size = ((ResultBuf255 *)cp)->bufString.length + 2 - 1;
          BlockMove(cp + 2, cp, size);
          SetHandleSize(size, newLog);

          size -= 2;
          gstr = (GSString255Ptr)cp;
          gstr->length = size;

          while (size)
          {
            size--;
            if (gstr->text[size] == ':') gstr->text[size] = '/';
          }
          SetCtlTextByID(win, CtrlLogStat, 2, (Ref)cp);

          DisposeHandle((Handle)myReply.nameRef);
          SetCtlValueByID(1, win, CtrlLog);
        }
      }
      break; // case CtrlLogBrowse

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


    oFile = GetCurResourceFile();
    SetCurResourceFile(fd);
    oDepth = SetResourceFileDepth(1);


    if (screens[0])
    {
      fAbort = GetCtlValueByID(win, CtrlAbort);
      fJail = GetCtlValueByID(win, CtrlJail);
      fTeach = GetCtlValueByID(win, CtrlTeach);

      GetLETextByID(win, CtrlPort, (StringPtr)buffer);
      fPort = Dec2Int(buffer + 1, buffer[0], 0);
      if (_toolErr || !fPort) fPort = 80;

      if (newPath)
      {
	if (fRootH) DisposeHandle(fRootH);

	SetConfigValue(rC1InputString, NameRoot, *newPath, GetHandleSize(newPath));
	fRootH = newPath;
	fRoot = (GSString255Ptr)*newPath;
      }
      if (!fRoot || fRoot->length == 0) fJail = false;


      SetConfigValue(1, NameAbort, &fAbort, sizeof(Word));
      SetConfigValue(1, NameJail, &fJail, sizeof(Word));
      SetConfigValue(1, NameTeach, &fTeach, sizeof(Word));
      SetConfigValue(1, NamePort, &fPort, sizeof(Word));

    }

    if (screens[1])
    {
      fDir = GetCtlValueByID(win, CtrlDir);
      fDirHidden = GetCtlValueByID(win, CtrlDirHidden);
      fDirAppleShare = GetCtlValueByID(win, CtrlDirAppleShare);
      fDirRemovable = GetCtlValueByID(win, CtrlDirRemovable);
      
      SetConfigValue(1, NameDir, &fDir, sizeof(Word));
      SetConfigValue(1, NameDirHidden, &fDirHidden, sizeof(Word));
      SetConfigValue(1, NameDirAppleShare, &fDirAppleShare, sizeof(Word));
      SetConfigValue(1, NameDirRemovable, &fDirRemovable, sizeof(Word));
    }

    if (screens[2])
    {
      fPut = GetCtlValueByID(win, CtrlPut);
      fPutMkdir = GetCtlValueByID(win, CtrlPutMkdir);
      fPutOverwrite = GetCtlValueByID(win, CtrlPutOverwrite);

      SetConfigValue(1, NamePut, &fPut, sizeof(Word));
      SetConfigValue(1, NamePutMkdir, &fPutMkdir, sizeof(Word));
      SetConfigValue(1, NamePutOverwrite, &fPutOverwrite, sizeof(Word));

    }

    if (screens[3])
    {
      fLog = GetCtlValueByID(win, CtrlLog);

      if (newLog)
      {
	if (fLogDirH) DisposeHandle(fLogDirH);

	SetConfigValue(rC1InputString, NameLogDir, *newLog, GetHandleSize(newLog));
	fLogDirH = newLog;
	fLogDir = (GSString255Ptr)*newLog;
      }
      if (!fLogDir || fLogDir->length == 0) fLog = false;

      SetConfigValue(1, NameLog, &fLog, sizeof(Word));

    }
    if (screens[4])
    {
   		fAppleSingle = GetCtlValueByID(win, Ctrl_AS) - (EncapNever);
   		fAppleDouble = GetCtlValueByID(win, Ctrl_AD) - (EncapNever);
   		fMacBinary = GetCtlValueByID(win, Ctrl_MB) - (EncapNever);
   		
   		SetConfigValue(1, NameAppleSingle, &fAppleSingle, sizeof(Word));
   		SetConfigValue(1, NameAppleDouble, &fAppleDouble, sizeof(Word));
   		SetConfigValue(1, NameMacBinary, &fMacBinary, sizeof(Word));
    }


    //SetConfigValue(1, NameMTU, &fMTU, sizeof(Word));


    UpdateResourceFile(fd);

    SetCurResourceFile(oFile);
    SetResourceFileDepth(oDepth);
  }

  CloseWindow(win);                             

  for (i = 0; i < SCREEN_COUNT; i++)
  	if (screens[i]) DisposeHandle(screens[i]);
}
