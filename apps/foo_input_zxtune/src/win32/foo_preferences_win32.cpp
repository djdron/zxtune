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


#include "resource.h"
#define UNICODE
#define _UNICODE
#include <helpers/foobar2000+atl.h>
#include <helpers/atl-misc.h>
#include <helpers/DarkMode.h>
#include <libPPUI/CListControlSimple.h>
#include <libPPUI/CListControl-Cells.h>
#undef UNICODE
#undef _UNICODE
#undef min

#include <core/plugins/player_plugin.h>


namespace ZXTune
{
const std::vector<PlayerPlugin::Ptr>& PlayerPlugins();
bool PlayerPluginEnabled(const PlayerPlugin::Ptr& pp);
void SetPlayerPluginsDisabled(const std::vector<PlayerPlugin::Ptr>& players_disabled);
}
//namespace ZXTune

class CMyPreferences : public CDialogImpl<CMyPreferences>, public preferences_page_instance
{
public:
	CMyPreferences(preferences_page_callback::ptr callback) : players_list(callback), m_callback(callback) {}

	//dialog resource ID
	enum { IDD = IDD_MYPREFERENCES };

	virtual t_uint32	get_state();
	virtual void		apply();
	virtual void		reset();

	//WTL message map
	BEGIN_MSG_MAP(CMyPreferences)
	MSG_WM_INITDIALOG(OnInitDialog)
	MSG_WM_SIZE(OnSize)
	END_MSG_MAP()
private:
	BOOL		OnInitDialog(CWindow, LPARAM);
	void		OnSize(UINT, const CSize&);
	bool		HasChangedNeedRestart();

	class CListPlayerPlugins : public CListControlReadOnly
	{
	public:
		std::set<int> disabled;
		const preferences_page_callback::ptr m_callback;
		CListPlayerPlugins(const preferences_page_callback::ptr callback) : m_callback(callback) {}

		virtual void RequestReorder(const size_t* order, size_t count) override {}
		virtual void RequestRemoveSelection() override {}
		virtual void ExecuteDefaultAction(t_size index) override {}

		virtual size_t GetItemCount() const override { return ZXTune::PlayerPlugins().size(); }
		virtual bool GetCellCheckState(size_t item, size_t subItem) const override
		{
			if(subItem == 0)
				return disabled.find(item) == disabled.end();
			return false;
		}
		virtual void SetCellCheckState(size_t item, size_t subItem, bool value) override
		{
			if(subItem == 0)
			{
				if(value)
					disabled.erase(item);
				else
					disabled.insert(item);
				__super::SetCellCheckState(item, subItem, value);
				m_callback->on_state_changed();
			}
		}
		virtual bool GetSubItemText(size_t item, size_t subItem, pfc::string_base& out) const override
		{
			if(item < ZXTune::PlayerPlugins().size())
			{
				auto pp = ZXTune::PlayerPlugins()[item];
				StringView out1;
				switch(subItem)
				{
				case 1: out1 = pp->Id(); break;
				case 2: out1 = pp->Description(); break;
				default: return false;
				}
				out.set_string(out1.data(), out1.length());
				return true;
			}
			return false;
		}
		virtual cellType_t GetCellType(size_t item, size_t subItem) const override
		{
			switch(subItem)
			{
			case 0:		return &PFC_SINGLETON(CListCell_Checkbox);
			default:	return &PFC_SINGLETON(CListCell_Text);
			}
		}
		virtual bool GetCellTypeSupported() const override { return true; }
		virtual bool AllowScrollbar(bool vertical) const override { return vertical; }
	};
	const preferences_page_callback::ptr m_callback;
	CListPlayerPlugins players_list;
	std::set<int> players_disabled;

	fb2k::CDarkModeHooks m_dark;
};

BOOL CMyPreferences::OnInitDialog(CWindow, LPARAM)
{
	ZXTune::PlayerPlugin::Enumerate(); // gather all plugins if not gathered yet
	for(auto p = ZXTune::PlayerPlugins().begin(); p != ZXTune::PlayerPlugins().end(); ++p)
	{
		if(!ZXTune::PlayerPluginEnabled(*p))
			players_disabled.insert(p - ZXTune::PlayerPlugins().begin());
	}
	players_list.disabled = players_disabled;
	players_list.CreateInDialog(*this, IDC_PLAYER_PLUGINS);
	players_list.AddColumnDLU("On", 20);
	players_list.AddColumnDLU("ID", 50);
	players_list.AddColumnAutoWidth("Description");
	players_list.ReloadData();

	m_dark.AddDialogWithControls(*this);
	return FALSE;
}
void CMyPreferences::OnSize(UINT, const CSize& s)
{
	players_list.SetWindowPos(HWND_TOP, 0, 0, s.cx, s.cy, 0);
}

t_uint32 CMyPreferences::get_state()
{
	t_uint32 state = preferences_state::resettable|preferences_state::dark_mode_supported;
	if(HasChangedNeedRestart())
		state |= preferences_state::changed|preferences_state::needs_restart;
	return state;
}

void CMyPreferences::reset()
{
	players_list.disabled.clear();
	players_list.ReloadData();
	m_callback->on_state_changed();
}

void CMyPreferences::apply()
{
	players_disabled = players_list.disabled;
	std::vector<ZXTune::PlayerPlugin::Ptr> pps;
	for(auto& i : players_disabled)
	{
		pps.push_back(ZXTune::PlayerPlugins()[i]);
	}
	ZXTune::SetPlayerPluginsDisabled(pps);
	m_callback->on_state_changed(); //our dialog content has not changed but the flags have - our currently shown values now match the settings so the apply button can be disabled
}

bool CMyPreferences::HasChangedNeedRestart()
{
	return players_disabled != players_list.disabled;
}

class preferences_page_myimpl : public preferences_page_impl<CMyPreferences>
{
public:
	const char * get_name() { return "ZXTune"; }
	GUID get_guid()
	{
		static const GUID guid = { 0xce451919, 0xd1db, 0x44ed, { 0x8e, 0x6a, 0x38, 0x87, 0xa7, 0x56, 0xfb, 0x81 } };
		return guid;
	}
	GUID get_parent_guid() { return guid_input; }
};

static preferences_page_factory_t<preferences_page_myimpl> g_preferences_page_myimpl_factory;

