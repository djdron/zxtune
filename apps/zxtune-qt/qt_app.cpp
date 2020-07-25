/**
* 
* @file
*
* @brief QT application implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "singlemode.h"
#include "ui/factory.h"
#include "ui/utils.h"
#include "supp/options.h"
//common includes
#include <contract.h>
//library includes
#include <platform/application.h>
#include <platform/version/api.h>
//qt includes
#include <QtGui/QApplication>
//std includes
#include <utility>
//text includes
#include "text/text.h"

namespace
{
  class QTApplication : public Platform::Application
  {
  public:
    QTApplication()
    {
    }

    int Run(Strings::Array argv) override
    {
      int fakeArgc = 1;
      char* fakeArgv[] = {""};
      QApplication qapp(fakeArgc, fakeArgv);
      //storageLocation(DataLocation) is ${profile}/[${organizationName}/][${applicationName}/]
      //applicationName cannot be empty since qt4.8.7 (binary name is used instead)
      //So, do not set  organization name and override application name
      qapp.setApplicationName(QLatin1String(Text::PROJECT_NAME));
      qapp.setApplicationVersion(ToQString(Platform::Version::GetProgramVersionString()));
      qapp.setOrganizationDomain(QLatin1String(Text::PROGRAM_SITE));
      const Parameters::Container::Ptr params = GlobalOptions::Instance().Get();
      const SingleModeDispatcher::Ptr mode = SingleModeDispatcher::Create(params, std::move(argv));
      if (mode->StartMaster()) {
        const MainWindow::Ptr win = WidgetsFactory::Instance().CreateMainWindow(params);
        Require(win->connect(mode, SIGNAL(OnSlaveStarted(const QStringList&)), SLOT(SetCmdline(const QStringList&))));
        win->SetCmdline(mode->GetCmdline());
        return qapp.exec();
      } else {
        return 0;
      }
    }
  };
}

namespace Platform
{
  std::unique_ptr<Application> Application::Create()
  {
    return std::unique_ptr<Application>(new QTApplication());
  }
}
