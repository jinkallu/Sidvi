#include "viewserver.h"

viewServer::viewServer()
{
    serverConnected = false;
    stopThread = false;
    iTH1 = iTH2 = iTH3 = 0;
    mon = new TMonitor;
    /*dbSQL = TSQLServer::Connect("mysql://localhost/sidanalysis", "root", "*let*it*go*");
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("sidanalysis");
    db.setUserName("root");
    db.setPassword("*let*it*go*");
    if( !db.open() )
    {
        qDebug() << db.lastError();
        qFatal( "Failed to connect." );
    }
    else
        cout<<"Database opened successfull "<<endl;*/
}

void viewServer::startConnectServerThread(){
    threadUpdate = new TThread((void (*) (void *))&ConnectServerThread, (void*) this);
    threadUpdate->Run();
}

void viewServer::ConnectServerThread(void *arg){
    TThread::SetCancelOn();
    TThread::SetCancelDeferred();
    viewServer* inst = (viewServer*) arg;
    Int_t meid=TThread::SelfId(); // get pthread id
    cout << "\nThread 0, id:" <<meid<< " is running..\n"<<std::endl;
    //while(inst->GetThreadRun()){
    // loop keeps thread alive...
    TThread::CancelPoint();
    inst->ConnectServer(); // call the user defined threaded function
    //}

}

