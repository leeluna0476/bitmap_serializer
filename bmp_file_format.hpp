#ifndef BMP_FILE_FORMAT_HPP
# define BMP_FILE_FORMAT_HPP

# include <cstdint>

// 14 bytes
#pragma pack(push, 1)
struct	bmp_file_header
{
	uint16_t	type;		// magic number "BM"
	uint32_t	size;		// bmp file size
	uint16_t	reserved_1;
	uint16_t	reserved_2;
	uint32_t	offbits;
};
#pragma pack(pop)

#endif
