#include "bmp_file_format.hpp"

int	bmp_serializer()
{
	struct bmp_file_header	file_header;
	struct bmp_info_header	info_header;

	info_header.size = sizeof(struct bmp_info_header);
	// width, height user decision
	info_header.width = 4;
	info_header.height = 4;
	info_header.color_plane = 1;
	// ildan heukbaek
	info_header.bits_per_pixel = 1;

	file_header.type = 0x4D42;
	// file header size + info header size + palette size + pixel data
	// 각 픽셀 크기는 bits_per_pixel에 따름, 픽셀 개수는 너비와 동, 패딩 사이즈 = (4 - (pixel % 4)) % 4
	// palette 크기는 조건부. 비트 깊이가 8비트 이하일 때만 적용.
	uint16_t	pixel_size = (info_header.bits_per_pixel + 7) / 8;
	uint16_t	padding = (4 - ((pixel_size * info_header.width) % 4)) % 4;
	file_header.size = \
					   sizeof(struct bmp_file_header) \
					   + sizeof(struct bmp_info_header) \
					   + (info_header.bits_per_pixel > 8 ? 0 : ((1 << info_header.bits_per_pixel) * 4)) \
					   + (info_header.height * ((info_header.width * pixel_size) + padding));

	file_header.reserved_1 = 0;
	file_header.reserved_2 = 0;
	file_header.offbits = \
						  sizeof(struct bmp_file_header) \
						  + sizeof(struct bmp_info_header) \
						  + (info_header.bits_per_pixel > 8 ? 0 : ((1 << info_header.bits_per_pixel) * 4));

	return 1;
}