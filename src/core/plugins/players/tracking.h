/*
Abstract:
  Tracked modules support

Last changed:
  $Id$

Author:
  (C) Vitamin/CAIG/2001
*/

#ifndef __CORE_PLUGINS_PLAYERS_TRACKING_H_DEFINED__
#define __CORE_PLUGINS_PLAYERS_TRACKING_H_DEFINED__

//local includes
#include <core/plugins/enumerator.h>
//common includes
#include <messages_collector.h>
//library includes
#include <core/module_attrs.h>
#include <core/module_types.h>
#include <sound/render_params.h>// for LoopMode
//std includes
#include <vector>
//boost includes
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

namespace ZXTune
{
  namespace Module
  {
    // Ornament is just a set of tone offsets
    struct SimpleOrnament
    {
      SimpleOrnament() : Loop(), Lines()
      {
      }

      template<class It>
      SimpleOrnament(uint_t loop, It from, It to) : Loop(loop), Lines(from, to)
      {
      }

      uint_t GetLoop() const
      {
        return Loop;
      }

      uint_t GetSize() const
      {
        return Lines.size();
      }

      int_t GetLine(uint_t pos) const
      {
        return Lines.size() > pos ? Lines[pos] : 0;
      }

    private:
      uint_t Loop;
      std::vector<int_t> Lines;
    };

    class TrackModuleData
    {
    public:
      typedef boost::shared_ptr<const TrackModuleData> Ptr;

      virtual ~TrackModuleData() {}

      //static
      virtual uint_t GetChannelsCount() const = 0;
      virtual uint_t GetLoopPosition() const = 0;
      virtual uint_t GetInitialTempo() const = 0;
      virtual uint_t GetPositions() const = 0;
      virtual uint_t GetPatternsCount() const = 0;
      //dynamic
      virtual uint_t GetCurrentPattern(const TrackState& state) const = 0;
      virtual uint_t GetCurrentPatternSize(const TrackState& state) const = 0;
      virtual uint_t GetNewTempo(const TrackState& state) const = 0;
    };

    class TrackStateIterator : public TrackState
    {
    public:
      typedef boost::shared_ptr<TrackStateIterator> Ptr;

      static Ptr Create(Information::Ptr info, TrackModuleData::Ptr data, Analyzer::Ptr analyze);

      virtual void Reset() = 0;

      virtual void ResetPosition() = 0;

      virtual bool NextFrame(uint64_t ticksToSkip, Sound::LoopMode mode) = 0;
    };

    class TrackInfo : public Information
    {
    public:
      typedef boost::shared_ptr<TrackInfo> Ptr;

      virtual void SetLogicalChannels(uint_t channels) = 0;
      virtual void SetModuleProperties(Parameters::Accessor::Ptr props) = 0;

      static Ptr Create(TrackModuleData::Ptr data);
    };

    // Basic template class for tracking support (used as simple parametrized namespace)
    template<uint_t ChannelsCount, class SampleType, class OrnamentType = SimpleOrnament>
    class TrackingSupport
    {
    public:
      // Define common types
      typedef SampleType Sample;
      typedef OrnamentType Ornament;

      struct Command
      {
        Command() : Type(), Param1(), Param2(), Param3()
        {
        }
        Command(uint_t type, int_t p1 = 0, int_t p2 = 0, int_t p3 = 0)
          : Type(type), Param1(p1), Param2(p2), Param3(p3)
        {
        }

        bool operator == (uint_t type) const
        {
          return Type == type;
        }

        uint_t Type;
        int_t Param1;
        int_t Param2;
        int_t Param3;
      };

      typedef std::vector<Command> CommandsArray;

      struct Line
      {
        Line() : Tempo(), Channels()
        {
        }
        //track attrs
        boost::optional<uint_t> Tempo;

        struct Chan
        {
          Chan() : Enabled(), Note(), SampleNum(), OrnamentNum(), Volume(), Commands()
          {
          }

          bool Empty() const
          {
            return !Enabled && !Note && !SampleNum && !OrnamentNum && !Volume && Commands.empty();
          }

          bool FindCommand(uint_t type) const
          {
            return Commands.end() != std::find(Commands.begin(), Commands.end(), type);
          }

          boost::optional<bool> Enabled;
          boost::optional<uint_t> Note;
          boost::optional<uint_t> SampleNum;
          boost::optional<uint_t> OrnamentNum;
          boost::optional<uint_t> Volume;
          CommandsArray Commands;
        };

        typedef boost::array<Chan, ChannelsCount> ChannelsArray;
        ChannelsArray Channels;
      };

      typedef std::vector<Line> Pattern;

      // Holder-related types
      class ModuleData : public TrackModuleData
      {
      public:
        typedef boost::shared_ptr<const ModuleData> Ptr;
        typedef boost::shared_ptr<ModuleData> RWPtr;

        static RWPtr Create()
        {
          return boost::make_shared<ModuleData>();
        }

        ModuleData()
          : LoopPosition(), InitialTempo()
          , Positions(), Patterns(), Samples(), Ornaments()
        {
        }

        virtual uint_t GetChannelsCount() const
        {
          return ChannelsCount;
        }

        virtual uint_t GetLoopPosition() const
        {
          return LoopPosition;
        }

        virtual uint_t GetInitialTempo() const
        {
          return InitialTempo;
        }

        virtual uint_t GetPositions() const
        {
          return Positions.size();
        }

        virtual uint_t GetPatternsCount() const
        {
          return std::count_if(Patterns.begin(), Patterns.end(),
            !boost::bind(&Pattern::empty, _1));
        }

        virtual uint_t GetCurrentPattern(const TrackState& state) const
        {
          return Positions[state.Position()];
        }

        virtual uint_t GetCurrentPatternSize(const TrackState& state) const
        {
          return Patterns[GetCurrentPattern(state)].size();
        }

        virtual uint_t GetNewTempo(const TrackState& state) const
        {
          if (const boost::optional<uint_t>& tempo = Patterns[GetCurrentPattern(state)][state.Line()].Tempo)
          {
            return *tempo;
          }
          return 0;
        }

        uint_t LoopPosition;
        uint_t InitialTempo;
        std::vector<uint_t> Positions;
        std::vector<Pattern> Patterns;
        std::vector<SampleType> Samples;
        std::vector<OrnamentType> Ornaments;
      };
    };

    //helper class to easy parse patterns
    struct PatternCursor
    {
      /*explicit*/PatternCursor(uint_t offset = 0)
        : Offset(offset), Period(), Counter()
      {
      }
      uint_t Offset;
      uint_t Period;
      uint_t Counter;

      void SkipLines(uint_t lines)
      {
        Counter -= lines;
      }

      static bool CompareByOffset(const PatternCursor& lh, const PatternCursor& rh)
      {
        return lh.Offset < rh.Offset;
      }

      static bool CompareByCounter(const PatternCursor& lh, const PatternCursor& rh)
      {
        return lh.Counter < rh.Counter;
      }
    };
  }
}

#endif //__CORE_PLUGINS_PLAYERS_TRACKING_H_DEFINED__
