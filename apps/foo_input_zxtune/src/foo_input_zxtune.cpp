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

#include "zxtune_player.h"

//library includes
#include <binary/container_factories.h>
#include <core/module_open.h>
#include <core/module_detect.h>
#include <core/additional_files_resolve.h>
#include <core/plugins/player_plugin.h>
#include <module/attributes.h>
#include <sound/sound_parameters.h>

namespace ZXTune
{
extern std::vector<PlayerPlugin::Ptr> player_plugins;
}
//namespace ZXTune

// Declaration of your component's version information
// Since foobar2000 v1.0 having at least one of these in your DLL is mandatory to let the troubleshooter tell different versions of your component apart.
// Note that it is possible to declare multiple components within one DLL, but it's strongly recommended to keep only one declaration per DLL.
// As for 1.1, the version numbers are used by the component update finder to find updates; for that to work, you must have ONLY ONE declaration per DLL. If there are multiple declarations, the component is assumed to be outdated and a version number of "0" is assumed, to overwrite the component with whatever is currently on the site assuming that it comes with proper version numbers.
DECLARE_COMPONENT_VERSION("ZXTune Decoders", "0.0.8",
"ZXTune (C) 2008 - 2020 by Vitamin/CAIG.\n"
"based on r4953 jul 23 2020\n"
"foobar2000 plugin by djdron (C) 2013 - 2020.\n"
"https://github.com/djdron/zxtune/tree/cmake\n\n"

"Technical credits:\n"
"AYEmul sources by S.Bulba\n"
"AYFly sources by Ander\n"
"xPlugins sources by elf/2\n"
"zlib library by Jean-loup Gailly and Mark Adler\n"
"z80ex library by Boo-boo\n"
"boost C++ library\n"
"Pusher sources by Himik/ZxZ\n"
"lhasa library by Simon Howard\n"
"libxmp library by Claudio Matsuoka\n"
"libsidplayfp by Simon White, Antti Lankila and Leandro Nini\n"
"snes_spc library by Shay Green\n"
"Game_Music_Emu library by Shay Green and Chris Moeller\n"
"HVL2WAV sources by Peter Gordon\n"
"Highly Experimental library by Neill Corlett\n"
"lazyusf2 library by Christopher Snowhill\n"
"mGBA sources by Jeffrey Pfau\n"
"vio2sf library by Christopher Snowhill\n"
"Highly Threoretical library by Neill Corlett and Christopher Snowhill\n"
"ASAP library by Piotr Fusik\n"
"v2m-player library by Farbrausch and Joakim L. Gilje\n"
"libvgm library by Valley Bell\n"
);

// This will prevent users from renaming your component around (important for proper troubleshooter behaviors) or loading multiple instances of it.
VALIDATE_COMPONENT_FILENAME("foo_input_zxtune.dll");

const std::vector<std::string>& SupportedExts();

// No inheritance. Our methods get called over input framework templates. See input_singletrack_impl for descriptions of what each method does.
class input_zxtune : public input_stubs
{
public:
	enum
	{
		raw_bits_per_sample = 16,
		raw_channels = 2,
		raw_sample_rate = 44100,

		raw_bytes_per_sample = raw_bits_per_sample / 8,
		raw_total_sample_width = raw_bytes_per_sample * raw_channels,
	};

	input_zxtune() : params(Parameters::Container::Create()) {}
	~input_zxtune() { close(); }

	void open(service_ptr_t<file> p_filehint, const char* p_path, t_input_open_reason p_reason, abort_callback& p_abort);
	void close();
	unsigned get_subsong_count();
	t_uint32 get_subsong(unsigned p_index);
	void get_info(t_uint32 p_subsong, file_info& p_info, abort_callback& p_abort);
	t_filestats get_file_stats(abort_callback & p_abort) { return m_file->get_stats(p_abort); }

	void decode_initialize(t_uint32 p_subsong, unsigned p_flags, abort_callback& p_abort);
	bool decode_run(audio_chunk& p_chunk, abort_callback& p_abort);
	void decode_seek(double p_seconds, abort_callback& p_abort);
	bool decode_can_seek() { return true; }
	bool decode_get_dynamic_info(file_info & p_out, double & p_timestamp_delta) { return false; }
	bool decode_get_dynamic_info_track(file_info & p_out, double & p_timestamp_delta) { return false; }
	void decode_on_idle(abort_callback & p_abort) {}

