/***************************************************************************
                          replacecfg.cpp  -  description
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

#include "replacecfg.h"

#include "replace.h"

ReplaceCfg::ReplaceCfg(QWidget *parent, ReplacePlugin *plugin)
    : QWidget(parent)
    , m_plugin( plugin )
{
    setupUi(this);
    unsigned uCount = m_plugin->value("Keys").toUInt();
    for (unsigned i = 0; i < uCount; i++){
        QString sKey = m_plugin->value("Key").toStringList().value(i);
        QString sValue = m_plugin->value("Value").toStringList().value(i);
        if (!sKey.isEmpty()) {
            addString( sKey, sValue );
        }
    }
    addString( QString(), QString() );
    connect(lstKeys,SIGNAL(cellChanged(int,int)),SLOT(cellChanged(int,int)));
    autosizeColumns();
}

ReplaceCfg::~ReplaceCfg()
{
}

void ReplaceCfg::apply()
{
    QStringList keys;
    QStringList values;
    for( int row = 0 ; row < lstKeys->rowCount() ; row++ ){
        QString sKey;
        QString sValue;
        if( string( row, sKey, sValue ) && !sKey.isEmpty() ) {
            keys.push_back(sKey);
            values.push_back(sValue);
        }
    }
    m_plugin->setValue("Key", keys);
    m_plugin->setValue("Value", values);
    m_plugin->setValue("Keys", keys.count());
}

bool ReplaceCfg::string( int row, QString &sKey, QString &sValue ) {
    QTableWidgetItem *item = lstKeys->item( row, 0 );
    if( NULL == item )
        sKey.clear();
    else
        sKey = item->data( Qt::DisplayRole ).toString();
    item = lstKeys->item( row, 1 );
    if( NULL == item )
        sValue.clear();
    else
        sValue = item->data( Qt::DisplayRole ).toString();

    return true;
}

void ReplaceCfg::addString( QString sKey, QString sValue ) {
    int row = lstKeys->rowCount();
    lstKeys->insertRow( row );
    if( !sKey.isEmpty() )
        lstKeys->setItem( row, 0, new QTableWidgetItem( sKey ) );
    if( !sValue.isEmpty() )
        lstKeys->setItem( row, 1, new QTableWidgetItem( sValue ) );
}

void ReplaceCfg::cellChanged( int row, int column ) {
    autosizeColumns();
    QString sKey;
    QString sValue;
    if( !string( row, sKey, sValue ) )
        return;
    if( sKey.isEmpty() && sValue.isEmpty() ) {
        lstKeys->removeRow( row );
    }
    if( lstKeys->rowCount() == 0 ) {
        addString( QString(), QString() );
        return;
    }
    if( !string( lstKeys->rowCount()-1, sKey, sValue ) )
        return;
    if( !sKey.isEmpty() || !sValue.isEmpty() ) {
        addString( QString(), QString() );
    }
}

void ReplaceCfg::autosizeColumns() {
    lstKeys->resizeColumnToContents( 0 );
    int width = lstKeys->columnWidth( 0 );
    if( width < 100 ) {
        lstKeys->setColumnWidth( 0, 100 );
    }
}
