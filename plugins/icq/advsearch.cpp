/***************************************************************************
                          advsearch.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#include "country.h"
#include "misc.h"

#include "advsearch.h"
#include <QShowEvent>

using namespace SIM;

extern const ext_info *p_genders;
extern const ext_info *p_languages;
extern const ext_info *p_occupations;
extern const ext_info *p_interests;
extern const ext_info *p_pasts;
extern const ext_info *p_affilations;

const ext_info ages[] =
    {
        { "18-22", 1 },
        { "23-29", 2 },
        { "30-39", 3 },
        { "40-49", 4 },
        { "50-59", 5 },
        { "> 60", 6 },
        { "", 0 }
    };

const ext_info *p_ages = ages;

AdvSearch::AdvSearch() : QWidget(NULL)
{
	setupUi(this);
    initCombo(cmbGender, 0, p_genders);
    initCombo(cmbAge, 0, ages);
    initCombo(cmbCountry, 0, getCountries(), true, getCountryCodes());
    initCombo(cmbLang, 0, p_languages);
    initCombo(cmbOccupation, 0, p_occupations);
    initCombo(cmbInterests, 0, p_interests);
    initCombo(cmbPast, 0, p_pasts);
    initCombo(cmbAffilation, 0, p_affilations);
}

void AdvSearch::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    emit enableOptions(false);
}