	void retag_set_info(t_uint32 p_subsong, const file_info & p_info,abort_callback & p_abort) { throw exception_io_unsupported_format(); }
	void retag_commit(abort_callback & p_abort) {}
	
	static bool g_is_our_content_type(const char * p_content_type) { return false; } // match against supported mime types here
	static bool g_is_our_path(const char* p_path, const char* p_extension);

	static const char* g_get_name() { return "ZXTune Decoders"; }
	static GUID g_get_guid();

private:
	void ParseModules(abort_callback& p_abort);
	std::string SubName(t_uint32 p_subsong);
	double FrameDuration(Parameters::Accessor::Ptr props) const;

private:
	pfc::array_t<t_uint8>	m_buffer;
	service_ptr_t<file>		m_file;
	std::string				m_file_path;
	Binary::Container::Ptr	input_file;

	struct ModuleDesc
	{
		Module::Holder::Ptr module;
		std::string			subname;
	};
	typedef std::vector<ModuleDesc> Modules;
	Modules					input_modules;
	Module::Holder::Ptr		input_module;
	ZXTune::PlayerWrapper::Ptr input_player;

	Parameters::Accessor::Ptr params;

	static_api_ptr_t<metadb> meta_db;

	const char* ZXTUNE_SUBNAME = "ZXTUNE_SUBNAME";
};

static input_factory_t<input_zxtune> g_input_zxtune_factory;

void input_zxtune::open(service_ptr_t<file> p_filehint, const char * p_path,t_input_open_reason p_reason,abort_callback & p_abort)
{
	if (p_reason == input_open_info_write)
		throw exception_io_unsupported_format();//our input does not support retagging.
	m_file_path = p_path;
	m_file = p_filehint;//p_filehint may be null, hence next line
	input_open_file_helper(m_file,p_path,p_reason,p_abort);//if m_file is null, opens file with appropriate privileges for our operation (read/write for writing tags, read-only otherwise).
	t_size size = (t_size)m_file->get_size(p_abort);
	std::unique_ptr<Dump> data(new Dump(size));
	m_file->read(&data->front(), size, p_abort);
	input_file = Binary::CreateContainer(std::move(data));
	if(!input_file)
		throw exception_io_unsupported_format();
	if(p_reason == input_open_info_read || m_file->is_remote())
		ParseModules(p_abort);
}
class FoobarFilesSource : public Module::AdditionalFilesSource
{
public:
	FoobarFilesSource(abort_callback& _abort, const std::string& _file_path) : m_abort(_abort), file_dir(_file_path.c_str())
	{
		file_dir.truncate_to_parent_path();
	}
	virtual Binary::Container::Ptr Get(const String& _name) const override
	{
		pfc::string8 file_path = file_dir;
		file_path += _name.c_str();
		service_ptr_t<file> file;
		input_open_file_helper(file, file_path.c_str(), input_open_info_read, m_abort);
		t_size size = (t_size)file->get_size(m_abort);
		std::unique_ptr<Dump> data(new Dump(size));
		file->read(&data->front(), size, m_abort);
		auto input_file = Binary::CreateContainer(std::move(data));
		if(!input_file)
			throw exception_io_unsupported_format();
		return input_file;
	}
private:
	abort_callback& m_abort;
	pfc::string8 file_dir;
};
void input_zxtune::ParseModules(abort_callback& p_abort)
{
	struct ModuleDetector : public Module::DetectCallback, public FoobarFilesSource
	{
		ModuleDetector(Modules* _mods, abort_callback& _abort, const std::string& _file_path) : modules(_mods), FoobarFilesSource(_abort, _file_path) {}
		virtual void ProcessModule(ZXTune::DataLocation::Ptr location, ZXTune::Plugin::Ptr, Module::Holder::Ptr holder) const
		{
			ModuleDesc m;
			m.module = holder;
			m.subname = location->GetPath()->AsString();
			if(m.subname.empty())
			{
				if(const auto files = dynamic_cast<const Module::AdditionalFiles*>(holder.get()))
				{
					Module::ResolveAdditionalFiles(*this, *files);
				}
			}
			modules->push_back(m);
		}
		virtual Log::ProgressCallback* GetProgress() const { return NULL; }
		Modules* modules;
	};

	ModuleDetector md(&input_modules, p_abort, m_file_path);
	Module::Detect(*params, input_file, md);
	if(input_modules.empty())
	{
		input_file.reset();
		throw exception_io_unsupported_format();
	}
}
void input_zxtune::close()
{
	if(input_file)
	{
		input_player.reset();
		input_modules.clear();
		input_module.reset();
		input_file.reset();
	}
}
unsigned input_zxtune::get_subsong_count()
{
	//@note: from input_info_reader: multi-subsong handling is disabled for remote files (see: filesystem::is_remote) for performance reasons.
	//Remote files are always assumed to be single-subsong, with null index.
	if(m_file->is_remote())
		return 1;
	return input_modules.size();
}
t_uint32 input_zxtune::get_subsong(unsigned p_index)
{
	if(get_subsong_count() > 1)
		return p_index + 1; // numerate 1..count
	else
		return 0;
}
std::string input_zxtune::SubName(t_uint32 p_subsong)
{
	metadb_handle_ptr h;
	meta_db->handle_create(h, playable_location_impl(m_file_path.c_str(), p_subsong));
	file_info_impl fi;
	h->get_info(fi);
	const char* subname = fi.meta_get(ZXTUNE_SUBNAME, 0);
	if(subname)
		return subname;
	return std::string();
}
double input_zxtune::FrameDuration(Parameters::Accessor::Ptr props) const
{
	Parameters::IntType frameDuration = Parameters::ZXTune::Sound::FRAMEDURATION_DEFAULT;
	props->FindValue(Parameters::ZXTune::Sound::FRAMEDURATION, frameDuration);
	return double(frameDuration) / Parameters::ZXTune::Sound::FRAMEDURATION_PRECISION;
}

