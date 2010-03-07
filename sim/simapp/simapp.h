
#ifndef SIMAPP_H
#define SIMAPP_H

#include "cfg.h"

#include <QApplication>
#include <QSessionManager>
#include "simapi.h"

class EXPORT SimApp : public QApplication
{
public:
    SimApp(int &argc, char **argv);
    ~SimApp();
protected:
    void commitData(QSessionManager&);
    void saveState(QSessionManager&);
};

#endif

// vim: set expandtab:

