// rez gui defintions

// windows / controls
#define rHTTPWindow 0x1000
#define rConfigWindow 0x2000
#define rStatWindow 0x3000
#define rAboutWindow 0x4000

#define HTTPWindowHeight 130
#define HTTPWindowWidth 280

#define CtrlRect rHTTPWindow + 100

#define CtrlTE rHTTPWindow + 1
#define CtrlTherm rHTTPWindow + 2
#define CtrlCount rHTTPWindow + 3

//--

#define CtrlPopUp rConfigWindow + 101
#define Ctrl_PU_1 rConfigWindow + 102
#define Ctrl_PU_2 rConfigWindow + 103
#define Ctrl_PU_3 rConfigWindow + 104
#define Ctrl_PU_4 rConfigWindow + 105
#define Ctrl_PU_5 rConfigWindow + 106
#define Ctrl_PU_6 rConfigWindow + 107

#define CtrlPathStat rConfigWindow + 1
#define CtrlPortStat rConfigWindow + 2
#define CtrlHomeStat rConfigWindow + 3
#define CtrlHomeBrowse rConfigWindow + 4
#define CtrlCancel rConfigWindow + 5
#define CtrlOk rConfigWindow + 6
#define CtrlDir rConfigWindow + 7
#define CtrlPort rConfigWindow + 8
#define CtrlJail rConfigWindow + 9
#define CtrlAbort rConfigWindow + 10
#define CtrlTeach rConfigWindow + 11

#define CtrlLogBrowse rConfigWindow + 12
#define CtrlLogStat rConfigWindow + 13
#define CtrlLog rConfigWindow + 14

#define CtrlPut rConfigWindow + 15
#define CtrlPutMkdir rConfigWindow + 16
#define CtrlPutOverwrite rConfigWindow + 17

#define Ctrl_AS_Stat rConfigWindow + 18
#define Ctrl_AD_Stat rConfigWindow + 19
#define Ctrl_MB_Stat rConfigWindow + 20
#define Ctrl_AS rConfigWindow + 21
#define Ctrl_AD rConfigWindow + 22
#define Ctrl_MB rConfigWindow + 23

#define EncapMenu rConfigWindow + 24
#define EncapNever rConfigWindow + 25
#define EncapAlways rConfigWindow + 26
#define EncapForked rConfigWindow + 27

#define CtrlDirHidden rConfigWindow + 28
#define CtrlDirAppleShare rConfigWindow + 29
#define CtrlDirRemovable rConfigWindow + 30

#define CtrlWebDAV rConfigWindow + 31


#define CtrlAbout rAboutWindow + 1

// menus

#define AppleMenu 0x0100
#define FileMenu 0x0200
#define EditMenu 0x0300
#define ServerMenu 0x0400

#define AboutMItem AppleMenu + 1
#define ConfigMItem AppleMenu + 2
#define StatsMItem AppleMenu + 3

#define SaveMItem FileMenu + 1
#define EmptyMItem FileMenu + 2

#define StartMMItem ServerMenu + 1
#define StopMMItem ServerMenu + 2
#define StartHTTPMItem ServerMenu + 3
#define StopHTTPMItem ServerMenu + 4
#define ResetMItem ServerMenu + 5

#define CutMItem EditMenu + 1
#define CopyMItem EditMenu + 2
#define PasteMItem EditMenu + 3
#define ClearMItem EditMenu + 4
#define SelectAllMItem EditMenu + 5
