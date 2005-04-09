#pragma nda NDAOpen NDAClose NDAAction NDAInit 30 0xffff  "--Silver Platter\\H**"

#pragma lint -1
#pragma optimize -1

#include <Types.h>

#include <Control.h>
#include <Desk.h>
#include <Event.h>
#include <gsos.h>
#include <Memory.h>
#include <Menu.h>
#include <Locator.h>
#include <MiscTool.h>
#include <quickdraw.h>
#include <Resources.h>
#include <TextEdit.h>
#include <Window.h>

#include <tcpip.h>

#include <kstring.h>

#include "rez.h"
#include "httpnda.h"
#include "config.h"
#include "toolbox.h"
#include "ftype.h"


extern void SaveLog(Word MyID, Handle teH);

typedef struct split
{
  word lo;
  word hi;
} split;


const char *ReqName = "\pTCP/IP~kelvin~http~";

/*
    variables
 */

WindowPtr MyWindow;
MenuBarRecHndl MyMenuBar;
Word WindowW;

Boolean FlagHTTP;
Boolean FlagConnect;

Boolean FlagQDAux;
Boolean FlagFM;
Boolean FlagTE;
Boolean FlagSF;
Boolean FlagTT;
Boolean FlagTCP;

Handle HandleFM;
Handle HandleTE;
Handle HandleSF;

Boolean FlagConfig;

Word MyID;
//Word MyRID;
Word Ipid;



void InsertString(word length, const char *cp)
{
Handle handle;
TERecord **temp;
longword oldStart, oldEnd;

  handle = (Handle)GetCtlHandleFromID(MyWindow, CtrlTE);
  temp = (TERecord **)handle;

  (**temp).textFlags &= (~fReadOnly);

//  TEGetSelection((pointer)&oldStart, (pointer)&oldEnd, handle);

  TESetSelection((Pointer)-1, (Pointer)-1, handle);
  TEInsert(teDataIsTextBlock, (Ref)cp, length,
	  NULL, NULL, /* no style info */
	  handle);

  (**temp).textFlags |= fReadOnly;

//  TESetSelection((Pointer)oldStart, (Pointer)oldEnd, handle);

}


// enable/disable the menu items.

void UpdateMenuStatus(void)
{
  if (FlagConnect == false)
  {
    EnableMItem(StartMMItem);
    DisableMItem(StopMMItem);

    DisableMItem(StartHTTPMItem);
    DisableMItem(StopHTTPMItem);
    DisableMItem(ResetMItem);
  }
  else
  {
    DisableMItem(StartMMItem);
    EnableMItem(StopMMItem);
    EnableMItem(ResetMItem);
                             
    if (FlagHTTP == false)
    {
      EnableMItem(StartHTTPMItem);
      DisableMItem(StopHTTPMItem);
      DisableMItem(ResetMItem);
    } 
    else
    {
      DisableMItem(StartHTTPMItem);
      EnableMItem(StopHTTPMItem);
      EnableMItem(ResetMItem);
    } 
  }
}


#pragma databank  1

/*
 *  watch for
 */
pascal word HandleRequest(word request, longword dataIn, longword dataOut)
{
Word oldRApp;
MenuBarRecHndl oldMenuBar;

  oldRApp = GetCurResourceApp();
  SetCurResourceApp(MyID);

  oldMenuBar = GetMenuBar();
  SetMenuBar(MyMenuBar);


  if (request == TCPIPSaysNetworkUp)
  {
    FlagConnect = true;
    UpdateMenuStatus();
  }

  if (request == TCPIPSaysNetworkDown)
  {
    if (FlagHTTP) StopServer();

    FlagConnect = false;
    Ipid = 0;
    UpdateMenuStatus();
  }

  SetMenuBar(oldMenuBar);
  SetCurResourceApp(oldRApp);

}

pascal void DrawInfo(void *rect, MenuBarRecHndl menubar, GrafPortPtr w)
{
Word oldRApp;

  oldRApp = GetCurResourceApp();
  SetCurResourceApp(MyID);

  SetMenuBar(menubar);
  DrawMenuBar();
  SetMenuBar(NULL);

  SetCurResourceApp(oldRApp);

}

void DrawWindow(void)
{
  DrawControls(MyWindow);
}
#pragma databank 0

