#include "mainwindow.h"

#include <assert.h>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

#define ENABLE_DEBUGMSG

#include "log.h"

#include "util.h"
#include "qtutil.h"
#include "aboutdialog.h"


enum
{
  COLUMN_IDX_NAME = 0,
COLUMN_IDX_TYPE = 1,
    COLUMN_IDX_VALUE = 2
};
//-------------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------------


MainWindow::MainWindow(QWidget *parent)
     : QMainWindow(parent)
      ,folderIcon(":/icons/folder.png")
    ,m_actionAddItemS8("Add S8")
    ,m_actionAddItemS32("Add S32")
    ,m_actionAddItemString("Add string")
    ,m_actionAddItemDouble("Add double")
    ,m_actionAddItemRemoveSelected("Remove selected")
    ,m_activeDir(nullptr)

{
    m_ui.setupUi(this);


    createUi();

   // createExample();

    setupPopupMenu();


  //  m_file.save("tmp.bin");


}


void MainWindow::setupPopupMenu()
{
    // Create right click context menu
    QTreeWidget *treeWidget = m_ui.treeWidget_list;
    treeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    treeWidget->addAction(&m_actionAddItemS8);
    treeWidget->addAction(&m_actionAddItemS32);
    treeWidget->addAction(&m_actionAddItemString);
    treeWidget->addAction(&m_actionAddItemDouble);
    treeWidget->addAction(&m_actionAddItemRemoveSelected);

    connect(&m_actionAddItemS8, &QAction::triggered, this, &MainWindow::onAddItem);
    connect(&m_actionAddItemS32, &QAction::triggered, this, &MainWindow::onAddItem);
    connect(&m_actionAddItemString, &QAction::triggered, this, &MainWindow::onAddItem);
    connect(&m_actionAddItemDouble, &QAction::triggered, this, &MainWindow::onAddItem);
    connect(&m_actionAddItemRemoveSelected, &QAction::triggered, this, &MainWindow::onRemoveSelected);

}



void MainWindow::onAddItem()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) return;

    QString name = QInputDialog::getText(
        this,
        "Item name",
        "Please enter name of entry"
        );

    if(name.isEmpty())
return;


#ifdef USE_QTREEFILE
    MxTreeFileEntryNode *node = new MxTreeFileEntryNode(name);
#else
    MxTreeFileEntryNode *node = new MxTreeFileEntryNode(name.toStdString());
#endif
    m_activeDir->addChild(node);

    if(action == &m_actionAddItemS8)
    {
                                  node->setS8(0);
    }
    else if(action == &m_actionAddItemS32)
    {
        node->setS32(0);
    }
    else if(action == &m_actionAddItemString)
    {
        node->setString(std::string(""));
    }
    else if(action == &m_actionAddItemDouble)
    {
        node->setDouble(0.0);
    }
    else
    {
        assert(0);
    }

    onTreeItemSelectionChanged(m_ui.treeWidget_tree->currentItem(), m_ui.treeWidget_tree->currentItem());

}

void MainWindow::onRemoveSelected()
{
    assert(m_activeDir != nullptr);

    for(QTreeWidgetItem* item : m_ui.treeWidget_list->selectedItems())
    {
        MxTreeFileEntryNode *node = getNode(*item);
        m_activeDir->removeChild(node);
        delete item;
    }

}

void MainWindow::createExample()
{

    MxTreeFileEntryNode *node = m_file.createNode("/dir1/dir1.2/example1");
    node->setDouble(1.2);

    node = m_file.createNode("/root_string");
    node->setString("string2");

    node = m_file.createNode("/dir1/dir1.2/example2_dbl");
    node->setDouble(1.3);
    node = m_file.createNode("/dir2/dir1.1/example2");
    node->setDouble(1.3);

    node = m_file.createNode("/dir2/dir1.1/entry__string");
    node->setString("test_string");

    node = m_file.createNode("/dir2/dir1.1/test_i8");
    node->setS8(66);


    node = m_file.createNode("/dir2/dir1.1/test_array_entry");
    node->setArrayVoid("test_array",11);

    fillInDirTree();

}

