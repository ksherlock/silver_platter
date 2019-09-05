#pragma lint - 1
#pragma noroot
#pragma optimize - 1
#pragma debug 0x8000

segment "WebDAV    ";

#include <gsos.h>
#include <memory.h>
#include <misctool.h>
#include <tcpip.h>

#include "timetool.h"

#include <stdio.h>

#include "MemBuffer.h"
#include "config.h"
#include "globals.h"
#include "http.h"
#include "pointer.h"
#include "server.h"

#define B(x) x->length, x->text
#define PRIB ".*s"

extern void tiTimeRec2ISO8601(TimeRecPtr t, char *str);
extern void tiTimeRec2GMTString(TimeRecPtr t, char *str);
extern const char *GetMimeString(GSString255Ptr, Word, LongWord);

GSString255Ptr MacRoman2UTF8(GSString255Ptr);
GSString255Ptr MacRoman2HTML(GSString255Ptr);
GSString255Ptr EncodeURL(GSString255Ptr);

Word GetNextVolume(Word cookie, VolumeRecGS *VolumeDCB);

static GSString255Ptr EmptyName = (GSString255Ptr) "\x00\x00";
static GSString255Ptr SlashName = (GSString255Ptr) "\x01\x00/";

static ResultBuf255 vName = {259};

static VolumeRecGS VolumeDCB = {6, NULL, &vName};
static DirEntryRecGS DirDCB = {14, 0, 0, 1, 1, &vName};

static char buffer32[32];

static GSString255Ptr BaseName(GSString255Ptr path) {
  Word i;
  Word len;

  GSString255Ptr str;

  for (len = i = path->length; i; i--) {
    if (path->text[i - 1] == '/') {
      len = len - i;
      str = (GSString255Ptr)NewPointer(3 + len);
      if (str) {
        str->length = len;
        TCPIPPtrToPtr(path->text + i, str->text, len);
        str->text[len] = 0;
      }
      return str;
    }
  }
  return NULL;
}

static Word AddFile(MemBuffer *m, GSString255Ptr path_uri,
                    GSString255Ptr file_uri, GSString255Ptr file_utf,
                    FileInfoRecGS *info, Word root) {
  Word i;
  Word err;

  if (root) {
    i = sprintf(buffer,
                "<D:response>\r\n"
                "<D:href>%" PRIB "</D:href>\r\n",
                B(path_uri));
  } else {
    i = sprintf(buffer,
                "<D:response>\r\n"
                "<D:href>%" PRIB "/%" PRIB "</D:href>\r\n",
                B(path_uri), B(file_uri));
  }
  err = BufferAppend(m, buffer, i);
  if (err)
    return 500;

#undef xstr
#define xstr "<D:propstat>\r\n<D:prop>\r\n"

  err = BufferAppend(m, xstr, sizeof(xstr) - 1);
  if (err)
    return 500;

  tiTimeRec2ISO8601(&info->createDateTime, buffer32);

  i = sprintf(buffer, "<D:creationdate>%b</D:creationdate>\r\n", buffer32);
  err = BufferAppend(m, buffer, i);
  if (err)
    return 500;

  tiTimeRec2GMTString(&info->modDateTime, buffer32);

  i = sprintf(buffer,
              "<D:displayname>%" PRIB "</D:displayname>\r\n"
              "<D:getlastmodified>%b</D:getlastmodified>\r\n"
              "<D:getcontentlength>%lu</D:getcontentlength>\r\n"
              "<D:getcontenttype>%s</D:getcontenttype>\r\n"
              "<D:resourcetype />\r\n",
              B(file_utf), buffer32, info->eof,
              GetMimeString(file_utf, info->fileType, info->auxType));

  err = BufferAppend(m, buffer, i);
  if (err)
    return 500;

#undef xstr
#define xstr                                                                   \
  "<D:supportedlock />\r\n"                                                    \
  "<D:iscollection>0</D:iscollection>\r\n"                                     \
  "<D:ishidden>0</D:ishidden>\r\n"                                             \
  "</D:prop>\r\n"                                                              \
  "<D:status>HTTP/1.1 200 OK</D:status>\r\n"                                   \
  "</D:propstat>\r\n"                                                          \
  "</D:response>\r\n"

  err = BufferAppend(m, xstr, sizeof(xstr) - 1);
  if (err)
    return err;

  return 0;
}

