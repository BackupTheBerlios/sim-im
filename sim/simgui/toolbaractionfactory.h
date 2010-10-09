#ifndef TOOLBARACTIONFACTORY_H
#define TOOLBARACTIONFACTORY_H

#include <QAction>
#include "commands/uicommand.h"
#include "simapi.h"

namespace SIM
{
class ToolbarActionFactory
{
public:
    virtual ~ToolbarActionFactory() {}

    virtual QAction* createAction(const UiCommandPtr& cmd, QWidget* parent) = 0;
    virtual QWidget* createWidget(const UiCommandPtr& cmd, QWidget* parent) = 0;
    virtual QAction* createSeparator(QWidget* parent) = 0;
};
}

#endif // TOOLBARACTIONFACTORY_H
