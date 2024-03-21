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

void MainWindow::read_lists()
{
QTreeWidgetItem *it;
QFile file(Listfilename);
QString *s;
char *c;
uint i, len, toplevelcount;
bool ok;

    ok = file.open(QFile::ReadOnly);
    if (ok == false)
        return;

    QDataStream in(&file);

    //number of lists
    in >> toplevelcount;

    //loop over list
    for (i = 0; i < toplevelcount; i++) {
        it = new QTreeWidgetItem (listree);
        listree->addTopLevelItem(it);

        in >> len;
        c = new char[len];
        in.readBytes(c, len);
        s = new QString (c);
        it->setText(0, *s);
        delete s;

        in >> len;
        c = new char[len];
        in.readBytes(c, len);
        s = new QString (c);
        it->setText(1, *s);
        delete s;
    }

    file.close();

    listreeempty = false;
}

void MainWindow::write_lists()
{
QFile file(Listfilename);
QTreeWidgetItem *it;
QByteArray b;
int i, len, toplevelcount;
bool ok;

    //number of categories
    toplevelcount = listree->topLevelItemCount();

    if (toplevelcount == 0) {//nothing to save
        file.remove();
        return;
    }

    ok = file.open(QFile::WriteOnly);
    if (ok == false)
        return;
    QDataStream out(&file);

    out << toplevelcount;

    //loop over lists
    for (i = 0; i < toplevelcount; i++){
        it = listree->topLevelItem(i);

        b = it->text(0).toLocal8Bit();
        len = b.size() + 1;
        out << len;
        out.writeBytes(b.data(), len);

        b = it->text(1).toLocal8Bit();
        len = b.size() + 1;
        out << len;
        out.writeBytes(b.data(), len);
    }

    file.close();
}

void MainWindow::save_lists_as_text()
{
    QString txtfile, s;
    QTreeWidgetItem *it;
    QTextDocument *doc;
    int i, toplevelcount;
    bool ok;

    //txtfile.append (Homepath);
    txtfile.append ("Notes.txt");
    QFile file(txtfile);
    ok = file.open(QFile::WriteOnly);
    if (ok == false)
        return;
    QTextStream out(&file);

    //number of categories
    toplevelcount = listree->topLevelItemCount();
    out << "Number of notes:" << toplevelcount << "\n";

    //loop over lists
    for (i = 0; i < toplevelcount; i++){
        it = listree->topLevelItem(i);

        doc = new QTextDocument ();
        doc->setHtml(it->text(1));
        s = doc->toPlainText();
        delete doc;

        out << "\nTitle of note:";
        out << s;
        out << "\n---\n";
    }

    file.close();
    s = QString (tr("Exported \"%1\"")).arg(txtfile);
    statustext->setText(s);
}

void MainWindow::backup_notes()
{
QString BackupFilename;

QTreeWidgetItem *it;
QByteArray b;
int i, len, toplevelcount;
bool ok;

    //create backup filename
    BackupFilename = Listfilename;
    //BackupFilename.append (getenv ("HOME"));
    BackupFilename.append(".backup");
    QFile file(BackupFilename);

    //remove old backup
    if (file.exists() == true) {
        file.remove();
    }

    //save notes in new backup file

    //number of categories
    toplevelcount = listree->topLevelItemCount();

    if (toplevelcount == 0) {//nothing to save
        file.remove();
        return;
    }

    ok = file.open(QFile::WriteOnly);
    if (ok == false)
        return;
    QDataStream out(&file);

    out << toplevelcount;

    //loop over lists
    for (i = 0; i < toplevelcount; i++){
        it = listree->topLevelItem(i);

        b = it->text(0).toLocal8Bit();
        len = b.size() + 1;
        out << len;
        out.writeBytes(b.data(), len);

        b = it->text(1).toLocal8Bit();
        len = b.size() + 1;
        out << len;
        out.writeBytes(b.data(), len);
    }

    file.close();

    statustext->setText(tr("Backed up \"Notes.jane.backup\" "));

}

