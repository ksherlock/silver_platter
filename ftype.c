/*
 * FType.apple support
 */

#pragma noroot
#pragma lint - 1
#pragma optimize - 1
#pragma debug 0x8000

#include <gsos.h>
#include <memory.h>
#include <types.h>

typedef struct FTHeader {
  Word version;
  Word flags;
  Word numEntries;
  Word space;
  Word recordSize;
  Word offset;
} FTHeader;

typedef struct FTEntry {
  Word fType;
  LongWord auxType;
  Word flags;
  Word offset;
} FTEntry;

// cheating since we know ftype is really 8-bit.
static Word offsets[256];
static Handle h = NULL;
static Pointer p = NULL;
static Word fRecordSize;

static OpenRecGS OpenDCB;
static IORecGS ReadDCB;

#define xstr "*:Icons:FType.Apple"
static GSString32 path = {sizeof(xstr) - 1, xstr};
#undef xstr

void LoadFTypes(Word MyID) {
  Word CloseDCB[2];

  OpenDCB.pCount = 15;
  OpenDCB.pathname = (GSString255Ptr)&path;
  OpenDCB.requestAccess = readEnable;
  OpenDCB.resourceNumber = 0;
  OpenDCB.optionList = NULL;
  OpenGS(&OpenDCB);
  if (_toolErr)
    return;
  if (OpenDCB.eof) {
    h = NewHandle(OpenDCB.eof, MyID, attrLocked | attrFixed, NULL);
    if (_toolErr) {
      h = NULL;
      p = NULL;
    } else {
      p = *h;
      ReadDCB.pCount = 4;
      ReadDCB.refNum = OpenDCB.refNum;
      ReadDCB.dataBuffer = p;
      ReadDCB.requestCount = OpenDCB.eof;
      ReadGS(&ReadDCB);
      if (_toolErr || ReadDCB.transferCount != ReadDCB.requestCount) {
        DisposeHandle(h);
        h = NULL;
        p = NULL;
      } else {
        FTEntry *ft;
        Word i;
        Word count;
        Word offset;
        Word f, lastf;

        fRecordSize = ((FTHeader *)p)->recordSize;
        count = ((FTHeader *)p)->numEntries;
        offset = ((FTHeader *)p)->offset;

        f = lastf = -1;
        // now build the offset table....
        for (i = 0; i < count; i++) {
          ft = (FTEntry *)(p + offset);
          f = ft->fType;
          if (f != lastf && f < 256) {
            offsets[f] = offset;
          }
          lastf = f;
          offset += fRecordSize;
        }
      }
    }
  }

  CloseDCB[0] = 1;
  CloseDCB[1] = OpenDCB.refNum;
  CloseGS(&CloseDCB);
}

void UnloadFTypes(void) {
  if (h)
    DisposeHandle(h);
  h = NULL;
  p = NULL;
}

// returns a pascal string.
const char *FindFType(Word fType, LongWord auxType) {
  char *cp;
  char *ret = "\pUnknown";
  FTEntry *ft;

  if (fType > 255 || !offsets[fType] || !p)
    return ret;

  cp = p + offsets[fType];
  ft = (FTEntry *)cp;

  // this will actually overflow on the last entry into the names,
  // but that's ok.

  do {
    // bit 15 == wildcard on the auxtype.
    if (ft->flags & 0x8000)
      ret = p + ft->offset;
    if (ft->auxType == auxType) {
      ret = p + ft->offset;
      break;
    }
    cp += fRecordSize;
    ft = (FTEntry *)cp;
  } while (ft->fType == fType);

  return ret;
}
