/*
Abstract:
  Z80 settings widget implementation

Last changed:
  $Id$

Author:
  (C) Vitamin/CAIG/2001

  This file is a part of zxtune-qt application based on zxtune library
*/

//local includes
#include "z80.h"
#include "z80.ui.h"
#include "supp/options.h"
#include "ui/utils.h"
#include "ui/tools/parameters_helpers.h"
//library includes
#include <core/core_parameters.h>

namespace
{
  class Z80OptionsWidget : public UI::Z80SettingsWidget
                         , public Ui::Z80Options
  {
  public:
    explicit Z80OptionsWidget(QWidget& parent)
      : UI::Z80SettingsWidget(parent)
      , Options(GlobalOptions::Instance().Get())
    {
      //setup self
      setupUi(this);

      using namespace Parameters;
      BigIntegerValue::Bind(*clockRateValue, *Options, ZXTune::Core::Z80::CLOCKRATE, ZXTune::Core::Z80::CLOCKRATE_DEFAULT);
      IntegerValue::Bind(*intDurationValue, *Options, ZXTune::Core::Z80::INT_TICKS, ZXTune::Core::Z80::INT_TICKS_DEFAULT);
    }
  private:
    const Parameters::Container::Ptr Options;
  };
}

namespace UI
{
  Z80SettingsWidget::Z80SettingsWidget(QWidget &parent)
    : QWidget(&parent)
  {
  }

  Z80SettingsWidget* Z80SettingsWidget::Create(QWidget &parent)
  {
    return new Z80OptionsWidget(parent);
  }
}
