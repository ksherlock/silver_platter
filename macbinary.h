#ifndef __MACBINARY__
#define __MACBINARY__

#include <types.h>

/*
  http://www.lazerware.com/formats/macbinary/macbinary.html
 */

// version 1

struct MB_1_Header {
  Byte zero;
  Byte nameLength;
  Byte name[63];
  LongWord fileType;
  LongWord fileCreator;
  Byte finderFlags;
  Byte zero1;
  Word vWindow;
  Word hWindow;
  Word fileID;
  Byte protected;
  Byte zero2;
  LongWord dataLength;
  LongWord resourceLength;
  LongWord creationDate;
  LongWord modificationDate;

  Byte reserved[29];
};

// version 2.
struct MB_2_Header {
  Byte oldVersion;
  Byte nameLength;
  Byte name[63];
  LongWord fileType;
  LongWord fileCreator;
  Byte finderFlags;
  Byte zero1;
  Word vWindow;
  Word hWindow;
  Word fileID;
  Byte protected;
  Byte zero2;
  LongWord dataLength;
  LongWord resourceLength;
  LongWord creationDate;
  LongWord modificationDate;
  Word infoLength;
  Byte finderFlags2;
  LongWord unpackedLength; // ignore.  set to 0.
  Word secondHeaderLength; // 0.
  Byte version;            // starts at 129
  Byte minVersion;         // starts 129
  Word crc;                // crc of previous 124 bytes.
  Word zero3;
};

#endif
