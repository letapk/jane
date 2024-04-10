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

//Last modified 9 April 2024

#include "jane.h"
#include <QFileDialog>
#include <QImageReader>
#include <QDesktopServices>

extern QString userpath;

void MainWindow::about()
//open a window to show program information and copyright license
{
QFile file(Gnugplfilename);
QTextStream in(&file);
QTextBrowser *gnugpl;
bool ok;

    ok = file.open(QFile::ReadOnly);
    if (ok == false)
        return;

    gnugpl = new QTextBrowser ();
    gnugpl->setGeometry(10, 10, 800, 600);
    gnugpl->setWindowTitle (QObject::tr("About Jane"));
    gnugpl->setPlainText(in.readAll());
    gnugpl->setAlignment(Qt::AlignLeft);
    gnugpl->show();

}

void MainWindow::help()
//open and show the user manual
{
bool ok;
QString s1;
QMessageBox msgBox;

    QFile file(Helpfilename);
    ok = file.exists();
    if (ok == false) {
        s1 = QObject::tr("The help file was not found.");
        s1.append (QObject::tr("Please make sure that it is present in the hidden Jane data directory."));

        msgBox.setText(s1);
        msgBox.exec();
    }
    else {
        QDesktopServices::openUrl (QUrl (Helpfilename));
    }
}

void check_qtdata_dir ()
{
QString qtpath, s1;
QDir qtdir;
QMessageBox msgBox;

    qtpath.append (userpath);
    qtdir = QDir (qtpath);

    if (qtdir.exists() == false) {
        s1.append (QObject::tr("The Jane data directory does not exist. "));
        s1.append (QObject::tr("This is required to store your work.\n"));
        s1.append (QObject::tr("Click OK to create a new, hidden subdirectory "));
        s1.append (QObject::tr("in your area with the name :\n"));
        s1.append (qtpath);
        msgBox.setText(s1);
        msgBox.exec();

        qtdir.mkdir(qtpath);
    }
}

MainWindow::~MainWindow()
{

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    save_other_data ();
    writeprefs();

    event->accept();
}

void MainWindow::resizeEvent(QResizeEvent *event)
//move and shift the widgets when the window size changes
{
int w, h;

    w = width();
    h = height();

    tb->resize(w - 20, 37);

    //resize the tree
    listree->setGeometry(10, 110, 300, h - 155);

    //resize the tabcontainer
    tabcontainer->resize(w - 320, h - 115);

    listeditor->setGeometry(5, 5, tabcontainer->width() - 15, tabcontainer->height() - 55);
    //search->setGeometry(10, 10, tabcontainer->width() - 20, tabcontainer->height() - 55);
    searchresults->setGeometry(5, 90, tabcontainer->width() - 15, tabcontainer->height() - 140);
    //move the status text area
    statustext->setGeometry(10, h - 45, 300, 35);

    //move the search this note area
    searchthisnotelabel->setGeometry(420, h - 45, 100, 35);
    searchthisnotetextbox->setGeometry(525, h - 40, 350, 35);
    searchthisnotebut->setGeometry(880, h - 40, 80, 35);

    //this makes the font size box visible on expanding the window and fixes
    //a bug which would prevent it from showing if the user has clicked the extension
    //button on the toolbar
    comboFont->setVisible(true);
    comboSize->setVisible(true);

    //pass the event up the chain
    QWidget::resizeEvent(event);

}

