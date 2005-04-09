#ifndef __TYPES__
#include <types.h>
#endif

extern word fAbort;
extern Word fJail;
extern Word fPort;
extern Word fDir;
extern Word fTeach;
extern Word fMTU;

extern GSString255Ptr fRoot;
extern Handle fRootH;

Word LoadConfig(Word);
void UnloadConfig(void);
void DoConfig(Word);
