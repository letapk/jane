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

#include <QHeaderView>
#include <QInputDialog>

#include "jane.h"

//userpath contains the path to the data subdirectory
QString userpath;
//void clean_up_and_quit ();
void check_qtdata_dir ();

int main(int argc, char *argv[])
//checks for the lockfile and the data directories
//start user-interface
{
    Q_INIT_RESOURCE(jane);
    QApplication app(argc, argv);

    //get the path to the user's home directory
    userpath.clear();
    //home/{account-name}
    userpath.append (getenv ("HOME"));

    userpath.append("/.jane");//home/{account-name}/.jane

    //check for the jane data directory and create it if required
    check_qtdata_dir();

    MainWindow mainwindow;
    mainwindow.setWindowTitle(QObject::tr("Jane 0.2"));

    mainwindow.show();

    return app.exec();

}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
//set up the user-interface
{
    //initial window size
    this->resize(1100,750);
    this->setMinimumHeight(750);
    this->setMinimumWidth(1100);

    //main menu
    filemenu = menuBar()->addMenu(tr("&File"));

    QAction *savelists = new QAction (tr("Export &notes as text"), this);
    filemenu->addAction(savelists);
    connect(savelists, SIGNAL(triggered()), this, SLOT(save_lists_as_text()));

    QAction *quit = new QAction(tr("E&xit"), this);
    filemenu->addAction(quit);
    connect(quit, SIGNAL(triggered()), this, SLOT(save_and_quit()));

    helpmenu = menuBar()->addMenu(tr("&Help"));

    QAction *helpitem = new QAction(tr("&Help"), this);
    helpmenu->addAction(helpitem);
    connect(helpitem, SIGNAL(triggered()), this, SLOT(help()));

    QAction *aboutitem = new QAction(tr("&About"), this);
    helpmenu->addAction(aboutitem);
    connect(aboutitem, SIGNAL(triggered()), this, SLOT(about()));

    QAction *aboutQtitem = new QAction(tr("About &Qt"), this);
    helpmenu->addAction(aboutQtitem);
    connect(aboutQtitem, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    //this is the default data subdirectory
    Datadirectory.clear();
    Datadirectory.append(userpath);

    //---------------------LEFT side

    //buttons for tree of notes
    listbut.setParent(this);
    listbut.setGeometry(10, 70, 115, 35);
    listbut.setText (tr("Ne&w Note"));
    listbut.setToolTip(tr("Create a new note"));
    connect (&listbut, SIGNAL(clicked(bool)), this, SLOT(add_list()));

    delist.setParent(this);
    delist.setGeometry(130, 70, 115, 35);
    delist.setText (tr("&Delete"));
    delist.setToolTip(tr("Delete this note"));
    connect (&delist, SIGNAL(clicked(bool)), this, SLOT(del_list()));

    listbut.show();
    delist.show();

    //tree of notes
    listree = new QTreeWidget (this);
    listree->setGeometry(10, 110, 300, 605);
    listree->setColumnCount(1);
    connect (listree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(save_list()));
    connect (listree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(set_list(QTreeWidgetItem*)));
    connect (listree, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(set_list(QTreeWidgetItem*)));

    QStringList listreeheader;
    listreeheader << tr("Note Titles");
    listree->setHeaderLabels(listreeheader);
    listree->show();

    //---------------------RIGHT side

    setuptoolbar ();

    //tabs
    tabcontainer = new QTabWidget;
    tabcontainer->setParent (this);
    tabcontainer->setGeometry(315, 70, 780, 645);
    connect (tabcontainer, SIGNAL(currentChanged(int)), this, SLOT(make_tab_visible()));

    //index 0 - notes
    listed = new QWidget ();
    listeditor = new QTextEdit (listed);
    listeditor->setGeometry(5, 5, 765, 590);

    connect(listeditor, SIGNAL(currentCharFormatChanged(QTextCharFormat)), this, SLOT(currentCharFormatChanged(QTextCharFormat)));
    connect(listeditor, SIGNAL(cursorPositionChanged()), this, SLOT(cursorPositionChanged()));
    connect(listeditor, SIGNAL(textChanged()), this, SLOT(save_list()));

    fontChanged(listeditor->font());
    colorChanged(listeditor->textColor());
    alignmentChanged(listeditor->alignment());

    //index 1 - search
    search = new QWidget();

    searchboxlabel = new QLabel;
    searchboxlabel->setParent(search);
    searchboxlabel->setGeometry(10, 10, 100, 35);
    searchboxlabel->setAlignment(Qt::AlignCenter);
    searchboxlabel->setText(tr("Search for"));

    searchtxtbox = new QLineEdit (search);
    searchtxtbox->setGeometry(120, 10, tabcontainer->width()-220, 35);
    searchtxtbox->setToolTip(tr("Enter text to search for (max 50 characters)"));
    searchtxtbox->setMaxLength(50);

    searchbut = new QPushButton (tr("S&earch"), search);
    searchbut->setGeometry(120, 50, 80, 35);
    searchbut->setToolTip(tr("Click to begin search"));
    connect (searchbut, SIGNAL(clicked()), this, SLOT(search_notes()));

    searchclrbut = new QPushButton (tr("C&lear"), search);
    searchclrbut->setGeometry(210, 50, 80, 35);
    searchclrbut->setToolTip(tr("Click to clear search results"));
    connect (searchclrbut, SIGNAL(clicked()), this, SLOT(search_clear_notes()));

    searchresults = new QTextEdit (search);
    searchresults->setReadOnly(true);
    searchresults->setGeometry(5, 90, 765, 505);

    //index 2 - preferences
    prefs = new QWidget();

    fontbut = new QPushButton (tr("Set application fon&t"), prefs);
    fontbut->setGeometry(5, 10, 225, 35);
    fontbut->setToolTip(tr("Click to select a different font for the application (except the notes)"));
    connect (fontbut, SIGNAL(clicked()), this, SLOT(select_font()));

    backupbut = new QPushButton (tr("&Create Backup"), prefs);
    backupbut->setGeometry(5, 50, 225, 35);
    backupbut->setToolTip(tr("Create a copy of the notes data file"));
    connect (backupbut, SIGNAL(clicked()), this, SLOT(backup_notes()));

    //add the tabs to the container
    tabcontainer->insertTab (2, search, tr("&Search"));
    tabcontainer->insertTab (1, prefs, tr("P&references"));
    tabcontainer->insertTab (0, listed, tr("&Notes"));

    //status bar
    statustext = new QLabel (this);
    statustext->setGeometry(10, 725, 300, 35);
    statustext->setText(tr("Status messages appear here"));
    statustext->setFrameStyle(QFrame::Plain);
    statustext->setAlignment(Qt::AlignBottom);

    //search this note text box
    searchthisnotelabel = new QLabel (this);
    searchthisnotelabel->setGeometry(420, 725, 100, 35);
    searchthisnotelabel->setFrameStyle(QFrame::Plain);
    searchthisnotelabel->setAlignment(Qt::AlignBottom);
    searchthisnotelabel->setText(tr("Search for"));

    searchthisnotetextbox = new QLineEdit (this);
    searchthisnotetextbox->setGeometry(525, 720, 350, 35);
    searchthisnotetextbox->setToolTip(tr("Enter text to find (max 50 characters)"));
    searchthisnotetextbox->setMaxLength(50);

    searchthisnotebut = new QPushButton (tr("F&ind"), this);
    searchthisnotebut->setGeometry(880, 720, 80, 35);
    searchthisnotebut->setToolTip(tr("Click to begin search"));
    connect (searchthisnotebut, SIGNAL(clicked()), this, SLOT(search_this_note()));

    checker = new QtSpell::TextEditChecker(this);
    checker->setTextEdit(listeditor);

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
    cur_list = new QTreeWidgetItem ();
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

