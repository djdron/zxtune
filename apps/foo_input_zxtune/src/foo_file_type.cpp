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

#define UNICODE
#include <SDK/foobar2000.h>
#undef UNICODE
#undef min

#include <core/plugins/player_plugins_enumerator.h>


static const char* containers_exts[] =
{
	"TRD",
	"SCL",
	"HRP",
	"SZX",
	"$B",
	"$C",
	"$M",
	"FDI",
	"DSQ",
	"MSP",
	"TRS",
	"LZS",
	"PCD",
	"HRM",
	"CC3",
	"CC4",
	"CC4PLUS",
	"ESV",
	"GAM",
	"GAMPLUS",
	"LOGO1",
	"BIN",
	"TLZ",
	"TLZP",
	"P",
	"TD0",
	"Z80",
	"SNA",
};

const std::vector<std::string>& SupportedExts()
{
	static const auto supported_exts = []()
	{
		std::vector<std::string> res;
		auto Add = [&res](const std::string& id)
		{
			if(id == "TXT")
				return;
			if(find(res.begin(), res.end(), id) == res.end())
				res.push_back(id);
		};
		for(auto p = ZXTune::PlayerPluginsEnumerator::Create()->Enumerate(); p->IsValid(); p->Next())
		{
			auto id = p->Get()->GetDescription()->Id();
			Add(id);
		}
		for(auto ext : containers_exts)
		{
			Add(ext);
		}
		return res;
	}();
	return supported_exts;
}

static const std::string& SupportedExtsStr()
{
	static const auto supported_exts = []()
	{
		std::string res;
		for(auto& p : SupportedExts())
		{
			if(!res.empty())
				res += ";";
			res += "*.";
			res += p;
		}
		return res;
	}();
	return supported_exts;
}

class input_file_type_zxtune : public service_impl_single_t<input_file_type>
{
public:
	virtual unsigned get_count() override { return 1; }
	virtual bool get_name(unsigned idx, pfc::string_base& out) override
	{
		if(idx != 0)
			return false;
		out = "ZXTune Audio Files";
		return true;
	}
	virtual bool get_mask(unsigned idx, pfc::string_base& out) override
	{
		if(idx != 0)
			return false;
		out = SupportedExtsStr().c_str();
		return true;
	}
	virtual bool is_associatable(unsigned idx) override { return true; }
};
static input_file_type_zxtune ft_zxtune;
static service_factory_single_ref_t<input_file_type_zxtune> g_filetype_service(ft_zxtune);
