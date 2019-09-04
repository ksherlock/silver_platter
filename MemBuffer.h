#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <Types.h>

typedef struct MemBuffer {
  Handle h;
  Word alloc;
  Word used;
} MemBuffer;

#define CREATE_BUFFER(name, h) MemBuffer name = {h, 0, 0}

Word BufferAppend(MemBuffer *, void *, Word);

#endif
