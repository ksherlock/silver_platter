#pragma noroot
#pragma optimize -1
#pragma debug 0x8000
#pragma lint -1


#include <gsos.h>
#include <string.h>

#include "server.h"
#include "globals.h"
#include "macbinary.h"

extern pascal Word swap16(Word);
extern pascal LongWord swap32(LongWord);



Word MacBinary(struct qEntry *q)
{
static struct MB_1_Header header;
Longword size;
static Word optionData[25 + 2];

	
	
	if (q->command == CMD_HEAD)
	{
		InfoDCB.pCount = 12;
    	InfoDCB.pathname = q->fullpath;
    	InfoDCB.optionList = NULL;
		GetFileInfoGS(&InfoDCB);

		if (_toolErr) return ProcessError(404, q);
		if (InfoDCB.fileType == 0x0f) return ProcessError(400, q);
		
		// file data is NULL-padded to 128 bytes.
		
		size = ((InfoDCB.eof + 127) & 0xffffff80)
			+ ((InfoDCB.resourceEOF + 127) & 0xffffff80)
			+ 128;
			
		SendHeader(q, 200, size, &InfoDCB.modDateTime, 
		"application/x-macbinary", NULL, 0);
		
		q->state = STATE_CLOSE;
		
		return 200;
	}
	
	// open the forks, send the header.
	
	
	memset(&header, 0, sizeof(struct MB_1_Header));
	
    OpenDCB.pCount = 15;
    OpenDCB.pathname = q->fullpath;
    OpenDCB.requestAccess = readEnable;
    OpenDCB.resourceNumber = 0;
    OpenDCB.optionList = (ResultBuf255Ptr)optionData;
	
	optionData[0] = 50 + 4;

    OpenGS(&OpenDCB);
    if (_toolErr && _toolErr != buffTooSmall)
      return ProcessError(404, q);

    q->fd = OpenDCB.refNum;
    
    if (OpenDCB.fileType == 0x0f) return ProcessError(400, q);
    
    header->dataLength = swap32(OpenDCB.eof);
    header->resourceLength = swap32(OpenDCB.resourceEOF);
    
  if (optionData[1] >= 10)
  {
    Word fileSysID;
    LongWord *lw;

    fileSysID = optionData[2];
    if (fileSysID == proDOSFSID
      || fileSysID == hfsFSID
      || fileSysID == appleShareFSID)
    {
    	lw = (LongWord *)optionData[3];
    	header.fileType = swap32(*lw);
    	lw = (LongWord *)optionData[5];
    	header.fileCreator = swap32(*lw);
    }
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
