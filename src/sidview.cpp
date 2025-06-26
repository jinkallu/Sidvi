#include "sidview.h"

const char *dnd_types[] = {
   "ROOT files",    "*.root",
   "ROOT macros",   "*.C",
   "All files",     "*",
    0,               0
};
const char gHelpDND[] = "\
                     Drag & Drop (DnD)\n\
Drag and Drop support is implemented on Linux via Xdnd, the\n\
drag and drop protocol for X window system, and on Windows\n\
via the Clipboard.\n\
Users can selects something in ROOT with a mouse press, drags\n\
it (moves the mouse while keeping the mouse button pressed) and\n\
releases the mouse button somewhere else. When the button is\n\
released the selected data is \"dropped\" at that location. This\n\
way, a histogram from an opened ROOT file in the browser can be\n\
dragged to any TCanvas. A script file from the browser can be\n\
dropped to a TGTextView or TGTextEdit widget in TGTextEditor.\n\
On Linux, it is possible to drag objects between ROOT and an\n\
external application. For example to drag a macro file from the\n\
ROOT browser to the Kate editor. On Windows, drag and drop works\n\
only within a single ROOT application, but it is possible to drag\n\
from the Windows Explorer to ROOT (e.g. a picture file to a canvas\n\
or a text file to a text editor).\n\
";

const char gReadyMsg[] = "Ready. You can drag list tree items to any \
pad in the canvas, or to the \"Base\" folder of the list tree itself...";

static Atom_t gRootObj  = kNone;
enum EMyMessageTypes {
   M_FILE_OPEN,
   M_FILE_SERVER,
   M_FILE_UPDATE,
   M_FILE_BROWSE,
   M_FILE_NEWCANVAS,
   M_FILE_CLOSEWIN,
   M_FILE_EXIT,
   M_HELP_ABOUT
};

bool ser = false;



