#pragma noroot
#pragma lint -1
#pragma optimize -1
#pragma debug 0x8000

#include <Types.h>
#include <string.h>

#include "config.h"

struct etable
{
  Word length;
  const char *ext;
  const char *mime;
};

static struct etable ascii[] =
{
  { 4,	"html",	"text/html"},
  { 3,	"htm",	"text/html"},
  { 3,	"xml",	"text/xml"},
  { 0, NULL, NULL}
};

static struct etable binary[] =
{
  { 3,	"bxy",	"application/x-BinaryII"},
  { 4,	"gif",	"image/gif"},             
  { 4,	"jpeg",	"image/jpeg"},
  { 3,	"jpg",	"image/jpeg"},
  { 3,	"png",	"image/png"},
  { 3,	"shk",	"application/x-ShrinkIt"},
  { 4,	"tiff",	"image/tiff"},
  { 0, NULL, NULL}         
};



const char *GetMimeString(const GSString255Ptr filename, Word fileType, LongWord auxType)
{
const char *mime;

Word i, l;
struct etable *e;

  mime = "application/octet-stream";
  l = filename->length;

  switch(fileType)
  {
  case 0x00:
  case 0x06:
    // check for extensions.
    for (e = binary; i = e->length; e++)
    {
      if (l > i && filename->text[l -i -  1] == '.'
        && !strincmp(e->ext, filename->text + l - i, i))
      {
        mime = e->mime;
        break;
      }
    }
    break;

  case 0x04:
  case 0xb0:
    mime = "text/plain";
    // check for extensions.
    for (e = ascii; i = e->length; e++)
    {
      if (l > i && filename->text[l -i -  1] == '.'
        && !strincmp(e->ext, filename->text + l - i, i))
      {
        mime = e->mime;
        break;
      }
    }
    break;

  case 0x0f:  // directory... special case.
    mime = "text/html";
    break;

  case 0x50:
    if (auxType == 0x5445 && fTeach) mime = "text/plain";
    break;

  case 0xc0:
    if (auxType == 0x8006) mime = "image/gif";
    break;

  case 0xe0:
    if (auxType == 0x0001) mime = "application/applefile";
    else if (auxType == 0x0002) mime = "multipart/appledouble";
    else if (auxType == 0x8000) mime = "application/x-BinaryII";
    else if (auxType == 0x8002) mime = "application/x-Shrinkit";
  }
  return mime;
}
