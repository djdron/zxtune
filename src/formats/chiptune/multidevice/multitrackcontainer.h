/**
* 
* @file
*
* @brief  Multitrack container support interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//common includes
#include <types.h>
//library includes
#include <formats/chiptune.h>

namespace Formats
{
  namespace Chiptune
  {
    namespace MultiTrackContainer
    {
      class Builder
      {
      public:
        virtual ~Builder() = default;

        /*
          Can be called to any of the entity (Module -> Track)
          Workflow:
            SetProperty() <- global props
            StartTrack(0)
            SetProperty() <- track0 props
            SetData()
            SetProperty() <- track0_data0 props
            SetData()
            SetProperty() <- track0_data1 props
            StartTrack(1)
            SetProperty() <- track1 props
            ...
        */
        virtual void SetAuthor(const String& author) = 0;
        virtual void SetTitle(const String& title) = 0;
        virtual void SetAnnotation(const String& annotation) = 0;
        //arbitrary property
        virtual void SetProperty(const String& name, const String& value) = 0;

        virtual void StartTrack(uint_t idx) = 0;
        virtual void SetData(Binary::Container::Ptr data) = 0;
      };

      Builder& GetStubBuilder();

      class ContainerBuilder : public Builder
      {
      public:
        typedef std::shared_ptr<ContainerBuilder> Ptr;
        virtual Binary::Data::Ptr GetResult() = 0;
      };

      ContainerBuilder::Ptr CreateBuilder();
      Formats::Chiptune::Container::Ptr Parse(const Binary::Container& data, Builder& target);
    }

    Decoder::Ptr CreateMultiTrackContainerDecoder();
  }
}
