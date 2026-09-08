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

//Last modified 5 September 2026

#include <QHeaderView>
#include <QInputDialog>
#include <QStandardPaths>
#include <QScreen>
#include <QtLogging>
#include <QMessageBox>
#include <QLockFile>
#include <QApplication>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QSplitter>
#include <QTextEdit>
#include <QTreeWidget>
#include <QFontComboBox>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDir>
#include <QTimer>
#include <cstdio>
#include <cstdlib>

#include "jane.h"

//void clean_up_and_quit ();
void check_qtdata_dir (const QString &userpath);

//holds the process lock on the data subdirectory for the life of the process
static QLockFile *lockFile = nullptr;

//acquire the process lock on the data subdirectory; returns true if the caller
//should continue (either the lock was acquired, or the user chose to ignore an
//existing lock held by another instance)
static bool acquire_lock (const QString &dataDir)
{
    lockFile = new QLockFile(dataDir + QLatin1String("/.jane.lock"));
    //a crashed instance leaves a stale lock; treat locks older than this as stale
    lockFile->setStaleLockTime(30000);

    if (!lockFile->tryLock(100)) {
        qint64 pid = -1;
        if (lockFile->error() == QLockFile::LockFailedError) {
            QString hostname;
            QString appname;
            lockFile->getLockInfo(&pid, &hostname, &appname);
        }

        QMessageBox box;
        box.setWindowTitle(QObject::tr("Jane already running"));
        box.setIcon(QMessageBox::Warning);
        if (pid > 0)
            box.setText(QObject::tr("Another instance of Jane is editing your notes "
                                    "data (process %1).\n\nRunning two instances at "
                                    "the same time may corrupt the notes file.\n"
                                    "Do you want to open Jane anyway?").arg(pid));
        else
            box.setText(QObject::tr("Another instance of Jane appears to be editing "
                                    "your notes data.\n\nRunning two instances at "
                                    "the same time may corrupt the notes file.\n"
                                    "Do you want to open Jane anyway?"));

        box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        box.setDefaultButton(QMessageBox::No);
        if (box.exec() != QMessageBox::Yes)
            return false;

        //user insisted: force-acquire the lock
        lockFile->removeStaleLockFile();
        if (!lockFile->tryLock(100))
            return false;
    }

    return true;
}

static QtMessageHandler previousHandler = nullptr;

static void janeMessageHandler(QtMsgType type,
                               const QMessageLogContext &context,
                               const QString &msg)
{
    //QtSpell emits this benign warning when the cursor is moved past the
    //end of a note; the message is misleading, so drop it
    if (type == QtWarningMsg &&
        msg.contains(QLatin1String("QTextCursor::setPosition")) &&
        msg.contains(QLatin1String("out of range"))) {
        return;
    }

    //Qt 6's Wayland plugin warns when the mouse is grabbed to drag the
    //splitter partition; the splitter still works, so the message is noise
    if (type == QtWarningMsg &&
        msg.contains(QLatin1String(
            "supports grabbing the mouse only for popup windows"))) {
        return;
    }

    if (previousHandler) {
        previousHandler(type, context, msg);
    } else {
        fprintf(stderr, "%s\n", qPrintable(qFormatLogMessage(type, context, msg)));
        fflush(stderr);
        if (type == QtFatalMsg)
            abort();
    }
}

int main(int argc, char *argv[])
//checks for the lockfile and the data directories
//start user-interface
{
    Q_INIT_RESOURCE(jane);
    previousHandler = qInstallMessageHandler(janeMessageHandler);
    QApplication app(argc, argv);

    //get the path to the user's home directory
    QString userpath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    //home/{account-name}/.jane
    userpath += QStringLiteral("/.jane");

    //check for the jane data directory and create it if required
    check_qtdata_dir(userpath);

    //warn/guard against two instances editing the same notes file
    if (!acquire_lock(userpath))
        return 0;

    MainWindow mainwindow(userpath);
    mainwindow.setWindowTitle(QObject::tr("Jane 0.3"));

    mainwindow.show();

    return app.exec();

}

QSize MainWindow::defaultWindowSize()
{
    QSize avail(1024, 768);
    if (QGuiApplication::primaryScreen())
        avail = QGuiApplication::primaryScreen()->availableGeometry().size();
    return QSize(avail.width() * 85 / 100, avail.height() * 85 / 100);
}