void input_zxtune::get_info(t_uint32 p_subsong, file_info & p_info,abort_callback & p_abort)
{
	Module::Information::Ptr mi;
	Parameters::Accessor::Ptr props;
	std::string subname;
	if(!input_modules.empty())
	{
		t_uint32 idx = p_subsong;
		if(idx > input_modules.size())
			idx = input_modules.size();
		if(idx)
			--idx;
		mi = input_modules[idx].module->GetModuleInformation();
		props = input_modules[idx].module->GetModuleProperties();
		if(!mi)
			throw exception_io_unsupported_format();
		subname = input_modules[idx].subname;
		if(!subname.empty())
			p_info.meta_set(ZXTUNE_SUBNAME, subname.c_str());
	}
	else
	{
		subname = SubName(p_subsong);
		if(!input_module)
			input_module = Module::Open(*params, input_file, subname);
		Module::Holder::Ptr m = input_module;
		if(!m)
			throw exception_io_unsupported_format();

		mi = m->GetModuleInformation();
		if(!mi)
			throw exception_io_unsupported_format();
		props = m->GetModuleProperties();
	}

	double len = mi->FramesCount() * FrameDuration(props);
	p_info.set_length(len);
	Parameters::IntType size;
	if(props->FindValue(Module::ATTR_SIZE, size))
		p_info.info_calculate_bitrate(size, len);

	//note that the values below should be based on contents of the file itself, NOT on user-configurable variables for an example. To report info that changes independently from file contents, use get_dynamic_info/get_dynamic_info_track instead.
	p_info.info_set_int("samplerate", raw_sample_rate);
	p_info.info_set_int("channels", raw_channels);
	p_info.info_set_int("bitspersample", raw_bits_per_sample);
	p_info.info_set("encoding", "synthesized");
	String type;
	if(props->FindValue(Module::ATTR_TYPE, type))
		p_info.info_set("codec", type.c_str());
	String author;
	if(props->FindValue(Module::ATTR_AUTHOR, author) && !author.empty())
		p_info.meta_set("ARTIST", author.c_str());
	String title;
	if(props->FindValue(Module::ATTR_TITLE, title) && !title.empty())
		p_info.meta_set("TITLE", title.c_str());
	else if(subname.length())
		p_info.meta_set("TITLE", subname.c_str());
	String comment;
	if(props->FindValue(Module::ATTR_COMMENT, comment) && !comment.empty())
		p_info.meta_set("COMMENT", comment.c_str());
	String program;
	if(props->FindValue(Module::ATTR_PROGRAM, program) && !program.empty())
		p_info.meta_set("PERFORMER", program.c_str());
	if(input_modules.size() > 1)
	{
		p_info.meta_set("TRACKNUMBER", pfc::format_uint(p_subsong));
		p_info.meta_set("TOTALTRACKS", pfc::format_uint(input_modules.size()));
	}
}
void input_zxtune::decode_initialize(t_uint32 p_subsong, unsigned p_flags, abort_callback & p_abort)
{
	std::string subname = SubName(p_subsong);
	if(!input_module)
		input_module = Module::Open(*params, input_file, subname);
	if(!input_module)
		throw exception_io_unsupported_format(); 

	if(subname.empty())
	{
		if(const auto files = dynamic_cast<const Module::AdditionalFiles*>(input_module.get()))
		{
			FoobarFilesSource ffs(p_abort, m_file_path);
			Module::ResolveAdditionalFiles(ffs, *files);
		}
	}

	Module::Information::Ptr mi = input_module->GetModuleInformation();
	if(!mi)
		throw exception_io_unsupported_format(); 
	input_player = ZXTune::PlayerWrapper::Create(input_module);
	if(!input_player)
		throw exception_io_unsupported_format();

	auto props = input_module->GetModuleProperties();
	String type;
	if(props && props->FindValue(Module::ATTR_TYPE, type))
	{
		using namespace ZXTune;
		auto it = std::find_if(player_plugins.begin(), player_plugins.end(), [&type](const PlayerPlugin::Ptr& p) { return p->GetDescription()->Id() == type; });
		if(it != player_plugins.end())
			console::formatter() << "ZXTune: using codec " << type.c_str() << " (" << (*it)->GetDescription()->Description().c_str() << ")";
	}
}
bool input_zxtune::decode_run(audio_chunk & p_chunk,abort_callback & p_abort)
{
	enum { deltaread = 1024 };
	m_buffer.set_size(deltaread * raw_total_sample_width);
	t_size deltaread_done = input_player->RenderSound(reinterpret_cast<Sound::Sample*>(m_buffer.get_ptr()), deltaread);
	if(deltaread_done == 0) return false;//EOF
	p_chunk.set_data_fixedpoint(m_buffer.get_ptr(),deltaread_done * raw_total_sample_width,raw_sample_rate,raw_channels,raw_bits_per_sample,audio_chunk::g_guess_channel_config(raw_channels));
	return deltaread_done == deltaread; // EOF when deltaread_done != deltaread
}
void input_zxtune::decode_seek(double p_seconds,abort_callback & p_abort)
{
	// IMPORTANT: convert time to sample offset with proper rounding! audio_math::time_to_samples does this properly for you.
	t_uint64 s = audio_math::time_to_samples(p_seconds, raw_sample_rate);
	Module::Information::Ptr mi = input_module->GetModuleInformation();
	Parameters::Accessor::Ptr props = input_module->GetModuleProperties();
	t_uint64 max_s = audio_math::time_to_samples(mi->FramesCount() * FrameDuration(props), raw_sample_rate);
	if(s > max_s)
		s = max_s;
	input_player->Seek((size_t)s);
}
bool input_zxtune::g_is_our_path(const char* p_path, const char* p_extension)
{
	for(const auto& ext : SupportedExts())
	{
		if(stricmp_utf8(p_extension, ext.c_str()) == 0)
			return true;
	}
	return false;
}
GUID input_zxtune::g_get_guid()
{
	static const GUID ZXTune_GUID = { 0x993f9a65, 0x2a67, 0x4bd5, { 0x97, 0xc2, 0x2d, 0x45, 0x53, 0xe4, 0xef, 0x96 } };
	return ZXTune_GUID;
}
