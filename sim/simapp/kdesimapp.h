
#ifndef KDESIMAPP_H
#define KDESIMAPP_H

#include <kuniqueapplication.h>
#include <QSessionManager>
#include <QApplication>

#include "simapi.h"

class EXPORT SimApp : public QApplication
{
public:
    SimApp(int& argc, char** argv);
    virtual ~SimApp();
    int newInstance();
    void commitData(QSessionManager&);
    void saveState(QSessionManager&);
protected:
    bool firstInstance;
};

#endif

// vim: set expandtab:

