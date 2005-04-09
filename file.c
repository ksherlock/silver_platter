/*
 *  send a file.
 *
 */

#pragma noroot
#pragma lint -1
#pragma optimize -1

#include <Memory.h>

#include <tcpip.h>
#include <gsos.h>

#include <kstring.h>

#include "server.h"
#include "config.h"
#include "ftype.h"

extern int orca_sprintf(char *, const char *, ...);

#undef MIN
#define MIN(a,b) (a) < (b) ? (a) : (b)


extern const char *GetMimeString(GSString255Ptr, Word, LongWord);

extern Handle MacRoman2HTML(const GSString255 *gstr);
extern Handle MangleName(const GSString255 *gstr);


extern Word MyID;


/* process head/get */

static ResultBuf32 dName = {36};
static ResultBuf255 vName = {259};

static FileInfoRecGS InfoDCB;
static OpenRecGS OpenDCB;
static DInfoRecGS DInfoDCB = {3, 0, &dName};
static VolumeRecGS VolumeDCB = {6, &dName.bufString, &vName};
static DirEntryRecGS DirDCB = {14, 0, 0, 1, 1, &vName};


static char _htmlHead[] =
"<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\r\n" \
"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"" \
  " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\r\n" \
"<html xmlns=\"http://www.w3.org/1999/xhtml\">\r\n" \
"<head>\r\n";



// list a directory.


// handle a redirect (301)
Word Redirect(struct qEntry *q, GSString255Ptr append)
{
Word ipid = q->ipid;
Word i;
volatile Handle h;
char *cp;

Word alloc;
Word total;

Handle g;

GSString255Ptr path;

  path = (GSString255Ptr)*q->pathname;

  g = MangleName(path);
  if (g)
  {
    HLock(g);
    path = (GSString255Ptr)*g;
  }

 // build the html...sigh

  h = NULL;
  total = -1;

  if (q->command != CMD_HEAD)
  {
    total = 0;
    alloc = 2048;

    h = q->workHandle;
    HUnlock(h);
    SetHandleSize(alloc, h);
    if (_toolErr)
    {
      if (g) DisposeHandle(g);
      return ProcessError(500, q);
    }
    HLock(h);

    cp = *h;

    BlockMove(_htmlHead, cp, sizeof(_htmlHead) - 1);
    cp += sizeof(_htmlHead) -1;
    total = sizeof(_htmlHead) -1;

#undef xstr
#define xstr \
"<title>301 Moved Permanently</title>\r\n" \
"</head>\r\n" \
"<body>\r\n" \
"<h1>301 Moved Permanently</h1>\r\n"

    BlockMove(xstr, cp, sizeof(xstr) - 1);
    cp += sizeof(xstr) -1;
    total += sizeof(xstr) -1;

#undef xstr
#define xstr "<p>The document has moved <a href=\"%s%s\">here</a></p>\r\n" \
"</body>\r\n<\html>\r\n"

    total += orca_sprintf(cp, xstr, path->text, append->text);
  }

  if (q->version >= 0x0100)
  {
    SendHeader(q, 301, total, NULL, "text/html", false);

    if (GetHandleSize(q->host))
    {
      GSString255Ptr host = (GSString255Ptr)*q->host;
      i = orca_sprintf(buffer, "Location: http://%s%s%s\r\n",
        host->text, path->text, append->text);
      TCPIPWriteTCP(ipid, buffer, i, false, false);
    }
    TCPIPWriteTCP(ipid, "\r\n", 2, false, false);
  }

  if (g) DisposeHandle(g);

  if (q->command == CMD_GET)
  {
    if (q->flags & FLAG_CHUNKED)
    {
      int i = orca_sprintf(buffer, "%x\r\n", total);
      TCPIPWriteTCP(ipid, buffer, i, false, false);
    }

    cp = *h;
    do
    {
      Word i = MIN(total, fMTU);

      TCPIPWriteTCP(ipid, cp, i, false, false);
      TCPIPPoll();

      cp += i;
      total -= i;
    } while (total);
    //TCPIPWriteTCP(ipid, cp, total, false, false);

    if (q->flags & FLAG_CHUNKED)
      TCPIPWriteTCP(ipid, "\r\n0\r\n\r\n", 7, false, false);
  }                 

  q->state = STATE_CLOSE;
  return 301;
}