sidView::sidView(const TGWindow *p, int w, int h) :
    TGMainFrame(p, w, h), fGraph(0), fHist1D(0), fHist2D(0)

 {
    // Constructor.


    iTH1 = 0;
    ServerConn = false;
    serverConnected = false;
    timer = new TTimer(10000);
    timer->Connect("Timeout()", "sidView", this, "RefreshCanvasInf()");

    serv.Connect("sigFinishedReg()", "sidView", this, "slotRegister()");
    serv.Connect("sigSetCanData()", "sidView", this, "slotSetCanData()");
    //Connect(&serv, "sigFinishedReg()", "viewServer", this, "slotRegister()");
    //timer->Start(1000, kFALSE);
    SetCleanup(kDeepCleanup);
    pic = 0;
    fMenuBar = new TGMenuBar(this, 35, 50, kHorizontalFrame);

    fMenuFile = new TGPopupMenu(gClient->GetRoot());
    fMenuFile->AddEntry(" &Server...\tCtrl+S", M_FILE_SERVER, 0,
                        gClient->GetPicture("bld_open.png"));

    fMenuFile->AddEntry(" &Open...\tCtrl+O", M_FILE_OPEN, 0,
                        gClient->GetPicture("bld_open.png"));
    fMenuFile->AddEntry(" &Browse...\tCtrl+B", M_FILE_BROWSE);
    fMenuFile->AddEntry(" &New Canvas\tCtrl+N", M_FILE_NEWCANVAS);
    fMenuFile->AddEntry(" &Close Window\tCtrl+W", M_FILE_CLOSEWIN);
    fMenuFile->AddSeparator();
    fMenuFile->AddEntry(" E&xit\tCtrl+Q", M_FILE_EXIT, 0,
                        gClient->GetPicture("bld_exit.png"));
    fMenuFile->Connect("Activated(Int_t)", "sidView", this,
                       "HandleMenu(Int_t)");
    fMenuUpdate = new TGPopupMenu(gClient->GetRoot());
    fMenuUpdate->AddEntry(" &Update...\tCtrl+U", M_FILE_UPDATE, 0,
                        gClient->GetPicture("bld_open.png"));
    fMenuUpdate->Connect("Activated(Int_t)", "sidView", this,
                       "HandleMenu(Int_t)");


    fMenuHelp = new TGPopupMenu(gClient->GetRoot());
    fMenuHelp->AddEntry(" &About...", M_HELP_ABOUT, 0,
                        gClient->GetPicture("about.xpm"));
    fMenuHelp->Connect("Activated(Int_t)", "sidView", this,
                       "HandleMenu(Int_t)");

    fMenuBar->AddPopup("&File", fMenuFile, new TGLayoutHints(kLHintsTop|kLHintsLeft,
                                                             0, 4, 0, 0));
    fMenuBar->AddPopup("&Update", fMenuUpdate, new TGLayoutHints(kLHintsTop|kLHintsCenterX));

    fMenuBar->AddPopup("&Help", fMenuHelp, new TGLayoutHints(kLHintsTop|kLHintsRight));

    AddFrame(fMenuBar, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 5));

    TGHorizontalFrame *hfrm = new TGHorizontalFrame(this, 10, 10);
    TGCanvas *canvas = new TGCanvas(hfrm, 150, 100);
    fListTree = new TGListTree(canvas, kHorizontalFrame);
    fListTree->Associate(this);
    fEc = new TRootEmbeddedCanvas("glec", hfrm, 550, 350);
    hfrm->AddFrame(canvas, new TGLayoutHints(kLHintsLeft | kLHintsExpandY, 5, 5));
    hfrm->AddFrame(fEc, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    AddFrame(hfrm, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    fEc->SetDNDTarget(kTRUE);
    fCanvas = fEc->GetCanvas();
    fCanvas->Divide(2, 4);
    fCanvas->SetBorderMode(1);
    fBaseLTI = fListTree->AddItem(0, "Base");

    TGHorizontalFrame *hf = new TGHorizontalFrame(this, 10, 10);

    fStatus = new TGLabel(hf, new TGHotString(gReadyMsg));
    fStatus->SetTextJustify(kTextLeft);
    fStatus->SetTextColor(0x0000ff);
    hf->AddFrame(fStatus, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY,
                 10, 10, 10, 10));

    fButtonExit = new TGTextButton(hf, "        &Exit...        ", 3);
    fButtonExit->Resize(fButtonExit->GetDefaultSize());
    fButtonExit->SetToolTipText("Exit Application (ROOT)");
    fButtonExit->Connect("Clicked()" , "TApplication", gApplication,
                         "Terminate()");
    hf->AddFrame(fButtonExit, new TGLayoutHints(kLHintsCenterY | kLHintsRight,
                                                10, 10, 10, 10));

    AddFrame(hf, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 5));

    gRootObj  = gVirtualX->InternAtom("application/root", kFALSE);

    /*TGraph *gr = (TGraph *)GetObject("Graph");
    pic = gClient->GetPicture("f1_t.xpm");
    item = fListTree->AddItem(fBaseLTI, gr->GetName(), gr, pic, pic);
    fListTree->SetToolTipItem(item, "Simple Graph");
    item->SetDNDSource(kTRUE);

    TH1F *hpx = (TH1F *)GetObject("1D Hist");
    pic = gClient->GetPicture("h1_t.xpm");
    item = fListTree->AddItem(fBaseLTI, hpx->GetName(), hpx, pic, pic);
    fListTree->SetToolTipItem(item, "1D Histogram");
    item->SetDNDSource(kTRUE);

    TH2F *h2 = (TH2F *)GetObject("2D Hist");
    pic = gClient->GetPicture("h2_t.xpm");
    item = fListTree->AddItem(fBaseLTI, h2->GetName(), h2, pic, pic);
    fListTree->SetToolTipItem(item, "2D Histogram");
    item->SetDNDSource(kTRUE);

    TH2F *htest = new TH2F("Hello", "hi", 100, 0, 100, 100, 0, 100);//(TH2F *)GetObject("2D Hist");
    pic = gClient->GetPicture("h2_t.xpm");
    item = fListTree->AddItem(fBaseLTI, htest->GetName(), htest, pic, pic);
    fListTree->SetToolTipItem(item, "2D Histogram1");
    item->SetDNDSource(kTRUE);*/

    TString rootsys(gSystem->UnixPathName(gSystem->Getenv("ROOTSYS")));
 #ifdef G__WIN32
    // remove the drive letter (e.g. "C:/") from $ROOTSYS, if any
    if (rootsys[1] == ':' && rootsys[2] == '/')
       rootsys.Remove(0, 3);
 #endif
    TString link = TString::Format("/%s/tutorials/image/rose512.jpg",
                                   rootsys.Data());
    if (!gSystem->AccessPathName(link.Data(), kReadPermission)) {
       TImage *img = TImage::Open(link.Data());
       if (img) {
          // create a 16x16 icon from the original picture
          img->Scale(16, 16);
          pic = gClient->GetPicturePool()->GetPicture("rose512", img->GetPixmap(),
                                                      img->GetMask());
          delete img;
       }
       else pic = gClient->GetPicture("psp_t.xpm");
       link.Prepend("file://");
       TObjString *ostr = new TObjString(link.Data());
       item = fListTree->AddItem(fBaseLTI, "Rose", ostr, pic, pic);
       fListTree->SetToolTipItem(item, link.Data());
       item->SetDNDSource(kTRUE);
    }

    // open the base list tree item and allow to drop into it
    fListTree->OpenItem(fBaseLTI);
    fListTree->GetFirstItem()->SetDNDTarget(kTRUE);

    // connect the DataDropped signal to be able to handle it
    fListTree->Connect("DataDropped(TGListTreeItem*, TDNDData*)", "sidView",
                       this, "DataDropped(TGListTreeItem*,TDNDData*)");

    SetWindowName("SIDVI");
    MapSubwindows();
    Resize(GetDefaultSize());
    Connect("CloseWindow()", "sidView", this, "DoCloseWindow()");
    DontCallClose(); // to avoid double deletions.
 }
