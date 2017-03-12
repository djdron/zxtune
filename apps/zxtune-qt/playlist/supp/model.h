/**
* 
* @file
*
* @brief Playlist model interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//local includes
#include "data.h"
#include "playlist/io/container.h"
//common includes
#include <progress_callback.h>
//std includes
#include <map>
#include <set>
//qt includes
#include <QtCore/QAbstractItemModel>

namespace Playlist
{
  namespace Item
  {
    class Storage;

    class StorageAccessOperation
    {
    public:
      typedef std::shared_ptr<StorageAccessOperation> Ptr;
      virtual ~StorageAccessOperation() = default;

      virtual void Execute(const Storage& storage, Log::ProgressCallback& cb) = 0;
    };

    class StorageModifyOperation
    {
    public:
      typedef std::shared_ptr<StorageModifyOperation> Ptr;
      virtual ~StorageModifyOperation() = default;

      virtual void Execute(Storage& storage, Log::ProgressCallback& cb) = 0;
    };
  }

  class Model : public QAbstractItemModel
  {
    Q_OBJECT
  protected:
    explicit Model(QObject& parent);
  public:
    enum Columns
    {
      COLUMN_TYPE,
      COLUMN_DISPLAY_NAME,
      COLUMN_DURATION,
      COLUMN_AUTHOR,
      COLUMN_TITLE,
      COLUMN_COMMENT,
      COLUMN_PATH,
      COLUMN_SIZE,
      COLUMN_CRC,
      COLUMN_FIXEDCRC,

      COLUMNS_COUNT
    };

    typedef Model* Ptr;
    typedef unsigned IndexType;
    class IndexSet : public std::set<IndexType>
    {
    public:
      typedef std::shared_ptr<const IndexSet> Ptr;
      typedef std::shared_ptr<IndexSet> RWPtr;
    };

    class OldToNewIndexMap : public std::map<IndexType, IndexType>
    {
    public:
      typedef std::shared_ptr<const OldToNewIndexMap> Ptr;
      typedef std::shared_ptr<OldToNewIndexMap> RWPtr;
      
      //! Finds new index after remapping
      const IndexType* FindNewIndex(IndexType oldIdx) const;
      //! Tryes to search any suitable mapping
      const IndexType* FindNewSuitableIndex(IndexType oldIdx) const;
    };

    //creator
    static Ptr Create(QObject& parent);

    virtual void PerformOperation(Item::StorageAccessOperation::Ptr operation) = 0;
    virtual void PerformOperation(Item::StorageModifyOperation::Ptr operation) = 0;
    virtual void WaitOperationFinish() = 0;

    //accessors
    virtual unsigned CountItems() const = 0;
    virtual Item::Data::Ptr GetItem(IndexType index) const = 0;
    virtual QStringList GetItemsPaths(const IndexSet& items) const = 0;
    virtual unsigned GetVersion() const = 0;

    //modifiers
    virtual void Clear() = 0;
    virtual void MoveItems(const IndexSet& items, IndexType target) = 0;
  public slots:
    virtual void AddItem(Playlist::Item::Data::Ptr item) = 0;
    virtual void AddItems(Playlist::Item::Collection::Ptr items) = 0;
    virtual void RemoveItems(Playlist::Model::IndexSet::Ptr items) = 0;
    virtual void CancelLongOperation() = 0;
  signals:
    void IndicesChanged(Playlist::Model::OldToNewIndexMap::Ptr);
    void OperationStarted();
    void OperationProgressChanged(int);
    void OperationStopped();
  };
}
