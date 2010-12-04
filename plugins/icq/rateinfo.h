#ifndef RATEINFO_H
#define RATEINFO_H

#include <QList>
#include <QSharedPointer>
#include <QDateTime>

class RateInfo
{
public:
    RateInfo(int group);

    int group() const;

    void addSnac(int type, int subtype);
    bool hasSnac(int type, int subtype) const;

    void setWindowSize(int size);
    void setLevels(int clear, int alert, int limit, int disconnect, int max);
    void setCurrentLevel(int level);
    int currentLevel() const;
    void update();

private:
    int m_group;

    int m_windowSize;
    int m_clearLevel;
    int m_alertLevel;
    int m_limitLevel;
    int m_disconnectLevel;
    int m_maxLevel;
    int m_currentLevel;

    QList<int> m_snacs;
    QDateTime m_lastTime;
};

typedef QSharedPointer<RateInfo> RateInfoPtr;

#endif // RATEINFO_H