// returns 0 if no index.html/index.htm file
// returns HTTP error if we handled it.

Word CheckIndex(struct qEntry *q)
{
Handle h;
GSString255Ptr newpath;
GSString255Ptr oldpath;
char *cp;
Word i;

#if 0
  h = q->fullpath ? q->fullpath : q->pathname;
#endif
  h = q->fullpath;

  oldpath = (GSString255Ptr)*h;

  #undef xstr
  #define xstr "index.html"

  i = oldpath->length;

  h = q->workHandle;
  HUnlock(h);
  SetHandleSize(i + sizeof(xstr) + 2, h);

  if (_toolErr)
  {
    return ProcessError(500, q);
  }
  HLock(h);

  newpath = (GSString255Ptr)*h;
  cp = newpath->text;
  BlockMove(oldpath->text, cp, i);

  cp[i++] = 'i';
  cp[i++] = 'n';
  cp[i++] = 'd';
  cp[i++] = 'e';
  cp[i++] = 'x';
  cp[i++] = '.';
  cp[i++] = 'h';
  cp[i++] = 't';
  cp[i++] = 'm';
  cp[i++] = 'l';
  cp[i] = 0;
  newpath->length = i;


  InfoDCB.pCount = 12;
  InfoDCB.pathname = newpath;
  InfoDCB.optionList = NULL;
  GetFileInfoGS(&InfoDCB);
  if (!_toolErr)
  {
    return Redirect(q, (GSString255Ptr)"\x0a\x00index.html");
  }

  // check for .htm
  newpath->length--;
  GetFileInfoGS(&InfoDCB);
  if (!_toolErr)
  {
    return Redirect(q, (GSString255Ptr)"\x09\x00index.htm");
  }

  return 0;
}

