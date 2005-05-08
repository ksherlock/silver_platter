#ifndef __TYPES__
#include <types.h>
#endif

extern word fAbort;
extern Word fJail;
extern Word fPort;
extern Word fDir;
extern Word fTeach;
extern Word fMTU;

extern Word fLog;
extern GSString255Ptr fLogDir;

extern GSString255Ptr fRoot;
extern Handle fRootH;

// PUT
extern Word fPut;
extern Word fPutOverWrite;
extern Word fPutMkdir;	



Word LoadConfig(Word);
void UnloadConfig(void);
void DoConfig(Word);
