#pragma noroot
#pragma lint -1
#pragma optimize -1
#pragma debug 0x8000


#include <gsos.h>
#include "config.h"


#define	dcRemovable		0x0004
#define dcBlockDevice	0x0080


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