GrafPortPtr NDAOpen(void)
{
const char *err = NULL;
//Pointer myPath;
Word oldLevel;
Word oldPrefs;
Word oldRApp; 
LevelRecGS levelDCB;
//SysPrefsRecGS prefsDCB;
Handle H;
Rect r;
CtlRec *ctl;

static Word menuColor[] = {0x00e0, 0x000f, 0x0000 };


  if (QDVersion() < 0x0308)
  {
    AlertWindow(awCString, NULL,
      (Ref)"24~System 6.0.1 or greater is required.~^Too Bad");
    return NULL;
  }

  if (!LoadNDATools(MyID)) return NULL;

  // Check if Marinetti Active.

  FlagConnect = TCPIPGetConnectStatus();

  // load our resource. -- see TN.iigs #71
  //oldRApp = GetCurResourceApp();
  //ResourceStartUp(MyID);
  //myPath = LGetPathname2(MyID, 1);

  levelDCB.pCount = 2;
  GetLevelGS(&levelDCB);
  oldLevel = levelDCB.level;
  levelDCB.level = 0;
  SetLevelGS(&levelDCB);

  //prefsDCB.pCount = 1;
  //GetSysPrefsGS(&prefsDCB);
  //oldPrefs = prefsDCB.preferences;
  //prefsDCB.preferences = (prefsDCB.preferences & 0x1fff) | 0x8000;
  //SetSysPrefsGS(&prefsDCB);

  oldPrefs = DoSysPrefs(0xE000, 0x8000);

  //MyRID = OpenResourceFile(readEnable, NULL, myPath);
  oldRApp = OpenResourceFileByID(readEnable, MyID);
  //
  MyWindow = NewWindow2(NULL, 0, DrawWindow, NULL,
    refIsResource, rHTTPWindow, rWindParam1);

  CenterWindow(MyWindow);
  WindowW = MyWindow->portRect.h2 - MyWindow->portRect.h1;

  SetInfoDraw(DrawInfo, MyWindow);

  MyMenuBar = NewMenuBar2(refIsResource,(Ref)1, (Pointer)MyWindow);
  SetInfoRefCon((LongWord)MyMenuBar, MyWindow);
  SetMenuBar(MyMenuBar);

  // adjust the rect...
  GetRectInfo(&r, MyWindow);
  ctl = *MyMenuBar;

  ctl->ctlRect.v1 = r.v1 - 1;
  ctl->ctlRect.h1 = r.h1 - 2;
  ctl->ctlRect.v2 = r.v2 + 1;
  //ctl->ctlRect.h2 = r.h2 + 2;

  ctl->ctlColor = (Pointer)menuColor;


  FixMenuBar();

  UpdateMenuStatus();

  AcceptRequests(ReqName, MyID, &HandleRequest);

  SetSysWindow(MyWindow);
  ShowWindow(MyWindow);
  SelectWindow(MyWindow);

  SetMenuBar(NULL);

  if (!FlagConfig) FlagConfig = LoadConfig(MyID);
  LoadFTypes(MyID);



  // restore...
  //prefsDCB.preferences = oldPrefs;
  //SetSysPrefsGS(&prefsDCB);
  DoSysPrefs(0xffff, oldPrefs);

  levelDCB.level = oldLevel;
  SetLevelGS(&levelDCB);

  SetCurResourceApp(oldRApp);

  return MyWindow;

}

void NDAClose(void)
{
    // if running, shut down.

    if (FlagHTTP) StopServer();

    CloseWindow(MyWindow);
    MyWindow = NULL;
    MyMenuBar = NULL;

    if (FlagConfig)
    {
      UnloadConfig();
      FlagConfig = 0;
    }
    UnloadFTypes();

    // todo -- don't shut down, so server can run w/o window???
    AcceptRequests(ReqName, MyID, NULL);
    //CloseResourceFile(MyRID);
    ResourceShutDown();
}

void NDAInit(Word code)
{
  if (code)
  {
    MyWindow = NULL;
    MyMenuBar = NULL;

    FlagHTTP = false;
    FlagConnect = false;

    FlagQDAux = false;
    FlagFM = false;
    FlagTE = false;
    FlagSF = false;
    FlagTCP = false;
    FlagTT = false;

    HandleFM = NULL;
    HandleTE = NULL;
    HandleSF = NULL;

    FlagConfig = false;

    MyID = MMStartUp();
    Ipid = 0;
    //MyRID = 0;

  }
  else
  {
    UnloadNDATools();
  }
}