MxTreeFileEntryNode *MainWindow::getNode(QTreeWidgetItem &item)
{
    void *raw =  item.data(0, Qt::UserRole).value<void*>();
    return static_cast<MxTreeFileEntryNode*>(raw);
}


void MainWindow::onEntryModified(QTreeWidgetItem *item, int column)
{
    MxTreeFileEntryNode *node = getNode(*item);

    if(column == COLUMN_IDX_NAME)
    {
        QString newName = item->text(column);

#ifdef USE_QTREEFILE
        node->setName(newName);
#else
        node->setName(newName.toStdString());
#endif
        return;
    }

    if(column != COLUMN_IDX_VALUE)
        return;

    QString s = item->text(column);

    if(node)
    {
        switch(node->getType())
        {
        default: assert(0);break;
        case MxTreeFileEntryNode::TYPE_S8: node->setS8(s.toInt());break;
        case MxTreeFileEntryNode::TYPE_S32: node->setS32(s.toInt());break;
        case MxTreeFileEntryNode::TYPE_S64: node->setS64(s.toLongLong());break;
        case MxTreeFileEntryNode::TYPE_STRING: node->setString(s.toStdString());break;
        case MxTreeFileEntryNode::TYPE_DOUBLE:
        {
            if(s.contains(","))
                s = s.replace(",",".");
            node->setDouble(s.toDouble());
        };break;
        case MxTreeFileEntryNode::TYPE_ARRAY: {};break;

        }
    }

}

void MainWindow::fillInDirTree(QTreeWidgetItem *parentWidgetItem, MxTreeFileNode *fileNode)
{
    assert(fileNode != nullptr);

    if(!fileNode->isDir())
        return;
    MxTreeFileDirNode *treeNode = fileNode->asDir();

    QTreeWidgetItem *child11 = new QTreeWidgetItem(parentWidgetItem);
   // if(fileNode->getName() == "")
   //     child11->setText(0, "/");
   // else

#ifdef USE_QTREEFILE
    child11->setText(0, fileNode->getName());
#else
    child11->setText(0, QString::fromStdString(fileNode->getName()));
#endif
    child11->setIcon(0, folderIcon);
    child11->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<void*>(fileNode)));

    {
        for(MxTreeFileNode* childNode : treeNode->getChildren())
    {
        fillInDirTree(child11, childNode);
    }
    }
}

void MainWindow::fillInDirTree()
{
   m_ui.treeWidget_tree->blockSignals(true);
    m_ui.treeWidget_tree->clear();
   onTreeItemSelectionChanged(nullptr,nullptr);
    m_ui.treeWidget_tree->blockSignals(false);

    if(m_file.getRoot())
        fillInDirTree(m_ui.treeWidget_tree->invisibleRootItem(),m_file.getRoot());

    m_ui.treeWidget_tree->expandAll();
}

void MainWindow::createUi()
{


// Apply special style for read only QPlainTextEdit
setStyleSheet("QPlainTextEdit[readOnly=\"true\"] {"
             // "color: #808080;"
              "background-color: #F0F0F0;"
              "border: 1px solid #B0B0B0;"
              "border-radius: 2px;}");



    connect(m_ui.actionQuit, SIGNAL(triggered()), SLOT(onQuit()));
    connect(m_ui.actionAbout, SIGNAL(triggered()), SLOT(onAbout()));
    connect(m_ui.actionLoad, SIGNAL(triggered()), SLOT(onLoad()));
    connect(m_ui.actionSave, SIGNAL(triggered()), SLOT(onSave()));



    // --- Left: Hierarchical Tree ---
    QTreeWidget *treeWidget = m_ui.treeWidget_tree;

    treeWidget->setHeaderLabel("Tree Structure");
 treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);

  //  QStyle *style = QApplication::style();
    //QIcon folderIcon = style->standardIcon(QStyle::SP_DirIcon);
//     QIcon fileIcon   = style->standardIcon(QStyle::SP_FileIcon);



            // ----- Connect selection change -----
            QObject::connect(treeWidget, &QTreeWidget::currentItemChanged, this, &MainWindow::onTreeItemSelectionChanged);

