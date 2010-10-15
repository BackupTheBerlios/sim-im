#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>

#include "commands/uicommand.h"
#include "simapi.h"
#include "toolbaractionfactory.h"
#include "toolitem.h"

namespace SIM {

class EXPORT ToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit ToolBar(const QString& name, QWidget *parent = 0);
    virtual ~ToolBar();

    void setToolbarActionFactory(ToolbarActionFactory* factory);

    void addUiCommand(const UiCommandPtr& cmd);
    void addSeparator();

    void loadCommandList(const QStringList& actions);
    QStringList saveCommandList() const;
signals:

public slots:

private:
    ToolItem* createItem(const UiCommandPtr& cmd, QAction* action);

    QList<ToolItem*> m_items;
    ToolbarActionFactory* m_factory;

};

} // namespace SIM

#endif // TOOLBAR_H
