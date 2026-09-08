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

//Last modified 5 Sep 2026

#include "jane.h"
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTextDocument>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>

void MainWindow::save_list ()
//item changed
{
QString s;

    s = listeditor->toHtml();
    cur_list->setText(1, s);
    modify_name(cur_list);
    dataModified = true;
}

void MainWindow::set_list (QTreeWidgetItem *it)
//item clicked
{
//QString s;

    save_list  ();

    cur_list = it;
    show_list();
}

void MainWindow::add_list()
{
QString s;

    if (listreeempty == false) {
        save_list();
    }

    con_item = new QTreeWidgetItem (listree);
    s.clear();
    s.append(tr("New note"));
    con_item->setText(0, s);
    s.clear();
    s.append(tr("New note"));
    con_item->setText(1, s);

    cur_list = con_item;

    listree->setCurrentItem(cur_list);
    listreeempty = false;
    dataModified = true;

    statustext->setText(tr("Added an empty note"));
    show_list();
}

void MainWindow::show_list ()
{
QString s;
int i;

    //bring notes tab to foreground
    i = tabcontainer->indexOf(listed);
    tabcontainer->setCurrentIndex(i);
    //first way
    s.clear();
    s.append(cur_list->text(1));
    listeditor->setHtml(s);

    //second way
    //QTextDocument *doc = new QTextDocument ();
    //doc->setHtml(cur_list->text(1));
    //listeditor->setDocument(doc);


}
void MainWindow::modify_name (QTreeWidgetItem *it)
{
    QTextDocument doc;
    doc.setHtml(it->text(1));
    QString s = doc.toPlainText();

    //use the first line of the note as its tree title
    int nl = s.indexOf(QLatin1Char('\n'));
    if (nl >= 0)
        s.truncate(nl);

    //truncate the title to the first five words, appending an ellipsis
    //when there is more content, so the title does not mirror the whole note
    const QStringList words = s.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (words.size() > 5) {
        QString truncated = words.mid(0, 5).join(QLatin1Char(' '));
        if (!truncated.endsWith(QLatin1Char('.')))
            truncated += QLatin1String("...");
        s = truncated;
    }

    it->setText(0, s);
}

void MainWindow::del_list ()
{
int j;
QTreeWidgetItem *above, *below;
QString s;
QTextDocument doc;
    doc.setHtml(cur_list->text(1));
    s = doc.toPlainText();

    if (s.length() != 0){
        statustext->setText(tr("Note contains data. Please delete that, first."));
        return;
    }

    j = listree->indexOfTopLevelItem(cur_list);
    dataModified = true;

    above = listree->itemAbove(cur_list);
    below = listree->itemBelow(cur_list);

    if (above != NULL){//there is an item above
        delete listree->takeTopLevelItem(j);
        statustext->setText(tr("Note deleted"));
        listree->setCurrentItem(above);
        cur_list = above;

        listreeempty = false;
        show_list();
    }
    else if (below != NULL) {//no item above but there is an item below
        delete listree->takeTopLevelItem(j);
        statustext->setText(tr("Note deleted"));

        listree->setCurrentItem(below);
        cur_list = below;
        listreeempty = false;
        show_list();
    }
    else {//cur_list is the last item
        delete listree->takeTopLevelItem(j);
        listreeempty = true;
        con_item = new QTreeWidgetItem();
        cur_list = con_item;
        listeditor->clear();
        statustext->setText(tr("Last note deleted"));
    }
}
