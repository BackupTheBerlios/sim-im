
#include <QWidget>
#include "kdesimapp.h"
#include "log.h"
#include "misc.h"

using namespace SIM;
SimApp::SimApp(int& argc, char** argv) : QApplication(argc, argv)
{
    firstInstance = true;
}


SimApp::~SimApp()
{
}

int SimApp::newInstance()
{
    if (firstInstance)
	{
        firstInstance = false;
    }
	else
	{
//        QWidgetList list = QApplication::topLevelWidgets();
//        foreach(QWidget* w, list)
//		{
//            if (w->inherits("MainWindow")){
//                raiseWidget(w);
//            }
//        }
    }
    return 0;
}

void SimApp::commitData(QSessionManager&)
{
}

void SimApp::saveState(QSessionManager &sm)
{
    QApplication::saveState(sm);
}

void simMessageOutput( QtMsgType, const char *msg )
{
    if (logEnabled())
        log(L_DEBUG, "QT: %s", msg);
}

// vim: set expandtab:

