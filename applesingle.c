#pragma noroot
#pragma lint -1
#pragma optimize -1
#pragma debug 0x8000

#include <gsos.h>
#include <misctool.h>
#include <tcpip.h>
#include <timetool.h>

#include "server.h"
#include "applesingle.h"
#include "globals.h"

extern pascal Word swap16(Word);
extern pascal LongWord swap32(LongWord);

extern int orca_sprintf(char *, const char *, ...);


extern pascal Word GetHFSInfo(GSString255Ptr, void *);


// seconds corresponding to jan 1, 2000.
#define MAC_2K 0xb492f400

Word AppleSingle(struct qEntry *q)
{
static ASHeader header;
static ASFileDates dates;
static ASEntry entry;
static ASProdosInfo pInfo;

ASFinderInfo finderData;
Word fi = false;

Word i;
GSString255Ptr g;

LongWord total;
LongWord eof;
LongWord resourceEOF;
TimeRec modDateTime;
Word subtotal;
Word numEntries;

LongWord secs;
tiPrefRec tiPrefs;
LongWord offset;
Word ipid;


Word asingle = (q->moreFlags == CGI_APPLESINGLE);

	total = 0;
	numEntries = 0;
	
  
	if (InfoDCB.fileType == 0x0f) return ProcessError(400, q);
	
	if (q->command == CMD_HEAD)
	{
		SendHeader(q, 200, -1L, &InfoDCB.modDateTime, 
		asingle ? "application/applesingle" : "multipart/appledouble", 
		NULL, 0);
		
		q->state = STATE_CLOSE;
		
		return 200;  	
	}
	
	OpenDCB.pCount = 15;
	OpenDCB.pathname = q->fullpath;
	OpenDCB.requestAccess = readEnable;
	OpenDCB.resourceNumber = 0;
	OpenDCB.optionList = NULL;
	
	OpenGS(&OpenDCB);
	if (_toolErr && _toolErr != buffTooSmall)
	  return ProcessError(404, q);
	
	q->fd = OpenDCB.refNum;
	
	
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




  // find the filename
  g = q->fullpath;
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

  if (GetHFSInfo(q->fullpath, &finderData) == 0)
  {
      fi++;
      total += sizeof(ASFinderInfo) + sizeof(ASEntry);
      subtotal += sizeof(ASEntry);
      numEntries++;        
  }

  if (eof && asingle)
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


  SendHeader(q, 200, total, &modDateTime, 
    asingle ? "application/applesingle" : "multipart/appledouble", NULL, 0);

  // need to send out the header data....
	
	
	
	ipid = q->ipid;
	
	offset = 0;
	
	if (q->flags & FLAG_CHUNKED)
	{
		int i = orca_sprintf(buffer, "%x\r\n", subtotal);
		TCPIPWriteTCP(ipid, buffer, i, false, false);
	}
	
	
	// sigh... I should give orca/c native ntohs support for constants.
	
	header.magicNum = swap32(asingle ? AS_MAGIC_NUMBER : AD_MAGIC_NUMBER);
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
	if (fi)
	{
		entry.entryID = swap32(AS_FINDER_INFO);
		entry.entryOffset = swap32(offset);
		entry.entryLength = swap32(sizeof(ASFinderInfo));
		TCPIPWriteTCP(ipid, (dataPtr)&entry, sizeof(ASEntry), false, false);
		offset += sizeof(ASFinderInfo);
	}
	
	// Data fork
	if (eof && asingle)
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
	if (fi)
	{
		TCPIPWriteTCP(ipid, (dataPtr)&finderData, sizeof(ASFinderInfo), false, false);
	}                       
	
	if (q->flags & FLAG_CHUNKED)
	TCPIPWriteTCP(ipid, "\r\n", 2, false, false);
	
	TCPIPPoll();
	
	
	// data and resource forks will be sent in server.c
	// unless they're empty...
	q->state = STATE_ASINGLE_1;
	if (resourceEOF == 0) q->state = STATE_WRITE;  // no rfork.
	
	if ((eof == 0) || (asingle == 0))
	{
		q->state = STATE_ASINGLE_2;
		if (resourceEOF == 0) q->state = STATE_CLOSE;
	}

	return 200;
}
