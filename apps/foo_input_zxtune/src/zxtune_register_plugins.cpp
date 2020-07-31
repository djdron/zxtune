/*
ZXTune foobar2000 decoder component by djdron (C) 2013 - 2020

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <core/plugins/archives/plugins.h>
#include <core/plugins/players/plugins_list.h>

namespace ZXTune
{

void RegisterArchivePlugins(ArchivePluginsRegistrator& registrator)
{
	//process raw container first
	RegisterRawContainer(registrator);
//	RegisterArchiveContainers(registrator);
	RegisterZXArchiveContainers(registrator);
	//process containers last
	RegisterMultitrackContainers(registrator);
//	RegisterZdataContainer(registrator);

//packed
	RegisterDepackPlugins(registrator);
	RegisterChiptunePackerPlugins(registrator);
	RegisterDecompilePlugins(registrator);
}

//non-filtered plugins list to display in preferences
std::vector<PlayerPlugin::Ptr> player_plugins;
bool PlayerPluginEnabled(const PlayerPlugin::Ptr& pp);

class PlayerPluginsFilter : public PlayerPluginsRegistrator
{
public:
	virtual void RegisterPlugin(PlayerPlugin::Ptr plugin)
	{
		player_plugins.push_back(plugin);
	}
	void Filter(PlayerPluginsRegistrator* plugins_filtered)
	{
		for(const auto& p : player_plugins)
		{
			if(PlayerPluginEnabled(p))
			{
				plugins_filtered->RegisterPlugin(p);
			}
		}
	}
};

void RegisterPlayerPlugins(PlayerPluginsRegistrator& registrator)
{
	PlayerPluginsFilter ppf;
	RegisterTSSupport(ppf);
	RegisterAYSupport(ppf);
	RegisterPT3Support(ppf);
	RegisterPT2Support(ppf);
	RegisterSTCSupport(ppf);
	RegisterST1Support(ppf);
	RegisterST3Support(ppf);
	RegisterASCSupport(ppf);
	RegisterSTPSupport(ppf);
	RegisterTXTSupport(ppf);
	RegisterPSGSupport(ppf);
	RegisterPDTSupport(ppf);
	RegisterCHISupport(ppf);
	RegisterSTRSupport(ppf);
	RegisterDSTSupport(ppf);
	RegisterSQDSupport(ppf);
	RegisterDMMSupport(ppf);
	RegisterPSMSupport(ppf);
	RegisterGTRSupport(ppf);
	RegisterPT1Support(ppf);
	RegisterVTXSupport(ppf);
	RegisterYMSupport(ppf);
	RegisterTFDSupport(ppf);
	RegisterTFCSupport(ppf);
	RegisterSQTSupport(ppf);
	RegisterPSCSupport(ppf);
	RegisterFTCSupport(ppf);
	RegisterCOPSupport(ppf);
	RegisterTFESupport(ppf);
	RegisterXMPPlugins(ppf);
	RegisterSIDPlugins(ppf);
	RegisterET1Support(ppf);
	RegisterAYCSupport(ppf);
	RegisterSPCSupport(ppf);
	RegisterMTCSupport(ppf);
	RegisterGMEPlugins(ppf);
	RegisterAHXSupport(ppf);
	RegisterPSFSupport(ppf);
	RegisterUSFSupport(ppf);
	RegisterGSFSupport(ppf);
	Register2SFSupport(ppf);
	RegisterSDSFSupport(ppf);
	RegisterASAPPlugins(ppf);
	RegisterV2MSupport(ppf);
	RegisterVGMPlugins(ppf);
	ppf.Filter(&registrator);
}

}
//namespace ZXTune
