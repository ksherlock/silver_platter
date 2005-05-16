#pragma lint -1
#pragma noroot
#pragma optimize -1
#pragma debug 0x8000

#include <gsos.h>
#include <memory.h>
#include <tcpip.h>

#include "server.h"
#include "globals.h"

extern int orca_sprintf(char *, const char *, ...);


extern void tiTimeRec2ISO8601(TimeRecPtr t, char *str);
extern void tiTimeRec2GMTString(TimeRecPtr t, char *str);
extern const char *GetMimeString(GSString255Ptr, Word, LongWord);


const char *BaseName(GSString255Ptr path)
{
Word i = path->length;

  for (i = path->length - 1; i; i--)
  {
    if (path->text[i] == '/') return path->text + i + 1;
  }
  return "";
}


struct mBuffer
{
  Handle h;
  Word alloc;
  Word used;
};

// returns _toolError on failure.
Word BufferAppend(struct mBuffer *buffer, void * data, Word size)
{
  char *cp;
  Handle h = buffer->h;

  if (buffer->used + size > buffer->alloc)
  {
    h = buffer->h;
    HUnlock(h);
    buffer->alloc += 2048;
    SetHandleSize(buffer->alloc, h);
    if (_toolErr) return _toolErr;
    HLock(h);
  }
  cp = *h + buffer->used;

  BlockMove(data, cp, size);
  buffer->used += size;

  return 0;
}


//  add XML for one entry.  if base, then no name will be sent.
Word AddEntry(
  struct mBuffer *m,
  GSString255Ptr path,  // path of the item
  FileInfoRecGS *info,  // data on the item
  Word child)          // true if subitem in a collection.
{
Word i;
Word err;
static char buffer32[32];

Word isdir;

  isdir = info ? (info->fileType == 0x0f) : true;

  if (child && info)
  {
    i = orca_sprintf(buffer,
      "<D:response>\r\n"
      "<D:href>%B/%B%s</D:href>\r\n",
      path, info->pathname, isdir ? "/" : "");
  }
  else
  {
    i = orca_sprintf(buffer,
      "<D:response>\r\n"
      "<D:href>%B%s</D:href>\r\n",
      path, isdir ? "/" : "");
  }
  err = BufferAppend(m, buffer, i);
  if (err) return err;

  #undef xstr
  #define xstr "<D:propstat>\r\n<D:prop>\r\n"

  err = BufferAppend(m, xstr, sizeof(xstr) - 1);
  if (err) return err;

  // creationDate....

  if (info)
  {
    tiTimeRec2ISO8601(&info->createDateTime, buffer32);

    i = orca_sprintf(buffer, "<D:creationdate>%b</D:creationdate>\r\n", buffer32);
    err = BufferAppend(m, buffer, i);
    if (err) return err;
  }

  if (child || isdir == false)
  {
    tiTimeRec2GMTString(&info->modDateTime, buffer32);

    i = orca_sprintf(buffer,
      "<D:displayname><![CDATA[%B]]></D:displayname>\r\n"
      "<D:getlastmodified>%b</D:getlastmodified>\r\n",
      info->pathname,
      buffer32);

      err = BufferAppend(m, buffer, i);
      if (err) return err;

      if (isdir)
      {
        #undef xstr
        #define xstr \
          "<D:resourcetype><D:collection /></D:resourcetype>\r\n"

        err = BufferAppend(m, xstr, sizeof(xstr) - 1);
        if (err) return err;

      }
      else
      {
        i = orca_sprintf(buffer,
          "<D:getcontentlength>%lu</D:getcontentlength>\r\n"
          "<D:getcontenttype>%s</D:getcontenttype>\r\n"
          "<D:resourcetype />\r\n",
          info->eof,
          GetMimeString(info->pathname, info->fileType, info->auxType));

        err = BufferAppend(m, buffer, i);
        if (err) return err;
      }
  }
  else
  {
    #undef xstr
    #define xstr \
      "<D:displayname><![CDATA[]]></D:displayname>\r\n" \
      "<D:resourcetype><D:collection /></D:resourcetype>\r\n"

    err = BufferAppend(m, xstr, sizeof(xstr) - 1);
    if (err) return err;
  }

  #undef xstr
  #define xstr \
    "<D:source></D:source>\r\n" \
    "<D:supportedlock></D:supportedlock>\r\n" \
    "</D:prop>\r\n" \
    "<D:status>HTTP/1.1 200 OK</D:status>\r\n" \
    "</D:propstat>\r\n" \
    "</D:response>\r\n" \

  err = BufferAppend(m, xstr, sizeof(xstr) - 1);
  if (err) return err;

  //
  return 0;

}


static ResultBuf32 dName = {36};
static ResultBuf255 vName = {259};

static DInfoRecGS DInfoDCB = {3, 0, &dName};
static VolumeRecGS VolumeDCB = {6, &dName.bufString, &vName};
static DirEntryRecGS DirDCB = {14, 0, 0, 1, 1, &vName};

