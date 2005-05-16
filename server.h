#ifndef __SERVER_H__
#define __SERVER_H__

// flags
enum
{
  FLAG_KA = 0x0001,		// keep-alive
  FLAG_CHUNKED = 0x0002,	// chunked transfer
  FLAG_TEXT = 0x0004,		// PUT - is text/*
  FLAG_CREATE = 0x0008		// PUT - created file.
};

// moreFlags
enum
{
  CGI_APPLESINGLE = 1,		// convert file to apple single
  CGI_HTML			// convert file to html
};


enum
{
  STATE_ESTABLISH = 1,
  STATE_READ,
  STATE_WRITE,
  STATE_CLOSE,
  STATE_LOGOUT,

  STATE_ASINGLE_1,
  STATE_ASINGLE_2,

  STATE_PUT
};

enum
{
  CMD_OPTIONS = 1,
  CMD_GET,
  CMD_HEAD,
  CMD_POST,
  CMD_PUT,
  CMD_DELETE,
  CMD_TRACE,
  CMD_CONNECT,

  // WebDAV
  CMD_PROPFIND,
  CMD_PROPPATCH,
  CMD_MKCOL,
  CMD_COPY,
  CMD_MOVE,
  CMD_LOCK,
  CMD_UNLOCK
};

struct qEntry
{
  Word state;
  Word ipid;
  Word fd;
  Word rfd;
  Word command;
  Word version;
  Word flags;
  Word moreFlags;
  LongWord tick;
  LongWord ip;
  LongWord filesize;
  Word depth;

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
Word ProcessFile(struct qEntry *);
Word ProcessPut(struct qEntry *);

void SendHeader(struct qEntry *q, Word status, LongWord size,
  const TimeRec * modTime, const char *mimeString, Boolean term);

Word AppleSingle(struct qEntry *);


#endif