// --- Right: Simple List (also QTreeWidget) ---
    QTreeWidget *listWidget = m_ui.treeWidget_list;


    listWidget->setColumnCount(3);
    QStringList headers;
    headers << "Name" << "Type" << "Value";
    listWidget->setHeaderLabels(headers);


    listWidget->setColumnWidth(0, 140);
    listWidget->setColumnWidth(1, 50);
    listWidget->setColumnWidth(2, 100);

    connect(listWidget, &QTreeWidget::itemChanged, this, &MainWindow::onEntryModified);




}


MainWindow::~MainWindow()
{

}

void MainWindow::onQuit()
{
    QApplication::instance()->quit();
}


/**
 * @brief Called when user presses "Help->About". Shows the about box.
 */
void
MainWindow::onAbout()
{
    AboutDialog dlg(this);
    dlg.exec();
}

void
MainWindow::onLoad()
{
    // Open file dialog for selecting a file to open
    QString openFileName = QFileDialog::getOpenFileName(
        this,
        "Select File to Open",
        QString(), // starting directory (empty = default)
        "All Files (*.*)"
        );

    if (openFileName.isEmpty()) {
        qDebug() << "No file selected to open.";
        return;
    }

    load(openFileName);
}

void MainWindow::load(QString openFileName)
{
    int rc;
#ifdef USE_QTREEFILE
    rc = m_file.load(openFileName);
#else
    rc = m_file.load(openFileName.toStdString());
#endif
    if(rc)
    {
        QMessageBox::critical(
            this,
            tr("Error"),
            tr("Could not open file:\n%1").arg(openFileName)
            );
    }
    fillInDirTree();


}


void
MainWindow::onSave()
{
    // Open file dialog for selecting a file to save
    QString saveFileName = QFileDialog::getSaveFileName(
        this,
        "Select File to Save As",
        QString(), // starting directory (empty = default)
        "All Files (*.*)"
        );

    if (saveFileName.isEmpty()) {
        qDebug() << "No file selected to save.";
        return;
    }
#ifdef USE_QTREEFILE
    m_file.save(saveFileName);
#else
    m_file.save(saveFileName.toStdString());
#endif

}

void MainWindow::onTreeItemSelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous);
    QTreeWidget *listWidget = m_ui.treeWidget_list;

    if(current == nullptr)
    {
        listWidget->clear();
        return;
    }

    MxTreeFileDirNode *node = static_cast<MxTreeFileDirNode*>(current->data(0, Qt::UserRole).value<void*>());
    assert(node->isDir());


    m_activeDir = node;



    listWidget->clear();


    listWidget->blockSignals(true);

    for(MxTreeFileNode* childNode : node->getChildren())
    {

        if(childNode->isDir())
            continue;
        MxTreeFileEntryNode* nodeEntry = childNode->asEntry();
#ifdef USE_QTREEFILE
        QString name = nodeEntry->getName();
        QString value;
        nodeEntry->getString(&value);
#else
        QString name = QString::fromStdString(nodeEntry->getName());
        std::string s;
        nodeEntry->getString(&s);
        QString value = QString::fromStdString(s);
#endif


        QTreeWidgetItem *item = new QTreeWidgetItem(listWidget);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setText(0, name);
        item->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<void*>(nodeEntry)));

        QString typeStr = "?";
        switch(nodeEntry->getType())
        {
        default: assert(0);break;
        case MxTreeFileEntryNode::TYPE_S8: typeStr = "S8";break;
        case MxTreeFileEntryNode::TYPE_S32:  typeStr = "S32";break;
        case MxTreeFileEntryNode::TYPE_S64:  typeStr = "S64";break;
        case MxTreeFileEntryNode::TYPE_STRING:  typeStr = "String";break;
        case MxTreeFileEntryNode::TYPE_DOUBLE: typeStr = "Double";break;
        case MxTreeFileEntryNode::TYPE_ARRAY:  typeStr = "Array";break;

        }

        item->setText(1, typeStr);
        item->setText(2, value);
    }

        listWidget->blockSignals(false);
}



