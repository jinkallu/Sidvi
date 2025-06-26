#ifndef VIEWSERVER_H
#define VIEWSERVER_H



#include "TServerSocket.h"
#include "TThread.h"
#include "Riostream.h"
#include "TMessage.h"
#include "TMonitor.h"
#include "TH1C.h"
#include "TH1S.h"
#include "TH1I.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TH2C.h"
#include "TH2S.h"
#include "TH2I.h"
#include "TH2F.h"
#include "TH2D.h"
#include "TH3C.h"
#include "TH3S.h"
#include "TH3I.h"
#include "TH3F.h"
#include "TH3D.h"
#include "TClass.h"
#include "TArray.h"
#include "TSQLServer.h"
#include "TSQLStatement.h"
#include "TString.h"
#include "TQObject.h"
#include <RQ_OBJECT.h>


using namespace std;
class viewServer : public TQObject
{
RQ_OBJECT("viewServer")
public:

    bool serverConnected, stopThread;

    TServerSocket *ss;
    TSocket *s0;
    TMessage *mess;
    TSocket  *s;
    TMonitor *mon;

    TThread *threadServer, *threadUpdate;

    char str[64];
    TH1 *h1Hist[100];
    TH2 *h2Hist[100];
    TH3 *h3Hist[100];
    int iTH1, iTH2, iTH3;
    TSQLServer *dbSQL;
    TSQLStatement* stmtSQL;
    TString strCanPrimitive[200];
    TString strHistName[200];
    int numPadCan;

    viewServer();
    //viewServer(void *buf, Int_t len) : TMessage(buf, len) { };
    static void ConnectServerThread(void *arg);
    static void UpdateServerThread(void *arg);
    virtual void ConnectServer();
    virtual void UpdateServer();
    void startConnectServerThread();
    void startUpdateServerThread();
    void stopServerThread();
    void sigFinishedReg();// *SIGNAL*
    void sigSetCanData();// *SIGNAL*
    ClassDef(viewServer, 0);

};
#endif // VIEWSERVER_H