void viewServer::ConnectServer(){
    ss = new TServerSocket(9090, kTRUE);
    // Accept a connection and return a full-duplex communication socket.
    s0 = ss->Accept();
    // tell the clients to start
    s0->Send("go 0");
    // Close the server socket (unless we will use it later to wait for
    // another connection).
    ss->Close();
    // Check some options of socket 0.
    int val;
    s0->GetOption(kSendBuffer, val);
    printf("sendbuffer size: %d\n", val);
    s0->GetOption(kRecvBuffer, val);
    printf("recvbuffer size: %d\n", val);
    // Get the remote addresses (informational only).
    TInetAddress adr = s0->GetInetAddress();
    adr.Print();
    // Create canvas and pads to display the histograms
    mon->Add(s0);
    while (1) {
        TMessage *mess;

        s = mon->Select();
        s->Recv(mess);
        //TString strClassName;
        char strClassName[64];
        if (mess->What() == kMESS_STRING) {
            mess->ReadString(strClassName, 64);
            cout<<"------------"<<strClassName<<endl;
            if(!strcmp(strClassName, "finished")){
                cout<<"------------------*********************Finished"<<endl;
                //mon->Remove(s);
                break;
            }
        } else if (mess->What() == kMESS_OBJECT) {
            //printf("got object of class: %s\n", mess->GetClass()->GetName());
            if(!strcmp(strClassName, "TH1C") ||!strcmp(strClassName, "TH1S") || !strcmp(strClassName, "TH1I") || !strcmp(strClassName, "TH1F") || !strcmp(strClassName, "TH1D")){
                h1Hist[iTH1] = (TH1*)mess->ReadObject(mess->GetClass());
                cout<<h1Hist[iTH1]->GetName() <<endl;
                iTH1++;
            }
            else if(!strcmp(strClassName, "TH2C") ||!strcmp(strClassName, "TH2S") || !strcmp(strClassName, "TH2I") || !strcmp(strClassName, "TH2F") || !strcmp(strClassName, "TH2D")){
                h2Hist[iTH2] = (TH2*)mess->ReadObject(mess->GetClass());
                cout<<h2Hist[iTH2]->GetName() <<endl;
                iTH2++;
            }
            else if(!strcmp(strClassName, "TH3C") ||!strcmp(strClassName, "TH3S") || !strcmp(strClassName, "TH3I") || !strcmp(strClassName, "TH3F") || !strcmp(strClassName, "TH3D")){
                h3Hist[iTH3] = (TH3*)mess->ReadObject(mess->GetClass());
                cout<<h3Hist[iTH3]->GetName() <<endl;
                iTH3++;
            }
        } else {
            printf("*** Unexpected message ***\n");
        }
        delete mess;
        //serverConnected = true;
    }
    printf("Client 0: bytes recv = %d, bytes sent = %d\n", s0->GetBytesRecv(),
           s0->GetBytesSent());
    // Close the socket.
    //s0->Close();
    serverConnected = true;
    sigFinishedReg();
    //TThread::Sleep(15, 0);

    while(1){

        TThread::Sleep(5, 0);
        sigSetCanData();
        for(int ii=0; ii<numPadCan; ii++){
            cout<<"vieserver "<<strCanPrimitive[ii].Data()<<endl;
        s->Send(strCanPrimitive[ii].Data());
        while (1) {
            TMessage *mess;
            //TSocket  *s;
            //s = mon->Select();
            s->Recv(mess);
            //TString strClassName;
            char strClassName[64];
            if (mess->What() == kMESS_STRING) {
                mess->ReadString(strClassName, 64);
                cout<<"------------"<<strClassName<<endl;
                if(!strcmp(strClassName, "finished")){
                    cout<<"------------------*********************Finished"<<endl;
                    //mon->Remove(s);
                    break;
                }
            } else if (mess->What() == kMESS_OBJECT) {
                //printf("got object of class: %s\n", mess->GetClass()->GetName());
                if(!strcmp(strClassName, "TH1C") ||!strcmp(strClassName, "TH1S") || !strcmp(strClassName, "TH1I") || !strcmp(strClassName, "TH1F") || !strcmp(strClassName, "TH1D")){
                    TH1 *h = (TH1*)mess->ReadObject(mess->GetClass());
                    cout<<"------------------reading "<<h->GetName()<<endl;
                    if (h) {
                        for(int i=0; i<iTH1; i++){
                            if(strcmp(h1Hist[i]->GetName(), h->GetName()) == 0){
                                h1Hist[i]->Reset();
                                //TString typeHist = "TH1F";
                                h1Hist[i]= (TH1*)h->Clone();
                                break;
                            }
                        }
                        delete h;       // delete histogram
                    }
                }
                else if(!strcmp(strClassName, "TH2C") ||!strcmp(strClassName, "TH2S") || !strcmp(strClassName, "TH2I") || !strcmp(strClassName, "TH2F") || !strcmp(strClassName, "TH2D")){
                    h2Hist[iTH2] = (TH2*)mess->ReadObject(mess->GetClass());
                    cout<<h2Hist[iTH2]->GetName() <<endl;
                    iTH2++;
                }
                else if(!strcmp(strClassName, "TH3C") ||!strcmp(strClassName, "TH3S") || !strcmp(strClassName, "TH3I") || !strcmp(strClassName, "TH3F") || !strcmp(strClassName, "TH3D")){
                    h3Hist[iTH3] = (TH3*)mess->ReadObject(mess->GetClass());
                    cout<<h3Hist[iTH3]->GetName() <<endl;
                    iTH3++;
                }
            } else {
                printf("*** Unexpected message ***\n");
            }
            delete mess;
            //serverConnected = true;
        }
    }
    }
}
void viewServer::startUpdateServerThread(){
    threadUpdate = new TThread((void (*) (void *))&UpdateServerThread, (void*) this);
    threadUpdate->Run();
}
void viewServer::UpdateServerThread(void *arg){
    //TMonitor *mon1;

    //TSocket *sss = mon->Select();
    TThread::SetCancelOn();
    TThread::SetCancelDeferred();
    viewServer* inst = (viewServer*) arg;
    Int_t meid=TThread::SelfId(); // get pthread id
    cout << "\nThread 0, id:" <<meid<< " Update is running..\n"<<std::endl;
    while(!inst->stopThread){
        TThread::Sleep(5, 0);
        // loop keeps thread alive...
        TThread::CancelPoint();
        //sss->Send("EnergySpectra1");
        inst->UpdateServer(); // call the user defined threaded function
    }
}
void viewServer::UpdateServer(){
    while (1) {
        TMessage *mess;
        //TSocket  *s;
        //s = mon->Select();
        s->Recv(mess);
        //TString strClassName;
        char strClassName[64];
        if (mess->What() == kMESS_STRING) {
            mess->ReadString(strClassName, 64);
            cout<<"------------"<<strClassName<<endl;
            if(!strcmp(strClassName, "finished")){
                cout<<"------------------*********************Finished"<<endl;
                //mon->Remove(s);
                break;
            }
        } else if (mess->What() == kMESS_OBJECT) {
            //printf("got object of class: %s\n", mess->GetClass()->GetName());
            if(strClassName=="TH1C" ||strClassName=="TH1S" || strClassName=="TH1I" || strClassName=="TH1F" || strClassName=="TH1D"){
                TH1 *h = (TH1*)mess->ReadObject(mess->GetClass());
                if (h) {
                    for(int i=0; i<iTH1; i++){
                        if(strcmp(h1Hist[i]->GetName(), h->GetName()) == 0){
                            h1Hist[i]->Reset();
                            //TString typeHist = "TH1F";
                            h1Hist[i]= (TH1*)h->Clone();
                            break;
                        }
                    }
                    delete h;       // delete histogram
                }
            }
            else if(!strcmp(strClassName, "TH2C") ||!strcmp(strClassName, "TH2S") || !strcmp(strClassName, "TH2I") || !strcmp(strClassName, "TH2F") || !strcmp(strClassName, "TH2D")){
                h2Hist[iTH2] = (TH2*)mess->ReadObject(mess->GetClass());
                cout<<h2Hist[iTH2]->GetName() <<endl;
                iTH2++;
            }
            else if(!strcmp(strClassName, "TH3C") ||!strcmp(strClassName, "TH3S") || !strcmp(strClassName, "TH3I") || !strcmp(strClassName, "TH3F") || !strcmp(strClassName, "TH3D")){
                h3Hist[iTH3] = (TH3*)mess->ReadObject(mess->GetClass());
                cout<<h3Hist[iTH3]->GetName() <<endl;
                iTH3++;
            }
        } else {
            printf("*** Unexpected message ***\n");
        }
        delete mess;
        //serverConnected = true;
    }

    /* QSqlQuery query;//(db.database());
    query.prepare("SELECT size, data, class FROM histograms");



    if(!query.exec()) {
        qWarning() << __FUNCTION__ <<":"<<__LINE__<<query.lastError();
    }

    while(query.next()){
        int bufferLength = query.value(0).toInt();
        QByteArray bb = query.value(1).toByteArray() ;
        QString strClassName = query.value(2).toString();
        QDataStream ss(bb);
        ss.setVersion(QDataStream::Qt_5_0);
        char *buf = new char[bufferLength];
        ss.readRawData(buf, bufferLength);


        viewServer *myTM = new viewServer(buf,bufferLength);
        if(strClassName=="TH1C" ||strClassName=="TH1S" || strClassName=="TH1I" || strClassName=="TH1F" || strClassName=="TH1D"){
            TH1 *h = (TH1*)myTM->ReadObject(myTM->GetClass());
            if (h) {
                for(int i=0; i<iTH1; i++){
                    if(strcmp(h1Hist[i]->GetName(), h->GetName()) == 0){
                        h1Hist[i]->Reset();
                        //TString typeHist = "TH1F";
                        h1Hist[i]= (TH1*)h->Clone();
                        break;
                    }
                }
                delete h;       // delete histogram
            }
        }
        else if(strClassName=="TH2C" ||strClassName=="TH2S" || strClassName=="TH2I" || strClassName=="TH2F" || strClassName=="TH2D"){
            TH2 *h = (TH2*)myTM->ReadObject(myTM->GetClass());
            if (h) {
                for(int i=0; i<iTH2; i++){
                    if(strcmp(h2Hist[i]->GetName(), h->GetName()) == 0){
                        h2Hist[i]->Reset();
                        //TString typeHist = "TH1F";
                        h2Hist[i]= (TH2*)h->Clone();
                        break;
                    }
                }
                delete h;       // delete histogram
            }
        }
        else if(strClassName=="TH3C" ||strClassName=="TH3S" || strClassName=="TH3I" || strClassName=="TH3F" || strClassName=="TH3D"){
            TH3 *h = (TH3*)myTM->ReadObject(myTM->GetClass());
            if (h) {
                for(int i=0; i<iTH3; i++){
                    if(strcmp(h3Hist[i]->GetName(), h->GetName()) == 0){
                        h3Hist[i]->Reset();
                        //TString typeHist = "TH1F";
                        h3Hist[i]= (TH3*)h->Clone();
                        break;
                    }
                }
                delete h;       // delete histogram
            }
        }
        serverConnected = true;
        delete myTM;
        bb.clear();
        ss.device()->reset();
    }*/
}

void viewServer::stopServerThread(){
    if(threadServer){
        TThread::Delete(threadServer);
        TThread::Delete(threadUpdate);
        delete threadServer;
        delete threadUpdate;
        threadServer = 0;
        threadUpdate = 0;
    }
}

void viewServer::sigFinishedReg(){
    Emit("sigFinishedReg()");
}
void viewServer::sigSetCanData(){
    Emit("sigSetCanData()");
}
