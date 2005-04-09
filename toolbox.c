#pragma optimize -1
#pragma lint -1
#pragma noroot

#include <dialog.h>
#include <gsos.h>
#include <Memory.h>
#include <quickdraw.h>
#include <stdfile.h>
#include <window.h>

#include "toolbox.h"

#if 0
struct ItemTemplate {
   Word itemID;
   Rect itemRect;
   Word itemType;
   Pointer itemDescr;
   Word itemValue;
   Word itemFlag;
   Pointer itemColor;                   /* pointer to appropriate type of color table */
   };
#endif

#define Width640 320
#define Height640 109 + 6

#define Width320  270
#define Height320 109 + 6


// -- 320 --

static ItemTemplate Save320 =
{
	1,			// itemID
	{ 43, 165, 55, 265 },	// itemRect
	buttonItem,		// itemType
	"\pSelect",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};

static ItemTemplate Open320 =
{
	2,			// itemID
	{ 61, 165, 73, 265 },	// itemRect
	buttonItem,		// itemType
	"\pOpen",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};


static ItemTemplate Close320 =
{
	3,			// itemID
	{ 79, 165, 91, 265 },	// itemRect
	buttonItem,		// itemType
	"\pClose",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};

static ItemTemplate Disks320 =
{
	4,			// itemID
	{ 25, 165, 37, 265 },	// itemRect
	buttonItem,		// itemType
	"\pDisks",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};


static ItemTemplate Cancel320 =
{
	5,			// itemID
	{ 97, 165, 109, 265 },	// itemRect
	buttonItem,		// itemType
	"\pCancel",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};


