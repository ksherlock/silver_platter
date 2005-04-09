// rez gui defintions

// windows / controls
#define rHTTPWindow	0x1000
#define rConfigWindow	0x2000
#define rStatWindow	0x3000
#define rAboutWindow	0x4000

#define HTTPWindowHeight	130
#define HTTPWindowWidth		280


#define CtrlTE		rHTTPWindow + 1
#define CtrlTherm	rHTTPWindow + 2
#define CtrlCount	rHTTPWindow + 3

#define CtrlPathStat	rConfigWindow + 1
#define CtrlPortStat	rConfigWindow + 2
#define CtrlPath	rConfigWindow + 3
#define CtrlBrowse	rConfigWindow + 4
#define CtrlCancel	rConfigWindow + 5
#define CtrlOk		rConfigWindow + 6
#define CtrlDir		rConfigWindow + 7
#define CtrlPort	rConfigWindow + 8
#define CtrlJail	rConfigWindow + 9
#define CtrlAbort	rConfigWindow + 10
#define CtrlTeach	rConfigWindow + 11

#define CtrlAbout	rAboutWindow + 1

// menus

#define AppleMenu	0x0100
#define FileMenu	0x0200
#define EditMenu	0x0300
#define ServerMenu	0x0400

#define AboutMItem	AppleMenu + 1
#define ConfigMItem	AppleMenu + 2
#define StatsMItem	AppleMenu + 3

#define SaveMItem	FileMenu + 1
#define EmptyMItem	FileMenu + 2

#define StartMMItem	ServerMenu + 1
#define StopMMItem	ServerMenu + 2
#define StartHTTPMItem	ServerMenu + 3
#define StopHTTPMItem	ServerMenu + 4
#define ResetMItem	ServerMenu + 5

#define CutMItem	EditMenu + 1
#define CopyMItem	EditMenu + 2
#define PasteMItem	EditMenu + 3
#define ClearMItem	EditMenu + 4
#define SelectAllMItem	EditMenu + 5