//______________________________________________________________________________
sidView::~sidView()
{
   // Destructor. Doesnt't do much here.
}

//______________________________________________________________________________
void sidView::DoCloseWindow()
{
   // Do some cleanup, disconnect signals and then really close the main window.

   if (fGraph) { delete fGraph; fGraph = 0; }
   if (fHist1D) { delete fHist1D; fHist1D = 0; }
   if (fHist2D) { delete fHist2D; fHist2D = 0; }
   fMenuFile->Disconnect("Activated(Int_t)", this, "HandleMenu(Int_t)");
   fMenuHelp->Disconnect("Activated(Int_t)", this, "HandleMenu(Int_t)");
   fButtonExit->Disconnect("Clicked()" , this, "CloseWindow()");
   fListTree->Disconnect("DataDropped(TGListTreeItem*, TDNDData*)", this,
                         "DataDropped(TGListTreeItem*,TDNDData*)");
   delete fListTree;
   CloseWindow();
}

//______________________________________________________________________________
void sidView::DataDropped(TGListTreeItem *, TDNDData *data)
{
   // Handle the drop event in the TGListTree. This will just create a new
   // list tree item and copy the received data into it.

   fStatus->SetTextColor(0xff0000);
   fStatus->ChangeText("I received data!!!");
   if (data) {
      const TGPicture *pic = 0;
      TGListTreeItem *itm = 0;
      char tmp[1000];
      if (data->fDataType == gRootObj) {
         TBufferFile buf(TBuffer::kRead, data->fDataLength, (void *)data->fData);
         buf.SetReadMode();
         TObject *obj = (TObject *)buf.ReadObjectAny(TObject::Class());
         sprintf(tmp, "Received DND data : Type = \"%s\"; Length = %d bytes;",
                 obj->ClassName(), data->fDataLength);
         if (obj->InheritsFrom("TGraph"))
            pic = gClient->GetPicture("f1_t.xpm");
         else if (obj->InheritsFrom("TH2"))
            pic = gClient->GetPicture("h2_t.xpm");
         else if (obj->InheritsFrom("TH1"))
            pic = gClient->GetPicture("h1_t.xpm");
         //itm = fListTree->AddItem(fBaseLTI, obj->GetName(), obj, pic, pic);
         //fListTree->SetToolTipItem(itm, obj->GetName());
      }
      else {
         sprintf(tmp, "Received DND data: \"%s\"", (char *)data->fData);
         TObjString *ostr = new TObjString((char *)data->fData);
         TImage *img1 = TImage::Open("doc_t.xpm");
         TImage *img2 = TImage::Open("slink_t.xpm");
         if (img1 && img2) {
            img1->Merge(img2);
            pic = gClient->GetPicturePool()->GetPicture("doc_lnk", img1->GetPixmap(),
                                                        img1->GetMask());
            delete img2;
            delete img1;
         }
         else pic = gClient->GetPicture("doc_t.xpm");
         //itm = fListTree->AddItem(fBaseLTI, "Link...", ostr, pic, pic);
         //fListTree->SetToolTipItem(itm, (const char *)data->fData);
      }
      //if (itm) itm->SetDNDSource(kTRUE);
      fStatus->ChangeText(tmp);
   }
   TTimer::SingleShot(3000, "sidView", this, "ResetStatus()");
}

