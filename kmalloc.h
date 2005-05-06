#ifndef __KMALLOC_H__
#define __KMaLLOC_H__

#ifndef TYPES
#include <types.h>
#endif

void kmstartup(Word);
void kmshutdown(void);

void *kmalloc(LongWord);
void *krealloc(void *, LongWord);

void kfree(void *);


#endif
