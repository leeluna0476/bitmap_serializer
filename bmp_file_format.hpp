#ifndef BMP_FILE_FORMAT_HPP
# define BMP_FILE_FORMAT_HPP

# include <cstdint>

// 14 bytes
#pragma pack(push, 1)
struct	bmp_file_header
{
	uint16_t	type;		// The header field used to identify the BMP and DIB file is 0x42 0x4D in hexadecimal, same as BM in ASCII.
	uint32_t	size;		// The size of the BMP file in bytes.
	uint16_t	reserved_1;	// Reserved; actual value depends on the application that creates the image, if created manually can be 0
	uint16_t	reserved_2;	// Reserved; actual value depends on the application that creates the image, if created manually can be 0
	uint32_t	offbits;	// The offset, i.e. starting address, of the byte where the bitmap image data (pixel array) can be found.
};
#pragma pack(pop)

#endif