//______________________________________________________________________________
TObject *sidView::GetObject(const char *obj)
{
   // Return the object specified in argument. If the object doesn't exist yet,
   // it is firt created.

   if (!strcmp(obj, "Graph")) {
      if (fGraph == 0) {
         const Int_t n = 20;
         Double_t x[n], y[n];
         for (Int_t i=0;i<n;i++) {
           x[i] = i*0.1;
           y[i] = 10*sin(x[i]+0.2);
         }
         fGraph = new TGraph(n, x, y);
      }
      return fGraph;
   }
   else if (!strcmp(obj, "1D Hist")) {
      if (fHist1D == 0) {
         fHist1D = new TH1F("1D Hist","This is the px distribution",100,-4,4);
         Float_t px, py;
         for ( Int_t i=0; i<10000; i++) {
            gRandom->Rannor(px, py);
            fHist1D->Fill(px);
         }
      }
      return fHist1D;
   }
   else if (!strcmp(obj, "2D Hist")) {
      if (fHist2D == 0) {
         Double_t params[] = {
            130,-1.4,1.8,1.5,1, 150,2,0.5,-2,0.5, 3600,-2,0.7,-3,0.3
         };
         TF2 *f2 = new TF2("f2","xygaus + xygaus(5) + xylandau(10)",
                           -4, 4, -4, 4);
         f2->SetParameters(params);
         fHist2D = new TH2F("2D Hist","xygaus+xygaus(5)+xylandau(10)",
                            20, -4, 4, 20, -4, 4);
         fHist2D->FillRandom("f2",40000);
      }
      return fHist2D;
   }
   return 0;
}

//______________________________________________________________________________
void sidView::HandleMenu(Int_t menu_id)
{
    // Handle menu events.

    TRootHelpDialog *hd;
    static TString dir(".");
    TGFileInfo fi;
    fi.fFileTypes = dnd_types;
    fi.fIniDir    = StrDup(dir);

    switch (menu_id) {
    case M_FILE_EXIT:
        // close the window and quit application
        DoCloseWindow();
        gApplication->Terminate(0);
        break;
    case M_FILE_SERVER:
        // Open server
        openServer();
        break;
    case M_FILE_UPDATE:
        // Open server
        Update();
        break;
    case M_FILE_OPEN:
        new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);
        dir = fi.fIniDir;
        // doesn't do much, but can be used to open a root file...
        break;
    case M_FILE_BROWSE:
        // start a root object browser
        new TBrowser();
        break;
    case M_FILE_NEWCANVAS:
        // open a root canvas
        gROOT->MakeDefCanvas();
        break;
    case M_FILE_CLOSEWIN:
        DoCloseWindow();
        break;
    case M_HELP_ABOUT:
        hd = new TRootHelpDialog(this, "About Drag and Drop...", 550, 250);
        hd->SetText(gHelpDND);
        hd->Popup();
        break;
    }
}

//______________________________________________________________________________
void sidView::ResetStatus()
{
   // Restore the original text of the status label and its original color.

   fStatus->SetTextColor(0x0000ff);
   fStatus->ChangeText(gReadyMsg);
}

void sidView::addHist(){

}

void sidView::openServer(){
    //threadServer = new TThread("Server",(void(*) (void *))serv.ConnectServer, (void*) this);
    //threadServer->Run();
    //serv.serverConnected = false;
    if(!serv.serverConnected){
        serv.startConnectServerThread();
        //gSystem->Sleep(15000);
        //while(!serv.serverConnected){}

    }
    else{
        std::cout<<"Server is already running";
    }

}

void sidView::Update(){
    cout<<"Updating "<<endl;
        /*for(int i=0; i<serv.iTH1; i++){
            if(fCanvas->cd(i+1)->GetPrimitive(serv.h1Hist[i]->GetName()) ){
                serv.h1Hist[i]->DrawCopy();
                fCanvas->Modified();
                fCanvas->Update();
            }
        }*/

        timer->TurnOn();
}