void MainWindow::setuptoolbar()
{
    //toolbar for the editor
    tb = new QToolBar;
    tb->setParent (this);
    tb->setGeometry(10, 37, 1080, 35);
    tb->setFloatable (false);
    tb->setMovable(false);

    actionTextBold = new QAction(QIcon::fromTheme("", QIcon(":/images/textbold.png")), tr("&Bold, Ctrl-B"), this);
    actionTextBold->setShortcut(Qt::CTRL | Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    connect(actionTextBold, SIGNAL(triggered()), this, SLOT(textBold()));
    tb->addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    actionTextItalic = new QAction(QIcon::fromTheme("", QIcon(":/images/textitalic.png")), tr("&Italic, Ctrl-I"), this);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL | Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    connect(actionTextItalic, SIGNAL(triggered()), this, SLOT(textItalic()));
    tb->addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    actionTextUnderline = new QAction(QIcon::fromTheme("", QIcon(":/images/textunder.png")), tr("&Underline, Ctrl-U"), this);
    actionTextUnderline->setShortcut(Qt::CTRL | Qt::Key_U);
    actionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    connect(actionTextUnderline, SIGNAL(triggered()), this, SLOT(textUnderline()));
    tb->addAction(actionTextUnderline);
    actionTextUnderline->setCheckable(true);

    QActionGroup *grp = new QActionGroup(this);
    connect(grp, SIGNAL(triggered(QAction*)), this, SLOT(textAlign(QAction*)));

    actionAlignLeft = new QAction(QIcon::fromTheme("", QIcon(":/images/textleft.png")),tr("&Left, Ctrl-L"), grp);
    actionAlignCenter = new QAction(QIcon::fromTheme("",QIcon(":/images/textcenter.png")),tr("C&enter, Ctrl-E"), grp);
    actionAlignRight = new QAction(QIcon::fromTheme("",QIcon(":/images/textright.png")),tr("&Right, Ctrl-R"), grp);
    actionAlignJustify = new QAction(QIcon::fromTheme("",QIcon(":/images/textjustify.png")),tr("&Justify, Ctrl-J"), grp);

    actionAlignLeft->setShortcut(Qt::CTRL | Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignLeft->setPriority(QAction::LowPriority);
    actionAlignCenter->setShortcut(Qt::CTRL | Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignCenter->setPriority(QAction::LowPriority);
    actionAlignRight->setShortcut(Qt::CTRL | Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignRight->setPriority(QAction::LowPriority);
    actionAlignJustify->setShortcut(Qt::CTRL | Qt::Key_J);
    actionAlignJustify->setCheckable(true);
    actionAlignJustify->setPriority(QAction::LowPriority);

    tb->addActions(grp->actions());

    QAction *actionInsertImage= new QAction(QIcon::fromTheme("", QIcon(":/images/insert-image.png")), tr("&Insert image"), this);
    actionInsertImage->setPriority(QAction::LowPriority);
    connect(actionInsertImage, SIGNAL(triggered()), this, SLOT(insertImage()));
    actionInsertImage->setCheckable(true);

    tb->addAction(actionInsertImage);

    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    actionTextColor = new QAction(pix, tr("&Color..."), this);
    connect(actionTextColor, SIGNAL(triggered()), this, SLOT(textColor()));
    tb->addAction(actionTextColor);

    comboFont = new QFontComboBox(tb);
    tb->addWidget(comboFont);
    connect(comboFont, SIGNAL(currentFontChanged(QFont)), this, SLOT(textFamily(QFont)));

    comboSize = new QComboBox(tb);
    comboSize->setObjectName("comboSize");
    tb->addWidget(comboSize);
    comboSize->setEditable(true);

    QFontDatabase db;
    foreach(int size, db.standardSizes())
        comboSize->addItem(QString::number(size));

    connect(comboSize, SIGNAL(activated(QString)), this, SLOT(textSize(QString)));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(QApplication::font().pointSize())));
}

void MainWindow::textBold()
{
QTextCharFormat fmt;

    fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void MainWindow::textItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(actionTextItalic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void MainWindow::textUnderline()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(actionTextUnderline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void MainWindow::textAlign(QAction *a)
{
QTextEdit *editor;

    editor = listeditor;

    if (a == actionAlignLeft)
        editor->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if (a == actionAlignCenter)
        editor->setAlignment(Qt::AlignHCenter);
    else if (a == actionAlignRight)
        editor->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if (a == actionAlignJustify)
        editor->setAlignment(Qt::AlignJustify);
}

void MainWindow::textColor()
{
QTextEdit *editor;

    editor = listeditor;

    QColor col = QColorDialog::getColor(editor->textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    colorChanged(col);
}

void MainWindow::insertImage()
{
QString s, filters, fname;
QFileInfo fi;
QTextEdit *editor;
QMessageBox msgBox;

    editor = listeditor;

    filters += tr("Common Graphics (*.png *.jpg *.jpeg *.gif);;");
    filters += tr("Portable Network Graphics (PNG) (*.png);;");
    filters += tr("JPEG (*.jpg *.jpeg);;");
    filters += tr("Graphics Interchange Format (*.gif);;");
    filters += tr("All Files (*)");

    QString file = QFileDialog::getOpenFileName(this, tr("Open image..."), QString(), filters);
    if (file.isEmpty())
        return;
    if (!QFile::exists(file))
        return;

    fi = QFileInfo(file);
    if (fi.path() != Homepath) {
        //copy the file to the data subdirectory
        fname.clear();
        fname.append(Homepath);
        fname.append("/");
        fname.append(fi.fileName());
        QFile::copy (fi.filePath(), fname);

        s.append (QObject::tr("The image file has been copied to the Jane data directory "));
        s.append (Homepath);
        s.append (QObject::tr("\nClick OK to continue"));
        msgBox.setText(s);
        msgBox.exec();
    }

    if (listree->topLevelItemCount() == 0)
        return;
    editor->insertHtml(QString ("<img src=\"%1/%2\">").arg(Homepath).arg(fi.fileName()));
}

void MainWindow::cursorPositionChanged()
{
QTextEdit *editor;

    editor = listeditor;
    alignmentChanged(editor->alignment());
}

void MainWindow::currentCharFormatChanged(const QTextCharFormat &format)
{
    fontChanged(format.font());
    colorChanged(format.foreground().color());
}

void MainWindow::fontChanged(const QFont &f)
{
    comboFont->setCurrentIndex(comboFont->findText(QFontInfo(f).family()));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));

    actionTextBold->setChecked(f.bold());
    actionTextItalic->setChecked(f.italic());
    actionTextUnderline->setChecked(f.underline());
}

void MainWindow::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}

void MainWindow::alignmentChanged(Qt::Alignment a)
{
    if (a & Qt::AlignLeft)
        actionAlignLeft->setChecked(true);
    else if (a & Qt::AlignHCenter)
        actionAlignCenter->setChecked(true);
    else if (a & Qt::AlignRight)
        actionAlignRight->setChecked(true);
    else if (a & Qt::AlignJustify)
        actionAlignJustify->setChecked(true);
}

void MainWindow::textFamily(const QFont &f)
{
    QTextCharFormat fmt;
    fmt.setFont(f);
    mergeFormatOnWordOrSelection(fmt);
}

void MainWindow::textSize(const QString &p)
{
    qreal pointSize = p.toFloat();
    if (p.toFloat() > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void MainWindow::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
QTextEdit *editor;

    editor = listeditor;

    QTextCursor cursor = editor->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    editor->mergeCurrentCharFormat(format);
}

void MainWindow::show_the_font()
{
QFont f;
QString s;

    f = listeditor->currentFont();
    s = f.family();
    comboFont->setCurrentFont(f);
    return;
}

void MainWindow::search_notes()
{
QStringList *result;
QTreeWidgetItem *it;
QString b1, b2;
int i, toplevelcount;
QString s;

    searchtxt = searchtxtbox->text();
    if (searchtxt.isEmpty() == true) {
        statustext->setText(tr("Please enter text in search box"));
        return;
    }
    result = new QStringList;
    result->append(tr(""));
    searchresults->append(s);

    //number of categories
    toplevelcount = listree->topLevelItemCount();

    //loop over notes, i is the serial number of note
    for (i = 0; i < toplevelcount; i++){
        it = listree->topLevelItem(i);

        //title of note
        b1 = it->text(0).toLocal8Bit();
        //note itself
        b2 = it->text(1).toLocal8Bit();

        if (b2.contains(searchtxt, Qt::CaseInsensitive) == true) {
            s = QString (tr("Found in the note number %1 titled \"%2\"").arg(i+1).arg(b1));
            result->append(s);
            searchresults->append(s);
        }
    }
}

void MainWindow::search_clear_notes()
{
    searchresults->clear();
}

void MainWindow::search_this_note()
{
QTextEdit *editor;
bool found;
QMessageBox msgBox;
int ret;

    editor = listeditor;

    //get string to search for from text box
    searchtxt = searchthisnotetextbox->text();
    if (searchtxt.isEmpty() == true) {
        statustext->setText(tr("Please enter text in search box"));
        return;
    }

    found = editor->find (searchtxt);
    if (found == false) {
        msgBox.setText("Reached end of note.\nDo you want to search from the beginning?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        ret = msgBox.exec();

        switch (ret) {
        case QMessageBox::Yes:
            editor->moveCursor(QTextCursor::Start);
            search_this_note();
            break;
        case QMessageBox::No:
            return;
            break;
        default:
            // should never be reached
            break;
        }
    }
}
