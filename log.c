#pragma optimize -1
#pragma noroot
#pragma lint -1

#include <gsos.h>
#include <memory.h>
#include <stdfile.h>
#include <textedit.h>
#include <window.h>


void SaveLog(Word MyID, Handle teH)
{
static IORecGS WriteDCB;
static CreateRecGS CreateDCB;
static OpenRecGS OpenDCB;
static NameRecGS DestroyDCB;

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