void sidView::RefreshCanvasInf(){
    if(serv.serverConnected){

        for(int i=0; i<serv.iTH1; i++){
            for(int cdCan=1; cdCan<9; cdCan++){
                if(fCanvas->cd(cdCan)->GetPrimitive(serv.h1Hist[i]->GetName()) ){
                    fCanvas->cd(i+1);
                    cout<<"update name "<<serv.h1Hist[i]->GetName()<<endl;
                    serv.h1Hist[i]->Draw();
                    fCanvas->Modified();
                    fCanvas->Update();
                    cout<<"Updating canvas"<<endl;
                }
            }
        }
        for(int i=0; i<serv.iTH2; i++){
            for(int cdCan=1; cdCan<9; cdCan++){
                if(fCanvas->cd(cdCan)->GetPrimitive(serv.h2Hist[i]->GetName()) ){
                    fCanvas->cd(i+1);
                    serv.h2Hist[i]->Draw();
                    fCanvas->Modified();
                    fCanvas->Update();
                    cout<<"Updating canvas"<<endl;
                }
            }
        }
        for(int i=0; i<serv.iTH3; i++){
            for(int cdCan=1; cdCan<9; cdCan++){
                if(fCanvas->cd(cdCan)->GetPrimitive(serv.h3Hist[i]->GetName()) ){
                    fCanvas->cd(i+1);
                    serv.h3Hist[i]->Draw();
                    fCanvas->Modified();
                    fCanvas->Update();
                    cout<<"Updating canvas"<<endl;
                }
            }
        }
    }
}

void sidView::slotRegister(){
    cout<<"-------------------- signal emitted"<<endl;
    if(serv.serverConnected){
        std::cout<<"Server Connected";
        //gSystem->Sleep(2000);
        for(int i=0; i<serv.iTH1; i++){
            pic = gClient->GetPicture("h1_t.xpm");
            item = fListTree->AddItem(fBaseLTI, serv.h1Hist[i]->GetName() , serv.h1Hist[i], pic, pic);
            fListTree->SetToolTipItem(item, "1D Histogram");
            item->SetDNDSource(kTRUE);
        }
        for(int i=0; i<serv.iTH2; i++){
            pic = gClient->GetPicture("h1_t.xpm");
            item = fListTree->AddItem(fBaseLTI, serv.h2Hist[i]->GetName() , serv.h2Hist[i], pic, pic);
            fListTree->SetToolTipItem(item, "2D Histogram");
            item->SetDNDSource(kTRUE);
        }
        for(int i=0; i<serv.iTH3; i++){
            pic = gClient->GetPicture("h1_t.xpm");
            item = fListTree->AddItem(fBaseLTI, serv.h3Hist[i]->GetName() , serv.h3Hist[i], pic, pic);
            fListTree->SetToolTipItem(item, "3D Histogram");
            item->SetDNDSource(kTRUE);
        }
        //serv.startUpdateServerThread();

        //serv.serverConnected = false;
    }
    else{
        std::cout<<"Server is not running";
        serv.stopServerThread();
    }
}

void sidView::slotSetCanData(){
    cout<<"-------------------- signal emitted setCanData"<<serv.h1Hist[0]->GetName()<<endl;
    serv.numPadCan = 0;
    for(int i=0; i<serv.iTH1; i++){
        for(int cdCan=1; cdCan<9; cdCan++){
            if(fCanvas->cd(cdCan)->GetPrimitive(serv.h1Hist[i]->GetName()) ){
                serv.strCanPrimitive[serv.numPadCan] = serv.h1Hist[i]->GetName();
                cout<<"sid view "<<serv.strCanPrimitive[serv.numPadCan].Data()<<endl;
                serv.numPadCan ++;
            }
        }
    }
    for(int i=0; i<serv.iTH2; i++){
        for(int cdCan=1; cdCan<9; cdCan++){
            if(fCanvas->cd(cdCan)->GetPrimitive(serv.h2Hist[i]->GetName()) ){
                serv.strCanPrimitive[serv.numPadCan] = serv.h2Hist[i]->GetName();
                serv.numPadCan ++;
            }
        }
    }
    for(int i=0; i<serv.iTH3; i++){
        for(int cdCan=1; cdCan<9; cdCan++){
            if(fCanvas->cd(cdCan)->GetPrimitive(serv.h3Hist[i]->GetName()) ){
                serv.strCanPrimitive[serv.numPadCan] = serv.h3Hist[i]->GetName();
                serv.numPadCan ++;
            }
        }
    }
}
