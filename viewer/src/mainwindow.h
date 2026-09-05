#ifndef FILE__MAINWINDOW_H
#define FILE__MAINWINDOW_H


#include <QSettings>
#include <QMainWindow>
#include <QApplication>

#include <QTreeWidgetItem>

#include "ui_mainwindow.h"

#include <QFileInfo>

#include "../../src/mtreefile.h"
#include "../../src/mqtreefile.h"

#include "config.h"

using namespace mtreefile;


#ifdef USE_QTREEFILE
typedef MQTreeFileDirNode MxTreeFileDirNode;
typedef MQTreeFileEntryNode MxTreeFileEntryNode;
typedef MQTreeFileNode MxTreeFileNode;
#else
typedef MTreeFileDirNode MxTreeFileDirNode;
typedef MTreeFileEntryNode MxTreeFileEntryNode;
typedef MTreeFileNode MxTreeFileNode;
#endif


class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
    MainWindow(QWidget *parent);
    virtual ~MainWindow();

    void load(QString openFileName);

private:
    void createUi();


    void fillInDirTree();
    void fillInDirTree(QTreeWidgetItem *parentWidgetItem, MxTreeFileNode *fileNode);


    void createExample();

private:
    MxTreeFileEntryNode *getNode(QTreeWidgetItem &item);
    void setupPopupMenu();

public slots:
    void onLoad();
    void onSave();
    void onQuit();
    void onAbout();
    void onTreeItemSelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void onEntryModified(QTreeWidgetItem *item, int column);

    void onAddItem();
    void onRemoveSelected();


    
private:
    Ui_MainWindow m_ui;

#ifdef USE_QTREEFILE
    MQTreeFile m_file;
#else
    MTreeFile m_file;
#endif
    QIcon folderIcon;

    // Popup menu actions
    QAction m_actionAddItemS8;
    QAction m_actionAddItemS32;
    QAction m_actionAddItemString;
    QAction m_actionAddItemDouble;
    QAction m_actionAddItemRemoveSelected;

    MxTreeFileDirNode *m_activeDir;

};

#endif //FILE__MAINWINDOW_H

