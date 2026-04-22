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

#define UNICODE
#define _UNICODE
#include <SDK/foobar2000-lite.h>
#include <SDK/cfg_var.h>
#include <SDK/hasher_md5.h>
#undef UNICODE
#undef _UNICODE
#undef min

#include <core/plugins/player_plugin.h>


extern const GUID preferences_guid = { 0xce451919, 0xd1db, 0x44ed, { 0x8e, 0x6a, 0x38, 0x87, 0xa7, 0x56, 0xfb, 0x81 } };

static const GUID player_plugins_disabled_guid = { 0xe8d7b860, 0x5a8b, 0x4954, { 0xb4, 0x7d, 0x4c, 0x9f, 0xfb, 0x23, 0xd0, 0xfc } };
static cfg_string player_plugins_disabled_cfg(player_plugins_disabled_guid, "");

static pfc::string8 PlayerPluginHash(const ZXTune::PlayerPlugin::Ptr& pp)
{
	auto d = pp->Description();
	auto hash = static_api_ptr_t<hasher_md5>()->process_single(d.data(), d.length()).asString();
	if(hash.length() > 8)
		hash.truncate(8);
	return hash;
}

namespace ZXTune
{

bool PlayerPluginEnabled(const PlayerPlugin::Ptr& pp)
{
	auto hash = PlayerPluginHash(pp);
	return player_plugins_disabled_cfg.get().find_first(hash.c_str()) == ~0;
}
void SetPlayerPluginsDisabled(const std::vector<PlayerPlugin::Ptr>& pps)
{
	pfc::string8 hashes;
	for(auto& i : pps)
	{
		auto hash = PlayerPluginHash(i);
		if(!hashes.is_empty())
			hashes.add_string(" ");
		hashes.add_string(hash);
	}
	player_plugins_disabled_cfg = hashes;
}

}
//namespace ZXTune
