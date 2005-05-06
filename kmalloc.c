#pragma noroot
#pragma lint -1
#pragma debug 0x8000
#pragma optimize -1

#include <Memory.h>
#include "kmalloc.h"

struct kinfo
{
  Handle handle;
  LongWord size;
};

static Word memid;
static Handle kHandle;
static struct kinfo *kData;
static Word alloc;
static Word used;


void kmstartup(Word id)
{
  memid = id;
  kHandle = NULL;
  kData = NULL;
  alloc = 0;
  used = 0;
}

void kmshutdown(void)
{
Word i;
struct kinfo *k;

  for (i = 0, k = kData; i < used; i++, k++)
  {
    DisposeHandle(k->handle);
  }

  if (kHandle) DisposeHandle(kHandle);

  kmstartup(0);
}

void *kmalloc(LongWord size)
{
Word i;
struct kinfo *k;
Handle h;
char *ptr;
Word err;


  h = NULL;

  // look for an existing handle to use.
  for (i = 0, k = kData; i < used; i++, k++)
  {
    if (k->size == -1)
    {
      h = k->handle;
      SetHandleSize(size + 4, h);
      if (_toolErr) return NULL;
      k->size = size;
      HLock(h);

      break;
    }
  }

  if (!h)
  {
    if (alloc == used) // must grow.
    {
      if (!kData)
      {
        kHandle = NewHandle(16 * sizeof(struct kinfo), memid,
          attrLocked | attrNoSpec, NULL);
        if (_toolErr) return NULL;

        alloc = 16;
        kData = (struct kinfo *)*kHandle;
        k = kData;
      }
      else
      {
        HUnlock(kHandle);
        SetHandleSize(alloc + 16 * sizeof(struct kinfo), kHandle);
        err = _toolErr;
        HLock(kHandle);
        kData = (struct kinfo *)*kHandle;

        if (err)
        {
          return NULL;
        }
        alloc += 16;
        k = &kData[used];
      }
    }

    h = NewHandle(size + 4, memid, attrLocked | attrNoSpec, NULL);
    if (_toolErr) return NULL;
    k->handle = h;
    k->size = size;
    used++;
  }

  ptr = *h;
  *((Handle *)ptr) = h;
  return ptr + 4;

}

void kfree(void *ptr)
{
Word i;
Handle h;
struct kinfo *k;

  if (!ptr) return;
  h = *(Handle *)((char *)ptr - 4);
  HUnlock(h);
  SetHandleSize(0, h);

  for (i = 0, k = kData; i < used; i++, k++)
  {
    if (k->handle == h)
    {
      k->size = -1;
      return;
    }
  }
}


void *krealloc(void *ptr, LongWord size)
{
Handle h;
LongWord hsize;
Word i;
struct kinfo *k;


  if (!ptr) return kmalloc(size);

  h = *(Handle *)((char *)ptr - 4);

  hsize = -1;
  for (i = 0, k = kData; i < used; i++, k++)
  {
    if (k->handle == h)
    {
      hsize = k->size;
      break;
    }
  }
  //???
  if (hsize == -1) return kmalloc(size);

  if (size == hsize) return ptr;
  if (size < hsize)
  {
    k->size = hsize;
    return ptr;
  }

  HUnlock(h);
  SetHandleSize(size + 4, h);
  if (_toolErr)
  {
    kfree(ptr);
    return NULL;
  }
  HLock(h);
  k->size = size;
  *((Handle *)*h) = h;
  return *h + 4;

}
