#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>

#include "Model/DirsModel.h"

class DirsModelView : public QMainWindow
{
  Q_OBJECT

public:
  DirsModelView(QWidget *parent = 0);
  ~DirsModelView();

private:
  QTreeView* _treeView;

  DirsModel model;
};

#endif // MAINWINDOW_H