Word ListDirectory(struct qEntry *q)
{
Word len;
Word total;
Word alloc;
Word err;

volatile Handle h; // must be volatile.

char *cp;
GSString255Ptr path = (GSString255Ptr)*q->pathname;

Handle hUrl, hHtml, hPath;
GSString255Ptr gUrl, gHtml;



  if (!fDir)
  {
    return ProcessError(403, q);
  }
  if (q->command == CMD_HEAD)
  {
    SendHeader(q, 200, -1, NULL, "text/html", true);
    return 200;
  }

  if (err = CheckIndex(q)) return err;


  DirDCB.refNum = q->fd;

  total = 0;
  alloc = 2048;

  h = q->workHandle;
  HUnlock(h);
  SetHandleSize(alloc, h);

  if (_toolErr)
  {
    return ProcessError(500, q);
  }
  HLock(h);
  cp = *h;

  // hardcoded ... ewww

  BlockMove(_htmlHead, cp, sizeof(_htmlHead) - 1);
  cp += sizeof(_htmlHead) -1;
  total = sizeof(_htmlHead) -1;

#undef xstr
#define xstr \
"<title>Index of %s</title>\r\n" \
"</head>\r\n" \
"<body>\r\n" \
"<h1>Index of %s</h1>\r\n"


  hPath = MangleName(path);
  hHtml = MacRoman2HTML(path);
  if (hPath) HLock(hPath);
  if (hHtml) HLock(hHtml);
  gHtml = hHtml ? (GSString255Ptr)*hHtml : path;

  path = hPath ? (GSString255Ptr)*hPath : path;

  len = orca_sprintf(buffer, xstr, gHtml->text, gHtml->text);
  BlockMove(buffer, cp, len);
  cp += len;
  total += len;

  // if we're jailed and at the root, no parent directory
  if (path->length > 1)
  {
    Word i, j;
    i = j = path->length;
    i -= 2; // -1 to convert o 0-index, -1 to skip trailing /
    while (path->text[i] != '/') i--;
    path->length = i + 1;

    len = orca_sprintf(buffer,
      "<p><a href=\"%s\">Parent Directory</a></p>\r\n",
      path->text);

    BlockMove(buffer, cp, len);
    cp += len;
    total += len;

    path->length = j;
  }

  if (hHtml) DisposeHandle(hHtml);


#undef xstr
#define xstr \
"<table border=\"0\" cellspacing=\"2\" cellpadding=\"2\">\r\n" \
"<thead align=\"left\">\r\n" \
"<tr>\r\n" \
"<th>Name</th><th>Size</th><th>Kind</th>\r\n" \
"</tr>\r\n" \
"</thead>\r\n"

  if (total + sizeof(xstr) - 1 > alloc)
  {
    HUnlock(h);
    alloc += 2048;
    SetHandleSize(alloc, h);
    if (_toolErr)
    {
      if (hPath) DisposeHandle(hPath);
      return ProcessError(500, q);
    }
    HLock(h);
    cp = *h + total;
  }

  BlockMove(xstr, cp, sizeof(xstr) - 1);
  total += sizeof(xstr) - 1;
  cp += sizeof(xstr) - 1;

  for(;;)
  {
    GetDirEntryGS(&DirDCB);
    if (_toolErr) break;

    hUrl = MangleName(&vName.bufString);
    hHtml = MacRoman2HTML(&vName.bufString);
    if (hUrl) HLock(hUrl);
    if (hHtml) HLock(hHtml);

    gUrl = hUrl ? (GSString255Ptr)*hUrl : &vName.bufString;
    gHtml = hHtml ? (GSString255Ptr)*hHtml : &vName.bufString;


    // folder -- no size, include trailing /
    if (DirDCB.fileType == 0x0f)
    {
      len = orca_sprintf(buffer,
        "<tr>"
          "<td><a href=\"%s%s/\">%s/</a></td>"
          "<td align=\"right\"> &mdash; </td>"
          "<td> Folder </td>"
        "</tr>\r\n",
        path->text, gUrl->text, gHtml->text);
    }
    else
    {
      const char *fType = FindFType(DirDCB.fileType, DirDCB.auxType);

      LongWord size = DirDCB.eof;
      size += 1023;
      size >>= 10;  // convert to K.

      len = orca_sprintf(buffer,
	"<tr>"
          "<td><a href=\"%s%s\">%s</a></td>"
          "<td align=\"right\"> %uK </td>"
          "<td> %b </td>"
	"</tr>\r\n",
	path->text, gUrl->text, gHtml->text,
        (Word)size,
	fType);
    }


    if (hUrl) DisposeHandle(hUrl);
    if (hHtml) DisposeHandle(hHtml);

    if (len + total > alloc)
    {
      HUnlock(h);
      alloc += 2048;
      SetHandleSize(alloc, h);
      if (_toolErr)
      {
        if (hPath) DisposeHandle(hPath);
        return ProcessError(500, q);
      }
      HLock(h);
      cp = *h + total;
    }
    BlockMove(buffer, cp, len);
    total += len;
    cp += len;
    
  }
#undef xstr
#define xstr "</table>\r\n</body>\r\n</html>\r\n"

  if (total + sizeof(xstr) - 1 > alloc)
  {
    HUnlock(h);
    alloc += sizeof(xstr) - 1;
    SetHandleSize(alloc, h);
    if (_toolErr)
    {
      if (hPath) DisposeHandle(hPath);
      return ProcessError(500, q);
    }
    HLock(h);
    cp = *h + total;
  }
  BlockMove(xstr, cp, sizeof(xstr) - 1);
  total += sizeof(xstr) - 1;

  cp = *h;

  SendHeader(q, 200, total, NULL, "text/html", true);


  if (q->flags & FLAG_CHUNKED)
  {
    int i = orca_sprintf(buffer, "%x\r\n", total);
    TCPIPWriteTCP(q->ipid, buffer, i, false, false);
  }

  //TCPIPWriteTCP(q->ipid, cp, total, false, false);
  do
  {
    Word i = MIN(total, fMTU);

    TCPIPWriteTCP(q->ipid, cp, i, false, false);
    TCPIPPoll();
            
    cp += i;
    total -= i;
  } while (total);
  if (q->flags & FLAG_CHUNKED)
    TCPIPWriteTCP(q->ipid, "\r\n0\r\n\r\n", 7, false, false);
    

  if (hPath) DisposeHandle(hPath);
  q->state = STATE_CLOSE;
  return 200;
}

