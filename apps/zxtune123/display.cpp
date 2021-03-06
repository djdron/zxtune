/**
* 
* @file
*
* @brief Display component implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "console.h"
#include "display.h"
//common includes
#include <error.h>
//library includes
#include <module/track_state.h>
#include <parameters/template.h>
#include <platform/application.h>
#include <strings/format.h>
#include <strings/template.h>
#include <time/duration.h>
#include <time/serialize.h>
//std includes
#include <thread>
//boost includes
#include <boost/program_options.hpp>
//text includes
#include "text/text.h"

namespace
{
  //layout constants
  //TODO: make dynamic calculation
  const std::size_t INFORMATION_HEIGHT = 5;
  const std::size_t TRACKING_HEIGHT = 3;
  const std::size_t PLAYING_HEIGHT = 2;

  inline void ShowTrackingStatus(const Module::TrackState& state)
  {
    const String& dump = Strings::Format(Text::TRACKING_STATUS,
      state.Position(), state.Pattern(),
      state.Line(), state.Quirk(),
      state.Channels(), state.Tempo());
    assert(TRACKING_HEIGHT == static_cast<std::size_t>(std::count(dump.begin(), dump.end(), '\n')));
    StdOut << dump;
  }

  inline Char StateSymbol(Sound::PlaybackControl::State state)
  {
    switch (state)
    {
    case Sound::PlaybackControl::STARTED:
      return '>';
    case Sound::PlaybackControl::PAUSED:
      return '#';
    default:
      return '\?';
    }
  }

  class DisplayComponentImpl : public DisplayComponent
  {
  public:
    DisplayComponentImpl()
      : Options(Text::DISPLAY_SECTION)
      , Silent(false)
      , Quiet(false)
      , ShowAnalyze(false)
      , Updatefps(10)
      , InformationTemplate(Strings::Template::Create(Text::ITEM_INFO))
      , ScrSize(Console::Self().GetSize())
      , TotalFrames(0)
      , FrameDuration()
    {
      using namespace boost::program_options;
      Options.add_options()
        (Text::SILENT_KEY, bool_switch(&Silent), Text::SILENT_DESC)
        (Text::QUIET_KEY, bool_switch(&Quiet), Text::QUIET_DESC)
        (Text::ANALYZER_KEY, bool_switch(&ShowAnalyze), Text::ANALYZER_DESC)
        (Text::UPDATEFPS_KEY, value<uint_t>(&Updatefps), Text::UPDATEFPS_DESC)
      ;
    }

    const boost::program_options::options_description& GetOptionsDescription() const override
    {
      return Options;
    }

    void Message(const String& msg) override
    {
      if (!Silent)
      {
        Console::Self().Write(msg);
        StdOut << std::endl;
      }
    }

    void SetModule(Module::Holder::Ptr module, Sound::Backend::Ptr player, Time::Microseconds frameDuration) override
    {
      const Module::Information::Ptr info = module->GetModuleInformation();
      const Parameters::Accessor::Ptr props = module->GetModuleProperties();
      TotalFrames = info->FramesCount();
      FrameDuration = frameDuration;
      State = player->GetState();
      TrackState = dynamic_cast<const Module::TrackState*>(State.get());
      if (!Silent && ShowAnalyze)
      {
        Analyzer = player->GetAnalyzer();
      }
      else
      {
        Analyzer.reset();
      }

      if (Silent)
      {
        return;
      }
      Message(InformationTemplate->Instantiate(Parameters::FieldsSourceAdapter<Strings::FillFieldsSource>(*props)));
      Message(Strings::Format(Text::ITEM_INFO_ADDON, Time::ToString(FrameDuration * info->FramesCount()), info->ChannelsCount()));
    }

    uint_t BeginFrame(Sound::PlaybackControl::State state) override
    {
      const uint_t curFrame = State->Frame();
      if (Silent || Quiet)
      {
        return curFrame;
      }
      ScrSize = Console::Self().GetSize();
      if (ScrSize.first <= 0 || ScrSize.second <= 0)
      {
        Silent = true;
        return curFrame;
      }
      const int_t trackingHeight = TrackState ? TRACKING_HEIGHT : 0;
      const int_t spectrumHeight = ScrSize.second - INFORMATION_HEIGHT - trackingHeight - PLAYING_HEIGHT - 1;
      if (spectrumHeight < 4)//minimal spectrum height
      {
        Analyzer.reset();
      }
      else if (ScrSize.second < int_t(trackingHeight + PLAYING_HEIGHT))
      {
        Quiet = true;
      }
      else
      {
        if (TrackState)
        {
          ShowTrackingStatus(*TrackState);
        }
        ShowPlaybackStatus(curFrame, state);
        if (Analyzer)
        {
          const auto& curAnalyze = Analyzer->GetState();
          AnalyzerData.resize(ScrSize.first);
          UpdateAnalyzer(curAnalyze, 10);
          ShowAnalyzer(spectrumHeight);
        }
      }
      return curFrame;
    }

    void EndFrame() override
    {
      const uint_t waitPeriod(std::max<uint_t>(1, 1000 / std::max<uint_t>(Updatefps, 1)));
      std::this_thread::sleep_for(std::chrono::milliseconds(waitPeriod));
      if (!Silent && !Quiet)
      {
        const int_t trackingHeight = TrackState ? TRACKING_HEIGHT : 0;
        Console::Self().MoveCursorUp(Analyzer ? ScrSize.second - INFORMATION_HEIGHT - 1 : trackingHeight + PLAYING_HEIGHT);
      }
    }
  private:
    void ShowPlaybackStatus(uint_t frame, Sound::PlaybackControl::State state) const
    {
      const Char MARKER = '\x1';
      String data = Strings::Format(Text::PLAYBACK_STATUS, Time::ToString(FrameDuration * frame), MARKER);
      const String::size_type totalSize = data.size() - 1 - PLAYING_HEIGHT;
      const String::size_type markerPos = data.find(MARKER);

      String prog(ScrSize.first - totalSize, '-');
      const std::size_t pos = frame * (ScrSize.first - totalSize) / TotalFrames;
      prog[pos] = StateSymbol(state);
      data.replace(markerPos, 1, prog);
      assert(PLAYING_HEIGHT == static_cast<std::size_t>(std::count(data.begin(), data.end(), '\n')));
      StdOut << data << std::flush;
    }

    void ShowAnalyzer(uint_t high)
    {
      const std::size_t width = AnalyzerData.size();
      std::string buffer(width, ' ');
      for (int_t y = high; y; --y)
      {
        const int_t limit = (y - 1) * 100 / high;
        std::transform(AnalyzerData.begin(), AnalyzerData.end(), buffer.begin(),
          [limit](const int_t val) {return val > limit ? '#' : ' ';});
        StdOut << buffer << '\n';
      }
      StdOut << std::flush;
    }

    void UpdateAnalyzer(const Module::Analyzer::SpectrumState& inState, int_t fallspeed)
    {
      for (uint_t band = 0, lim = std::min(AnalyzerData.size(), inState.Data.size()); band < lim; ++band)
      {
        AnalyzerData[band] = std::max(AnalyzerData[band] - fallspeed, int_t(inState.Data[band].Raw()));
      }
    }

  private:
    boost::program_options::options_description Options;
    bool Silent;
    bool Quiet;
    bool ShowAnalyze;
    uint_t Updatefps;
    const Strings::Template::Ptr InformationTemplate;
    //context
    Console::SizeType ScrSize;
    uint_t TotalFrames;
    Time::Microseconds FrameDuration;
    Module::State::Ptr State;
    const Module::TrackState* TrackState;
    Module::Analyzer::Ptr Analyzer;
    std::vector<int_t> AnalyzerData;
  };
}

DisplayComponent::Ptr DisplayComponent::Create()
{
  return DisplayComponent::Ptr(new DisplayComponentImpl);
}
