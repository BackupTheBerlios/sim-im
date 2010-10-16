#ifndef STANDARDTOOLBARACTIONFACTORY_H
#define STANDARDTOOLBARACTIONFACTORY_H

#include "toolbaractionfactory.h"
#include "simapi.h"

namespace SIM {

class EXPORT StandardToolbarActionFactory : public ToolbarActionFactory
{
public:
    virtual QAction* createAction(const UiCommandPtr& cmd, QWidget* parent);
    virtual QWidget* createWidget(const UiCommandPtr& cmd, QWidget* parent);
    virtual QAction* createSeparator(QWidget* parent);

private:
    QWidget* createButton(const UiCommandPtr& cmd, QWidget* parent);
    QWidget* createCombobox(const UiCommandPtr& cmd, QWidget* parent);
    QMenu* createMenuWithCommands(const QList<UiCommandPtr>& cmds, QWidget* parent);
};

} // namespace SIM

#endif // STANDARDTOOLBARACTIONFACTORY_H
