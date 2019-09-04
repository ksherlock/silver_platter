#ifndef __APPLESINGLE_H__
#define __APPLESINGLE_H__

#ifndef __TYPES__
#include <TYPES.h>
#endif

#define AS_MAGIC_NUMBER 0x00051600
#define AS_VERSION_NUMBER 0x00020000

#define AD_MAGIC_NUMBER 0x00051607
#define AD_VERSION_NUMBER 0x00020000

// entry_id values
#define AS_DATA_FORK 0x01
#define AS_RESOURCE_FORK 0x02
#define AS_REAL_NAME 0x03
#define AS_COMMENT 0x04
#define AS_ICON_BW 0x05
#define AS_ICON_COLOR 0x06
#define AS_FILE_DATES 0x08
#define AS_FINDER_INFO 0x09
#define AS_MACINTOSH_INFO 0x0a
#define AS_PRODOS_INFO 0x0b
#define AS_MSDOS_INFO 0x0c
#define AS_SHORT_NAME 0x0d
#define AS_AFP_INFO 0x0e
#define AS_AS_DIR_ID 0x0f

typedef struct ASHeader {
  LongWord magicNum;
  LongWord versionNum;
  Byte filler[16];
  Word numEntries;
} ASHeader;

typedef struct ASEntry {
  LongWord entryID;
  LongWord entryOffset;
  LongWord entryLength;
} ASEntry;

// signed number of seconds before or after 1/1/2000 GMT.
// unknown should be 0x8000000
typedef struct ASFileDates {
  LongWord create;
  LongWord modify;
  LongWord backup;
  LongWord access;

} ASFileDates;

typedef struct ASFinderInfo {
  LongWord fdType;
  LongWord fdCreator;
  Word fdFlags;
  Word fdLocation[2];
  Word fdFolder;

  Word fdIconID;
  Word fdUnused[4];
  Word fdComment;
  LongWord fdDirectory;
} ASFinderInfo;

typedef struct ASProdosInfo {
  Word access;
  Word filetype;
  LongWord auxtype;

} ASProdosInfo;

#endif
