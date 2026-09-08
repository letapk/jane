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
#include <QSettings>
#include <QDir>
#include <QFontDialog>
#include <QApplication>
#include <QSplitter>
#include <QScreen>
#include <QComboBox>

void MainWindow::writeprefs()
{
    QSettings settings(QStringLiteral("jane"), QStringLiteral("jane"));

    settings.setValue(QStringLiteral("pos"), pos());//window position

    settings.setValue(QStringLiteral("size"), size());//window size

    settings.setValue(QStringLiteral("font"), curfont.toString());//selected font

    settings.setValue(QStringLiteral("defdatadir"), Datadirectory);

    settings.setValue(QStringLiteral("splitter"), mainSplitter->saveState());

    settings.setValue(QStringLiteral("spelllang"),
                      spelllang->itemData(spelllang->currentIndex()).toString());
}

//ensure the window is visible on some screen; if the restored position lies
//entirely off-screen (e.g. the display it was on was unplugged or the layout
//changed), recenter on the primary screen
static void ensureOnScreen(QWidget *win)
{
    const QRect frame = win->frameGeometry();

    for (const QScreen *screen : QGuiApplication::screens()) {
        if (frame.intersects(screen->availableGeometry()))
            return;
    }

    QRect target = QGuiApplication::primaryScreen()->availableGeometry();
    win->move(target.center() - QPoint(win->frameGeometry().width() / 2,
                                       win->frameGeometry().height() / 2));
}

void MainWindow::readprefs()
{
    QSettings settings(QStringLiteral("jane"), QStringLiteral("jane"));

    QPoint pos = settings.value(QStringLiteral("pos"), QPoint(20, 20)).toPoint();

    QSize size = settings.value(QStringLiteral("size"), defaultWindowSize()).toSize();

    QString s = settings.value(QStringLiteral("font"),
                                QApplication::font().toString()).toString();
    curfont.fromString(s);
    QApplication::setFont(curfont);

    //honor a previously stored data directory, falling back to the default
    s = settings.value(QStringLiteral("defdatadir"), Datadirectory).toString();
    if (QDir(s).exists())
        Datadirectory = s;

    mainSplitter->restoreState(
        settings.value(QStringLiteral("splitter")).toByteArray());

    //restore the spell check language (combo keeps "Automatic", i.e. an empty code)
    const QString lang = settings.value(QStringLiteral("spelllang")).toString();
    if (!lang.isEmpty())
        checker->setLanguage(lang);
    int langidx = spelllang->findData(lang);
    if (langidx < 0)
        langidx = 0;
    spelllang->setCurrentIndex(langidx);

    resize(size);
    move(pos);
    ensureOnScreen(this);
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
