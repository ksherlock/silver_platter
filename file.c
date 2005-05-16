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
#include "globals.h"

#include "server.h"
#include "config.h"
#include "ftype.h"
#include "kmalloc.h"
#include "MemBuffer.h"

extern int orca_sprintf(char *, const char *, ...);


extern const char *GetMimeString(GSString255Ptr, Word, LongWord);

extern Handle MacRoman2HTML(const GSString255 *gstr);
extern Handle MangleName(const GSString255 *gstr);


extern Word MyID;


/* process head/get */

static ResultBuf32 dName = {36};
static ResultBuf255 vName = {259};

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
Handle g;

GSString255Ptr path;

  CREATE_BUFFER(m, q->workHandle);
  	
  HUnlock(q->workHandle);
  SetHandleSize(0, q->workHandle);


  path = q->pathname;

  g = MangleName(path);
  if (g)
  {
    HLock(g);
    path = (GSString255Ptr)*g;
  }

 // build the html...sigh

  if (q->command != CMD_HEAD)
  {
  	Word err;

	
	err = BufferAppend(&m, _htmlHead, sizeof(_htmlHead) - 1);
	if (err) return ProcessError(500, q);

#undef xstr
#define xstr \
"<title>301 Moved Permanently</title>\r\n" \
"</head>\r\n" \
"<body>\r\n" \
"<h1>301 Moved Permanently</h1>\r\n"

	err = BufferAppend(&m, xstr, sizeof(xstr) - 1);
	if (err) return ProcessError(500, q);

#undef xstr
#define xstr "<p>The document has moved <a href=\"%B%B\">here</a></p>\r\n" \
"</body>\r\n</html>\r\n"

    i = orca_sprintf(buffer, xstr, path, append);
	err = BufferAppend(&m, xstr, sizeof(xstr) - 1);
	if (err) return ProcessError(500, q);
  }


  SendHeader(q, 301, q->command == CMD_HEAD ? (LongWord)-1 : m.used,
    NULL, "text/html", false);

  if (q->host)
  {
    i = orca_sprintf(buffer, "Location: http://%B%B%B\r\n",
      q->host, path, append);
    TCPIPWriteTCP(ipid, buffer, i, false, false);
  }
  if (q->version >= 0x0100)
    TCPIPWriteTCP(ipid, "\r\n", 2, false, false);


  if (g) DisposeHandle(g);

  if (q->command == CMD_GET)
  {
  	WriteData(q, *m.h, m.used);
  	WriteData(q, NULL, 0);
  }                 

  q->state = STATE_CLOSE;
  return 301;
}

// returns 0 if no index.html/index.htm file
// returns HTTP error if we handled it.

Word CheckIndex(struct qEntry *q)
{
GSString255Ptr newpath;
GSString255Ptr oldpath;
char *cp;
Word i;

  oldpath = q->fullpath;

  #undef xstr
  #define xstr "index.html"

  i = oldpath->length;

  newpath = kmalloc(2 + i + sizeof(xstr));


  if (newpath == NULL)
  {
    return ProcessError(500, q);
  }

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
    kfree(newpath);
    return Redirect(q, (GSString255Ptr)"\x0a\x00index.html");
  }

  // check for .htm
  newpath->length--;
  GetFileInfoGS(&InfoDCB);
  if (!_toolErr)
  {
    kfree(newpath);
    return Redirect(q, (GSString255Ptr)"\x09\x00index.htm");
  }

  kfree(newpath);

  return 0;
}

