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
#include "pointer.h"
#include "MemBuffer.h"

#define	dcRemovable		0x0004
#define dcBlockDevice	0x0080

extern int orca_sprintf(char *, const char *, ...);


extern const char *GetMimeString(GSString255Ptr, Word, LongWord);

extern void * MacRoman2HTML(const GSString255 *gstr);
extern void * EncodeURL(const GSString255 *gstr);


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
void *path_uri;

GSString255Ptr path;

  CREATE_BUFFER(m, q->workHandle);
  	
  HUnlock(q->workHandle);
  SetHandleSize(0, q->workHandle);


  path = q->pathname;

  path_uri = EncodeURL(path);
  if (path_uri == NULL)
  {
    path_uri = path;
    RetainPointer(path);
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

    i = orca_sprintf(buffer, xstr, path_uri, append);
	err = BufferAppend(&m, xstr, sizeof(xstr) - 1);
	if (err) return ProcessError(500, q);
  }

  i = 0;
  if (q->host)
  {
    i = orca_sprintf(buffer, "Location: http://%B%B%B\r\n",
      q->host, path_uri, append);
  }
  
  SendHeader(q, 301, q->command == CMD_HEAD ? (LongWord)-1 : m.used,
    NULL, "text/html", buffer, i);


  ReleasePointer(path_uri);

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

  newpath = NewPointer(2 + i + sizeof(xstr));


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
    DisposePointer(newpath);
    return Redirect(q, (GSString255Ptr)"\x0a\x00index.html");
  }

  // check for .htm
  newpath->length--;
  GetFileInfoGS(&InfoDCB);
  if (!_toolErr)
  {
    DisposePointer(newpath);
    return Redirect(q, (GSString255Ptr)"\x09\x00index.htm");
  }

  DisposePointer(newpath);

  return 0;
}

Word ListDirectory(struct qEntry *q)
{
Word i;
Word err;

GSString255Ptr path;  

GSString255Ptr path_uri;
GSString255Ptr path_html;

CREATE_BUFFER(m, q->workHandle);

  q->state = STATE_CLOSE;
  path = q->pathname;

  if (err = CheckIndex(q)) return err;

  if (!fDir)
  {
    return ProcessError(403, q);
  }
  if (q->command == CMD_HEAD)
  {
    SendHeader(q, 200, -1, NULL, "text/html", NULL, 0);
    return 200;
  }


  DirDCB.refNum = q->fd;

  HUnlock(q->workHandle);
  SetHandleSize(0, q->workHandle);
  
  
  path_uri = (GSString255Ptr)EncodeURL(path);
  path_html = (GSString255Ptr)MacRoman2HTML(path);
  if (path_uri == NULL)
  {
    path_uri = path;
    RetainPointer(path);	
  }
  if (path_html == NULL)
  {
    path_html = path;
    RetainPointer(path);	
  }
  
  
  // do ... while (false) to allow breaking.
  do
  {
  	err = BufferAppend(&m, _htmlHead, sizeof(_htmlHead) - 1);
  	if (err) break;
  	

  	i = orca_sprintf(buffer,
	  "<title>Index of %B</title>\r\n" \
	  "</head>\r\n" \
	  "<body>\r\n" \
	  "<h1>Index of %B</h1>\r\n",  	 
  	  path_html, path_html);

  	err = BufferAppend(&m, buffer, i);
  	if (err) break;
  	

	// if we're jailed and at the root, no parent directory
	if (path->length > 1)
	{
	  Word i, j;
	  i = j = path_uri->length;
	  i -= 2; // -1 to convert o 0-index, -1 to skip trailing /
	  while (path_uri->text[i] != '/') i--;
	  path_uri->length = i + 1;
	
	  i = orca_sprintf(buffer,
	    "<p><a href=\"%B\">Parent Directory</a></p>\r\n",
	    path_uri);
	
  	  err = BufferAppend(&m, buffer, i);
  	  if (err) break;
	
	  path_uri->length = j;
	}
    

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
	void *file_uri;
	void *file_html;
	Word alloc = 0;
		
	  GetDirEntryGS(&DirDCB);
	  if (_toolErr) break;
	
	  if ((DirDCB.access & 0x0004) 
	    && (fDirHidden == false))
	    continue;
	
	  file_uri = EncodeURL(&vName.bufString);
	  file_html = MacRoman2HTML(&vName.bufString);
	  
	  if (file_uri == NULL)
	  {
	    file_uri = &vName.bufString; 
	  }
	  else alloc |= 0x0001;
	  if (file_html == NULL)
	  {
	    file_html = &vName.bufString;
	  }	
	  else alloc |= 0x0002;
	
	  // folder -- no size, include trailing /
	  if (DirDCB.fileType == 0x0f)
	  {
	    i = orca_sprintf(buffer,
	        "<tr>"
	          "<td><a href=\"%B%B/\">%B/</a></td>"
	          "<td align=\"right\"> &mdash; </td>"
	          "<td> Folder </td><td></td>"
	        "</tr>\r\n",
	        path_uri, file_uri, file_html);
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
		      path_uri, file_uri, file_html,
	          (Word)size,
		      fType,
	          path_uri, file_uri);
	    }
	    else
	    {
		  i = orca_sprintf(buffer,
		        "<tr>"
	            "<td><a href=\"%B%B\">%B</a></td>"
	            "<td align=\"right\"> %uK </td>"
	            "<td> %b </td> <td></td>"
		        "</tr>\r\n",
		        path_uri, file_uri, file_html,
	            (Word)size,
		        fType);
	     }
	   }
	
	
	   if (alloc & 0x01) DisposePointer(file_uri);
	   if (alloc & 0x02) DisposePointer(file_html);	   
	   
	  	err = BufferAppend(&m, buffer, i);
	    if (err) break;
	  } // dir entry listing.
	  if (err) break;
	  
	#undef xstr
	#define xstr "</tbody></table>\r\n</body>\r\n</html>\r\n"
	
	err = BufferAppend(&m, xstr, sizeof(xstr) -1);
	if (err) break;	
  	
  } while (false);

  
  ReleasePointer(path_html);
  ReleasePointer(path_uri);

  
  if (err) return ProcessError(500, q);

  SendHeader(q, 200, m.used, NULL, "text/html", NULL, 0);

  WriteData(q, *m.h, m.used);
  WriteData(q, NULL, 0);

  return 200; 
}





