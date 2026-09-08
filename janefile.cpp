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
#include <QSaveFile>
#include <QFile>
#include <QTreeWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextDocument>
#include <QMessageBox>
#include <QDataStream>
#include <QTextStream>

namespace {
const quint32 JaneFileMagic = 0x4A414E45; // "JANE"
const quint32 JaneFileVersion = 1;
const quint32 MaxNotes = 100000;
const quint32 MaxLegacyStringBytes = 64 * 1024 * 1024;

bool readLegacyString(QDataStream &in, QString &value)
{
    quint32 declaredLength;
    quint32 encodedLength;

    in >> declaredLength;
    if (in.status() != QDataStream::Ok ||
        declaredLength > MaxLegacyStringBytes) {
        return false;
    }

    // Older Jane files wrote the length twice: once explicitly and once
    // through QDataStream::writeBytes().
    in >> encodedLength;
    if (in.status() != QDataStream::Ok ||
        encodedLength != declaredLength ||
        encodedLength > MaxLegacyStringBytes) {
        return false;
    }

    QByteArray bytes(static_cast<int>(encodedLength), '\0');
    if (encodedLength > 0 &&
        in.readRawData(bytes.data(), static_cast<int>(encodedLength))
            != static_cast<int>(encodedLength)) {
        return false;
    }

    if (!bytes.isEmpty() && bytes.endsWith('\0'))
        bytes.chop(1);

    value = QString::fromLocal8Bit(bytes);
    return in.status() == QDataStream::Ok;
}

bool writeNotes(QDataStream &out, QTreeWidget *listree)
{
    const int count = listree->topLevelItemCount();

    out << JaneFileMagic;
    out << JaneFileVersion;
    out << static_cast<quint32>(count);

    for (int i = 0; i < count; ++i) {
        QTreeWidgetItem *it = listree->topLevelItem(i);
        out << it->text(0);
        out << it->text(1);
    }

    return out.status() == QDataStream::Ok;
}
}

void MainWindow::read_lists()
{
    QFile file(Listfilename);

    if (!file.open(QFile::ReadOnly))
        return;

    QDataStream in(&file);
    quint32 header;
    in >> header;

    if (in.status() != QDataStream::Ok)
        return;

    quint32 toplevelcount;
    const bool newFormat = (header == JaneFileMagic);

    if (newFormat) {
        quint32 version;
        in >> version;
        if (in.status() != QDataStream::Ok ||
            version > JaneFileVersion) {
            return;
        }
        in >> toplevelcount;
    } else {
        // The old format stored the note count as the first 32-bit value.
        toplevelcount = header;
    }

    if (in.status() != QDataStream::Ok || toplevelcount > MaxNotes)
        return;

    for (quint32 i = 0; i < toplevelcount; ++i) {
        QString title;
        QString content;

        bool ok = false;
        if (newFormat) {
            in >> title >> content;
            ok = (in.status() == QDataStream::Ok);
        } else {
            ok = readLegacyString(in, title) &&
                 readLegacyString(in, content);
        }

        if (!ok)
            return;

        QTreeWidgetItem *it = new QTreeWidgetItem();
        it->setText(0, title);
        it->setText(1, content);
        listree->addTopLevelItem(it);
    }

    if (toplevelcount > 0)
        listreeempty = false;
}

bool MainWindow::writeListsToFile(bool showErrors)
{
    const int toplevelcount = listree->topLevelItemCount();

    if (toplevelcount == 0) {
        QFile::remove(Listfilename);
        return true;
    }

    QSaveFile file(Listfilename);
    if (!file.open(QFile::WriteOnly)) {
        if (showErrors) {
            QMessageBox::critical(this, tr("Save failed"),
                                  tr("Could not open the notes file for writing:\n%1\n\n%2")
                                      .arg(Listfilename, file.errorString()));
        }
        return false;
    }

    QDataStream out(&file);
    if (!writeNotes(out, listree) || !file.commit()) {
        if (showErrors) {
            QMessageBox::critical(this, tr("Save failed"),
                                  tr("Could not save your notes. Your changes may be lost.\n%1")
                                      .arg(file.errorString()));
        }
        return false;
    }

    return true;
}

void MainWindow::write_lists()
{
    writeListsToFile(true);
}

void MainWindow::autosave()
{
    if (!dataModified)
        return;
    if (writeListsToFile(false))
        dataModified = false;
}

void MainWindow::save_lists_as_text()
{
    QString txtfile, s;
    QTreeWidgetItem *it;
    int i, toplevelcount;

    //txtfile.append (Homepath);
    txtfile.append ("Notes.txt");
    QFile file(txtfile);
    if (!file.open(QFile::WriteOnly))
        return;

    QTextStream out(&file);

    //number of categories
    toplevelcount = listree->topLevelItemCount();
    out << "Number of notes:" << toplevelcount << "\n";

    //loop over lists
    for (i = 0; i < toplevelcount; i++){
        it = listree->topLevelItem(i);
        QTextDocument doc;
        doc.setHtml(it->text(1));
        s = doc.toPlainText();

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
    const QString BackupFilename = Listfilename + ".backup";
    const int toplevelcount = listree->topLevelItemCount();

    if (toplevelcount == 0) {
        QFile::remove(BackupFilename);
        return;
    }

    QSaveFile file(BackupFilename);
    if (!file.open(QFile::WriteOnly))
        return;

    QDataStream out(&file);
    if (!writeNotes(out, listree) || !file.commit())
        return;

    statustext->setText(tr("Backed up \"Notes.jane.backup\" "));
}
