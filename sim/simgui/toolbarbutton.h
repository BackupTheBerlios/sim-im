#ifndef TOOLBARBUTTON_H
#define TOOLBARBUTTON_H

#include <QToolButton>
#include "commands/uicommand.h"

namespace SIM {

class ToolbarButton : public QToolButton
{
    Q_OBJECT
public:
    explicit ToolbarButton(const UiCommandPtr& cmd, QWidget *parent = 0);

protected:
    virtual void mouseReleaseEvent(QMouseEvent* e);

private:
    UiCommandPtr m_cmd;

};

} // namespace SIM

#endif // TOOLBARBUTTON_H