Word ListDirectory(struct qEntry *q)
{
Word i;
Word err;

GSString255Ptr path = q->pathname;

Handle hUrl, hHtml, hPath;
GSString255Ptr gUrl, gHtml;

CREATE_BUFFER(m, q->workHandle);

  if (err = CheckIndex(q)) return err;

  if (!fDir)
  {
    return ProcessError(403, q);
  }
  if (q->command == CMD_HEAD)
  {
    SendHeader(q, 200, -1, NULL, "text/html", true);
    return 200;
  }


  DirDCB.refNum = q->fd;

  HUnlock(q->workHandle);
  SetHandleSize(0, q->workHandle);
  
  
  hPath = MangleName(path);
  hHtml = MacRoman2HTML(path);
  if (hPath) HLock(hPath);
  if (hHtml) HLock(hHtml);
  
  gHtml = hHtml ? (GSString255Ptr)*hHtml : path;
  path = hPath ? (GSString255Ptr)*hPath : path;
  
  // do ... while (false) to allow breaking.
  do
  {
  	err = BufferAppend(&m, _htmlHead, sizeof(_htmlHead) - 1);
  	if (err) break;
  	
  	#undef xstr
	#define xstr \
	"<title>Index of %B</title>\r\n" \
	"</head>\r\n" \
	"<body>\r\n" \
	"<h1>Index of %B</h1>\r\n"

  	i = orca_sprintf(buffer, xstr, gHtml, gHtml);

  	err = BufferAppend(&m, buffer, i);
  	if (err) break;
  	
 
	   
	// if we're jailed and at the root, no parent directory
	if (path->length > 1)
	{
	  Word i, j;
	  i = j = path->length;
	  i -= 2; // -1 to convert o 0-index, -1 to skip trailing /
	  while (path->text[i] != '/') i--;
	  path->length = i + 1;
	
	  i = orca_sprintf(buffer,
	    "<p><a href=\"%B\">Parent Directory</a></p>\r\n",
	    path);
	
  	  err = BufferAppend(&m, buffer, i);
  	  if (err) break;
	
	  path->length = j;
	}
	
	if (hHtml) DisposeHandle(hHtml);
    hHtml = NULL;
    

    #undef xstr
	#define xstr \
	"<table border=\"0\" cellspacing=\"2\" cellpadding=\"2\">\r\n" \
	"<thead align=\"left\">\r\n" \
	"<tr>\r\n" \
	"<th>Name</th><th>Size</th><th>Kind</th><th></th>\r\n" \
	"</tr>\r\n" \
	"</thead>\r\n" \
	"<tbody>\r\n"
	
	err = BufferAppend(&m, xstr, sizeof(xstr) -1);
	if (err) break;

    for(;;)
	{
	  GetDirEntryGS(&DirDCB);
	  if (_toolErr) break;
	
	  if ((InfoDCB.access & 0x0100) 
	    && (fDirHidden == false))
	    continue;
	
	    
	  hUrl = MangleName(&vName.bufString);
	  hHtml = MacRoman2HTML(&vName.bufString);
	  if (hUrl) HLock(hUrl);
	  if (hHtml) HLock(hHtml);
	
	  gUrl = hUrl ? (GSString255Ptr)*hUrl : &vName.bufString;
	  gHtml = hHtml ? (GSString255Ptr)*hHtml : &vName.bufString;
	
	
	  // folder -- no size, include trailing /
	  if (DirDCB.fileType == 0x0f)
	  {
	    i = orca_sprintf(buffer,
	        "<tr>"
	          "<td><a href=\"%B%B/\">%B/</a></td>"
	          "<td align=\"right\"> &mdash; </td>"
	          "<td> Folder </td><td></td>"
	        "</tr>\r\n",
	        path, gUrl, gHtml);
	  }
	  else
	  {
	    const char *fType = FindFType(DirDCB.fileType, DirDCB.auxType);
	
	    LongWord size = DirDCB.eof;
	    Word as = false;
	
	    size += 1023;
	    size >>= 10;  // convert to K.
	
	    switch(fAppleSingle)
	    {
	    case 0:
	      as = false;
	      break;
	    case 1:
	      as = true;
	      break;
	   case 2:
	     as = (DirDCB.flags & 0x8000);
	     break;
	    }
	
	    if (as)
	    {
		i = orca_sprintf(buffer,
		      "<tr>"
	          "<td><a href=\"%B%B\">%B</a></td>"
	          "<td align=\"right\"> %uK </td>"
	          "<td> %b </td>"
	          "<td><a href=\"%B%B?applesingle\">AppleSingle</a></td>"
		      "</tr>\r\n",
		      path, gUrl, gHtml,
	          (Word)size,
		      fType,
	          path, gUrl);
	    }
	    else
	    {
		  i = orca_sprintf(buffer,
		        "<tr>"
	            "<td><a href=\"%B%B\">%B</a></td>"
	            "<td align=\"right\"> %uK </td>"
	            "<td> %b </td> <td></td>"
		        "</tr>\r\n",
		        path, gUrl, gHtml,
	            (Word)size,
		        fType);
	     }
	   }
	
	
	   if (hUrl) DisposeHandle(hUrl);
	   if (hHtml) DisposeHandle(hHtml);
	   
	   
	  	err = BufferAppend(&m, buffer, i);
	    if (err) break;
	  } // dir entry listing.
	  if (err) break;
	  
	#undef xstr
	#define xstr "</tbody></table>\r\n</body>\r\n</html>\r\n"
	
	err = BufferAppend(&m, xstr, sizeof(xstr) -1);
	if (err) break;	
  	
  } while (false);

  
  if (hPath) DisposeHandle(hPath);
  if (hHtml) DisposeHandle(hHtml);
  if (hUrl) DisposeHandle(hUrl);
  
  if (err) return ProcessError(500, q);


  SendHeader(q, 200, m.used, NULL, "text/html", true);

  WriteData(q, *m.h, m.used);
  WriteData(q, NULL, 0);

  q->state = STATE_CLOSE;
  return 200; 
}





