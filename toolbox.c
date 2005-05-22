#pragma optimize -1
#pragma lint -1
#pragma noroot
#pragma debug 0x8000

#include <control.h>
#include <dialog.h>
#include <gsos.h>
#include <LineEdit.h>
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
#define Height640 128

#define Width320  270
#define Height320 128


// -- 320 --

static ItemTemplate Save320 =
{
	1,			// itemID
	{ 59, 165, 72, 265 },	// itemRect
	buttonItem,		// itemType
	"\pSelect",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};

static ItemTemplate Open320 =
{
	2,			// itemID
	{ 77, 165, 90, 265 },	// itemRect
	buttonItem,		// itemType
	"\pOpen",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};


static ItemTemplate Close320 =
{
	3,			// itemID
	{ 93, 165, 106, 265 },	// itemRect
	buttonItem,		// itemType
	"\pClose",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};

static ItemTemplate Disks320 =
{
	4,			// itemID
	{ 25, 165, 38, 265 },	// itemRect
	buttonItem,		// itemType
	"\pDisks",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};


static ItemTemplate Cancel320 =
{
	5,			// itemID
	{ 109, 165, 122, 265 },	// itemRect
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
	{ 25, 5, 122, 160 - 17 },	// itemRect
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
	{ 41, 165, 54, 265 },	// itemRect
	buttonItem,		// itemType
	"\pNew Folder\xc9",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};



// -- 640 --


static ItemTemplate Disks640 =
{
	4,			// itemID
	{ 25, 204, 38, Width640 - 10 },	// itemRect
	buttonItem,		// itemType
	"\pDisks",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};

static ItemTemplate Folder640 =
{
	12,			// itemID
	{ 41, 204, 54, Width640 - 10 },	// itemRect
	buttonItem,		// itemType
	"\pNew Folder\xc9",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};


static ItemTemplate Save640 =
{
	1,			// itemID
	{ 59, 204, 72, Width640 - 10 },	// itemRect
	buttonItem,		// itemType
	"\pSelect",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};

static ItemTemplate Open640 =
{
	2,			// itemID
	{ 77, 204, 90, Width640 - 10 },	// itemRect
	buttonItem,		// itemType
	"\pOpen",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};

static ItemTemplate Close640 =
{
	3,			// itemID
	{ 93, 204, 106, Width640 - 10 },	// itemRect
	buttonItem,		// itemType
	"\pClose",		// itemDescr
	0,			// itemValue
	0,			// itemFlag
	NULL			// itemColor
};



static ItemTemplate Cancel640 =
{
	5,			// itemID
	{ 109, 204, 122, Width640 - 10 },	// itemRect
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
	{ 25, 10, 122, 170 },	// itemRect
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





struct sfdlg
{
	Rect bRect;
	Word vFlag;
	LongWord refCon;
	ItemTemplate *items[14];
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
		//&Folder640,
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
		//&Folder320,
		NULL
	}
};

static Word fSuccess;

static char folderName[32];



static StaticTextTemplate ctrl_stat640 =
{
	{
		8,
		4,
		{ 6, 15, 16, 320 },
		0x81000000,
		0x0000,
		0x3000,
		0,
	},
	#define xstr "Name of folder to create?"
	(Ref)xstr,
	sizeof(xstr) - 1,
	0	
};

static LineEditTemplate ctrl_le640 = 
{
	{
		8,
		1,
		{19, 15, 32, 320 },
		0x83000000,
		0x0000,
		0x7000,
		0,
	},
	31,
	NULL,
	0
};

static SimpleButtonTemplate ctrl_create640 = 
{
	{
		9,
		2,
		{36, 200, 49, 300 },
		0x80000000,
		0x0001,
		0x3000,
		0,
	},
	(Ref)"\pCreate",
	NULL,
	{
		0x0d, 0x0d, 0, 0
	}	
};


static SimpleButtonTemplate ctrl_cancel640 = 
{
	{
		9,
		3,
		{36, 30, 49, 130 },
		0x80000000,
		0x0000,
		0x3000,
		0,
	},
	(Ref)"\pCancel",
	NULL,
	{
		0x1b, 0x1b, 0, 0
	}	
};



