#include "rateinfo.h"

RateInfo::RateInfo(int group) : m_group(group)
{
}

int RateInfo::group() const
{
    return m_group;
}

void RateInfo::addSnac(int type, int subtype)
{
    if(hasSnac(type, subtype))
        return;
    int id = type | (subtype << 16);
    m_snacs.append(id);
}

bool RateInfo::hasSnac(int type, int subtype) const
{
    int id = type | (subtype << 16);
    return m_snacs.contains(id);
}

void RateInfo::setWindowSize(int size)
{
    m_windowSize = size;
}

void RateInfo::setLevels(int clear, int alert, int limit, int disconnect, int max)
{
    m_clearLevel = clear;
    m_alertLevel = alert;
    m_limitLevel = limit;
    m_disconnectLevel = disconnect;
    m_maxLevel = max;
}

void RateInfo::setCurrentLevel(int level)
{
    m_currentLevel = level;
}

int RateInfo::currentLevel() const
{
    return m_currentLevel;
}

void RateInfo::update()
{
    QDateTime now = QDateTime::currentDateTime();
    if(!m_lastTime.isValid())
        m_lastTime = now;
    else
    {
        int diff = m_lastTime.msecsTo(now);
        m_currentLevel = (float)(m_windowSize - 1) / m_windowSize * m_currentLevel + 1. / m_windowSize * diff;
        if(m_currentLevel > m_maxLevel)
            m_currentLevel = m_maxLevel;
    }
}
