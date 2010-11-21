#include "standardimagestorage.h"

#include <algorithm>
#include <cstdio>

#include "jispiconset.h"
#include "log.h"
#include "cfg.h"
#include "paths.h"

namespace SIM
{
    static QImage merge(const QImage &p1, const QImage &p2)
    {
        QImage img1 = (p1.depth() != 32) ? p1.convertToFormat(QImage::Format_ARGB32) : p1.copy();
        QImage img2 = (p2.depth() != 32) ? p2.convertToFormat(QImage::Format_ARGB32) : p2.copy();

        unsigned int *data1 = (unsigned int*)img1.bits();
        unsigned int *data2 = (unsigned int*)img2.bits();
        for (int y1 = 0; y1 < img1.height(); y1++){
            int y2 = y1 - (img1.height() - img2.height()) / 2;
            if ((y2 < 0) || (y2 >= img2.height()))
                continue;
            unsigned int *line1 = data1 + y1 * img1.width();
            unsigned int *line2 = data2 + y2 * img2.width();
            int w = img1.width();
            if (w < img2.width()){
                line2 += (img2.width() - w) / 2;
            }else if (w > img2.width()){
                w = img2.width();
                line1 += (img1.width() - w) / 2;
            }
            for (int i = 0; i < w; i++, line1++, line2++){
                int r1 = qRed(*line1);
                int g1 = qGreen(*line1);
                int b1 = qBlue(*line1);
                int a1 = qAlpha(*line1);
                int r2 = qRed(*line2);
                int g2 = qGreen(*line2);
                int b2 = qBlue(*line2);
                int a2 = qAlpha(*line2);
                r1 = (r2 * a2 + r1 * (255 - a2)) / 255;
                g1 = (g2 * a2 + g1 * (255 - a2)) / 255;
                b1 = (b2 * a2 + b1 * (255 - a2)) / 255;
                if (a2 > a1)
                    a1 = a2;
                *line1 = qRgba(r1, g1, b1, a1);
            }
        }
        return img1;
    }

    StandardImageStorage::StandardImageStorage()
    {
        log(L_DEBUG, "ImageStorage::ImageStorage()");
        foreach(const QString& s, QStringList() <<
                "icons/gpl.jisp" << "icons/smiles.jisp" <<
                "icons/icqlite.jisp" << "icons/additional.jisp") {
            JispIconSet* set = new JispIconSet();
            if(!set->load(SIM::PathManager::appFile(s)))
            {
                printf("Unable to load default icon set: %s\n", qPrintable(s));
                delete set;
            }
            else
            {
                addIconSet(set);
            }
        }
    }

    StandardImageStorage::~StandardImageStorage()
    {
        foreach(IconSet* set, m_sets) {
            delete set;
        }
    }

    QIcon StandardImageStorage::icon(const QString& id)
    {
        return QIcon(pixmap(id));
    }

    QImage StandardImageStorage::image(const QString& id)
    {
        return pixmap(id).toImage();
    }

    QPixmap StandardImageStorage::pixmap(const QString& id)
    {
        //log(L_DEBUG, "pixmap request: %s", qPrintable(id));
        foreach(IconSet* set, m_sets) {
            if(set->hasIcon(id)) {
                return set->pixmap(id);
            }
        }
        int slashIndex = id.indexOf('_');
        if((slashIndex > 0) && (slashIndex != id.length() - 1))
        {
            QString id1 = id.left(slashIndex);
            QString id2 = id.mid(slashIndex + 1);
            QImage base = image(id1);
            QImage overlay = image(id2);
            //log(L_DEBUG, "pixmap subrequest: %s/%s", qPrintable(id1), qPrintable(id2));
            return QPixmap::fromImage(merge(base, overlay));
        }
        return QPixmap();
    }

    bool StandardImageStorage::addIconSet(IconSet* set)
    {
        // TODO check for double adding
        m_sets.append(set);
        return true;
    }

    bool StandardImageStorage::removeIconset(const QString& id)
    {
        for(QList<IconSet*>::iterator it = m_sets.begin(); it != m_sets.end(); ++it) {
            if((*it)->id() == id) {
                m_sets.erase(it);
                return true;
            }
        }
        return false;
    }

    QString StandardImageStorage::parseSmiles(const QString& input)
    {
        QString result = input;
        foreach(IconSet* set, m_sets) {
            result = set->parseSmiles(result);
        }
        return result;
    }

}
