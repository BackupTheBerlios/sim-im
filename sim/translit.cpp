/***************************************************************************
                          translit.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "simapi.h"
#include <QString>

namespace SIM
{
#include "translit.h"
#include "cjk_variants.h"
#include "johab_hangul.h"

EXPORT QString toTranslit(const QString &str)
{
    QString res;
    for(int i = 0; i < str.length(); i++)
	{
        unsigned short wc = str[(int)i].unicode();
        res += johab_hangul_decompose(wc);
        int indx = -1;
        if (wc == 0x3006){
            indx = 0;
        }else if (wc == 0x30f6){
            indx = 1;
        }else if (wc >= 0x4e00 && wc < 0xa000){
            indx = cjk_variants_indx[wc-0x4e00];
            if (indx >= 0) {
                for (;; indx++) {
                    unsigned short variant = cjk_variants[indx];
                    variant &= 0x7fff;
                    variant += 0x3000;
                    res += QChar(variant);
                    res += QChar(0x303e);
                }
            }
        }else if (wc >= 0x2018 && wc <= 0x201a){
            res += QChar(wc == 0x201a ? 0x2018 : wc);
        }else{
            int indx = translit_index(wc);
            if (indx >= 0) {
                const unsigned short *cp = &translit_data[indx];
                unsigned short num = *cp++;
                for (unsigned i = 0; i < num; i++) {
                    res += QChar(cp[i]);
                }
            }else{
                res += QChar(wc);
            }
        }
    }
    return res;
}

}
