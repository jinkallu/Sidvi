#ifndef SIDVIEW_H
#define SIDVIEW_H

#include "TROOT.h"
#include "TApplication.h"
#include "TSystem.h"
#include "TGFrame.h"
#include "TGButton.h"
#include "TGLabel.h"
#include "TGMenu.h"
#include "TGFileDialog.h"
#include "TBrowser.h"
#include "TRootEmbeddedCanvas.h"
#include "TRootHelpDialog.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF2.h"
#include "TGraph.h"
#include "TImage.h"
#include "TRandom.h"
#include "TGMsgBox.h"
#include "TGPicture.h"
#include "TGListTree.h"
#include "TObjString.h"
#include "TMessage.h"
#include "TTimer.h"
#include "TGDNDManager.h"
#include <cmath>
//#include "Rtypes.h"
#include "TServerSocket.h"
#include "TMonitor.h"
#include "TMessage.h"
#include "TH1.h"
#include "TString.h"
#include "TClass.h"
#include "TThread.h"

#include "viewserver.h"
#include "TTimer.h"

class viewServer;
class sidView : public TGMainFrame
{
protected:
    TRootEmbeddedCanvas  *fEc;          // embedded canvas
    TGTextButton         *fButtonExit;  // "Exit" text button
    TGMenuBar            *fMenuBar;     // main menu bar
    TGPopupMenu          *fMenuFile;    // "File" popup menu entry
    TGPopupMenu          *fMenuUpdate;    // "Help" popup menu entry
    TGPopupMenu          *fMenuHelp;    // "Help" popup menu entry
    TCanvas              *fCanvas;      // canvas
    TGListTree           *fListTree;    // left list tree
    TGListTreeItem       *fBaseLTI;     // base (root) list tree item
    TGLabel              *fStatus;      // label used to display status
    TGraph               *fGraph;       // TGraph object
    TH1F                 *fHist1D;      // 1D histogram
    TH2F                 *fHist2D;      // 2D histogram

    TH1F *hWire[100];
    int iTH1;

    TServerSocket *ss;
    TSocket *s0;
    TMessage *mess;
    TSocket  *s;
    TMonitor *mon;

    TThread *threadServer;

    viewServer serv;
    TTimer *timer;

    bool ServerConn, serverConnected;

    TString strHist[50];
    char str[64];

public:
    sidView(const TGWindow *p, int w, int h);
    virtual ~sidView();
    // Generate linkdef
    // rootcint -f sidviewDict.cpp -c sidview.h sidviewLinkDef.h
    //rootcint -f viewserverDict.cpp -c viewserver.h viewserverLinkDef.h
    ClassDef(sidView, 1);

    void DoCloseWindow();
    void HandleMenu(Int_t);
    TObject  *GetObject(const char *obj);
    void DataDropped(TGListTreeItem* item, TDNDData* data);
    void ResetStatus();
    void addHist();
    void openServer();

    void Update();
    void RefreshCanvasInf();
    void slotRegister();
    void slotSetCanData();

    const TGPicture *pic;
    TGListTreeItem *item;

};

#endif // SIDVIEW_H
