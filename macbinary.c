#pragma noroot
#pragma optimize -1
#pragma debug 0x8000
#pragma lint -1


#include <gsos.h>
#include <misctool.h>
#include <string.h>

#include "server.h"
#include "globals.h"
#include "macbinary.h"
#include "AppleSingle.h"

extern pascal Word swap16(Word);
extern pascal LongWord swap32(LongWord);

extern pascal Word GetHFSInfo(GSString255Ptr, void *);



Word MacBinary(struct qEntry *q)
{
static struct MB_1_Header header;
Longword size;
GSString255Ptr path;
static ASFinderInfo finderData;
Word i;
Word len;
Word j;

	if (InfoDCB.fileType == 0x0f) return ProcessError(400, q);
	
	if (q->command == CMD_HEAD)
	{				
		SendHeader(q, 200, -1L, &InfoDCB.modDateTime, 
		"application/x-macbinary", NULL, 0);
		
		q->state = STATE_CLOSE;
		
		return 200;
	}
	
	// open the forks, send the header.
	
	
	memset(&header, 0, sizeof(struct MB_1_Header));
	


	// copy the name over
	path = q->fullpath;
	
	len = path->length;
	for (i = len; i; i--)
	{
		if (path->text[i - 1] == '/') break;	
	}
	header.nameLength = len - i;
	for (j = 0;i < len; i++)
		header.name[j++] = path->text[i];
	
    OpenDCB.pCount = 15;
    OpenDCB.pathname = q->fullpath;
    OpenDCB.requestAccess = readEnable;
    OpenDCB.resourceNumber = 0;
    OpenDCB.optionList = NULL;

    OpenGS(&OpenDCB);
    if (_toolErr)
      return ProcessError(404, q);

    q->fd = OpenDCB.refNum;
        
    header.dataLength = swap32(OpenDCB.eof);
    header.resourceLength = swap32(OpenDCB.resourceEOF);

    header.creationDate =
          swap32(ConvSeconds(TimeRec2Secs, 0,
            (Pointer)&OpenDCB.createDateTime));

    header.modificationDate =
          swap32(ConvSeconds(TimeRec2Secs, 0,
            (Pointer)&OpenDCB.modDateTime));


  if (GetHFSInfo(q->fullpath, &finderData) == 0)
  {
    header.fileType = finderData.fdType;
    header.fileCreator = finderData.fdCreator;
  }

  // if resource data, must open the fork now...
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
  
	size = ((OpenDCB.eof + 127) & 0xffffff80)
		+ ((OpenDCB.resourceEOF + 127) & 0xffffff80)
		+ 128;
			
	SendHeader(q, 200, size, &OpenDCB.modDateTime, 
		"application/x-macbinary", NULL, 0);
		
	WriteData(q, (char *)&header, 128);
		
	q->state = STATE_ASINGLE_1;
	
	return 200;
	
}
