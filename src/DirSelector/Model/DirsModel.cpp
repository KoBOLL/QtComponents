#include <QDebug>

#include "DirsModel.h"

DirsModel::DirsModel(QObject* parent):
  QAbstractItemModel(parent)
{
  _rootItem = new DirItem("", nullptr, 0, DirItem::Root);
}

DirsModel::~DirsModel()
{
  delete _rootItem;
}

QVariant DirsModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (role != Qt::DisplayRole)
    return QVariant();

  DirItem* item = static_cast<DirItem*>(index.internalPointer());

  return item->data();
}

Qt::ItemFlags DirsModel::flags(const QModelIndex& index) const
{
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant DirsModel::headerData(int section, Qt::Orientation orientation,
  int role /*= Qt::DisplayRole*/) const
{
  return QVariant();
}

QModelIndex DirsModel::index(int row, int column,
  const QModelIndex& parent /*= QModelIndex()*/) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  DirItem* parentItem;

  if (!parent.isValid())
    parentItem = _rootItem;
  else
    parentItem = static_cast<DirItem*>(parent.internalPointer());

  DirItem* childItem = parentItem->child(row);
  if (childItem != nullptr)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

QModelIndex DirsModel::parent(const QModelIndex& index) const
{
  if (!index.isValid())
    return QModelIndex();

  DirItem* childItem = static_cast<DirItem*>(index.internalPointer());
  DirItem* parentItem = childItem->parent();

  if (parentItem == _rootItem)
    return QModelIndex();

  return createIndex(parentItem->row(), 0, parentItem);
}

int DirsModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
  DirItem* parentItem;
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    parentItem = _rootItem;
  else
    parentItem = static_cast<DirItem*>(parent.internalPointer());

  return parentItem->childCount();
}

int DirsModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
  return 1;
}

bool DirsModel::hasChildren(const QModelIndex& parent /*= QModelIndex()*/) const
{
  DirItem* parentItem;
  if (parent.column() > 0)
    return false;

  if (!parent.isValid())
    parentItem = _rootItem;
  else
    parentItem = static_cast<DirItem*>(parent.internalPointer());

  return parentItem->hasChilds();
}

bool DirsModel::canFetchMore(const QModelIndex& parent) const
{
  DirItem* parentItem;
  if (parent.column() > 0)
    return false;

  if (!parent.isValid())
    parentItem = _rootItem;
  else
    parentItem = static_cast<DirItem*>(parent.internalPointer());

  return parentItem->canFetchMore();
}

void DirsModel::fetchMore(const QModelIndex& parent)
{
  DirItem* parentItem;
  if (parent.column() > 0)
    return;

  if (!parent.isValid())
    parentItem = _rootItem;
  else
    parentItem = static_cast<DirItem*>(parent.internalPointer());

  connect(parentItem, &DirItem::itemsAdded, this, &DirsModel::_onDirItemsAdded);
  parentItem->populate();
}

void DirsModel::_onDirItemsAdded()
{
  emit layoutChanged();
  /*if (parent->parent() == nullptr)
  {
    beginInsertRows(QModelIndex(), begin, end);
  }
  else
  {
    QModelIndex index = createIndex(parent->row(), 0, parent);
    beginInsertRows(index, begin, end);
  }
  endInsertRows();*/
}