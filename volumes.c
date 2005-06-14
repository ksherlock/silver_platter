#pragma noroot
#pragma lint -1
#pragma optimize -1
#pragma debug 0x8000


#include <gsos.h>
#include <ctype.h>

#include "config.h"


#define	dcRemovable		0x0004
#define dcBlockDevice	0x0080


// returns TRUE if path is/is in the system folder, false otherwise.
Word IsSystemFolder(GSString255Ptr path)
{
static Word fSystem = false;
static ResultBuf32 bootName = { 36 };
static GetNameRecGS NameDCB= {1, (ResultBuf255Ptr)&bootName};


Word i;
Word j;

	if (fSystem == false)
	{
		GetBootVol(&NameDCB);
		fSystem = true;
		for (i = 0; i < bootName.bufString.length; i++)
		{
			Word c = bootName.bufString.text[i];
			if (c == ':') c = '/';
			else if (islower(c)) c = _toupper(c);
			else continue;
			
			
			bootName.bufString.text[i] = c;	
		}
	}
	
	
	// 
	if (path->length < (6 + bootName.bufString.length)) 
		return false;
		
	// check if the volume matches.
	
	for (i = 0; i < bootName.bufString.length; i++)
	{
		Word c = bootName.bufString.text[i];
		Word d = path->text[i];
		if (islower(d)) d = _toupper(d);
		
		if (d != c) return false;	
		
	}
	
	// ok, it matched... now check for "system/
	for (j = 0; j < 6; j++)
	{
		Word c;
		
		c = path->text[i + j];
		if (islower(c)) c = _toupper(c);
		
		if (c != "SYSTEM"[j]) return false;
	}
	
	if (path->length == 6 + i) return true;
	if (path->text[i + 6] == '/') return true;
	
	return false; 
	
}


/*
 * iterate through the volumes.
 * returns 0 when done.
 * start with cookie of 1, next cookie will be returned,
 */
Word GetNextVolume(Word cookie, VolumeRecGS *VolumeDCB)
{
  static ResultBuf32 dName = { 32 };
  static DInfoRecGS DInfoDCB = {3, 0, &dName};
  
  static Word rcache[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  Word i, j;

  if (cookie == 0) return 0;

  // index into removable cache.
  i = 1;
  for (j = cookie & 0x0f; j; j--) i <<= 1;
  j = cookie >> 4;

  VolumeDCB->devName = &dName.bufString;

  for ( ; ; cookie++, i <<= 1)
  {
    if (i == 0)
    {
      j++;
      i++;	// inc <dp faster than lda #1 sta <dp
    }
    if ((fDirRemovable == false) && (rcache[j] & i)) continue;

    DInfoDCB.devNum = cookie;
    DInfoGS(&DInfoDCB);
    if (_toolErr) break;

    if (DInfoDCB.characteristics & dcBlockDevice == 0) continue;

    // if remoavble, update array.
    if (DInfoDCB.characteristics & dcRemovable)
    {
      rcache[j] |= i;

      if (fDirRemovable == false) continue;
    }

    VolumeGS(VolumeDCB);
    if (_toolErr) continue;


    if ((VolumeDCB->fileSysID == appleShareFSID) 
        && (fDirAppleShare == false))
      	continue;

    return cookie + 1;

  }

  return 0;
}
