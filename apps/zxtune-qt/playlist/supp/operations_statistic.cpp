/**
* 
* @file
*
* @brief Playlist statistic operations implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "operations_helpers.h"
#include "operations_statistic.h"
#include "storage.h"
//common includes
#include <make_ptr.h>

namespace
{
  class CollectStatisticOperation : public Playlist::Item::TextResultOperation
                                  , private Playlist::Item::Visitor
  {
  public:
    explicit CollectStatisticOperation(Playlist::Item::StatisticTextNotification::Ptr result)
      : SelectedItems()
      , Result(result)
    {
    }

    CollectStatisticOperation(Playlist::Model::IndexSet::Ptr items, Playlist::Item::StatisticTextNotification::Ptr result)
      : SelectedItems(items)
      , Result(result)
    {
    }

    virtual void Execute(const Playlist::Item::Storage& stor, Log::ProgressCallback& cb)
    {
      ExecuteOperation(stor, SelectedItems, *this, cb);
      emit ResultAcquired(Result);
    }
  private:
    virtual void OnItem(Playlist::Model::IndexType /*index*/, Playlist::Item::Data::Ptr data)
    {
      //check for the data first to define is data valid or not
      const String type = data->GetType();
      if (data->GetState())
      {
        Result->AddInvalid();
      }
      else
      {
        assert(!type.empty());
        Result->AddValid();
        Result->AddType(type);
        Result->AddDuration(data->GetDuration());
        Result->AddSize(data->GetSize());
        Result->AddPath(data->GetFilePath());
      }
    }
  private:
    const Playlist::Model::IndexSet::Ptr SelectedItems;
    const Playlist::Item::StatisticTextNotification::Ptr Result;
  };
}

namespace Playlist
{
  namespace Item
  {
    TextResultOperation::Ptr CreateCollectStatisticOperation(StatisticTextNotification::Ptr result)
    {
      return MakePtr<CollectStatisticOperation>(result);
    }

    TextResultOperation::Ptr CreateCollectStatisticOperation(Playlist::Model::IndexSet::Ptr items, StatisticTextNotification::Ptr result)
    {
      return MakePtr<CollectStatisticOperation>(items, result);
    }
  }
}
