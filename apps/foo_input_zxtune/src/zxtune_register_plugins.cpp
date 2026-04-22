/*
ZXTune foobar2000 decoder component by djdron (C) 2013 - 2026

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
#include <core/plugins/player_plugins_registrator.h>
#include <core/plugins/archive_plugins_registrator.h>
#include <vector>
#include <algorithm>

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
static std::vector<PlayerPlugin::Ptr> player_plugins;
const std::vector<PlayerPlugin::Ptr>& PlayerPlugins() { return player_plugins; }
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

void RegisterTSSupport(PlayerPluginsRegistrator& players);
void RegisterAYSupport(PlayerPluginsRegistrator& players);
void RegisterPSGSupport(PlayerPluginsRegistrator& players);
void RegisterSTCSupport(PlayerPluginsRegistrator& players);
void RegisterST1Support(PlayerPluginsRegistrator& players);
void RegisterST3Support(PlayerPluginsRegistrator& players);
void RegisterPT2Support(PlayerPluginsRegistrator& players);
void RegisterPT3Support(PlayerPluginsRegistrator& players);
void RegisterASCSupport(PlayerPluginsRegistrator& players);
void RegisterSTPSupport(PlayerPluginsRegistrator& players);
void RegisterTXTSupport(PlayerPluginsRegistrator& players);
void RegisterPDTSupport(PlayerPluginsRegistrator& players);
void RegisterCHISupport(PlayerPluginsRegistrator& players);
void RegisterSTRSupport(PlayerPluginsRegistrator& players);
void RegisterDSTSupport(PlayerPluginsRegistrator& players);
void RegisterSQDSupport(PlayerPluginsRegistrator& players);
void RegisterDMMSupport(PlayerPluginsRegistrator& players);
void RegisterPSMSupport(PlayerPluginsRegistrator& players);
void RegisterGTRSupport(PlayerPluginsRegistrator& players);
void RegisterPT1Support(PlayerPluginsRegistrator& players);
void RegisterVTXSupport(PlayerPluginsRegistrator& players);
void RegisterYMSupport(PlayerPluginsRegistrator& players);
void RegisterTFDSupport(PlayerPluginsRegistrator& players);
void RegisterTFCSupport(PlayerPluginsRegistrator& players);
void RegisterSQTSupport(PlayerPluginsRegistrator& players);
void RegisterPSCSupport(PlayerPluginsRegistrator& players);
void RegisterFTCSupport(PlayerPluginsRegistrator& players);
void RegisterCOPSupport(PlayerPluginsRegistrator& players);
void RegisterTFESupport(PlayerPluginsRegistrator& players);
void RegisterXMPPlugins(PlayerPluginsRegistrator& players);
void RegisterSIDPlugins(PlayerPluginsRegistrator& players, ArchivePluginsRegistrator& archives);
void RegisterET1Support(PlayerPluginsRegistrator& players);
void RegisterAYCSupport(PlayerPluginsRegistrator& players);
void RegisterSPCSupport(PlayerPluginsRegistrator& players);
void RegisterMTCSupport(PlayerPluginsRegistrator& players);
void RegisterGMEPlugins(PlayerPluginsRegistrator& players, ArchivePluginsRegistrator& archives);
void RegisterAHXSupport(PlayerPluginsRegistrator& players);
void RegisterPSFSupport(PlayerPluginsRegistrator& players);
void RegisterUSFSupport(PlayerPluginsRegistrator& players);
void RegisterGSFSupport(PlayerPluginsRegistrator& players);
void Register2SFSupport(PlayerPluginsRegistrator& players);
void RegisterNCSFSupport(PlayerPluginsRegistrator& players);
void RegisterSDSFSupport(PlayerPluginsRegistrator& players);
void RegisterASAPPlugins(PlayerPluginsRegistrator& players, ArchivePluginsRegistrator& archives);
void RegisterV2MSupport(PlayerPluginsRegistrator& players);
void RegisterVGMPlugins(PlayerPluginsRegistrator& players);
void RegisterMPTPlugins(PlayerPluginsRegistrator& players);
//void RegisterVGMStreamPlugins(PlayerPluginsRegistrator& players, ArchivePluginsRegistrator& archives);

void RegisterPlayerPlugins(PlayerPluginsRegistrator& players, ArchivePluginsRegistrator& archives)
{
	PlayerPluginsFilter ppf;
	RegisterTSSupport(ppf);
	RegisterAYSupport(ppf);
	RegisterPSGSupport(ppf);
	RegisterSTCSupport(ppf);
	RegisterST1Support(ppf);
	RegisterST3Support(ppf);
	RegisterPT2Support(ppf);
	RegisterPT3Support(ppf);
	RegisterASCSupport(ppf);
	RegisterSTPSupport(ppf);
	RegisterTXTSupport(ppf);
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
	RegisterSIDPlugins(ppf, archives);
	RegisterET1Support(ppf);
	RegisterAYCSupport(ppf);
	RegisterSPCSupport(ppf);
	RegisterMTCSupport(ppf);
	RegisterGMEPlugins(ppf, archives);
	RegisterAHXSupport(ppf);
	RegisterPSFSupport(ppf);
	RegisterUSFSupport(ppf);
	RegisterGSFSupport(ppf);
	Register2SFSupport(ppf);
	RegisterNCSFSupport(ppf);
	RegisterSDSFSupport(ppf);
	RegisterASAPPlugins(ppf, archives);
	RegisterV2MSupport(ppf);
	RegisterVGMPlugins(ppf);
	RegisterMPTPlugins(ppf);
//	RegisterVGMStreamPlugins(ppf, archives);
	
	ppf.Filter(&players);
	std::stable_sort(player_plugins.begin(), player_plugins.end(),
		[](const PlayerPlugin::Ptr& p1, const PlayerPlugin::Ptr& p2)
		{
			return p1->Id() < p2->Id();
		}
	);
}

}
//namespace ZXTune