// list the volumes
Word ListVolumes(struct qEntry *q)
{
Word i;
Word d;

Handle hUrl, hHtml;
GSString255Ptr gUrl, gHtml;
Word err;

CREATE_BUFFER(m, q->workHandle);

  if (!fDir)
  {
    return ProcessError(403, q);
  }
  if (q->command == CMD_HEAD)
  {
    SendHeader(q, 200, -1, NULL, "text/html", true);
    return 200;
  }

  HUnlock(q->workHandle);
  SetHandleSize(0, q->workHandle);

  do 
  {
    err = BufferAppend(&m, _htmlHead, sizeof(_htmlHead) - 1);
	if (err) break;


	#undef xstr
	#define xstr \
	"<title>Index of /</title>\r\n" \
	"</head>\r\n" \
	"<body>\r\n" \
	"<h1>Index of /</h1>\r\n" 

	err = BufferAppend(&m, xstr, sizeof(xstr) -1);
	if (err) break;	
  	
  	#undef xstr
	#define xstr \
	"<table border=\"0\" cellspacing=\"2\" cellpadding=\"2\">\r\n" \
	"<thead align=\"left\">\r\n" \
	"<tr>\r\n" \
	"<th>Name</th><th>Size</th><th>Kind</th>\r\n" \
	"</tr>\r\n" \
	"</thead>\r\n" \
	"<tbody>\r\n"
  	
	err = BufferAppend(&m, xstr, sizeof(xstr) -1);
	if (err) break;
	
	
	for (d = 1; ; i++)
  	{
      DInfoDCB.devNum = d;
      DInfoGS(&DInfoDCB);
      if (_toolErr) break;
      if (DInfoDCB.characteristics & 0x80 == 0) continue;

      VolumeGS(&VolumeDCB);
      if (_toolErr) continue;
    
      if ((VolumeDCB.fileSysID == appleShareFSID) 
        && (fDirAppleShare == false))
        continue;
      
      
      // convert first char from ':' --> '/'
      vName.bufString.text[0] = '/';

      hUrl = MangleName(&vName.bufString);
      hHtml = MacRoman2HTML(&vName.bufString);

      if (hUrl) HLock(hUrl);
      if (hHtml) HLock(hHtml);

      gUrl = hUrl ? (GSString255Ptr)*hUrl : &vName.bufString;
      gHtml = hHtml ? (GSString255Ptr)*hHtml : &vName.bufString;


      i = orca_sprintf(buffer,
        "<tr>"
        "<td><a href=\"%B/\">%B/</a></td>"
        "<td align=\"right\"> &mdash; </td>"
        "<td> Folder </td>"
        "</tr>\r\n",
        gUrl, gHtml);


       if (hUrl) DisposeHandle(hUrl);
       if (hHtml) DisposeHandle(hHtml);


  	   err = BufferAppend(&m, buffer, i);
	   if (err) break;
    }

    #undef xstr
    #define xstr "</tbody></table>\r\n</body>\r</html>\r\n"

 	err = BufferAppend(&m, xstr, sizeof(xstr) -1);
	if (err) break;

  } while (false);

  if (err) return ProcessError(500, q);


  SendHeader(q, 200, m.used, NULL, "text/html", true);

  WriteData(q, *m.h, m.used);
  WriteData(q, NULL, 0);
  
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
  if (!q->fullpath)
  {
    return ProcessError(400, q);
  }

  path = q->fullpath;


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