// list the volumes
Word ListVolumes(struct qEntry *q)
{
int i;
Word len;
Word total;
Word alloc;

volatile Handle h;
char *cp;

Handle hUrl, hHtml;
GSString255Ptr gUrl, gHtml;


  if (!fDir)
  {
    return ProcessError(403, q);
  }
  if (q->command == CMD_HEAD)
  {
    SendHeader(q, 200, -1, NULL, "text/html", true);
    return 200;
  }

  total = 0;
  alloc = 2048;

  h = q->workHandle;
  HUnlock(h);
  SetHandleSize(alloc, h);
  if (_toolErr)
  {
    return ProcessError(500, q);
  }
  HLock(h);
  cp = *h;

  // hardcoded ... ewww

  BlockMove(_htmlHead, cp, sizeof(_htmlHead) - 1);
  cp += sizeof(_htmlHead) -1;
  total = sizeof(_htmlHead) -1;


#undef xstr
#define xstr \
"<title>Index of /</title>\r\n" \
"</head>\r\n" \
"<body>\r\n" \
"<h1>Index of /</h1>\r\n" 

  BlockMove(xstr, cp, sizeof(xstr) - 1);
  cp += sizeof(xstr) -1;
  total += sizeof(xstr) -1;

#undef xstr
#define xstr \
"<table border=\"0\" cellspacing=\"2\" cellpadding=\"2\">\r\n" \
"<thead align=\"left\">\r\n" \
"<tr>\r\n" \
"<th>Name</th><th>Size</th><th>Kind</th>\r\n" \
"</tr>\r\n" \
"</thead>\r\n"

  if (total + sizeof(xstr) - 1 > alloc)
  {
    HUnlock(h);
    alloc += 2048;
    SetHandleSize(alloc, h);
    if (_toolErr)
    {
      return ProcessError(500, q);
    }
    HLock(h);
    cp = *h + total;
  }

  BlockMove(xstr, cp, sizeof(xstr) - 1);
  total += sizeof(xstr) - 1;
  cp += sizeof(xstr) - 1;


  for (i = 1; ; i++)
  {
    DInfoDCB.devNum = i;
    DInfoGS(&DInfoDCB);
    if (_toolErr) break;
    if (DInfoDCB.characteristics & 0x80 == 0) continue;

    VolumeGS(&VolumeDCB);
    if (_toolErr) continue;
    // convert first char from ':' --> '/'
    vName.bufString.text[0] = '/';

    hUrl = MangleName(&vName.bufString);
    hHtml = MacRoman2HTML(&vName.bufString);

    if (hUrl) HLock(hUrl);
    if (hHtml) HLock(hHtml);

    gUrl = hUrl ? (GSString255Ptr)*hUrl : &vName.bufString;
    gHtml = hHtml ? (GSString255Ptr)*hHtml : &vName.bufString;


    len = orca_sprintf(buffer,
      "<tr>"
        "<td><a href=\"%s/\">%s/</a></td>"
        "<td align=\"right\"> &mdash; </td>"
        "<td> Folder </td>"
      "</tr>\r\n",
      gUrl->text, gHtml->text);


     if (hUrl) DisposeHandle(hUrl);
     if (hHtml) DisposeHandle(hHtml);

    if (len + total > alloc)
    {
      HUnlock(h);
      alloc += 2048;
      SetHandleSize(alloc, h);
      if (_toolErr)
      {
        return ProcessError(500, q);
      }
      HLock(h);
      cp = *h + total;
    }
    BlockMove(buffer, cp, len);
    total += len;
    cp += len;
  }

#undef xstr
#define xstr "</table>\r\n</body>\r</html>\r\n"

  if (total + sizeof(xstr) - 1 > alloc)
  {
    HUnlock(h);
    alloc += sizeof(xstr) - 1;
    SetHandleSize(alloc, h);
    if (_toolErr)
    {
      return ProcessError(500, q);
    }
    HLock(h);
    cp = *h + total;
  }
  BlockMove(xstr, cp, sizeof(xstr) - 1);
  total += sizeof(xstr) - 1;

  cp = *h;

  SendHeader(q, 200, total, NULL, "text/html", true);

  if (q->flags & FLAG_CHUNKED)
  {
    int i = orca_sprintf(buffer, "%x\r\n", total);
    TCPIPWriteTCP(q->ipid, buffer, i, false, false);
  }

  //TCPIPWriteTCP(q->ipid, cp, total, false, false);
  do
  {
    Word i = MIN(total, fMTU);

    TCPIPWriteTCP(q->ipid, cp, i, false, false);
    TCPIPPoll();
            
    cp += i;
    total -= i;
  } while (total);
  if (q->flags & FLAG_CHUNKED)
    TCPIPWriteTCP(q->ipid, "\r\n0\r\n\r\n", 7, false, false);
  
  q->state = STATE_CLOSE;
  return 200;
}


