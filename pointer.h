#ifndef __POINTER_H__
#define __POINTER_H__

#ifndef TYPES
#include <types.h>
#endif

extern pascal void PointerStartUp(Word);
extern pascal void *NewPointer(LongWord);
extern pascal void DisposePointer(void *);
extern pascal void RetainPointer(void *);
extern pascal void ReleasePointer(void *);
extern pascal void *ReallocPointer(void *, LongWord);

#endif
