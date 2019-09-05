#ifndef __TYPES__
#include <Types.h>
#endif

#ifdef __CONTROL__
extern pascal void SetCtlTextByID(WindowPtr, LongWord, Word, Ref);
#endif

#ifdef __STDFILE__
extern pascal void SFGetFolder2(Word, Word, Word, Ref, SFReplyRec2Ptr);
#endif

#ifdef __WINDOW__
extern pascal void CenterWindow(WindowPtr);
#endif
