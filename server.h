#ifndef __SERVER_H__
#define __SERVER_H__

// flags
enum {
  FLAG_KA = 0x0001,      // keep-alive
  FLAG_CHUNKED = 0x0002, // chunked transfer
  FLAG_TEXT = 0x0004,    // PUT - is text/*
  FLAG_CREATE = 0x0008,  // PUT - created file.
  FLAG_RANGE = 0x0010,
  FLAG_RANGE0 = 0x0020, // Range: request (range start valid)
  FLAG_RANGE1 = 0x0040, // Range: request (range end valid)
};

// moreFlags - see cgi.h

enum {
  STATE_ESTABLISH = 1,
  STATE_READ,
  STATE_WRITE,
  STATE_CLOSE,
  STATE_LOGOUT,

  STATE_ASINGLE_1,
  STATE_ASINGLE_2,

  STATE_PUT
};

#include "cgi.h"
#include "methods.h"

struct qEntry {
  Word state;
  Word ipid;
  Word fd;
  Word rfd;
  Word method;
  Word version;
  Word flags;
  Word moreFlags;
  LongWord tick;
  LongWord ip;
  LongWord contentLength;
  Word depth;

  LongWord range[2];

  GSString255Ptr host;
  GSString255Ptr request;

  GSString255Ptr pathname;
  GSString255Ptr fullpath;

  Handle buffer;
  Handle workHandle;
};

extern char buffer[4096];

void WriteData(struct qEntry *, const char *, Word);

Word ProcessError(Word, struct qEntry *);
Word RemapError(Word);
Word ProcessFile(struct qEntry *);
Word ProcessPut(struct qEntry *);

void SendHeader(struct qEntry *q, Word status, LongWord size,
                const TimeRec *modTime, const char *mimeString,
                const char *extra, Word);

Word AppleSingle(struct qEntry *);

#endif