MainWindow::MainWindow(const QString &datadirectory, QWidget *parent) : QMainWindow(parent)
//set up the user-interface
{
    //initial window size (screen-aware, note-friendly floor of 800x600)
    QSize def = defaultWindowSize();
    this->resize(def);

    QSize minSize(800, 600);
    if (QGuiApplication::primaryScreen()) {
        QSize avail = QGuiApplication::primaryScreen()->availableGeometry().size();
        minSize.setWidth(qMin(minSize.width(), avail.width()));
        minSize.setHeight(qMin(minSize.height(), avail.height()));
    }
    this->setMinimumSize(minSize);

    //main menu
    filemenu = menuBar()->addMenu(tr("&File"));

    QAction *savelists = new QAction (tr("Export &notes as text"), this);
    filemenu->addAction(savelists);
    connect(savelists, &QAction::triggered, this, &MainWindow::save_lists_as_text);

    QAction *quit = new QAction(tr("&Quit"), this);
    filemenu->addAction(quit);
    quit->setShortcut(Qt::CTRL | Qt::Key_Q);
    connect(quit, &QAction::triggered, this, &MainWindow::save_and_quit);

    helpmenu = menuBar()->addMenu(tr("&Help"));

    QAction *helpitem = new QAction(tr("&Help"), this);
    helpmenu->addAction(helpitem);
    connect(helpitem, &QAction::triggered, this, &MainWindow::help);

    QAction *aboutitem = new QAction(tr("&About"), this);
    helpmenu->addAction(aboutitem);
    connect(aboutitem, &QAction::triggered, this, &MainWindow::about);

    QAction *aboutQtitem = new QAction(tr("About &Qt"), this);
    helpmenu->addAction(aboutQtitem);
    connect(aboutQtitem, &QAction::triggered, qApp, &QApplication::aboutQt);

    //this is the default data subdirectory
    Datadirectory = datadirectory;

    //spell checker (attached to the editor once that is created below)
    checker = new QtSpell::TextEditChecker(this);

    //toolbar for the editor
    setuptoolbar ();

    //central widget holding everything below the toolbar
    QWidget *central = new QWidget (this);
    setCentralWidget(central);
    QVBoxLayout *rootLayout = new QVBoxLayout (central);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(10);

    //main row: note tree on the left, tab pages on the right
    mainSplitter = new QSplitter(Qt::Horizontal);

    //buttons for tree of notes
    listbut.setText (tr("Ne&w Note"));
    listbut.setToolTip(tr("Create a new note"));
    connect (&listbut, &QPushButton::clicked, this, &MainWindow::add_list);

    delist.setText (tr("&Delete"));
    delist.setToolTip(tr("Delete this note"));
    connect (&delist, &QPushButton::clicked, this, &MainWindow::del_list);

    //tree of notes
    listree = new QTreeWidget ();
    listree->setColumnCount(1);
    connect (listree, &QTreeWidget::itemChanged, this, &MainWindow::save_list);
    connect (listree, &QTreeWidget::itemClicked, this, &MainWindow::set_list);
    connect (listree, &QTreeWidget::itemActivated, this, &MainWindow::set_list);

    QStringList listreeheader;
    listreeheader << tr("Note Titles");
    listree->setHeaderLabels(listreeheader);
    listree->header()->setStretchLastSection(true);
    listree->setMinimumWidth(280);

    QVBoxLayout *leftCol = new QVBoxLayout();
    leftCol->setSpacing(5);
    QHBoxLayout *treeButtons = new QHBoxLayout();
    treeButtons->setSpacing(5);
    listbut.setMinimumWidth(110);
    delist.setMinimumWidth(110);
    treeButtons->addWidget(&listbut);
    treeButtons->addWidget(&delist);
    treeButtons->addStretch();
    leftCol->addLayout(treeButtons);
    leftCol->addWidget(listree, 1);

    QWidget *leftPanel = new QWidget();
    leftPanel->setLayout(leftCol);
    mainSplitter->addWidget(leftPanel);

    //tabs
    tabcontainer = new QTabWidget;
    mainSplitter->addWidget(tabcontainer);

    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);
    QSize splitAvail(1024, 768);
    if (QGuiApplication::primaryScreen())
        splitAvail = QGuiApplication::primaryScreen()->availableGeometry().size();
    mainSplitter->setSizes({splitAvail.width() * 30 / 100,
                            splitAvail.width() * 70 / 100});

    rootLayout->addWidget(mainSplitter, 1);

    //index 0 - notes
    listed = new QWidget ();
    QVBoxLayout *notesLay = new QVBoxLayout (listed);
    listeditor = new QTextEdit (listed);
    notesLay->addWidget(listeditor);

    connect(listeditor, &QTextEdit::currentCharFormatChanged, this, &MainWindow::currentCharFormatChanged);
    connect(listeditor, &QTextEdit::cursorPositionChanged, this, &MainWindow::cursorPositionChanged);
    connect(listeditor, &QTextEdit::textChanged, this, &MainWindow::save_list);
    connect(listeditor, &QTextEdit::textChanged, this, &MainWindow::updateWordCount);
    connect(listeditor, &QTextEdit::textChanged, this, &MainWindow::clearHighlights);

    fontChanged(listeditor->font());
    colorChanged(listeditor->textColor());
    alignmentChanged(listeditor->alignment());

    //attach the spell checker to the notes editor
    checker->setTextEdit(listeditor);

    //index 1 - search
    search = new QWidget();
    QVBoxLayout *searchLay = new QVBoxLayout (search);
    searchLay->setSpacing(5);

    searchboxlabel = new QLabel (search);
    searchboxlabel->setAlignment(Qt::AlignCenter);
    searchboxlabel->setText(tr("Search for"));

    searchtxtbox = new QLineEdit (search);
    searchtxtbox->setToolTip(tr("Enter text to search for (max 50 characters)"));
    searchtxtbox->setMaxLength(50);

    searchbut = new QPushButton (tr("S&earch"), search);
    searchbut->setToolTip(tr("Click to begin search"));
    connect (searchbut, &QPushButton::clicked, this, &MainWindow::search_notes);

    searchclrbut = new QPushButton (tr("C&lear"), search);
    searchclrbut->setToolTip(tr("Click to clear search results"));
    connect (searchclrbut, &QPushButton::clicked, this, &MainWindow::search_clear_notes);

    QHBoxLayout *searchButtons = new QHBoxLayout();
    searchButtons->addWidget(searchbut);
    searchButtons->addWidget(searchclrbut);
    searchButtons->addStretch();

    searchresults = new QTextEdit (search);
    searchresults->setReadOnly(true);

    searchLay->addWidget(searchboxlabel);
    searchLay->addWidget(searchtxtbox);
    searchLay->addLayout(searchButtons);
    searchLay->addWidget(searchresults, 1);

    //index 2 - preferences
    prefs = new QWidget();
    QVBoxLayout *prefsLay = new QVBoxLayout (prefs);
    prefsLay->setSpacing(5);

    fontbut = new QPushButton (tr("Set application fon&t"), prefs);
    fontbut->setToolTip(tr("Click to select a different font for the application (except the notes)"));
    connect (fontbut, &QPushButton::clicked, this, &MainWindow::select_font);

    backupbut = new QPushButton (tr("&Create Backup"), prefs);
    backupbut->setToolTip(tr("Create a copy of the notes data file"));
    connect (backupbut, &QPushButton::clicked, this, &MainWindow::backup_notes);

    QLabel *spelllanglabel = new QLabel(tr("Spell check language"), prefs);
    spelllanglabel->setAlignment(Qt::AlignHCenter);
    spelllang = new QComboBox(prefs);
    spelllang->setToolTip(tr("Select the dictionary used for spell checking"));
    spelllang->addItem(tr("Automatic (system)"), QString());
    for (const QString &lang : QtSpell::Checker::getLanguageList()) {
        QString label = QtSpell::Checker::decodeLanguageCode(lang);
        if (label.isEmpty())
            label = lang;
        spelllang->addItem(label, lang);
    }
    int langidx = spelllang->findData(checker->getLanguage());
    if (langidx < 0)
        langidx = 0;
    spelllang->setCurrentIndex(langidx);
    //match the combo's height to the buttons above it; width stays full-tab
    spelllang->setFixedHeight(backupbut->sizeHint().height());
    connect(spelllang, QOverload<int>::of(&QComboBox::activated), this,
            [this](int idx) {
                checker->setLanguage(spelllang->itemData(idx).toString());
            });

    prefsLay->addWidget(fontbut);
    prefsLay->addWidget(backupbut);
    prefsLay->addWidget(spelllanglabel);
    prefsLay->addWidget(spelllang);
    prefsLay->addStretch();

    //add the tabs to the container
    tabcontainer->insertTab (2, search, tr("&Search"));
    tabcontainer->insertTab (1, prefs, tr("P&references"));
    tabcontainer->insertTab (0, listed, tr("&Notes"));

    //connect after the tabs are created so construction does not trigger it
    connect (tabcontainer, &QTabWidget::currentChanged, this, &MainWindow::make_tab_visible);

    //status bar
    statustext = new QLabel (statusBar ());
    statustext->setText(tr("Status messages appear here"));
    statustext->setFrameStyle(QFrame::Plain);
    statustext->setMinimumWidth(300);
    statusBar()->addWidget(statustext);

    wordcount = new QLabel (QObject::tr("Words: 0"), statusBar());
    wordcount->setFrameStyle(QFrame::Plain);
    statusBar()->addPermanentWidget(wordcount);

    //search this note: bottom row under the editor
    searchthisnotelabel = new QLabel (central);
    searchthisnotelabel->setFrameStyle(QFrame::Plain);
    searchthisnotelabel->setAlignment(Qt::AlignVCenter);
    searchthisnotelabel->setText(tr("Search for"));

    searchthisnotetextbox = new QLineEdit (central);
    searchthisnotetextbox->setToolTip(tr("Enter text to find (max 50 characters)"));
    searchthisnotetextbox->setMaxLength(50);

    searchthisnotebut = new QPushButton (tr("F&ind"), central);
    searchthisnotebut->setToolTip(tr("Click to begin search"));
    searchthisnotebut->setEnabled(false);
    connect (searchthisnotebut, &QPushButton::clicked, this, &MainWindow::search_this_note);

    //enable the Find button only while the cursor is in the notes editor
    //(or in the find row itself, so typing a search then clicking Find works)
    connect(qApp, &QApplication::focusChanged, this,
            [this](QWidget * /*old*/, QWidget *now) {
                const bool inEditor = (now == listeditor) ||
                                      (now != nullptr && listeditor->isAncestorOf(now));
                const bool inFindRow = (now == searchthisnotetextbox) ||
                                       (now == searchthisnotebut);
                searchthisnotebut->setEnabled(inEditor || inFindRow);
            });

    QHBoxLayout *bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(5);
    bottomRow->addWidget(searchthisnotelabel);
    bottomRow->addWidget(searchthisnotetextbox, 1);
    bottomRow->addWidget(searchthisnotebut);
    rootLayout->addLayout(bottomRow);

    //autosave: quietly write the notes file every minute if anything changed
    autosaveTimer = new QTimer(this);
    autosaveTimer->setInterval(60 * 1000);
    connect(autosaveTimer, &QTimer::timeout, this, &MainWindow::autosave);
    autosaveTimer->start();

    //read the data file and show the first note
    initialize ();
}