// list the volumes
Word ListVolumes(struct qEntry *q)
{
Word i;
Word d;

Word err;


CREATE_BUFFER(m, q->workHandle);

  q->state = STATE_CLOSE;

  if (!fDir)
  {
    return ProcessError(403, q);
  }
  if (q->command == CMD_HEAD)
  {
    SendHeader(q, 200, -1, NULL, "text/html", NULL, 0);
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
	
	
	for (d = 1; ; d++)
  	{	
  	void *dev_uri;
	void *dev_html;
	Word alloc = 0;
  		
      DInfoDCB.devNum = d;
      DInfoGS(&DInfoDCB);
      if (_toolErr) break;
      if (DInfoDCB.characteristics & dcBlockDevice == 0) continue;
      
      if ((DInfoDCB.characteristics & dcRemovable)
      	&& (fDirRemovable == false))
        continue;

      VolumeGS(&VolumeDCB);
      if (_toolErr) continue;
    
      if ((VolumeDCB.fileSysID == appleShareFSID) 
        && (fDirAppleShare == false))
        continue;
      
      
      // convert first char from ':' --> '/'
      vName.bufString.text[0] = '/';

      dev_uri = EncodeURL(&vName.bufString);
      dev_html = MacRoman2HTML(&vName.bufString);

	  if (dev_uri == NULL)
	  {
	    dev_uri = &vName.bufString;
	  }
	  else alloc |= 0x0001;

	  if (dev_html == NULL)
	  {
	    dev_html = &vName.bufString;
	  }
	  else alloc |= 0x0002;


      i = orca_sprintf(buffer,
        "<tr>"
        "<td><a href=\"%B/\">%B/</a></td>"
        "<td align=\"right\"> &mdash; </td>"
        "<td> Folder </td>"
        "</tr>\r\n",
        dev_uri, dev_html);


	  if (alloc & 0x0001) DisposePointer(dev_uri);
	  if (alloc & 0x0002) DisposePointer(dev_html);

  	   err = BufferAppend(&m, buffer, i);
	   if (err) break;
    }

    #undef xstr
    #define xstr "</tbody></table>\r\n</body>\r</html>\r\n"

 	err = BufferAppend(&m, xstr, sizeof(xstr) -1);
	if (err) break;

  } while (false);

  if (err) return ProcessError(500, q);


  SendHeader(q, 200, m.used, NULL, "text/html", NULL, 0);

  WriteData(q, *m.h, m.used);
  WriteData(q, NULL, 0);
  
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
    eof = InfoDCB.eof;
    fileType = InfoDCB.fileType;
    auxType = InfoDCB.auxType;
    modDateTime = InfoDCB.modDateTime;
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
    GetMimeString(path, fileType, auxType), NULL, 0);

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