// this initiates HEAD/GET commands.
Word ProcessFile(struct qEntry *q)
{
Word i;
GSString255Ptr path;

LongWord eof;
Word fileType;
LongWord auxType;
TimeRec modDateTime;
Word ipid;


  ipid = q->ipid;

  //
  if (!GetHandleSize(q->fullpath))
  {
    return ProcessError(400, q);
  }

  HLock(q->fullpath);
  path = (GSString255Ptr)*q->fullpath;


  // ``/''  -- special case.
  if (path->length == 1)
  {
      return ListVolumes(q);
  }

  if (q->command == CMD_GET)
  {
    OpenDCB.pCount = 15;
    OpenDCB.pathname = path;
    OpenDCB.requestAccess = readEnable;
    OpenDCB.resourceNumber = 0;
    OpenDCB.optionList = NULL;
    OpenGS(&OpenDCB);
    if (_toolErr)
    {
      return ProcessError(404, q);
    }
    q->fd = OpenDCB.refNum;

    eof = OpenDCB.eof;
    fileType = OpenDCB.fileType;
    auxType = OpenDCB.auxType;
    modDateTime = OpenDCB.modDateTime;
  }
  else if (q->command == CMD_HEAD)
  {
    if (q->version < 0x0100)
    {
      return ProcessError(400, q);                 
    }

    InfoDCB.pCount = 12;
    InfoDCB.pathname = path;
    InfoDCB.optionList = NULL;
    GetFileInfoGS(&InfoDCB);
    if (_toolErr)
    {
      return ProcessError(404, q);                 
    }

    eof = InfoDCB.eof;
    fileType = InfoDCB.fileType;
    auxType = InfoDCB.auxType;
    modDateTime = InfoDCB.modDateTime;
  }
  else
  {
    return ProcessError(405, q); // not allowed.   
  }

  // if it's a folder, try listing the directory.
  if (fileType == 0x0f) // directory -- special case.
  {
    if (path->text[path->length - 1] != '/')
    {
      return Redirect(q, (GSString255Ptr)"\x01\x00/");
    }
    return ListDirectory(q);
  }

  SendHeader(q, 200,
    eof,
    &modDateTime,
    GetMimeString(path, fileType, auxType), true);

  if (q->command == CMD_GET)
  {
    q->state = STATE_WRITE;
  }
  else
  {
    q->state = STATE_CLOSE;
  }
  // actual writing takes place in server.c
  return 200;
}
