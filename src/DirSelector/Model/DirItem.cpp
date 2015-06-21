#pragma comment(lib, "netapi32.lib")

#include <QDebug>
#include <QDirIterator>
#include <QDir>

#include <windows.h>
#include <lm.h>

#include "DirItem.h"

DirItem::DirItem(const QString& path, DirItem* parent, int row,
  DirItemType type/*= Directory*/, QObject* parentObject/*= nullptr*/):
  _path(path),
  _parent(parent),
  _row(row),
  _type(type),
  QObject(parentObject),
  _populating(false),
  _terminatePopulating(false),
  _childCount(0),
  _unpopulatedCount(0)
{
  switch (_type)
  {
  case Directory:
  case NetworkComputer:
  case HardDrive:
  case FloppyDrive:
  case CDDrive:
  case DVDDrive:
  case NetDrive:
  {
    QDirIterator it(_path + "/", QDir::AllDirs | QDir::NoDotAndDotDot);
    while (it.hasNext())
    {
      _unpopulated.emplace_back(it.next());
      _unpopulatedCount++;
    }
    break;
  }
  case Computer:
  {
    for (QFileInfo entry : QDir::drives())
    {
      QString drive = entry.absolutePath();
      drive.chop(1);
      _unpopulated.emplace_back(drive);
      _unpopulatedCount++;
    }
    break;
  }
  case Network:
    _unpopulatedCount = 1;
    break;
  case Root:
  {
    _childs.emplace_back(new DirItem("", this, 0, Computer));
    _childs.emplace_back(new DirItem("", this, 1, Network));
    _childCount = 2;
  }
  }
}

DirItem::~DirItem()
{
  _terminatePopulating = true;
  if (asyncCall.valid())
    asyncCall.wait();

  for (DirItem* item : _childs)
  {
    delete item;
    item = nullptr;
  }

  _childs.clear();
}

QString DirItem::data() const
{
  QString result;
  switch (_type)
  {
  case Computer:
    result = "Computer";
    break;
  case Network:
    result = "Network";
    break;
  case Directory:
  default:
    result = _path.section("/", -1, -1);
    break;
  }

  return result;
}

DirItem* DirItem::parent() const
{
  return _parent;
}

bool DirItem::canFetchMore() const
{
  if (_populating)
    return false;

  return _unpopulatedCount > 0;
}

bool DirItem::hasChilds() const
{
  //qDebug() << _childCount << _unpopulatedCount;
  return (_childCount > 0) || (_unpopulatedCount > 0);
}

int DirItem::childCount() const
{
  return _childCount;
}

DirItem* DirItem::child(int id) const
{
  if (id >= _childCount)
    return nullptr;
  else
    return _childs[id];
}

void DirItem::populate()
{
  _populating = true;

  if (_type != Network)
    asyncCall = std::async(std::launch::async, &DirItem::_populate, this);
  else
    asyncCall = std::async(std::launch::async, &DirItem::_populateNetwork, this);
}

int DirItem::row() const
{
  return _row;
}

DirItem::DirItemType DirItem::type() const
{
  return _type;
}

void DirItem::_populate()
{
  int row = 0;

  _childs.reserve(sizeof(DirItem*) * _unpopulated.size());

  for (QString entry : _unpopulated)
  {
    if (_terminatePopulating == true)
    {
      emit itemsAdded();
      break;
    }

    if (_type == Computer)
      _childs.emplace_back(new DirItem(entry, this, row, _getDeviceType(entry)));
    else
      _childs.emplace_back(new DirItem(entry, this, row));
    _unpopulatedCount--;

    if (row % 100 == 0)
    {
      if (row != 0)
      {
        _childCount = row;
        emit itemsAdded();
      }
    }
    row++;
  }

  _populating = false;
  _unpopulated.clear();
  _childCount = row;
  emit itemsAdded();
}

void DirItem::_populateNetwork()
{
  NET_API_STATUS status;
  LPSERVER_INFO_101 pBuf = NULL;
  LPSERVER_INFO_101 pTmpBuf;
  DWORD dwEntriesRead = 0;
  DWORD dwTotalEntries = 0;

  status = NetServerEnum(NULL, 101, (LPBYTE*)&pBuf, MAX_PREFERRED_LENGTH,
    &dwEntriesRead, &dwTotalEntries, SV_TYPE_SERVER, NULL, NULL);
  if ((status == NERR_Success) || (status == ERROR_MORE_DATA))
  {
    if ((pTmpBuf = pBuf) != NULL)
    {
      for (int row = 0; row < dwEntriesRead; ++row)
      {
        if (pTmpBuf == NULL)
          break;

        if (_terminatePopulating == true)
          break;

        //qDebug() << "Platform:" << pTmpBuf->sv101_platform_id;
        //qDebug() << "Name:" << QString::fromWCharArray(pTmpBuf->sv101_name);
        //qDebug() << "Type:" << pTmpBuf->sv101_type;
        QString servName = QString::fromWCharArray(pTmpBuf->sv101_name);
        _childs.emplace_back(new DirItem("//" + servName, this, row, NetworkComputer));
        _childCount = row + 1;
        pTmpBuf++;
        emit itemsAdded();
      }
    }
  }
  if (pBuf != NULL)
    NetApiBufferFree(pBuf);

  //_childs.emplace_back(new DirItem("//DEVSERV1", this, _childCount, Directory));
  //_childCount += 1;

  _unpopulatedCount = 0;
  _populating = false;
}

DirItem::DirItemType DirItem::_getDeviceType(const QString& devicePath)
{
  DirItem::DirItemType result;
  UINT type = GetDriveType((wchar_t *)devicePath.utf16());
  switch (type)
  {
  case DRIVE_REMOVABLE:
    result = FloppyDrive;
    break;
  case DRIVE_FIXED:
    result = HardDrive;
    break;
  case DRIVE_REMOTE:
    result = NetDrive;
    break;
  case DRIVE_CDROM:
    result = CDDrive;
    break;
  case DRIVE_RAMDISK:
  case DRIVE_UNKNOWN:
  case DRIVE_NO_ROOT_DIR:
  default:
    result = Directory;
    break;
  }

  return result;
}
