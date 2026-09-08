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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextCharFormat>
#include <QFont>
#include <QPushButton>
#include "QtSpell.hpp"

class QMenu;
class QToolBar;
class QAction;
class QFontComboBox;
class QComboBox;
class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;
class QTabWidget;
class QSplitter;
class QWidget;
class QFrame;
class QTextEdit;
class QLabel;
class QLineEdit;
class QTimer;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{

    Q_OBJECT
    QMenu *filemenu, *helpmenu;

    //toolbar and its contents
    QToolBar *tb;
    QAction *actionTextBold, *actionTextColor, *actionTextItalic, *actionTextUnderline;
    QAction *actionAlignLeft, *actionAlignCenter, *actionAlignRight, *actionAlignJustify;
    QAction *actionInsertImage;

    QFontComboBox *comboFont;
    QComboBox *comboSize;

    QTreeWidget *listree;
    QTreeWidgetItem *cur_list, *con_item;
    QPushButton listbut, delist;
    bool listreeempty;

    QTabWidget *tabcontainer;
    QSplitter *mainSplitter;
    QWidget *listed, *search, *prefs;
    QTextEdit *listeditor;

    //search
    QLabel *searchboxlabel;
    QLineEdit *searchtxtbox;
    QPushButton *searchbut;
    QPushButton *searchclrbut;
    QTextEdit *searchresults;
    QString searchtxt;

    QLabel *searchthisnotelabel;
    QLineEdit *searchthisnotetextbox;
    QPushButton *searchthisnotebut;

    //preferences
    QPushButton *fontbut;
    QPushButton *backupbut;
    QComboBox *spelllang;

    QFont curfont;

    QLabel *statustext;
    QLabel *wordcount;


    //stores the path to the data subdirectory
    QString Homepath;

    //data files
    QString Listfilename;
    QString Gnugplfilename;
    QString Helpfilename;

    //data subdirectory for this session
    QString Datadirectory;

    QTimer *autosaveTimer;
    bool dataModified = false;

    void resizeEvent(QResizeEvent *);

    bool writeListsToFile(bool showErrors);
    void autosave();
    void updateWordCount();
    void clearHighlights();

    static QSize defaultWindowSize();

    //Q_OBJECT

    QtSpell::TextEditChecker* checker;

public:
    explicit MainWindow(const QString &datadirectory, QWidget *parent = nullptr);
    ~MainWindow();
    friend class ComboBoxItemDelegate;

public slots:
    //menu
    void save_lists_as_text();
    void save_and_quit();

    void help ();
    void about ();

    //toolbar
    void setuptoolbar();

    void textBold();
    void textItalic();
    void textUnderline();
    void textAlign(QAction*);
    void textColor();

    void currentCharFormatChanged(const QTextCharFormat &format);
    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);
    void alignmentChanged(Qt::Alignment a);
    void cursorPositionChanged();

    void textFamily(const QFont &f);
    void textSize(int i);

    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);

    void insertImage();

    //lists
    void show_list ();
    void save_list();

    void add_list();
    void del_list();
    void modify_name (QTreeWidgetItem *it);
    void set_list(QTreeWidgetItem *);

    void read_lists ();
    void write_lists ();

    void show_the_font();

    //search
    void search_notes();
    void search_clear_notes();

    void search_this_note();

    //preferences
    void select_font();
    void writeprefs();
    void readprefs();
    void backup_notes();

    //other functions
    void save_other_data();
    void make_tab_visible();
    void initialize ();

    void closeEvent(QCloseEvent *event);

};

#endif // MAINWINDOW_H