word NDAAction(void *param, int code)
{
word eventCode;
static EventRecord event;
Rect r;
Handle h;

  if (code == runAction)
  {
    if (FlagHTTP) Server();
    return 1;
  }
  if (code == copyAction)
  {
    h = (Handle)GetCtlHandleFromID(MyWindow, CtrlTE);
    TECopy(h);
    return 1; // yes we handled it.
  }

  if (code == eventAction)
  {
  Word skip_tmda = 0;

    memzero(&event, sizeof(event));

    BlockMove((Pointer)param, (Pointer)&event, 16);

    if (event.what == autoKeyEvt || event.what == keyDownEvt)
    {
      SetMenuBar(MyMenuBar);                            
      StartInfoDrawing(&r, MyWindow);
      MenuKey(&event, (MenuRecHndl)MyMenuBar);

      if ((word)event.wmTaskData)  // menu item selected...
      {
        eventCode = wInMenuBar;
        skip_tmda++;
        //HiliteMenu(0, ((split)event.wmTaskData).hi);
      }
      else
      {
        EndInfoDrawing();          
        SetMenuBar(NULL);
      }
    }

    if (!skip_tmda)
    {
      event.wmTaskMask = 0x001FFFFF;
      eventCode = TaskMasterDA(0, &event);
    }

    // do menu tracking, convert to a menu item....
    if (eventCode == wInInfo)
    {
    GrafPortPtr win = (GrafPortPtr)event.wmTaskData;

      SetMenuBar(MyMenuBar);
      StartInfoDrawing(&r, win);
      MenuSelect(&event, (MenuRecHndl)MyMenuBar);
      if ((word)event.wmTaskData)  // menu item selected...
      {
        eventCode = wInMenuBar;
        //HiliteMenu(0, ((split)event.wmTaskData).hi);
      }
      else
      {
        EndInfoDrawing();          
        SetMenuBar(NULL);
      }
    }

    switch(eventCode)
    {
    case wInControl:
      // if it's the TE control, we may be resized... barf.
      if (event.wmTaskData4 == CtrlTE)
      {
        Word w;
        CtlRecHndl ctrlH;
        CtlRecPtr ctrlP;
        GrafPortPtr oldPort;

        w = MyWindow->portRect.h2 - MyWindow->portRect.h1;
        if (w != WindowW)
        {
          WindowW = w;

          oldPort = GetPort();
          SetPort(MyWindow);

          ctrlH = GetCtlHandleFromID(MyWindow, CtrlTherm);
          ctrlP = *ctrlH;
          EraseRect(&ctrlP->ctlRect);
          ctrlP->ctlRect.h2 = w - 4;

          SetPort(oldPort);
        }
      }
      break;
    case wInMenuBar:

      switch((word)event.wmTaskData)
      {
      case SelectAllMItem:
        h = (Handle)GetCtlHandleFromID(MyWindow, CtrlTE);
        TESetSelection((Pointer)0, (Pointer)-1, h);
        break;

      // cut/copy/paste/clear
      case 251:
        h = (Handle)GetCtlHandleFromID(MyWindow, CtrlTE);
        TECut(h);
        break;

      case 252:
        h = (Handle)GetCtlHandleFromID(MyWindow, CtrlTE);
	TECopy(h);
        break;

      case 253:
        h = (Handle)GetCtlHandleFromID(MyWindow, CtrlTE);
        TEPaste(h);
        break;

      case 254:
        h = (Handle)GetCtlHandleFromID(MyWindow, CtrlTE);
        TEClear(h);
        break;

      case AboutMItem:
        {
        WindowPtr win;
        static EventRecord ev;

          memzero(&ev, sizeof(ev));
          ev.wmTaskMask = 0x001f0004;

          win = NewWindow2(NULL, NULL, NULL, NULL,
            refIsResource, (long)rAboutWindow, rWindParam1);

          CenterWindow(win);
          ShowWindow(win);
          for(;;)
          {
            DoModalWindow(&ev, NULL, NULL, NULL, 0x0008);
            if (ev.what == keyDownEvt
              || ev.what == autoKeyEvt
              || ev.what == mouseDownEvt)
              break;
          }
          CloseWindow(win);
        }
        break;

      case ConfigMItem:
        DoConfig(MyID);
        break;

      case SaveMItem:
        h = (Handle)GetCtlHandleFromID(MyWindow, CtrlTE);
        SaveLog(MyID, h);
        break;

      case EmptyMItem:
        h = (Handle)GetCtlHandleFromID(MyWindow, CtrlTE);
        TESetText(teDataIsTextBlock, (Ref)"", 0,
	  NULL, NULL, h);
        break;

      case StartMMItem:
        if (TCPIPGetConnectStatus())
        {
          FlagConnect = true;
          UpdateMenuStatus();
        }
        else
        {
          TCPIPConnect(NULL);
        }
        break;

      case StopMMItem:
        if (!TCPIPGetConnectStatus())
        {
          FlagConnect = false;
          UpdateMenuStatus();
        }
        else
        {
	  if (FlagHTTP) StopServer();
          // if option key down, force a shutdown.
          TCPIPDisconnect(event.modifiers & optionKey, NULL);
        }
        break;

      case StartHTTPMItem:
        StartServer();
        UpdateMenuStatus();
        break;

      case StopHTTPMItem:
        StopServer();
        UpdateMenuStatus();
        break;

      case ResetMItem:
        ResetServer();
        break;
      } // switch((word)event.wmTaskData)
                              
      HiliteMenu(0, ((split)event.wmTaskData).hi);
      EndInfoDrawing();          
      SetMenuBar(NULL);
      break; // case wInMenu

    case updateEvt:
      BeginUpdate(MyWindow);
      DrawWindow();
      EndUpdate(MyWindow);
      break; // case updateEvt
    } // switch(eventCode)
  }  // if (code == eventAction

 return 0;

}
