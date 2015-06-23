#include <QDebug>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <Shellapi.h>
#else
#include <QApplication>
#include <QStyle>
#endif //Q_OS_WIN

#include "DirsModel.h"

#ifdef Q_OS_WIN
Q_GUI_EXPORT QPixmap qt_pixmapFromWinHICON(HICON icon);

static QPixmap getWinIcon(SHSTOCKICONID id)
{
  QPixmap result;
  SHSTOCKICONINFO sii;
  memset(&sii, 0, sizeof(SHSTOCKICONINFO));
  sii.cbSize = sizeof(SHSTOCKICONINFO);
  SHGetStockIconInfo(id, SHGSI_SMALLICON | SHGSI_ICON, &sii);
  result = qt_pixmapFromWinHICON(sii.hIcon);
  DestroyIcon(sii.hIcon);
  return result;
}
#endif //Q_OS_WIN

DirsModel::DirsModel(QObject* parent):
  QAbstractItemModel(parent)
{
  _initIconsCashe();

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

  if ((role != Qt::DisplayRole) && (role != Qt::DecorationRole))
    return QVariant();

  DirItem* item = static_cast<DirItem*>(index.internalPointer());

  switch (role)
  {
  case Qt::DisplayRole:
    return item->data();
    break;
  case Qt::DecorationRole:
  {
    auto it = _icons.find(item->type());
    if (it != _icons.end())
      return it->second;
  }
    break;
  default:
    break;
  }

  return QVariant();
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

void DirsModel::_initIconsCashe()
{
#ifdef Q_OS_WIN
  _icons.insert({DirItem::Directory, getWinIcon(SIID_FOLDER)});
  _icons.insert({DirItem::HardDrive, getWinIcon(SIID_DRIVEFIXED)});
  _icons.insert({DirItem::FlashDrive, getWinIcon(SIID_DRIVEREMOVE)});
  _icons.insert({DirItem::CDDrive, getWinIcon(SIID_DRIVECD)});
  _icons.insert({DirItem::DVDDrive, getWinIcon(SIID_DRIVEDVD)});
  _icons.insert({DirItem::NetDrive, getWinIcon(SIID_DRIVENET)});
  _icons.insert({DirItem::Computer, getWinIcon(SIID_DESKTOPPC)});
  _icons.insert({DirItem::Network, getWinIcon(SIID_MYNETWORK)});
  _icons.insert({DirItem::NetworkComputer, getWinIcon(SIID_SERVER)});
#else
  QStyle* style = QApplication::style();
  _icons.insert({DirItem::Directory, style->standardPixmap(QStyle::SP_DirClosedIcon)});
  _icons.insert({DirItem::HardDrive, style->standardPixmap(QStyle::SP_DriveHDIcon)});
  _icons.insert({DirItem::FlashDrive, style->standardPixmap(QStyle::SP_DriveFDIcon)});
  _icons.insert({DirItem::CDDrive, style->standardPixmap(QStyle::SP_DriveCDIcon)});
  _icons.insert({DirItem::DVDDrive, style->standardPixmap(QStyle::SP_DriveCDIcon)});
  _icons.insert({DirItem::NetDrive, style->standardPixmap(QStyle::SP_DriveNetIcon)});
  _icons.insert({DirItem::Computer, style->standardPixmap(QStyle::SP_ComputerIcon)});
  _icons.insert({DirItem::Network, style->standardPixmap(QStyle::SP_DirIcon)});
  _icons.insert({DirItem::NetworkComputer,
    style->standardPixmap(QStyle::SP_ComputerIcon)});
#endif //Q_OS_WIN
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