static Word AddFolder(MemBuffer *m, GSString255Ptr path_uri,
                      GSString255Ptr file_uri, GSString255Ptr file_utf,
                      FileInfoRecGS *info,
                      Word root) // if this is the first folder of a collection.
{
  Word i;
  Word err;

  if (root) {
    i = sprintf(buffer,
                "<D:response>\r\n"
                "<D:href>%" PRIB "/</D:href>\r\n",
                B(path_uri));

  } else {
    i = sprintf(buffer,
                "<D:response>\r\n"
                "<D:href>%" PRIB "/%" PRIB "/</D:href>\r\n",
                B(path_uri), B(file_uri));
  }

  err = BufferAppend(m, buffer, i);
  if (err)
    return 500;

#undef xstr
#define xstr "<D:propstat>\r\n<D:prop>\r\n"

  err = BufferAppend(m, xstr, sizeof(xstr) - 1);
  if (err)
    return 500;

  i = sprintf(buffer, "<D:displayname>%" PRIB "</D:displayname>\r\n",
              B(file_utf));

  err = BufferAppend(m, buffer, i);
  if (err)
    return 500;

  if (info) {
    tiTimeRec2ISO8601(&info->createDateTime, buffer32);

    i = sprintf(buffer, "<D:creationdate>%b</D:creationdate>\r\n", buffer32);
    err = BufferAppend(m, buffer, i);
    if (err)
      return 500;
  }
  if (info) {
    tiTimeRec2GMTString(&info->modDateTime, buffer32);
    i = sprintf(buffer, "<D:getlastmodified>%b</D:getlastmodified>\r\n",
                buffer32);
    err = BufferAppend(m, buffer, i);
    if (err)
      return 500;
  }

#undef xstr
#define xstr                                                                   \
  "<D:resourcetype><D:collection /></D:resourcetype>\r\n"                      \
  "<D:supportedlock />\r\n"                                                    \
  "<D:getcontentlength>0</D:getcontentlength>\r\n"                             \
  "<D:getcontenttype>application/octet-stream</D:getcontenttype>\r\n"          \
  "<D:iscollection>1</D:iscollection>\r\n"                                     \
  "<D:ishidden>0</D:ishidden>\r\n"                                             \
  "</D:prop>\r\n"                                                              \
  "<D:status>HTTP/1.1 200 OK</D:status>\r\n"                                   \
  "</D:propstat>\r\n"                                                          \
  "</D:response>\r\n"

  err = BufferAppend(m, xstr, sizeof(xstr) - 1);
  if (err)
    return 500;

  return 0;
}

// todo -- cache volume list xml.

