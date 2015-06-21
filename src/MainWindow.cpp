#include "MainWindow.h"

DirsModelView::DirsModelView(QWidget *parent):
  QMainWindow(parent),
  model(this)
{
  _treeView = new QTreeView(this);
  _treeView->setSelectionMode(QAbstractItemView::SingleSelection);
  _treeView->setProperty("uniformRowHeights", true);
  _treeView->setModel(&model);

  setCentralWidget(_treeView);
}

DirsModelView::~DirsModelView()
{

}