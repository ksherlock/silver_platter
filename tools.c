#pragma noroot
#pragma lint - 1
#pragma optimize - 1
#pragma debug 0x8000

// load/unload tools I require.

#include <types.h>

#include <Font.h>
#include <Locator.h>
#include <QDAux.h>
#include <StdFile.h>
#include <TextEdit.h>
#include <Window.h>
#include <memory.h>

#include <TimeTool.h>
#include <tcpip.h>

#include "httpnda.h"

// returns 0 on error.
Word LoadNDATools(Word MyID) {

  if (!QDAuxStatus() || _toolErr) {
    LoadOneTool(0x12, 0);
    if (!_toolErr)
      QDAuxStartUp();
    if (_toolErr) {
      AlertWindow(awCString, NULL,
                  (Ref) "24~Unable to start QuickDraw Aux.~^Too Bad");
      return 0;
    }
    FlagQDAux = true;
  }

  if (!FMStatus() || _toolErr) {
    Handle h;
    LoadOneTool(0x1b, 0);
    if (!_toolErr)
      HandleFM = NewHandle(0x0100, MyID, 0xc005, 0);
    if (!_toolErr)
      FMStartUp(MyID, (Word)*HandleFM);
    if (_toolErr) {
      if (HandleFM)
        DisposeHandle(HandleFM);

      AlertWindow(awCString, NULL,
                  (Ref) "24~Unable to start Font Manager.~^Too Bad");
      return 0;
    }
    FlagFM = true;
  }

  if (!TEStatus() || _toolErr) {
    LoadOneTool(0x22, 0x0);
    if (!_toolErr)
      HandleTE = NewHandle(0x0100, MyID, 0xc005, 0);
    if (!_toolErr)
      TEStartUp(MyID, (Word)*HandleTE);
    if (_toolErr) {
      if (HandleTE)
        DisposeHandle(HandleTE);

      AlertWindow(awCString, NULL,
                  (Ref) "24~Unable to start Text Edit.~^Too Bad");
      return 0;
    }
    FlagTE = true;
  }

  if (!SFStatus() || _toolErr) {
    LoadOneTool(0x17, 0);
    if (!_toolErr)
      HandleSF = NewHandle(0x0100, MyID, 0xc005, 0);
    if (!_toolErr)
      SFStartUp(MyID, (Word)*HandleSF);
    if (_toolErr) {
      if (HandleSF)
        DisposeHandle(HandleSF);

      AlertWindow(awCString, NULL,
                  (Ref) "24~Unable to start Standard Filer.~^Too Bad");
      return 0;
    }
    FlagSF = true;
  }

  if (!tiStatus() || _toolErr) {
    LoadOneTool(0x38, 0x104);
    if (!_toolErr)
      tiStartUp();
    if (_toolErr) {
      AlertWindow(awCString, NULL,
                  (Ref) "24~Unable to start Timezone tool.~^Too Bad");
      return 0;
    }
    FlagTT = true;
  } else if (tiVersion() < 0x0104) {
    AlertWindow(awCString, NULL,
                (Ref) "24~Timezone tool 1.0.4 or newer is required.~^Too Bad");
    return 0;
  }

  if (!TCPIPStatus() || _toolErr) {
    LoadOneTool(0x36, 0x0200);
    if (!_toolErr)
      TCPIPStartUp();
    if (_toolErr) {
      AlertWindow(awCString, NULL,
                  (Ref) "24~Unable to start Marinetti.~^Too Bad");
      return 0;
    }
    FlagTCP = true;
  } else if (TCPIPLongVersion() < 0x03000000) {
    AlertWindow(awCString, NULL,
                (Ref) "24~Marinetti 3.0 or newer is required.~^Too Bad");
    return 0;
  }

  return 1;
}

void UnloadNDATools(void) {
  if (FlagTCP && !TCPIPGetConnectStatus()) {
    TCPIPShutDown();
    UnloadOneTool(0x36);
  }
  if (FlagTT) {
    tiShutDown();
    UnloadOneTool(0x38);
  }
  if (FlagSF) {
    SFShutDown();
    UnloadOneTool(0x17);
    DisposeHandle(HandleSF);
  }
  if (FlagTE) {
    TEShutDown();
    UnloadOneTool(0x22);
    DisposeHandle(HandleTE);
  }
  if (FlagFM) {
    FMShutDown();
    UnloadOneTool(0x1b);
    DisposeHandle(HandleFM);
  }
  if (FlagQDAux) {
    QDAuxShutDown();
    UnloadOneTool(0x12);
  }
}
