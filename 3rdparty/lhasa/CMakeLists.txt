project(liblhasa)

set(SRCS_C "lib/crc16.c"
	"lib/ext_header.c"
	"lib/lha_arch_unix.c"
	"lib/lha_arch_win32.c"
	"lib/lha_decoder.c"
	"lib/lha_endian.c"
	"lib/lha_file_header.c"
	"lib/lha_input_stream.c"
	"lib/lha_basic_reader.c"
	"lib/lha_reader.c"
	"lib/macbinary.c"
	"lib/null_decoder.c"
	"lib/lh1_decoder.c"
	"lib/lh5_decoder.c"
	"lib/lh6_decoder.c"
	"lib/lh7_decoder.c"
	"lib/lz5_decoder.c"
	"lib/lzs_decoder.c"
	"lib/pm2_decoder.c")

file(GLOB SRCS_H "lib/public/*.h")

add_library(lhasa ${SRCS_C} ${SRCS_H})
