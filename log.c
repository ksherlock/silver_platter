#pragma optimize -1
#pragma noroot
#pragma lint -1
#pragma debug 0x8000

#include <gsos.h>
#include <memory.h>
#include <MiscTool.h>
#include <stdfile.h>
#include <textedit.h>
#include <window.h>

#include "config.h"
#include "pointer.h"


extern int orca_sprintf(char *, const char *, ...);


static IORecGS WriteDCB;
static CreateRecGS CreateDCB;
static OpenRecGS OpenDCB;
static NameRecGS DestroyDCB;
static SetPositionRecGS SPositionDCB;


// create a log file, return fd.
Word CreateLog(void)
{
Word fd = 0;
GSString255Ptr gstr;
Word i;
TimeRec tr;
Word oldPrefs;

  oldPrefs = DoSysPrefs(0xffff, 0x6000);

  // :wwwyyyymmdd.txt
  if (fLog && fLogDir)
  {

    tr = ReadTimeHex();

    i = fLogDir->length;

    gstr = NewPointer(i + 17 + 2); // + 2 for length
    if (gstr == NULL) return 0;

    i = orca_sprintf(gstr->text, "%B/www%04u%02u%02u.txt",
      fLogDir, tr.year + 1900, tr.month + 1, tr.day + 1);

    gstr->length = i;

    CreateDCB.pCount = 4;
    CreateDCB.pathname = gstr;
    CreateDCB.access = 0xC3;
    CreateDCB.fileType = 4;
    CreateDCB.auxType = 0;

    OpenDCB.pCount = 15;
    OpenDCB.pathname = gstr;
    OpenDCB.requestAccess = writeEnable;
    OpenDCB.resourceNumber = 0;
    OpenDCB.optionList = NULL;

    CreateGS(&CreateDCB);
    // ignore error.

    OpenGS(&OpenDCB);
    if(_toolErr == 0)
    {
      fd = OpenDCB.refNum;

      SPositionDCB.pCount = 3;
      SPositionDCB.refNum = fd;
      SPositionDCB.base = eofMinus;
      SPositionDCB.displacement = 0;
      SetMarkGS(&SPositionDCB);
    }
    DisposePointer(gstr);
  }

  DoSysPrefs(0xffff, oldPrefs);

  return fd;
}


void SaveLog(Word MyID, Handle teH)
{

Word CloseDCB[2];
SFReplyRec2 myReply;
GSString255Ptr gstr;
Handle h;
LongWord total;


  myReply.nameRefDesc = refIsNewHandle;
  myReply.pathRefDesc = refIsNewHandle;

  SFPutFile2(
    20, 20,
    refIsPointer, (Ref)"\pSave Log File",
    refIsPointer, (Ref)"\x07\x00www.log",
    &myReply);

  if (myReply.good)
  {
    h = (Handle)myReply.pathRef;
    HLock(h);
    gstr = (GSString255Ptr)(*h + 2);

    h = NewHandle(1, MyID, 0, NULL);
    if (_toolErr) h = NULL;
    else do
    {
      total = TEGetText(teDataIsTextBlock+refIsHandle*8, (Ref)h,
        0, 0, (Ref)0, teH);

      DestroyDCB.pCount = 1;
      DestroyDCB.pathname = gstr;
      DestroyGS(&DestroyDCB);
      //if (_toolErr && _toolErr != fileNotFound)
      //{
      //  ErrorWindow(0, NULL, _toolErr);
      //  break;
      //}

      CreateDCB.pCount = 4;
      CreateDCB.pathname = gstr;
      CreateDCB.access = 0xC3;
      CreateDCB.fileType = 4;
      CreateDCB.auxType = 0;
      CreateGS(&CreateDCB);
      if (_toolErr)
      {
        ErrorWindow(0, NULL, _toolErr);
        break;
      }

      OpenDCB.pCount = 15;
      OpenDCB.pathname = gstr;
      OpenDCB.requestAccess = writeEnable;
      OpenDCB.resourceNumber = 0;
      OpenDCB.optionList = NULL;
      OpenGS(&OpenDCB);
      if (_toolErr)
      {
        ErrorWindow(0, NULL, _toolErr);
        break;
      }

      CloseDCB[0] = 1;
      CloseDCB[1] = OpenDCB.refNum;

      if (total)
      {
        WriteDCB.pCount = 4;
        WriteDCB.refNum = OpenDCB.refNum;
        WriteDCB.dataBuffer = *h;
        WriteDCB.requestCount = total;
        WriteGS(&WriteDCB);
        if (_toolErr)
        {
          ErrorWindow(0, NULL, _toolErr);
        }
      }
      CloseGS(CloseDCB);
    } while (false);

    if (h) DisposeHandle(h);
    DisposeHandle((Handle)myReply.pathRef);
    DisposeHandle((Handle)myReply.nameRef);
  }
}
