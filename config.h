#ifndef __TYPES__
#include <types.h>
#endif

extern word fAbort;
extern Word fJail;
extern Word fPort;

extern Word fTeach;
extern Word fMTU;

extern Word fLog;
extern GSString255Ptr fLogDir;

extern GSString255Ptr fRoot;
extern Handle fRootH;

// PUT
extern Word fPut;
extern Word fPutOverwrite;
extern Word fPutMkdir;	

extern Word fAppleSingle;
extern Word fAppleDouble;
extern Word fMacBinary;


extern Word fDir;
extern Word fDirHidden;
extern Word fDirAppleShare;
extern Word fDirRemovable;

extern Word fWebDav;

Word LoadConfig(Word);
void UnloadConfig(void);
void DoConfig(Word);
