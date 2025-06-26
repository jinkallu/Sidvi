#include <iostream>
#include <TApplication.h>
#include <TGClient.h>
#include "sidview.h"

using namespace std;

int main(int argc, char **argv)
{
    TApplication theApp("App", &argc, argv);
    sidView *view = new sidView(gClient->GetRoot(), 700, 400);
    view->addHist();
    view->MapWindow();
    theApp.Run();
    return 0;
}

