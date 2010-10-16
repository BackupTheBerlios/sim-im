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

class EXPORT CommandSet;
typedef QSharedPointer<CommandSet> CommandSetPtr;

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

//    void addSubCommand(const UiCommandPtr& subcmd);
//    QList<UiCommandPtr> subCommands() const;
//    void clearSubcommands();
//    void setParentCommand(UiCommand* cmd);

    void setSubcommands(const CommandSetPtr& cmdset);
    CommandSetPtr subcommands() const;

    bool subscribeTo(QObject* obj, const char* slot);
    bool unsubscribe(QObject* obj, const char* slot);

    enum WidgetType
    {
        wtNone = 0,
        wtButton,
        wtCombobox,
        wtEdit,
        wtLabel
    };

    WidgetType widgetType() const;
    void setWidgetType(WidgetType wt);

    bool isCheckable() const;
    void setCheckable(bool b);

    bool isChecked() const;


    bool isAutoExclusive() const;
    void setAutoExclusive(bool ae);


signals:
    void triggered();
    void checked(bool b);
    void subcommandsRemoved();

public slots:
    void trigger();
    void setChecked(bool b);

protected:
    explicit UiCommand(const QString& text, const QString& iconId, const QString& id, const QStringList& tags = QStringList(), QObject* parent = NULL);

private:
    QString m_id;
    QStringList m_tags;
    QString m_iconId;
    CommandSetPtr m_subcommands;
    UiCommand* m_parent;
    QString m_text;
    QIcon m_icon;
    WidgetType m_widgetType;
    bool m_checkable;
    bool m_checked;
    bool m_autoExclusive;
};
}

#endif // UICOMMAND_H
