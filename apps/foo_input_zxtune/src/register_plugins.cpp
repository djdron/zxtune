/*
ZXTune foobar2000 decoder component by djdron (C) 2013 - 2017

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
#include <core/plugins/players/plugins.h>

bool enable_ay();
bool enable_ym();
bool enable_ts();
bool enable_pt1();
bool enable_pt2();
bool enable_pt3();
bool enable_stc();
bool enable_st1();
bool enable_st3();
bool enable_asc();
bool enable_stp();
bool enable_txt();
bool enable_psg();
bool enable_pdt();
bool enable_chi();
bool enable_str();
bool enable_dst();
bool enable_sqd();
bool enable_dmm();
bool enable_psm();
bool enable_gtr();
bool enable_vtx();
bool enable_tfc();
bool enable_tfd();
bool enable_tfe();
bool enable_sqt();
bool enable_psc();
bool enable_ftc();
bool enable_cop();
bool enable_et1();
bool enable_ayc();
bool enable_spc();
bool enable_mtc();
bool enable_ahx();
bool enable_xmp();
bool enable_sid();
bool enable_gme();

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

void RegisterPlayerPlugins(PlayerPluginsRegistrator& registrator)
{
	//try TS & AY first
	if(enable_ts())		RegisterTSSupport(registrator);
	if(enable_ay())		RegisterAYSupport(registrator);
	if(enable_pt3())	RegisterPT3Support(registrator);
	if(enable_pt2())	RegisterPT2Support(registrator);
	if(enable_stc())	RegisterSTCSupport(registrator);
	if(enable_st1())	RegisterST1Support(registrator);
	if(enable_st3())	RegisterST3Support(registrator);
	if(enable_asc())	RegisterASCSupport(registrator);
	if(enable_stp())	RegisterSTPSupport(registrator);
	if(enable_txt())	RegisterTXTSupport(registrator);
	if(enable_psg())	RegisterPSGSupport(registrator);
	if(enable_pdt())	RegisterPDTSupport(registrator);
	if(enable_chi())	RegisterCHISupport(registrator);
	if(enable_str())	RegisterSTRSupport(registrator);
	if(enable_dst())	RegisterDSTSupport(registrator);
	if(enable_sqd())	RegisterSQDSupport(registrator);
	if(enable_dmm())	RegisterDMMSupport(registrator);
	if(enable_psm())	RegisterPSMSupport(registrator);
	if(enable_gtr())	RegisterGTRSupport(registrator);
	if(enable_pt1())	RegisterPT1Support(registrator);
	if(enable_vtx())	RegisterVTXSupport(registrator);
	if(enable_ym())		RegisterYMSupport(registrator);
	if(enable_tfd())	RegisterTFDSupport(registrator);
	if(enable_tfc())	RegisterTFCSupport(registrator);
	if(enable_sqt())	RegisterSQTSupport(registrator);
	if(enable_psc())	RegisterPSCSupport(registrator);
	if(enable_ftc())	RegisterFTCSupport(registrator);
	if(enable_cop())	RegisterCOPSupport(registrator);
	if(enable_tfe())	RegisterTFESupport(registrator);
	if(enable_xmp())	RegisterXMPPlugins(registrator);
	if(enable_sid())	RegisterSIDPlugins(registrator);
	if(enable_et1())	RegisterET1Support(registrator);
	if(enable_ayc())	RegisterAYCSupport(registrator);
	if(enable_spc())	RegisterSPCSupport(registrator);
	if(enable_mtc())	RegisterMTCSupport(registrator);
	if(enable_gme())	RegisterGMEPlugins(registrator);
	if(enable_ahx())	RegisterAHXSupport(registrator);
}

}
//namespace ZXTune
