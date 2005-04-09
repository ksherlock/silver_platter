#pragma noroot
#pragma lint -1
#pragma optimize -1


#include <gsos.h>
#include <misctool.h>
#include <tcpip.h>
#include <timetool.h>

#include "server.h"
#include "applesingle.h"
#include <kstring.h>

extern pascal Word swap16(Word);
extern pascal LongWord swap32(LongWord);

extern int orca_sprintf(char *, const char *, ...);

struct optionData
{
  Word fileSysID;
  LongWord fileType;
  LongWord creator;
  Word finderFlags;
  LongWord iconLoc;
  Word fileWindow;




};



// seconds corresponding to jan 1, 2000.
#define MAC_2K 0xb492f400

Word AppleSingle(struct qEntry *q)
{
static ASHeader header;
static ASFileDates dates;
static ASEntry entry;
static ASProdosInfo pInfo;
static OpenRecGS OpenDCB;
static FileInfoRecGS InfoDCB;
static Word optionData[25 + 2];

ASFinderInfo *fdInfo;

Word i;
GSString255Ptr g;

LongWord total;
LongWord eof;
LongWord resourceEOF;
TimeRec modDateTime;
Word subtotal;
Word numEntries;

  total = 0;
  numEntries = 0;

  if (q->command = CMD_GET)
  {
    OpenDCB.pCount = 15;
    OpenDCB.pathname = (GSString255Ptr)*q->fullpath;
    OpenDCB.requestAccess = readEnable;
    OpenDCB.resourceNumber = 0;
    OpenDCB.optionList = (ResultBuf255Ptr)optionData;

    optionData[0] = 50 + 4;

    OpenGS(&OpenDCB);
    if (_toolErr && _toolErr != buffTooSmall)
      return ProcessError(404, q);

    q->fd = OpenDCB.refNum;

    // We don't do directories.
    if (OpenDCB.fileType == 0x0f) return ProcessError(400, q);

    // copy the values...
    eof = OpenDCB.eof;
    resourceEOF = OpenDCB.resourceEOF;
    modDateTime = OpenDCB.modDateTime;

    pInfo.access = swap16(OpenDCB.access);
    pInfo.filetype = swap16(OpenDCB.fileType);
    pInfo.auxtype = swap32(OpenDCB.auxType);

    // now open resource fork...
    if (OpenDCB.resourceEOF)
    {
      OpenDCB.resourceNumber = 1;
      OpenDCB.optionList = NULL;

      OpenGS(&OpenDCB);
      if (_toolErr)
      {
        return ProcessError(404, q);
      }
      q->rfd = OpenDCB.refNum;
    }                  
  } //CMD_GET
  else
  {
    InfoDCB.pCount = 12;
    InfoDCB.pathname = (GSString255Ptr)*q->fullpath;
    InfoDCB.optionList = (ResultBuf255Ptr)optionData;

    optionData[0] = 50 + 4;

    GetFileInfoGS(&InfoDCB);
    if (_toolErr && _toolErr != buffTooSmall)
      return ProcessError(404, q);

    // We don't do directories.
    if (InfoDCB.fileType == 0x0f) return ProcessError(400, q);

    // copy the values...
    eof = InfoDCB.eof;
    resourceEOF = InfoDCB.resourceEOF;
    modDateTime = InfoDCB.modDateTime;

  }


  // find the filename
  g = (GSString255Ptr)*q->fullpath;
  i = g->length;
  // i is 1 based, so --i is correct.
  while (g->text[--i] != '/');
  // i now points to the / character.  Add 1 since it won't be sent.
  i++;

  total += g->length - i;

  // at a minimum, it will have a name, prodos info, and date info.
  total += sizeof(ASHeader) + sizeof(ASProdosInfo) + sizeof(ASFileDates)
    + sizeof(ASEntry) + sizeof(ASEntry) + sizeof(ASEntry);

  numEntries = 3;

  subtotal = total;  // amount we're sending now.

  fdInfo = NULL;
  // optionData[0] = buffersize
  // optionData[1] = resultsize
  // optionData[2] = fileSysID
  // optionData[3..] = Mac finder data

  if (optionData[1] >= sizeof(ASFinderInfo) + 2)
  {
    Word fileSysID;

    fileSysID = optionData[2];
    if (fileSysID == proDOSFSID
      || fileSysID == hfsFSID
      || fileSysID == appleShareFSID)
    {
      fdInfo = (ASFinderInfo *)&optionData[3];
      total += sizeof(ASFinderInfo) + sizeof(ASEntry);
      subtotal += sizeof(ASEntry);
      numEntries++;        
    }
  }

  if (eof)
  {
    total += eof + sizeof(ASEntry);
    subtotal += sizeof(ASEntry);
    numEntries++;
  }
  if (resourceEOF)
  {
    total += resourceEOF + sizeof(ASEntry);
    subtotal += sizeof(ASEntry);
    numEntries++;        
  }                     


  SendHeader(q, 200, total, &modDateTime, "application/applesingle", true);

  // need to send out the header data....
  if (q->command == CMD_GET)
  {
  LongWord secs;
  tiPrefRec tiPrefs;
  LongWord offset;
  Word ipid;

    ipid = q->ipid;

    offset = 0;

    if (q->flags & FLAG_CHUNKED)
    {
      int i = orca_sprintf(buffer, "%x\r\n", subtotal);
      TCPIPWriteTCP(ipid, buffer, i, false, false);
    }


    // sigh... I should give orca/c native ntohs support for constants.

    header.magicNum = swap32(AS_MAGIC_NUMBER);
    header.versionNum = swap32(AS_VERSION_NUMBER);
    header.numEntries = swap16(numEntries);

    offset = sizeof(ASHeader) + numEntries * sizeof(ASEntry);

    TCPIPWriteTCP(ipid, (dataPtr)&header, sizeof(ASHeader), false, false);

    // filename...
    entry.entryID = swap32(AS_REAL_NAME);
    entry.entryOffset = swap32(offset);
    entry.entryLength = swap32(g->length - i);
    TCPIPWriteTCP(ipid, (dataPtr)&entry, sizeof(ASEntry), false, false);
    offset += g->length - i;

    // Prodos info
    entry.entryID = swap32(AS_PRODOS_INFO);
    entry.entryOffset = swap32(offset);
    entry.entryLength = swap32(sizeof(ASProdosInfo));
    TCPIPWriteTCP(ipid, (dataPtr)&entry, sizeof(ASEntry), false, false);
    offset += sizeof(ASProdosInfo);              

    // Dates info
    entry.entryID = swap32(AS_FILE_DATES);
    entry.entryOffset = swap32(offset);
    entry.entryLength = swap32(sizeof(ASFileDates));
    TCPIPWriteTCP(ipid, (dataPtr)&entry, sizeof(ASEntry), false, false);
    offset += sizeof(ASFileDates);              

    // macintosh finder info
    if (fdInfo)
    {
      entry.entryID = swap32(AS_FINDER_INFO);
      entry.entryOffset = swap32(offset);
      entry.entryLength = swap32(sizeof(ASFinderInfo));
      TCPIPWriteTCP(ipid, (dataPtr)&entry, sizeof(ASEntry), false, false);
      offset += sizeof(ASFinderInfo);
    }

    // Data fork
    if (eof)
    {
      entry.entryID = swap32(AS_DATA_FORK);
      entry.entryOffset = swap32(offset);
      entry.entryLength = swap32(eof);
      TCPIPWriteTCP(ipid, (dataPtr)&entry, sizeof(ASEntry), false, false);
      offset += eof;
    }

    // Resource fork
    if (resourceEOF)
    {
      entry.entryID = swap32(AS_RESOURCE_FORK);
      entry.entryOffset = swap32(offset);
      entry.entryLength = swap32(resourceEOF);
      TCPIPWriteTCP(ipid, (dataPtr)&entry, sizeof(ASEntry), false, false);
      offset += resourceEOF;
    }

    // now send the file name.
    TCPIPWriteTCP(ipid, (dataPtr)&g->text[i], g->length - i, false, false);

    // now send the ProdosInfo
    TCPIPWriteTCP(ipid, (dataPtr)&pInfo, sizeof(ASProdosInfo), false, false);

    // now send the dates...

    tiPrefs.pCount = 3;
    tiGetTimePrefs(&tiPrefs);

    secs = ConvSeconds(TimeRec2Secs, 0, (Pointer)&OpenDCB.createDateTime);
    secs += tiPrefs.secOffset;
    dates.create = swap32(secs - MAC_2K);

    secs = ConvSeconds(TimeRec2Secs, 0, (Pointer)&OpenDCB.modDateTime);
    secs += tiPrefs.secOffset;
    dates.modify = swap32(secs - MAC_2K);

    // these aren't available on ProDOS, so fake them.
    dates.backup = dates.create;
    dates.access = dates.modify;

    TCPIPWriteTCP(ipid, (dataPtr)&dates, sizeof(ASFileDates), false, false);

    // macintosh finder info
    if (fdInfo)
    {
      TCPIPWriteTCP(ipid, (dataPtr)fdInfo, sizeof(ASFinderInfo), false, false);
    }                       

    if (q->flags & FLAG_CHUNKED)
      TCPIPWriteTCP(ipid, "\r\n", 2, false, false);

    TCPIPPoll();


    // data and resource forks will be sent in server.c
    // unless they're empty...
    q->state = STATE_ASINGLE_1;
    if (eof == 0)
    {
      q->state = STATE_ASINGLE_2;
      if (resourceEOF == 0) q->state = STATE_CLOSE;
    }
  } // q->command == CMD_GET
  else  
    q->state = STATE_CLOSE;

  return 200;
}
