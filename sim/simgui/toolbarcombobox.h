#ifndef TOOLBARCOMBOBOX_H
#define TOOLBARCOMBOBOX_H

#include <QPushButton>

#include "commands/uicommand.h"

class QAction;
namespace SIM {

class ToolbarComboBox : public QPushButton
{
    Q_OBJECT
public:
    explicit ToolbarComboBox(const UiCommandPtr& cmd, QWidget *parent = 0);

protected:
    virtual void mouseReleaseEvent(QMouseEvent* e);

public slots:
    void actionSelected();

private:
    UiCommandPtr m_cmd;

};

} // namespace SIM

#endif // TOOLBARCOMBOBOX_H