static LongWord controls640[] =
{
	(LongWord)&ctrl_le640,
	(LongWord)&ctrl_create640,
	(LongWord)&ctrl_cancel640,
	(LongWord)&ctrl_stat640,
	0
};

static WindParam1  windowTemp640 =
{
	0x50,
	0x0020,			// frame fVis
	NULL,			// title
	NULL,			// ref
	{0, 0, 0, 0},
	NULL,
	0, 0,			// origin
	0, 0,			// data size
	0, 0,			// max size
	0, 0,			// scroll size
	0, 0,			// page size
	0,			// info refcon
	0,			// info height
	NULL,			// frame def proc
	NULL,			// info def proc
	NULL,			// content def proc
	{35, 285, 92, 620}, 	//position
	(WindowPtr)0xffffffff,		// plane
	(Long)controls640,
	3					// PtrToPtr
};


//--- 320 mode

static StaticTextTemplate ctrl_stat320 =
{
	{
		8,
		4,
		{ 6, 8, 16, 160 },
		0x81000000,
		0x0000,
		0x3000,
		0,
	},
        #undef xstr
	#define xstr "Name of folder?"
	(Ref)xstr,
	sizeof(xstr) - 1,
	0	
};

static LineEditTemplate ctrl_le320 = 
{
	{
		8,
		1,
		{19, 8, 32, 160 },
		0x83000000,
		0x0000,
		0x7000,
		0,
	},
	31,
	NULL,
	0
};

static SimpleButtonTemplate ctrl_create320 = 
{
	{
		9,
		2,
		{36, 90, 49, 150 },
		0x80000000,
		0x0001,
		0x3000,
		0,
	},
	(Ref)"\pCreate",
	NULL,
	{
		0x0d, 0x0d, 0, 0
	}	
};


static SimpleButtonTemplate ctrl_cancel320 = 
{
	{
		9,
		3,
		{36, 15, 49, 75 },
		0x80000000,
		0x0000,
		0x3000,
		0,
	},
	(Ref)"\pCancel",
	NULL,
	{
		0x1b, 0x1b, 0, 0
	}	
};




static LongWord controls320[] =
{
	(LongWord)&ctrl_le320,
	(LongWord)&ctrl_create320,
	(LongWord)&ctrl_cancel320,
	(LongWord)&ctrl_stat320,
	0
};

static WindParam1  windowTemp320 =
{
	0x50,
	0x0020,			// frame fVis
	NULL,			// title
	NULL,			// ref
	{0, 0, 0, 0},
	NULL,
	0, 0,			// origin
	0, 0,			// data size
	0, 0,			// max size
	0, 0,			// scroll size
	0, 0,			// page size
	0,			// info refcon
	0,			// info height
	NULL,			// frame def proc
	NULL,			// info def proc
	NULL,			// content def proc
	{35, 142, 92, 310}, 	//position
	(WindowPtr)0xffffffff,		// plane
	(Long)controls320,
	3					// PtrToPtr
};


static Word GetFolder(void)
{
WindowPtr w;
GrafPortPtr oldPort;
Word ret = 0;

static EventRecord event;

Word done;
	
	event.wmTaskMask = 0x001f0004;
	
	oldPort = GetPort();
	w = NewWindow2(NULL, NULL, NULL, NULL, 
	  refIsPointer, 
	  GetMasterSCB() & 0x80 ? (Long)&windowTemp640 : (Long)&windowTemp320,
	  0x800e);

	if (_toolErr) return 0;
	SetPort(w);

	//NewControl2(w, 3, (Ref)&controls);
	
	SetLETextByID(w, 1, (StringPtr)"\pNew.Folder");	
	
	for (done = false; !done; )
	{
		Word control;
		
		control = (Word)DoModalWindow(&event, NULL, NULL, NULL, 0x0008);
		if (control == 2)
		{
			
			GetLETextByID(w, 1, (StringPtr)folderName);
			
			ret = 1;
			done = true;
		}
		else if (control == 3)
		{
			ret = 0;
			done = true;	
		}
	}
	
	CloseWindow(w);
	
	SetPort(oldPort);
	return ret;
}


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
  else if (item == 12)
  {
  	if (GetFolder())
  	{
  		SetIText(dialog, 10, folderName); 
  	}
  	else *itemPtr = 0;
  }
  else if (item == 10)
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
