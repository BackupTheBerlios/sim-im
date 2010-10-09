#ifndef TOOLITEM_H
#define TOOLITEM_H

#include <commands/uicommand.h>

namespace SIM {

class ToolItem
{
public:
    ToolItem(const UiCommandPtr& cmd, QAction* action);

    UiCommandPtr cmd() const;
    QAction* action() const;

private:
    UiCommandPtr m_cmd;
    QAction* m_action;
};

} // namespace SIM

#endif // TOOLITEM_H
