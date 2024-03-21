/*

Jane

begin                : 28 Jan 2024
copyright            : (C) Kartik Patel
email                : letapk@gmail.com

*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *

*/

//Last modified 28 Jan 2024

#include "jane.h"

//userpath contains the path to the data subdirectory
extern QString userpath;

void MainWindow::writeprefs()
{
    QSettings settings(tr("jane"), tr("jane"));

    settings.setValue(tr("pos"), pos());//window position

    settings.setValue(tr("size"), size());//window size

    settings.setValue(tr("Font"), QString(curfont.toString()));//selected font

    settings.setValue(tr("Defdatadir"), Datadirectory);
}

void MainWindow::readprefs()
{
//int i;
QString s, s1;
QFont f;

    QSettings settings(tr("jane"), tr("jane"));

    QPoint pos = settings.value(tr("pos"), QPoint(20, 20)).toPoint();

    QSize size = settings.value(tr("size"), QSize(800, 630)).toSize();

    f = QApplication::font();
    s1 = f.toString();
    s = settings.value(tr("Font"), QString(s1)).toString();
    curfont.fromString(s);
    QApplication::setFont(curfont);

    s = settings.value(tr("Defdatadir"), userpath).toString();

    resize(size);
    move(pos);
}

void MainWindow::select_font()
{
bool ok;
QFont f;

    f = QFontDialog::getFont(&ok, curfont, this);
    if (ok == true) {
        //set the user selected font everywhere
        QApplication::setFont(f);
        curfont = f;
    } else {
        return;
    }
}
