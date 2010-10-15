#include <QMenu>
#include "toolbar.h"
#include "commands/commandhub.h"
#include "standardtoolbaractionfactory.h"
#include "log.h"

namespace SIM {

ToolBar::ToolBar(const QString& name, QWidget *parent) :
    QToolBar(name, parent)
{
    m_factory = new StandardToolbarActionFactory();
}

ToolBar::~ToolBar()
{
    if(m_factory)
        delete m_factory;
    qDeleteAll(m_items);
}

void ToolBar::addUiCommand(const UiCommandPtr& cmd)
{
    if(!cmd)
        return;
    if(m_factory)
    {
        QAction* action = addWidget(m_factory->createWidget(cmd, this));
        m_items.append(createItem(cmd, action));
    }
}

ToolItem* ToolBar::createItem(const UiCommandPtr& cmd, QAction* action)
{
    return new ToolItem(cmd, action);
}

void ToolBar::addSeparator()
{
    if(m_factory)
    {
        QAction* sep = m_factory->createSeparator(this);
        m_items.append(createItem(UiCommand::create(QString(), QString(), "__separator"), sep));
        addAction(sep);
    }
}

void ToolBar::setToolbarActionFactory(ToolbarActionFactory* factory)
{
    if(m_factory)
        delete m_factory;
    m_factory = factory;
}

void ToolBar::loadCommandList(const QStringList& actions)
{

}

QStringList ToolBar::saveCommandList() const
{
    QStringList result;
    return result;
}

} // namespace SIM
