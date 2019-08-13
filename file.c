/*
 *  send a file.
 *
 */

#pragma noroot
#pragma lint -1
#pragma optimize -1
#pragma debug 0x8000

#include <Memory.h>

#include <tcpip.h>
#include <gsos.h>

#include <stdio.h>

#include "globals.h"

#include "server.h"
#include "config.h"
#include "ftype.h"
#include "pointer.h"
#include "MemBuffer.h"

#define B(x) x->length, x->text
#define PRIB ".*s"

extern const char *GetMimeString(GSString255Ptr, Word, LongWord);

extern void * MacRoman2HTML(const GSString255 *gstr);
extern void * EncodeURL(const GSString255 *gstr);

extern Word MacBinary(struct qEntry *q);
extern Word AppleSingle(struct qEntry *q);


Word GetNextVolume(Word cookie, VolumeRecGS *VolumeDCB);


extern Word MyID;


/* process head/get */

static ResultBuf255 vName = {259};

static VolumeRecGS VolumeDCB = {6, NULL, &vName};
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
GSString255Ptr path_uri;

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
#define xstr "<p>The document has moved <a href=\"%" PRIB "%" PRIB "\">here</a></p>\r\n" \
"</body>\r\n</html>\r\n"

    i = sprintf(buffer, xstr, B(path_uri), B(append));
	err = BufferAppend(&m, xstr, sizeof(xstr) - 1);
	if (err) return ProcessError(500, q);
  }

  i = 0;
  if (q->host)
  {
    i = sprintf(buffer, "Location: http://%" PRIB "%" PRIB "%" PRIB "\r\n",
      B(q->host), B(path_uri), B(append));
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
  	

  	i = sprintf(buffer,
	  "<title>Index of %" PRIB "</title>\r\n" \
	  "</head>\r\n" \
	  "<body>\r\n" \
	  "<h1>Index of %" PRIB "</h1>\r\n",  	 
  	  B(path_html), B(path_html));

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
	
	  i = sprintf(buffer,
	    "<p><a href=\"%" PRIB "\">Parent Directory</a></p>\r\n",
	    B(path_uri));
	
  	  err = BufferAppend(&m, buffer, i);
  	  if (err) break;
	
	  path_uri->length = j;
	}
    

    #undef xstr
	#define xstr \
	"<table border=\"0\" cellspacing=\"2\" cellpadding=\"2\">\r\n" \
	"<thead align=\"left\">\r\n" \
	"<tr>\r\n" \
	"<th>Name</th><th>Size</th><th>Kind</th><th></th><th></th><th></th>\r\n" \
	"</tr>\r\n" \
	"</thead>\r\n" \
	"<tbody>\r\n"
	
	err = BufferAppend(&m, xstr, sizeof(xstr) -1);
	if (err) break;

    for(;;)
	{
	GSString255Ptr file_uri;
	GSString255Ptr file_html;
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
	    i = sprintf(buffer,
	        "<tr>"
	          "<td><a href=\"%" PRIB "/\">%" PRIB "/</a></td>"
	          "<td align=\"right\"> &mdash; </td>"
	          "<td colspan=\"4\"> Folder </td>"
	        "</tr>\r\n",
	        B(file_uri), B(file_html));
	  }
	  else
	  {
	    const char *fType = FindFType(DirDCB.fileType, DirDCB.auxType);
	
	    LongWord size = DirDCB.eof;
	    Word as = false;
	
	    size += 1023;
	    size >>= 10;  // convert to K.
	
	
		
		i = sprintf(buffer,
			"<tr>"
			"<td><a href=\"%" PRIB "\">%" PRIB "</a></td>"
			"<td align=\"right\"> %uK </td>"
			"<td> %b </td>",
			B(file_uri), B(file_html),
			(Word)size,
			fType);
	

		// applesingle...
		as = fAppleSingle;
		if ((as == 2) && (DirDCB.flags & 0x8000 == 0)) as = 0;

		if (as)
		{
			i += sprintf(buffer + i,
			  "<td>"
			  "<a href=\"%" PRIB "?applesingle\">AppleSingle</a>"
			  "</td>",
			  B(file_uri));
		}
		else
		i += sprintf(buffer + i, "<td></td>");


		// appledouble...
		as = fAppleDouble;
		if ((as == 2) && (DirDCB.flags & 0x8000 == 0)) as = 0;

		if (as)
		{
			i += sprintf(buffer + i,
			  "<td>"
			  "<a href=\"._%" PRIB "\">AppleDouble</a>"
			  "</td>",
			  B(file_uri));
		}
		else
		i += sprintf(buffer + i, "<td></td>");
		
		// macbinary
		as = fAppleDouble;
		if ((as == 2) && (DirDCB.flags & 0x8000 == 0)) as = 0;

		if (as)
		{
			i += sprintf(buffer + i,
			  "<td>"
			  "<a href=\"%" PRIB "?macbinary\">MacBinary</a>"
			  "</td></tr>\r\n",
			  B(file_uri));
		}
		else
		i += sprintf(buffer + i, "<td></td></tr>\r\n");		

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
	
	
        d = 1;
        while (d = GetNextVolume(d, &VolumeDCB))
  	{	
  	GSString255Ptr dev_uri;
	GSString255Ptr dev_html;
	Word alloc = 0;
  		

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


      i = sprintf(buffer,
        "<tr>"
        "<td><a href=\"%" PRIB "/\">%" PRIB "/</a></td>"
        "<td align=\"right\"> &mdash; </td>"
        "<td> Folder </td>"
        "</tr>\r\n",
        B(dev_uri), B(dev_html));


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

       
	InfoDCB.pCount = 12;
	InfoDCB.pathname = q->fullpath;
	InfoDCB.optionList = NULL;
	GetFileInfoGS(&InfoDCB);
				
	if (_toolErr)
	{
		return ProcessError(404, q);			
	}
	     
	switch (q->moreFlags)
	{
	case CGI_APPLESINGLE:
	case CGI_APPLEDOUBLE:
		return AppleSingle(q);
		break;
	case CGI_MACBINARY:
		return MacBinary(q);
		break;
	}
	
	
	if (q->command == CMD_HEAD)
	{
		eof = InfoDCB.eof;
		fileType = InfoDCB.fileType;
		auxType = InfoDCB.auxType;
		modDateTime = InfoDCB.modDateTime;
		
		q->state = STATE_CLOSE;
	}
	else
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
		
		q->state = STATE_WRITE;
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
	
	// actual writing takes place in server.c
	return 200;
}
