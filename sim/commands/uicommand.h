#ifndef UICOMMAND_H
#define UICOMMAND_H

#include <QAction>
#include <QString>
#include <QStringList>
#include <QSharedPointer>
#include "simapi.h"

namespace SIM
{

class EXPORT UiCommand;
typedef QSharedPointer<UiCommand> UiCommandPtr;

class EXPORT UiCommand : public QObject
{
    Q_OBJECT
public:
    static UiCommandPtr create(const QString& text, const QString& iconId, const QString& id, const QStringList& tags = QStringList(), QObject* parent = NULL);

    QString text() const;
    QIcon icon() const;

    void setIconId(const QString& id);
    QString iconId() const;

    QString id() const;

    void addTag(const QString& tag);
    void clearTags();
    QStringList tags() const;

    void addSubCommand(const UiCommandPtr& subcmd);
    QList<UiCommandPtr> subCommands() const;

    bool subscribeTo(QObject* obj, const char* slot);
    bool unsubscribe(QObject* obj, const char* slot);

    enum WidgetType
    {
        wtNone = 0,
        wtButton,
        wtEdit,
        wtLabel
    };

    WidgetType widgetType() const;
    void setWidgetType(WidgetType wt);

    bool isCheckable() const;
    void setCheckable(bool b);

    bool isChecked() const;
    void setChecked(bool b);


signals:
    void triggered();
    void checked(bool b);

public slots:
    void trigger();

protected:
    explicit UiCommand(const QString& text, const QString& iconId, const QString& id, const QStringList& tags = QStringList(), QObject* parent = NULL);

private:
    QString m_id;
    QStringList m_tags;
    QString m_iconId;
    QList<UiCommandPtr> m_subcmds;
    QString m_text;
    QIcon m_icon;
    WidgetType m_widgetType;
    bool m_checkable;
    bool m_checked;
};
}

#endif // UICOMMAND_H