void MainWindow::initialize()
//read the data files and show the first note
//initialize some data variables
{
//QString p, s1, s2, s3;

    //read the user's preferences
    readprefs();

    //set the path to the notes file
    Homepath.append(Datadirectory);
    //set the working directory to the data subdirectory
    QDir::setCurrent(Homepath);

    //set up the empty list tree
    cur_list = nullptr;
    con_item = new QTreeWidgetItem ();

    listreeempty = true;

    //lists file to read
    Listfilename.append (Homepath);
    Listfilename.append ("/Notes.jane");
    read_lists ();

    //set the first list as the current list
    if (listreeempty == false) {
        cur_list = listree->topLevelItem(0);
        listree->setCurrentItem(cur_list);
        listeditor->setHtml(cur_list->text(1));
    }
    else {
        cur_list = con_item;
    }

    //sort the list
    listree->setSortingEnabled(true);
    listree->sortByColumn(0, Qt::AscendingOrder);
    listree->setSortingEnabled(false);

    tabcontainer->setCurrentWidget(listed);

    //help file to read
    Helpfilename.append (Homepath);
    Helpfilename.append ("/janehelp.pdf");

    //"COPYING" file to read
    Gnugplfilename.append (Homepath);
    Gnugplfilename.append ("/COPYING");

    return;
}

void MainWindow::save_other_data()
//transfer user data from editor to note array
//save data to files
{
    write_lists ();//save lists
}

void MainWindow::save_and_quit()
{
    MainWindow::close();
}


void MainWindow::make_tab_visible()
//show the tab which has been selected and adjust the tree-related buttons if required
{
    //tabcontainer->setCurrentIndex(i);

    if (tabcontainer->currentWidget() == listed) {
         tb->setEnabled(true);
         show_list();
    }
     else//disable it
        tb->setDisabled(true);
}