static Word ListVolumes(struct qEntry *q) {
  Word err;

  LongWord secs;
  tiPrefRec tiPrefs;
  TimeRec tr;
  Word bram;

  CREATE_BUFFER(m, q->workHandle);

  if (q->moreFlags)
    return ProcessError(HTTP_UNPROCESSABLE_ENTITY, q);

  HUnlock(q->workHandle);
  SetHandleSize(0, q->workHandle);

  // volumes don't have create/modified dates, so use the current date/time.
  tiPrefs.pCount = 3;
  tiGetTimePrefs(&tiPrefs);

  secs = ConvSeconds(getCurrTimeInSecs, 0, 0);
  secs += tiPrefs.secOffset;

  // adjust for dst.
  bram = ReadBParam(0x5e);
  if (bram & 0x0002 == 0)
    secs += 3600;

  ConvSeconds(secs2TimeRec, secs, (Pointer)&tr);

  InfoDCB.createDateTime = tr;
  InfoDCB.modDateTime = tr;

#undef xstr
#define xstr                                                                   \
  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"                             \
  "<D:multistatus xmlns:D=\"DAV:\">\r\n"

  err = BufferAppend(&m, xstr, sizeof(xstr) - 1);
  if (err)
    return ProcessError(500, q);

  err = AddFolder(&m, EmptyName, EmptyName, SlashName, &InfoDCB, true);

  if (err)
    return ProcessError(err, q);

  if (q->depth != 0) {
    Word d;
    Word len;
    d = 1;
    while (d = GetNextVolume(d, &VolumeDCB)) {
      GSString255Ptr dev_uri;
      GSString255Ptr dev_utf;
      GSString255Ptr file_utf;
      Word alloc = 0;

      // convert first char from ':' --> '/'
      vName.bufString.text[0] = '/';

      dev_uri = EncodeURL(&vName.bufString);
      dev_utf = MacRoman2UTF8(&vName.bufString);
      // dev_utf = MacRoman2HTML(&vName.bufString);

      if (dev_uri)
        alloc |= 0x0001;
      else
        dev_uri = &vName.bufString;

      if (dev_utf)
        alloc |= 0x0002;
      else
        dev_utf = &vName.bufString;

      file_utf = BaseName(dev_utf);
      err = AddFolder(&m, dev_uri, EmptyName, file_utf, &InfoDCB, true);

      if (alloc & 0x0001)
        ReleasePointer(dev_uri);
      if (alloc & 0x0002)
        ReleasePointer(dev_utf);
      if (file_utf)
        ReleasePointer(file_utf);

      if (err)
        return ProcessError(500, q);
    }
  }

#undef xstr
#define xstr "</D:multistatus>\r\n"

  err = BufferAppend(&m, xstr, sizeof(xstr) - 1);
  if (err)
    return ProcessError(500, q);

  SendHeader(q, 207, m.used, NULL, "text/xml", NULL, 0);

  WriteData(q, *m.h, m.used);
  WriteData(q, NULL, 0);

  q->state = STATE_CLOSE;
  return 207;
}

static Word ListDirectory(MemBuffer *m, GSString255Ptr path_uri,
                          struct qEntry *q) {

  Word err = 0;

  err = AddFolder(m, path_uri, EmptyName, SlashName, &InfoDCB, true);
  if (err)
    return err;

  if (q->depth != 0) {

    OpenDCB.pCount = 15;
    OpenDCB.pathname = q->fullpath;
    OpenDCB.requestAccess = readEnable;
    OpenDCB.resourceNumber = 0;
    OpenDCB.optionList = NULL;
    OpenGS(&OpenDCB);
    if (_toolErr) {
      return 403;
    }
    q->fd = OpenDCB.refNum;

    DirDCB.refNum = OpenDCB.refNum;

    for (;;) {
      Word alloc = 0;
      GSString255Ptr file_utf;
      GSString255Ptr file_uri;

      GetDirEntryGS(&DirDCB);
      if (_toolErr)
        break;

      // check for hidden files
      if ((DirDCB.access & 0x0004) && (fDirHidden == false))
        continue;

      file_uri = EncodeURL(&vName.bufString);
      if (file_uri)
        alloc |= 0x0001;
      else
        file_uri = &vName.bufString;

      file_utf = MacRoman2UTF8(&vName.bufString);

      if (file_utf)
        alloc |= 0x0002;
      else
        file_utf = &vName.bufString;

      InfoDCB.fileType = DirDCB.fileType;
      InfoDCB.auxType = DirDCB.auxType;
      InfoDCB.createDateTime = DirDCB.createDateTime;
      InfoDCB.modDateTime = DirDCB.modDateTime;
      InfoDCB.eof = DirDCB.eof;

      if (DirDCB.fileType == 0x0f) {
        err = AddFolder(m, path_uri, file_uri, file_utf, &InfoDCB, false);
      } else {
        err = AddFile(m, path_uri, file_uri, file_utf, &InfoDCB, false);
      }

      if (alloc & 0x0001)
        ReleasePointer(file_uri);
      if (alloc & 0x0002)
        ReleasePointer(file_utf);

      if (err)
        return err;
    }

    return err;
  }
}

