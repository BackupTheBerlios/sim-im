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

class EXPORT UiCommand : public QAction
{
    Q_OBJECT
public:
    static UiCommandPtr create(const QString& text, const QString& iconId, const QString& id, const QStringList& tags = QStringList(), QObject* parent = NULL);

    void setIconId(const QString& id);
    QString iconId() const;

    QString id() const;

    void addTag(const QString& tag);
    void clearTags();
    QStringList tags() const;

    void addSubCommand(const UiCommandPtr& subcmd);
signals:

public slots:

private:
    explicit UiCommand(const QString& text, const QString& iconId, const QString& id, const QStringList& tags = QStringList(), QObject* parent = NULL);

    QString m_id;
    QStringList m_tags;
    QString m_iconId;
    QList<UiCommandPtr> m_subcmds;
};
}

#endif // UICOMMAND_H
