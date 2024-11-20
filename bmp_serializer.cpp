#include "bmp_file_format.hpp"
#include <exception>
#include <iostream>
#include <fstream>

int	bmp_serializer()
{
	struct bmp_file_header	file_header;
	struct bmp_info_header	info_header;

// INFO HEADER 채우기
	info_header.size = sizeof(struct bmp_info_header);
	// width, height user decision
	info_header.width = 1920;
	info_header.height = 1080;
	info_header.color_plane = 1;
	// ildan heukbaek. user decision
	info_header.bits_per_pixel = 8;
	// BI_RGB = 0. no compression.
	info_header.compression = 0;
	// resolution = 미터당 픽셀 밀집도. 기본값 = 0.
	info_header.horizontal_resolution = 0;
	info_header.vertical_resolution = 0;
	// palette를 사용하지 않을 때(bits_per_pixel > 8)는 0.
	info_header.color_number = info_header.bits_per_pixel > 8 ? 0 : (1 << info_header.bits_per_pixel);
	info_header.color_number_important = 0;

	// 색상 개수 * 4. 바이트 단위.
	uint32_t	color_table_size = info_header.color_number << 2;
// FILE HEADER 채우기
	file_header.type = 0x4D42;
	// file header size + info header size + palette size + pixel data
	// 각 픽셀 크기는 bits_per_pixel에 따름, 픽셀 개수는 너비와 동, 패딩 사이즈 = (4 - (pixel % 4)) % 4
	// palette 크기는 조건부. 비트 깊이가 8비트 이하일 때만 적용.
	// 사이즈는 모두 바이트 단위.
	// 픽셀 하나가 몇 바이트인지.
	uint16_t	pixel_size = (info_header.bits_per_pixel + 7) >> 3;
	// 행 하나에 할당되는 바이트 수.
	uint32_t	real_row_size = info_header.width * pixel_size;
	// 각 행을 4의 배수로 패딩.
	uint32_t	padding = (4 - (real_row_size % 4)) % 4;
	// 패딩 처리한 행의 바이트 수.
	uint32_t	padded_row_size = real_row_size + padding;
	// 패딩 처리한 행 * 높이.
	uint32_t	padded_matrix_size = info_header.height * padded_row_size;
	file_header.size = \
					   sizeof(struct bmp_file_header) \
					   + sizeof(struct bmp_info_header) \
//					   + (info_header.color_number == 0 ? 0 : (info_header.color_number << 2))
					   + color_table_size \
					   + padded_matrix_size;

	file_header.reserved_1 = 0;
	file_header.reserved_2 = 0;
	file_header.offbits = \
						  sizeof(struct bmp_file_header) \
						  + sizeof(struct bmp_info_header) \
						  + color_table_size;

/////COLOR//////TABLE//////////////////////////////////////////////////////////////////////////////////////

	uint8_t*	color_table = 0;
	if (info_header.color_number > 0)
	{
		try
		{
			color_table = new uint8_t[color_table_size];
			// gray table
			uint8_t	gap = 255 / (info_header.color_number - 1);
			for (uint32_t i = 0; i < info_header.color_number; i++)
			{
				uint32_t	index = i << 2;
				uint8_t		gray = i * gap;
				color_table[index] = gray;		// blue
				color_table[index + 1] = gray;	// green
				color_table[index + 2] = gray;	// red
				color_table[index + 3] = 0x0;	// reserved
			}
		}
		catch (const std::exception& e)
		{
			std::cout << "exception" << std::endl;
			return 0;
		}
	}

	uint16_t*	pixel_data = 0;
	uint32_t	pixel_data_size = padded_matrix_size;
	uint32_t	pixel_data_row = info_header.width;
	uint32_t	pixel_data_padded_row = padded_row_size;
	try
	{
		// 비트 깊이가 8 이하일 때는 배열의 요소 개수를 반으로 줄이고, 값을 할당할 때 두 개의 픽셀값을 한번에 할당.
		// 8비트 이하일 때는 배열의 요소 개수가 실제 픽셀 개수의 반이기 때문에 값을 할당할 때 width를 조정해야 한다.
		if (color_table_size != 0)
		{
			pixel_data_size >>= 1;
			pixel_data_row >>= 1;
			pixel_data_padded_row >>= 1;
		}
		pixel_data = new uint16_t[pixel_data_size];
		for (uint32_t j = 0; j < info_header.height; j++)
		{
			uint32_t	line_gap = j * pixel_data_padded_row;
			uint32_t	i = 0;
			for ( ; i < pixel_data_row; i++)
			{
				// 더미 데이터 할당. 실제 값을 할당할 때는 리틀 엔디언을 고려해야 한다.
				// a, b 순으로 저장하고 싶다면
				// pixel_data[line_gap + i] = (b << 8) | a;
				uint8_t	data = i * 255 / info_header.width;
				pixel_data[line_gap + i] = data | (data << 8);
			}
			for ( ; i < pixel_data_padded_row; i++)
			{
				pixel_data[line_gap + i] = 0;
			}
		}
	}
	catch (const std::exception& e)
	{
		delete[] color_table;
		std::cout << "exception" << std::endl;
		return 0;
	}

	std::ofstream	outfile("test.bmp", std::ios::binary);
	if (outfile.is_open() == 0)
	{
		std::cout << "Cannot open file" << std::endl;
		delete[] color_table;
		delete[] pixel_data;
		return 0;
	}

	// write file header
	outfile.write(reinterpret_cast<const char*>(&file_header), sizeof(struct bmp_file_header));

	// write info header
	outfile.write(reinterpret_cast<const char*>(&info_header), sizeof(struct bmp_info_header));

	// write palette if needed
	if (info_header.color_number != 0)
	{
		outfile.write(reinterpret_cast<const char*>(color_table), color_table_size);
	}

	// write pixel data
	for (uint32_t i = pixel_data_size - pixel_data_padded_row; i > 0; i -= pixel_data_padded_row)
	{
		outfile.write(reinterpret_cast<const char*>(&pixel_data[i]), padded_row_size);
	}
	outfile.write(reinterpret_cast<const char*>(&pixel_data[0]), padded_row_size);

	outfile.close();

	delete[] color_table;
	delete[] pixel_data;

	return 1;
}