// this provides support for the WebDAV PROPFIND command.
// an XML description of the resource will be created and sent.

Word ProcessPropfind(struct qEntry *q) {
  LongWord size;
  Word i;
  Word resNumber = 0;
  GSString255Ptr path;
  GSString255Ptr fullpath;

  GSString255Ptr path_utf;
  GSString255Ptr path_uri;

  Word err;
  static char buffer32[32];

  CREATE_BUFFER(m, q->workHandle);

  if (fWebDav == false)
    return ProcessError(HTTP_METHOD_NOT_ALLOWED, q);

  if (!q->fullpath)
    return ProcessError(HTTP_BAD_REQUEST, q);

  if (q->contentlength)
    return ProcessError(HTTP_BAD_REQUEST, q);

  path = q->pathname;
  fullpath = q->fullpath;

  if (fullpath->length == 1) {
    return ListVolumes(q);
  }

  InfoDCB.pCount = 12;
  InfoDCB.pathname = fullpath;
  InfoDCB.optionList = NULL;
  GetFileInfoGS(&InfoDCB);
  if (_toolErr) {
    return ProcessError(404, q);
  }


  if (q->moreFlags = CGI_RESOURCE && InfoDCB.fileType != 0x0f) {
    resNumber = 1;
    q->moreFlags = 0;
  }

  if (q->moreFlags)
    return ProcessError(HTTP_UNPROCESSABLE_ENTITY, q);

  HUnlock(q->workHandle);
  SetHandleSize(0, q->workHandle);




  // if it's a directory, strip / off end ...
  // it will be tacked on later.

  if (InfoDCB.fileType == 0x0f) {
    i = path->length - 1;
    if (path->text[i] == '/') {
      path->length = i;
    }
  }

  err = 0;

  path_utf = MacRoman2UTF8(path);
  if (path_utf == NULL) {
    path_utf = path;
    RetainPointer(path);
  }

  path_uri = EncodeURL(path);
  if (path_uri == NULL) {
    path_uri = path;
    RetainPointer(path);
  }

  // start with the header...

#undef xstr
#define xstr                                                                   \
  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"                             \
  "<D:multistatus xmlns:D=\"DAV:\">\r\n"

  err = BufferAppend(&m, xstr, sizeof(xstr) - 1);
  if (!err) {

    if (InfoDCB.fileType == 0x0f) {
      err = ListDirectory(&m, path_uri, q);
    } else {
      GSString255Ptr file_utf = BaseName(path_utf);
      if (resNumber) {
        /* quick and dirty hack */
        InfoDCB.fileType = 0x06;
        InfoDCB.auxType = 0x0000;
        InfoDCB.eof = InfoDCB.resourceEOF;
      }
      err = AddFile(&m, path_uri, EmptyName, file_utf, &InfoDCB, true);
      ReleasePointer(file_utf);
    }

// finish with the trailer.
#undef xstr
#define xstr "</D:multistatus>\r\n"
    if (!err)
      err = BufferAppend(&m, xstr, sizeof(xstr) - 1);
  }

  ReleasePointer(path_uri);
  ReleasePointer(path_utf);

  if (err)
    return ProcessError(err, q);

  SendHeader(q, 207, m.used, NULL, "text/xml", NULL, 0);

  WriteData(q, *m.h, m.used);
  WriteData(q, NULL, 0);

  q->state = STATE_CLOSE;
  return 207;
}
