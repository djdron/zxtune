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
#include "preferences.h"

namespace ZXTune
{

void RegisterHVLSupport(PlayerPluginsRegistrator& registrator);

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
	if(EnableTS())	RegisterTSSupport(registrator);
	if(EnableAY())	RegisterAYSupport(registrator);
	if(EnablePT3())	RegisterPT3Support(registrator);
	if(EnablePT2())	RegisterPT2Support(registrator);
	if(EnableSTC())	RegisterSTCSupport(registrator);
	if(EnableST1())	RegisterST1Support(registrator);
	if(EnableST3())	RegisterST3Support(registrator);
	if(EnableASC())	RegisterASCSupport(registrator);
	if(EnableSTP())	RegisterSTPSupport(registrator);
	if(EnableTXT())	RegisterTXTSupport(registrator);
	if(EnablePSG())	RegisterPSGSupport(registrator);
	if(EnablePDT())	RegisterPDTSupport(registrator);
	if(EnableCHI())	RegisterCHISupport(registrator);
	if(EnableSTR())	RegisterSTRSupport(registrator);
	if(EnableDST())	RegisterDSTSupport(registrator);
	if(EnableSQD())	RegisterSQDSupport(registrator);
	if(EnableDMM())	RegisterDMMSupport(registrator);
	if(EnablePSM())	RegisterPSMSupport(registrator);
	if(EnableGTR())	RegisterGTRSupport(registrator);
	if(EnablePT1())	RegisterPT1Support(registrator);
	if(EnableVTX())	RegisterVTXSupport(registrator);
	if(EnableYM())	RegisterYMSupport(registrator);
	if(EnableTFD())	RegisterTFDSupport(registrator);
	if(EnableTFC())	RegisterTFCSupport(registrator);
	if(EnableSQT())	RegisterSQTSupport(registrator);
	if(EnablePSC())	RegisterPSCSupport(registrator);
	if(EnableFTC())	RegisterFTCSupport(registrator);
	if(EnableCOP())	RegisterCOPSupport(registrator);
	if(EnableTFE())	RegisterTFESupport(registrator);
	if(EnableXMP())	RegisterXMPPlugins(registrator);
	if(EnableSID())	RegisterSIDPlugins(registrator);
	if(EnableET1())	RegisterET1Support(registrator);
	if(EnableAYC())	RegisterAYCSupport(registrator);
	if(EnableSPC())	RegisterSPCSupport(registrator);
	if(EnableMTC())	RegisterMTCSupport(registrator);
	if(EnableGME())	RegisterGMEPlugins(registrator);
	if(EnableAHX())	RegisterAHXSupport(registrator);
	if(EnableHVL())	RegisterHVLSupport(registrator);
	if(EnablePSF())	RegisterPSFSupport(registrator);
	if(EnableUSF())	RegisterUSFSupport(registrator);
	if(EnableGSF())	RegisterGSFSupport(registrator);
	if(Enable2SF())	Register2SFSupport(registrator);
	if(EnableSDS())	RegisterSDSFSupport(registrator);
	if(EnableASAP()) RegisterASAPPlugins(registrator);
	if(EnableV2M())	RegisterV2MSupport(registrator);
	if(EnableVGM())	RegisterVGMPlugins(registrator);
}

}
//namespace ZXTune