#if 0
static Word ListVolumes(struct qEntry *q)
{
struct mBuffer m;
Word err;


  SetHandleSize(0, q->workHandle);

  m.h = q->workHandle;
  m.alloc = 0;
  m.used = 0;


  #undef xstr
  #define xstr \
  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" \
  "<D:multistatus xmlns:D=\"DAV:\">\r\n"

  err = BufferAppend(&m, xstr, sizeof(xstr) - 1);
  if (err) return ProcessError(500,q);

  
  err = AddEntry(&m, (GSString255Ptr)"\x00\x00", "/", NULL, true);
  if (err) return ProcessError(500,q);
      

  if (q->depth != 0)
  {
    Word i;
    Word len;
    for(i = 0; ; i++)
    {

      DInfoDCB.devNum = i;
      DInfoGS(&DInfoDCB);
      if (_toolErr) break;
      if (DInfoDCB.characteristics & 0x80 == 0) continue;

      VolumeGS(&VolumeDCB);
      if (_toolErr) continue;

      // convert first char from ':' --> '/'
      vName.bufString.text[0] = '/';

      err = AddEntry(&m, (GSString255Ptr)"\x00\x00", vName.bufString, NULL, false);
      if (err) return ProcessError(500,q);
                  
    }
  }


  #define xstr "</D:multistatus>\r\n"

  err = BufferAppend(&m, xstr, sizeof(xstr) - 1);
  if (err) return ProcessError(500,q);




  SendHeader(q, 207, m.used, NULL, "text/xml", true);


  // todo - write function that will break up, handle chunking, etc.
  TCPIPWriteTCP(q->ipid, *m.h, m.used, false, false);

  q->state = STATE_CLOSE;
  return 207;

}

#endif



// this provides support for the WebDAV PROPFIND command.
// an XML description of the resource will becreated and sent.

Word ProcessPropfind(struct qEntry *q)
{
LongWord size;
Word i;
GSString255Ptr path;

struct mBuffer m;
Word err;

static char buffer32[32];


  SetHandleSize(0, q->workHandle);

  m.h = q->workHandle;
  m.alloc = 0;
  m.used = 0;

  path = q->fullpath;

#if 0
  // todo -- if / do a volume listing.
  if (path->length == 1)
  {
    return ListVolumes(q);
  }
#endif


  InfoDCB.pCount = 12;
  InfoDCB.pathname = path;
  InfoDCB.optionList = NULL;
  GetFileInfoGS(&InfoDCB);
  if (_toolErr)
  {
    return ProcessError(404, q);                 
  }

  // if it's a directory, strip / off end ...
  // it will be tacked on later.

  if (InfoDCB.fileType == 0x0f)
  {
    GSString255Ptr path;
    path = q->pathname;

    i = path->length - 1;
    if (path->text[i] == '/')
    {
      path->length = i;
    }

    path = q->fullpath;
    i = path->length - 1;
    if (path->text[i] == '/')
    {
      path->length = i;
    }
  }

  


  // start with the header...

#undef xstr
#define xstr \
  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" \
  "<D:multistatus xmlns:D=\"DAV:\">\r\n" 

  err = BufferAppend(&m, xstr, sizeof(xstr) - 1);
  if (err) return ProcessError(500,q);

  // send the info for the file in question.
  err = AddEntry(&m, q->pathname, &InfoDCB, false);
  if (err) return ProcessError(500,q);

  if ((InfoDCB.fileType == 0x0f) && (q->depth != 0))
  {

    OpenDCB.pCount = 15;
    OpenDCB.pathname = path;
    OpenDCB.requestAccess = readEnable;
    OpenDCB.resourceNumber = 0;
    OpenDCB.optionList = NULL;
    OpenGS(&OpenDCB);
    if (_toolErr)
    {
      return ProcessError(403, q);
    }
    q->fd = OpenDCB.refNum;

    DirDCB.refNum = OpenDCB.refNum;

    for(;;)
    {
      GetDirEntryGS(&DirDCB);
      if (_toolErr) break;

      InfoDCB.fileType = DirDCB.fileType;
      InfoDCB.auxType = DirDCB.auxType;
      InfoDCB.createDateTime = DirDCB.createDateTime;
      InfoDCB.modDateTime = DirDCB.modDateTime;
      InfoDCB.eof = DirDCB.eof;
      InfoDCB.pathname = &vName.bufString;

      err = AddEntry(&m, q->pathname, &InfoDCB, true);
      if (err) return ProcessError(500,q);
    }
  }


  // finish with the trailer.
  #undef xstr
  #define xstr "</D:multistatus>\r\n"

  err = BufferAppend(&m, xstr, sizeof(xstr) - 1);
  if (err) return ProcessError(500,q);

  SendHeader(q, 207, m.used, NULL, "text/xml", true);

  WriteData(q, *m.h, m.used);
  WriteData(q, NULL, 0);

  q->state = STATE_CLOSE;
  return 207;
}