static ItemTemplate Scroll320 =
{
	6,			// itemID
	{ 0, 0, 0, 0 },		// itemRect
	userItem,		// itemType
	NULL,			// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};

static ItemTemplate Files320 =
{
	8,			// itemID
	{ 25, 5, 109, 160 - 17 },	// itemRect
	userItem | itemDisable, // itemType
	NULL,		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};

static ItemTemplate Prompt320 =
{
	9,			// itemID
	{ 3 ,5, 12, Width320 - 5},	// itemRect
	statText | itemDisable, // itemType
	NULL,			// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};

static ItemTemplate Paths320 =
{
	7,			// itemID
	{ 10, 6, 24, Width320 - 5 },	// itemRect
	userItem,		// itemType
	NULL,			// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};


static ItemTemplate Edit320 =
{
	10,			// itemID
	{ Height320 + 10, 0, 0, 0 },	// itemRect
	editLine | itemDisable, // itemType
	NULL,			// itemDescr
	0x20,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};


static ItemTemplate Free320 =
{
	11,			// itemID
	{ Height320 + 10, 0, 0, 0 },	// itemRect
	statText | itemDisable, // itemType
	"\p^0 free of ^1K.",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};


static ItemTemplate Folder320 =
{
	12,			// itemID
	{ Height320 + 10, 0, 0, 0 },	// itemRect
	buttonItem,		// itemType
	"\pNew Folder",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};



// -- 640 --
static ItemTemplate Save640 =
{
	1,			// itemID
	{ 43, 204, 55, 310 },	// itemRect
	buttonItem,		// itemType
	"\pSelect",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};

static ItemTemplate Open640 =
{
	2,			// itemID
	{ 61, 204, 73, 310 },	// itemRect
	buttonItem,		// itemType
	"\pOpen",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};


static ItemTemplate Close640 =
{
	3,			// itemID
	{ 79, 204, 91, 310 },	// itemRect
	buttonItem,		// itemType
	"\pClose",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};

static ItemTemplate Disks640 =
{
	4,			// itemID
	{ 25, 204, 37, 310 },	// itemRect
	buttonItem,		// itemType
	"\pDisks",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};


static ItemTemplate Cancel640 =
{
	5,			// itemID
	{ 97, 204, 109, 310 },	// itemRect
	buttonItem,		// itemType
	"\pCancel",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};


static ItemTemplate Scroll640 =
{
	6,			// itemID
	{ 0, 0, 0, 0 },		// itemRect
	userItem,		// itemType
	NULL,			// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};

static ItemTemplate Files640 =
{
	8,			// itemID
	{ 25, 10, 109, 170 },	// itemRect
	userItem | itemDisable, // itemType
	NULL,		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};

static ItemTemplate Prompt640 =
{
	9,			// itemID
	{ 3 ,10, 12, 200 },	// itemRect
	statText | itemDisable, // itemType
	NULL,		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};

static ItemTemplate Paths640 =
{
	7,			// itemID
	{ 10, 12, 24, 315 },	// itemRect
	userItem,		// itemType
	NULL,		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};


static ItemTemplate Edit640 =
{
	10,			// itemID
	{ Height640 + 10, 0, 0, 0 },	// itemRect
	editLine | itemDisable, // itemType
	NULL,			// itemDescr
	0x20,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};


static ItemTemplate Free640 =
{
	11,			// itemID
	{ Height640 + 10, 0, 0, 0 },	// itemRect
	statText | itemDisable, // itemType
	"\p^0 free of ^1K.",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};


static ItemTemplate Folder640 =
{
	12,			// itemID
	//{ 29, 204, 41, 310 },	// itemRect
	{ Height640 + 10, 0, 0, 0 },	// itemRect
	buttonItem,		// itemType
	"\pNew Folder",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};



struct sfdlg
{
	Rect bRect;
	Word vFlag;
	LongWord refCon;
	ItemTemplate *items[13];
};

static struct sfdlg dlg640 =
{
	{0, 0, Height640, Width640},
	-1,
	0,
	{
		&Save640,
		&Open640,
		&Close640,
		&Disks640,
		&Cancel640,
		&Scroll640,
		&Paths640,
		&Files640,
		&Prompt640,
		&Edit640,
		&Free640,
		&Folder640,
		NULL
	}
};


static struct sfdlg dlg320 =
{
	{0, 0, Height320, Width320},
	-1,
	0,
	{
		&Save320,
		&Open320,
		&Close320,
		&Disks320,
		&Cancel320,
		&Scroll320,
		&Paths320,
		&Files320,
		&Prompt320,
		&Edit320,
		&Free320,
		&Folder320,
		NULL
	}
};

static Word fSuccess;

#pragma databank 1
static pascal HitTest(DialogPtr dialog, Word *itemPtr)
{
Word item;
  item = *itemPtr;
  if (item == 1)
  {
    *itemPtr = 5; // cancel
    fSuccess = 1;
  }
  else if (item == 10 || item == 12)
  {
    *itemPtr = 0;
  }
}
#pragma databank 0

/* select a folder ... */
pascal void SFGetFolder2(Word x, Word y, Word prDesc, Ref prRef,
  SFReplyRec2Ptr reply)
{
static PrefixRecGS PrefixDCB;
static Word buffer[6] = { 4, 0, 0};

ResultBuf255Ptr rbp;

Word MemID;
Handle pH;

Handle h;
Word size;

  fSuccess = 0;
  SFPPutFile2(x, y, NULL, prDesc,  prRef,
    refIsPointer, (Ref)"\x05\x00hello",
    GetMasterSCB() & 0x80 ? (Pointer)dlg640 : (Pointer)dlg320,
    (VoidProcPtr)HitTest, reply);

  if (_toolErr) return;

  // prefix 0 will be set to the current path.
  if (fSuccess)
  {
    MemID = MMStartUp();

    // call twice, first time to get length...
    PrefixDCB.pCount = 2;
    PrefixDCB.prefixNum = 0;
    PrefixDCB.buffer.getPrefix = (ResultBuf255Ptr)buffer;
    GetPrefixGS(&PrefixDCB);

    size = buffer[1] + 4;
    pH = NewHandle(size, MemID, attrLocked, NULL);

    if (!_toolErr)
    {
      rbp = (ResultBuf255Ptr)*pH;
      rbp->bufSize = size;
      PrefixDCB.buffer.getPrefix = rbp;
      GetPrefixGS(&PrefixDCB);
                                      
      reply->good = 1;
      reply->fileType = 0x0f;
      reply->auxType = 0;

      h = (Handle)reply->nameRef;

      switch(reply->nameRefDesc)
      {
      case refIsNewHandle:
        h = NewHandle (size, MemID, 0, NULL);
        if (!_toolErr)
          HandToHand(pH, h, size);
        reply->nameRef = (Ref)h;
        break;

      case refIsHandle:
        if (h)
        {
          SetHandleSize(size, h);
          if (!_toolErr)
            HandToHand(pH, h, size);
        }               
        break;
      case refIsPointer:
        if (h)
        {
          if ( ((ResultBuf255Ptr)h)->bufSize >= size)
          {
            BlockMove((Pointer)rbp->bufString, (Pointer)h + 2, size -2);
          }
        }
      }
      // now set the path
      h = (Handle)reply->pathRef;

      switch(reply->pathRefDesc)
      {
      case refIsNewHandle:
        h = NewHandle (size, MemID, 0, NULL);
        if (!_toolErr)
          HandToHand(pH, h, size);
        reply->pathRef = (Ref)h;
        break;

      case refIsHandle:
        if (h)
        {
          SetHandleSize(size, h);
          if (!_toolErr)
            HandToHand(pH, h, size);
        }               
        break;
      case refIsPointer:
        if (h)
        {
          if ( ((ResultBuf255Ptr)h)->bufSize >= size)
          {
            BlockMove((Pointer)rbp->bufString, (Pointer)h + 2, size -2);
          }
        }
      }
    }       
    DisposeHandle(pH);
  }
}

pascal void CenterWindow(WindowPtr win)
{
Word x, y;
Word h, w;
Word is640;

  if (win)
  {
    is640 = GetMasterSCB() & 0x80;

    w = win->portRect.h2 - win->portRect.h1;
    h = win->portRect.v2 - win->portRect.v1;

    y = (200 - h) >> 1;

    if (is640)
    {
      x = (640 - w) >> 1;
      x &= ~3;  // align to 4-pixel
    }
    else
    {
      x = (320 - w) >> 1;
    }
    y += 24; // fudge for menubar & window title.

    MoveWindow(x, y, win);
  }
}
