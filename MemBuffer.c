#pragma lint -1
#pragma optimize -1
#pragma noroot
#pragma debug 0x8000

#include "Memory.h"
#include "MemBuffer.h"

// returns _toolErr on failure.
Word BufferAppend(MemBuffer *m, void *data, Word size)
{
char *cp;
Handle h = m->h;

  if (m->used + size > m->alloc)
  {
    HUnlock(h);
    m->alloc += 2048;
    SetHandleSize(m->alloc, h);
    if (_toolErr) return _toolErr;
    HLock(h);
  }
  cp = *h + m->used;

  BlockMove(data, cp, size);
  m->used += size;

  return 0;	
	
}
