
#include "simapp.h"
#include "cfg.h"
#include "log.h"
#include "misc.h"

SimApp::SimApp(int &argc, char **argv)
      : QApplication(argc, argv)
{

}
SimApp::~SimApp()
{

}

void SimApp::commitData(QSessionManager&)
{
	//SIM::save_state();
}

void SimApp::saveState(QSessionManager &sm)
{
    QApplication::saveState(sm);
}


// vim: set expandtab:

