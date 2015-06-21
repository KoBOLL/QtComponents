#ifndef DIR_ITEM_H__
#define DIR_ITEM_H__

#include <vector>
#include <thread>
#include <future>
#include <mutex>
#include <QObject>
#include <QString>

class DirItem: public QObject
{
  Q_OBJECT

signals:
  void itemsAdded();

public:
  enum DirItemType
  {
    Directory,
    Computer,
    Network,
    Root
  };

  DirItem(const QString& path, DirItem* parent, int row, DirItemType type = Directory,
    QObject* parentObject = nullptr);
  ~DirItem();

  QString data() const;
  DirItem* parent() const;
  bool canFetchMore() const;
  bool hasChilds() const;
  int childCount() const;
  DirItem* child(int id) const;
  void populate();
  int row() const;

private:
  QString _path;
  DirItem* _parent;
  int _row;
  DirItemType _type;
  bool _populating;
  bool _terminatePopulating;

  int _childCount;
  int _unpopulatedCount;

  std::future<void> asyncCall;

  std::vector<QString> _unpopulated;
  std::vector<DirItem*> _childs;

  void _populate();
  void _populateNetwork();

  DirItem(const DirItem&) = delete;
  DirItem& operator= (const DirItem&) = delete;
};

#endif //#ifndef DIR_ITEM_H__
