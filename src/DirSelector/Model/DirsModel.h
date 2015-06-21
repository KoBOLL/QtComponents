#ifndef DIRS_MODEL_H__
#define DIRS_MODEL_H__

#include <QAbstractItemModel>
#include <QPixmap>
#include "DirItem.h"

class DirsModel: public QAbstractItemModel
{
  Q_OBJECT

public:
  DirsModel(QObject* parent);
  virtual ~DirsModel();

  QVariant data(const QModelIndex& index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
    int role = Qt::DisplayRole) const override;

  QModelIndex index(int row, int column,
    const QModelIndex& parent = QModelIndex()) const override;

  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;

  bool canFetchMore(const QModelIndex& parent) const override;
  void fetchMore(const QModelIndex& parent) override;

private:
  DirItem* _rootItem;
  std::map<DirItem::DirItemType, QPixmap> _icons;

  void _initIconsCashe();
  void _onDirItemsAdded();

  DirsModel(const DirsModel&) = delete;
  DirsModel& operator= (const DirsModel&) = delete;
};

#endif //DIRS_MODEL_H__