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
#define _UNICODE
#include <helpers/foobar2000+atl.h>
#include <helpers/atl-misc.h>
#include "resource.h"

#define DECLARE_OPTION(opt, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8)							\
static const GUID guid_cfg_##opt = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } };		\
static cfg_bool cfg_##opt(guid_cfg_##opt, true);												\
bool opt() { return cfg_##opt; }																\
void opt(bool v) { cfg_##opt = v; }																\

DECLARE_OPTION(EnableAY ,	0xa798b71d, 0x1f5d, 0x4046, 0x80, 0x18, 0x50, 0x23, 0x2b, 0x23, 0xfa, 0x43);
DECLARE_OPTION(EnableYM ,	0xd547bd63, 0xe0f0, 0x4409, 0xab, 0xf2, 0x97, 0x8b, 0x62, 0x25, 0xae, 0x77);
DECLARE_OPTION(EnableTS ,	0x36f32dad, 0xa8f2, 0x409d, 0x96, 0x1b, 0x46, 0x9b, 0xd4, 0x84, 0x0c, 0xcd);
DECLARE_OPTION(EnablePT1,	0xe633c351, 0xf4e8, 0x42e0, 0xb0, 0xa8, 0x0b, 0xef, 0x88, 0x29, 0xdf, 0xf0);
DECLARE_OPTION(EnablePT2,	0x69c8a332, 0x1afc, 0x4eab, 0xb7, 0xd6, 0x5a, 0xf9, 0x8c, 0x15, 0xcc, 0x97);
DECLARE_OPTION(EnablePT3,	0xd50a46c5, 0xfe19, 0x473b, 0xa6, 0xb5, 0xec, 0x95, 0xb6, 0x92, 0x24, 0x19);
DECLARE_OPTION(EnableSTC,	0xba802db8, 0x0de5, 0x4972, 0x9c, 0xbc, 0x61, 0x1d, 0x22, 0x3a, 0xf4, 0x32);
DECLARE_OPTION(EnableST1,	0x6979f70a, 0x616e, 0x4ad2, 0xa9, 0x00, 0xfe, 0x03, 0xda, 0x5c, 0x82, 0xfd);
DECLARE_OPTION(EnableST3,	0x893430e7, 0x3053, 0x4993, 0x84, 0x82, 0xa1, 0x9d, 0xff, 0x9a, 0x6e, 0xcd);
DECLARE_OPTION(EnableASC,	0xec946873, 0x9b63, 0x4f5c, 0xb0, 0xd6, 0xf3, 0x82, 0xde, 0x7a, 0x7d, 0x31);
DECLARE_OPTION(EnableSTP,	0x37d9401a, 0xabb2, 0x4519, 0xb4, 0x1a, 0xdd, 0x9d, 0x52, 0xe6, 0xbe, 0x8e);
DECLARE_OPTION(EnableTXT,	0x969196b3, 0x44a5, 0x4ffb, 0x9d, 0xdd, 0x95, 0xf5, 0x52, 0xc9, 0xe6, 0xd0);
DECLARE_OPTION(EnablePSG,	0xcc0ce34d, 0x670a, 0x41be, 0xac, 0x69, 0x97, 0xef, 0x78, 0x28, 0x78, 0x6a);
DECLARE_OPTION(EnablePDT,	0xe4ac9f27, 0xcf76, 0x4d56, 0xb8, 0x34, 0xb9, 0x93, 0x4b, 0xa5, 0xd2, 0x71);
DECLARE_OPTION(EnableCHI,	0x34beb321, 0x9399, 0x4916, 0xab, 0xd7, 0x2e, 0x3c, 0x85, 0xa9, 0x95, 0x04);
DECLARE_OPTION(EnableSTR,	0x12cf4fd9, 0x862f, 0x4a2e, 0xa0, 0x55, 0xfb, 0x3a, 0xcc, 0x54, 0xac, 0xb8);
DECLARE_OPTION(EnableDST,	0x49c92f0b, 0x9ed5, 0x49fe, 0xb6, 0x7e, 0xc3, 0x5c, 0x08, 0x37, 0xa4, 0x6a);
DECLARE_OPTION(EnableSQD,	0x71b9bebe, 0xb966, 0x43be, 0x9b, 0x1f, 0x91, 0x7d, 0x09, 0x6c, 0xdf, 0xc9);
DECLARE_OPTION(EnableDMM,	0x915a1fd0, 0x67a9, 0x41ed, 0xa4, 0xf3, 0x30, 0x17, 0x16, 0x8b, 0x8d, 0xd4);
DECLARE_OPTION(EnablePSM,	0x038dcee1, 0x416d, 0x4fc3, 0xa8, 0xbc, 0x2c, 0x61, 0x79, 0xcb, 0x61, 0xed);
DECLARE_OPTION(EnableGTR,	0x8a3cd43a, 0x9e8d, 0x4a0b, 0xb5, 0x3d, 0x23, 0x0c, 0x50, 0xd9, 0x03, 0x0c);
DECLARE_OPTION(EnableVTX,	0x3c54be38, 0x85ba, 0x4608, 0xb7, 0xab, 0xe9, 0xf1, 0x3d, 0x2c, 0xd3, 0x5f);
DECLARE_OPTION(EnableTFC,	0x67ea6dad, 0x33ae, 0x4871, 0xb7, 0xbd, 0x24, 0x08, 0xfb, 0x5e, 0x3d, 0x49);
DECLARE_OPTION(EnableTFD,	0x018dba89, 0x9010, 0x4203, 0x97, 0xcf, 0x2a, 0xc2, 0x33, 0xf2, 0xcf, 0x38);
DECLARE_OPTION(EnableTFE,	0x3de59adf, 0xddf8, 0x4951, 0x9d, 0xb1, 0x5e, 0x39, 0x71, 0x88, 0x08, 0xc1);
DECLARE_OPTION(EnableSQT,	0x10ecf97c, 0xd363, 0x421c, 0xaf, 0x0a, 0x0b, 0x84, 0x0d, 0x5f, 0xe4, 0x2e);
DECLARE_OPTION(EnablePSC,	0x09b5a850, 0xc925, 0x4c76, 0xb7, 0x1a, 0xb8, 0x21, 0x3c, 0xcd, 0xc6, 0xc9);
DECLARE_OPTION(EnableFTC,	0x392e66de, 0x04dc, 0x4900, 0xad, 0xca, 0x26, 0xb5, 0x9e, 0xc8, 0xdd, 0x2c);
DECLARE_OPTION(EnableCOP,	0x480559aa, 0x29b8, 0x4d52, 0x9b, 0x2c, 0x84, 0xda, 0x0b, 0x74, 0xde, 0xcc);
DECLARE_OPTION(EnableET1,	0x08376310, 0x3e80, 0x4cbc, 0xb4, 0xb8, 0xf9, 0x4d, 0xbd, 0x39, 0x49, 0xe2);
DECLARE_OPTION(EnableAYC,	0x7a375ed2, 0x3b32, 0x4bcb, 0x82, 0xdc, 0x6a, 0x21, 0xd5, 0xfa, 0x5c, 0x8a);
DECLARE_OPTION(EnableSPC,	0xa469c480, 0x1ec0, 0x4b90, 0x95, 0x45, 0x7f, 0xbe, 0x4c, 0x86, 0x31, 0xb2);
DECLARE_OPTION(EnableMTC,	0x33738398, 0xdb20, 0x4957, 0xb6, 0x3a, 0xf4, 0xf8, 0xed, 0x04, 0x36, 0x5b);
DECLARE_OPTION(EnableAHX,	0x20aa15ef, 0x62b8, 0x4d23, 0x9a, 0xe8, 0x8f, 0x8e, 0x42, 0x9c, 0x41, 0x91);
DECLARE_OPTION(EnableHVL,	0x50aca341, 0x206e, 0x4749, 0x8d, 0xf6, 0x4a, 0x7d, 0xcd, 0x21, 0x62, 0xe1);
DECLARE_OPTION(EnableXMP,	0x1e7fd7d6, 0x5dae, 0x4c7d, 0xbb, 0xae, 0xd0, 0x18, 0x14, 0x66, 0xfe, 0xc1);
DECLARE_OPTION(EnableSID,	0x246b7e86, 0x1c96, 0x4254, 0x81, 0x00, 0xd6, 0x75, 0x33, 0x7f, 0xe8, 0x5e);
DECLARE_OPTION(EnableGME,	0x59a72db7, 0x9ce9, 0x457d, 0x99, 0xec, 0x6a, 0x23, 0x3f, 0x1f, 0xfd, 0xf3);
DECLARE_OPTION(EnablePSF,	0x4c047b38,	0x19bf, 0x4d8e, 0x8e, 0xe2, 0x64, 0xb3, 0x4a, 0xf1, 0xa6, 0x57);
DECLARE_OPTION(EnableUSF,	0xffe0e29f, 0x39f9, 0x4b04, 0xbe, 0x6e, 0x47, 0x4b, 0x87, 0xd2, 0x3a, 0xb7);
DECLARE_OPTION(EnableGSF,	0x329e961f, 0x4de0, 0x4bc9, 0x8f, 0x67, 0xd2, 0x0e, 0x6d, 0xec, 0x10, 0x13);
DECLARE_OPTION(Enable2SF,	0x1342d256, 0x4e59, 0x4d9d, 0x8c, 0x20, 0x33, 0x16, 0xf8, 0x29, 0x59, 0x85);
DECLARE_OPTION(EnableSDS,	0x2f9ee7b6, 0xa9c8, 0x40e6, 0x8e, 0xc6, 0x49, 0xcc, 0x20, 0x9d, 0xe1, 0x14);
DECLARE_OPTION(EnableASAP,	0xc310d8a8, 0xf9ca, 0x4069, 0xaa, 0xdd, 0x63, 0x57, 0x42, 0xd1, 0x4a, 0x5d);
DECLARE_OPTION(EnableV2M,	0x73f65130, 0x66eb, 0x4102, 0xa8, 0x5a, 0x3a, 0xb7, 0x14, 0x67, 0xe9, 0xd1);
DECLARE_OPTION(EnableVGM,	0xfba9bf1f, 0x7bf8, 0x4669, 0x9e, 0x55, 0x1d, 0xc5, 0x1a, 0x70, 0x68, 0x37);

class CMyPreferences : public CDialogImpl<CMyPreferences>, public preferences_page_instance
{
public:
	CMyPreferences(preferences_page_callback::ptr callback) : m_callback(callback) {}

	//dialog resource ID
	enum { IDD = IDD_MYPREFERENCES };

	virtual t_uint32	get_state();
	virtual void		apply();
	virtual void		reset();

	//WTL message map
	BEGIN_MSG_MAP(CMyPreferences)
	MSG_WM_INITDIALOG(OnInitDialog)
	for(int cmd = IDC_CHECK_FIRST; cmd <= IDC_CHECK_LAST; ++cmd)
	{
		COMMAND_HANDLER_EX(cmd, BN_CLICKED, OnCheck);
	}
	END_MSG_MAP()
private:
	BOOL		OnInitDialog(CWindow, LPARAM);
	void		OnCheck(UINT, int, CWindow);
	bool		HasChangedNeedRestart();
	void		OnChanged();

	const preferences_page_callback::ptr m_callback;
};

BOOL CMyPreferences::OnInitDialog(CWindow, LPARAM)
{
	uButton_SetCheck(m_hWnd, IDC_CHECK_AY,	EnableAY());
	uButton_SetCheck(m_hWnd, IDC_CHECK_YM,	EnableYM());
	uButton_SetCheck(m_hWnd, IDC_CHECK_TS,	EnableTS());
	uButton_SetCheck(m_hWnd, IDC_CHECK_PT2, EnablePT2());
	uButton_SetCheck(m_hWnd, IDC_CHECK_PT3, EnablePT3());
	uButton_SetCheck(m_hWnd, IDC_CHECK_STC, EnableSTC());
	uButton_SetCheck(m_hWnd, IDC_CHECK_ST1, EnableST1());
	uButton_SetCheck(m_hWnd, IDC_CHECK_ST3, EnableST3());
	uButton_SetCheck(m_hWnd, IDC_CHECK_ASC, EnableASC());
	uButton_SetCheck(m_hWnd, IDC_CHECK_STP, EnableSTP());
	uButton_SetCheck(m_hWnd, IDC_CHECK_TXT, EnableTXT());
	uButton_SetCheck(m_hWnd, IDC_CHECK_PSG, EnablePSG());
	uButton_SetCheck(m_hWnd, IDC_CHECK_PDT, EnablePDT());
	uButton_SetCheck(m_hWnd, IDC_CHECK_CHI, EnableCHI());
	uButton_SetCheck(m_hWnd, IDC_CHECK_STR, EnableSTR());
	uButton_SetCheck(m_hWnd, IDC_CHECK_DST, EnableDST());
	uButton_SetCheck(m_hWnd, IDC_CHECK_SQD, EnableSQD());
	uButton_SetCheck(m_hWnd, IDC_CHECK_DMM, EnableDMM());
	uButton_SetCheck(m_hWnd, IDC_CHECK_PSM, EnablePSM());
	uButton_SetCheck(m_hWnd, IDC_CHECK_GTR, EnableGTR());
	uButton_SetCheck(m_hWnd, IDC_CHECK_PT1, EnablePT1());
	uButton_SetCheck(m_hWnd, IDC_CHECK_VTX, EnableVTX());
	uButton_SetCheck(m_hWnd, IDC_CHECK_TFD, EnableTFD());
	uButton_SetCheck(m_hWnd, IDC_CHECK_TFC, EnableTFC());
	uButton_SetCheck(m_hWnd, IDC_CHECK_SQT, EnableSQT());
	uButton_SetCheck(m_hWnd, IDC_CHECK_PSC, EnablePSC());
	uButton_SetCheck(m_hWnd, IDC_CHECK_FTC, EnableFTC());
	uButton_SetCheck(m_hWnd, IDC_CHECK_COP, EnableCOP());
	uButton_SetCheck(m_hWnd, IDC_CHECK_TFE, EnableTFE());
	uButton_SetCheck(m_hWnd, IDC_CHECK_ET1, EnableET1());
	uButton_SetCheck(m_hWnd, IDC_CHECK_AYC, EnableAYC());
	uButton_SetCheck(m_hWnd, IDC_CHECK_SPC, EnableSPC());
	uButton_SetCheck(m_hWnd, IDC_CHECK_MTC, EnableMTC());
	uButton_SetCheck(m_hWnd, IDC_CHECK_AHX, EnableAHX());
	uButton_SetCheck(m_hWnd, IDC_CHECK_HVL, EnableHVL());
	uButton_SetCheck(m_hWnd, IDC_CHECK_XMP, EnableXMP());
	uButton_SetCheck(m_hWnd, IDC_CHECK_SID, EnableSID());
	uButton_SetCheck(m_hWnd, IDC_CHECK_GME, EnableGME());
	return FALSE;
}

void CMyPreferences::OnCheck(UINT, int, CWindow)
{
	OnChanged();
}

t_uint32 CMyPreferences::get_state()
{
	t_uint32 state = preferences_state::resettable;
	if(HasChangedNeedRestart()) state |= preferences_state::changed|preferences_state::needs_restart;
	return state;
}

void CMyPreferences::reset()
{
	for(int cmd = IDC_CHECK_FIRST; cmd <= IDC_CHECK_LAST; ++cmd)
	{
		uButton_SetCheck(m_hWnd, cmd, true);
	}
	OnChanged();
}

void CMyPreferences::apply()
{
	EnableAY (uButton_GetCheck(m_hWnd, IDC_CHECK_AY));
	EnableYM (uButton_GetCheck(m_hWnd, IDC_CHECK_YM));
	EnableTS (uButton_GetCheck(m_hWnd, IDC_CHECK_TS));
	EnablePT2(uButton_GetCheck(m_hWnd, IDC_CHECK_PT2));
	EnablePT3(uButton_GetCheck(m_hWnd, IDC_CHECK_PT3));
	EnableSTC(uButton_GetCheck(m_hWnd, IDC_CHECK_STC));
	EnableST1(uButton_GetCheck(m_hWnd, IDC_CHECK_ST1));
	EnableST3(uButton_GetCheck(m_hWnd, IDC_CHECK_ST3));
	EnableASC(uButton_GetCheck(m_hWnd, IDC_CHECK_ASC));
	EnableSTP(uButton_GetCheck(m_hWnd, IDC_CHECK_STP));
	EnableTXT(uButton_GetCheck(m_hWnd, IDC_CHECK_TXT));
	EnablePSG(uButton_GetCheck(m_hWnd, IDC_CHECK_PSG));
	EnablePDT(uButton_GetCheck(m_hWnd, IDC_CHECK_PDT));
	EnableCHI(uButton_GetCheck(m_hWnd, IDC_CHECK_CHI));
	EnableSTR(uButton_GetCheck(m_hWnd, IDC_CHECK_STR));
	EnableDST(uButton_GetCheck(m_hWnd, IDC_CHECK_DST));
	EnableSQD(uButton_GetCheck(m_hWnd, IDC_CHECK_SQD));
	EnableDMM(uButton_GetCheck(m_hWnd, IDC_CHECK_DMM));
	EnablePSM(uButton_GetCheck(m_hWnd, IDC_CHECK_PSM));
	EnableGTR(uButton_GetCheck(m_hWnd, IDC_CHECK_GTR));
	EnablePT1(uButton_GetCheck(m_hWnd, IDC_CHECK_PT1));
	EnableVTX(uButton_GetCheck(m_hWnd, IDC_CHECK_VTX));
	EnableTFD(uButton_GetCheck(m_hWnd, IDC_CHECK_TFD));
	EnableTFC(uButton_GetCheck(m_hWnd, IDC_CHECK_TFC));
	EnableSQT(uButton_GetCheck(m_hWnd, IDC_CHECK_SQT));
	EnablePSC(uButton_GetCheck(m_hWnd, IDC_CHECK_PSC));
	EnableFTC(uButton_GetCheck(m_hWnd, IDC_CHECK_FTC));
	EnableCOP(uButton_GetCheck(m_hWnd, IDC_CHECK_COP));
	EnableTFE(uButton_GetCheck(m_hWnd, IDC_CHECK_TFE));
	EnableET1(uButton_GetCheck(m_hWnd, IDC_CHECK_ET1));
	EnableAYC(uButton_GetCheck(m_hWnd, IDC_CHECK_AYC));
	EnableSPC(uButton_GetCheck(m_hWnd, IDC_CHECK_SPC));
	EnableMTC(uButton_GetCheck(m_hWnd, IDC_CHECK_MTC));
	EnableAHX(uButton_GetCheck(m_hWnd, IDC_CHECK_AHX));
	EnableHVL(uButton_GetCheck(m_hWnd, IDC_CHECK_HVL));
	EnableXMP(uButton_GetCheck(m_hWnd, IDC_CHECK_XMP));
	EnableSID(uButton_GetCheck(m_hWnd, IDC_CHECK_SID));
	EnableGME(uButton_GetCheck(m_hWnd, IDC_CHECK_GME));
	OnChanged(); //our dialog content has not changed but the flags have - our currently shown values now match the settings so the apply button can be disabled
}

bool CMyPreferences::HasChangedNeedRestart()
{
	return	EnableAY()  != uButton_GetCheck(m_hWnd, IDC_CHECK_AY)  ||
			EnableYM()  != uButton_GetCheck(m_hWnd, IDC_CHECK_YM)  ||
			EnableTS()  != uButton_GetCheck(m_hWnd, IDC_CHECK_TS)  ||
			EnablePT2() != uButton_GetCheck(m_hWnd, IDC_CHECK_PT2) ||
			EnablePT3() != uButton_GetCheck(m_hWnd, IDC_CHECK_PT3) ||
			EnableSTC() != uButton_GetCheck(m_hWnd, IDC_CHECK_STC) ||
			EnableST1() != uButton_GetCheck(m_hWnd, IDC_CHECK_ST1) ||
			EnableST3() != uButton_GetCheck(m_hWnd, IDC_CHECK_ST3) ||
			EnableASC() != uButton_GetCheck(m_hWnd, IDC_CHECK_ASC) ||
			EnableSTP() != uButton_GetCheck(m_hWnd, IDC_CHECK_STP) ||
			EnableTXT() != uButton_GetCheck(m_hWnd, IDC_CHECK_TXT) ||
			EnablePSG() != uButton_GetCheck(m_hWnd, IDC_CHECK_PSG) ||
			EnablePDT() != uButton_GetCheck(m_hWnd, IDC_CHECK_PDT) ||
			EnableCHI() != uButton_GetCheck(m_hWnd, IDC_CHECK_CHI) ||
			EnableSTR() != uButton_GetCheck(m_hWnd, IDC_CHECK_STR) ||
			EnableDST() != uButton_GetCheck(m_hWnd, IDC_CHECK_DST) ||
			EnableSQD() != uButton_GetCheck(m_hWnd, IDC_CHECK_SQD) ||
			EnableDMM() != uButton_GetCheck(m_hWnd, IDC_CHECK_DMM) ||
			EnablePSM() != uButton_GetCheck(m_hWnd, IDC_CHECK_PSM) ||
			EnableGTR() != uButton_GetCheck(m_hWnd, IDC_CHECK_GTR) ||
			EnablePT1() != uButton_GetCheck(m_hWnd, IDC_CHECK_PT1) ||
			EnableVTX() != uButton_GetCheck(m_hWnd, IDC_CHECK_VTX) || 
			EnableTFD() != uButton_GetCheck(m_hWnd, IDC_CHECK_TFD) ||
			EnableTFC() != uButton_GetCheck(m_hWnd, IDC_CHECK_TFC) ||
			EnableSQT() != uButton_GetCheck(m_hWnd, IDC_CHECK_SQT) ||
			EnablePSC() != uButton_GetCheck(m_hWnd, IDC_CHECK_PSC) ||
			EnableFTC() != uButton_GetCheck(m_hWnd, IDC_CHECK_FTC) ||
			EnableCOP() != uButton_GetCheck(m_hWnd, IDC_CHECK_COP) ||
			EnableTFE() != uButton_GetCheck(m_hWnd, IDC_CHECK_TFE) ||
			EnableET1() != uButton_GetCheck(m_hWnd, IDC_CHECK_ET1) ||
			EnableAYC() != uButton_GetCheck(m_hWnd, IDC_CHECK_AYC) ||
			EnableSPC() != uButton_GetCheck(m_hWnd, IDC_CHECK_SPC) ||
			EnableMTC() != uButton_GetCheck(m_hWnd, IDC_CHECK_MTC) ||
			EnableAHX() != uButton_GetCheck(m_hWnd, IDC_CHECK_AHX) ||
			EnableHVL() != uButton_GetCheck(m_hWnd, IDC_CHECK_HVL) ||
			EnableXMP() != uButton_GetCheck(m_hWnd, IDC_CHECK_XMP) ||
			EnableSID() != uButton_GetCheck(m_hWnd, IDC_CHECK_SID) ||
			EnableGME() != uButton_GetCheck(m_hWnd, IDC_CHECK_GME);
}

void CMyPreferences::OnChanged()
{
	m_callback->on_state_changed();
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
