#include "BmpFileFormat.hpp"
#include "Serializer.hpp"
#include "env.hpp"
#include <exception>
#include <iostream>
#include <fstream>

// OCCF
Serializer::Serializer()
{
}

// later change cast
Serializer::Serializer(const Serializer& other)
{
	(void)other;
}

void	Serializer::operator=(const Serializer& other)
{
	(void)other;
}

Serializer::~Serializer()
{
}

// uintptr_t의 정체?
uintptr_t	Serializer::serialize(Data* ptr)
{
	struct BmpFileHeader	file_header;
	struct BmpInfoHeader	info_header;
	const Interface&	user_data = ptr->user_data;

/////INFO//////HEADER///////////////////////////////////
	info_header.size = sizeof(struct BmpInfoHeader);
	// width, height user decision
	info_header.width = user_data.getRealWidth();
	info_header.height = user_data.getRealHeight();
	info_header.color_plane = 1;
	// ildan heukbaek. user decision
	info_header.bits_per_pixel = BITS_DEFAULT;
	// BI_RGB = 0. no compression.
	info_header.compression = 0;
	// resolution = 미터당 픽셀 밀집도. 기본값 = 0.
	info_header.horizontal_resolution = 0;
	info_header.vertical_resolution = 0;
	// palette를 사용하지 않을 때(bits_per_pixel > 8)는 0.
	info_header.color_number = info_header.bits_per_pixel > 8 ? 0 : (1 << info_header.bits_per_pixel);
	info_header.color_number_important = 0;

	// 색상 개수 * 4. 바이트 단위.
	uint32_t	palette_size = info_header.color_number << 2;
/////FILE//////HEADER///////////////////////////////////
	file_header.type = 0x4D42;
	// file header size + info header size + palette size + pixel data
	// 각 픽셀 크기는 bits_per_pixel에 따름, 픽셀 개수는 너비와 동, 패딩 사이즈 = (4 - (pixel % 4)) % 4
	// palette 크기는 조건부. 비트 깊이가 8비트 이하일 때만 적용.
	// 사이즈는 모두 바이트 단위.
	// 픽셀 하나가 몇 바이트인지.
	uint16_t	pixel_size = (info_header.bits_per_pixel + 7) >> 3;
	// 행 하나에 할당되는 바이트 수.
	uint32_t	pseudo_row_size = info_header.width * pixel_size;
	// 각 행을 4의 배수로 패딩.
	uint32_t	padding = (4 - (pseudo_row_size % 4)) % 4;
	// 패딩 처리한 행의 바이트 수.
	uint32_t	padded_row_size = pseudo_row_size + padding;
	// 패딩 처리한 행 * 높이.
	uint32_t	padded_matrix_size = info_header.height * padded_row_size;
	file_header.size = \
					   sizeof(struct BmpFileHeader) \
					   + sizeof(struct BmpInfoHeader) \
					   + palette_size \
					   + padded_matrix_size;

	file_header.reserved_1 = 0;
	file_header.reserved_2 = 0;
	file_header.offbits = \
						  sizeof(struct BmpFileHeader) \
						  + sizeof(struct BmpInfoHeader) \
						  + palette_size;

/////COLOR//////TABLE///////////////////////////////////

	uint8_t*	palette = NULL;
	if (info_header.color_number > 0)
	{
		// palette type 도 추후에 변수로 받는다.
		if ((palette = generate_palette(RGB, palette_size, info_header.color_number)) == NULL)
		{
			std::cerr << "exception" << std::endl;
			return 0;
		}
	}

/////PIXEL//////DATA////////////////////////////////////

//	// 비트 깊이가 8 이하일 때는 배열의 요소 개수를 반으로 줄이고, 값을 할당할 때 두 개의 픽셀값을 한번에 할당.
//	// 8비트 이하일 때는 배열의 요소 개수가 실제 픽셀 개수의 반이기 때문에 값을 할당할 때 width를 조정해야 한다.
//	++ 복잡하다 그냥 8비트로 하자. 어차피 색상도 제한할 거고... 굳이 16비트 픽셀을 지원할 이유가 없다.
	uint8_t*	pixel_data = 0;
//	uint32_t	pixel_data_size = padded_matrix_size;
//	uint32_t	pixel_data_row = info_header.width;
//	uint32_t	pixel_data_padded_row = padded_row_size;

	try
	{
		pixel_data = new uint8_t[padded_matrix_size];
		const uint8_t**	real_pixel_data = user_data.getTerminalPixelData();

//		uint32_t		terminal_width = user_data.getTerminalWidth();
//		uint32_t		terminal_height = user_data.getTerminalHeight();
		const uint8_t	color[5] = { user_data.getBgcolor(), COLOR_1, COLOR_2, COLOR_3, ~(ptr->user_data.getBgcolor()) };

		for (uint32_t j = 0; j < info_header.height; j++)
		{
			uint32_t	line_gap = j * padded_row_size;;
			uint32_t	_j = j / 10;
			uint32_t	i = 0;
			for ( ; i < info_header.width; i++)
			{
				pixel_data[line_gap + i] = color[real_pixel_data[_j][i / 10]];
			}

			for ( ; i < padded_row_size; i++)
			{
				pixel_data[line_gap + i] = 0;
			}
		}

//		for (uint32_t j = 0; j < info_header.height; j++)
//		{
//			uint32_t	line_gap = j * pixel_data_padded_row;
//			uint32_t	i = 0;
//
//			for ( ; i < pixel_data_row; i++)
//			{
//				pixel_data[line_gap + i] = color[real_pixel_data[j][i]];
//			}
//
//			for ( ; i < pixel_data_padded_row; i++)
//			{
//				pixel_data[line_gap + i] = 0;
//			}
//		}
	}
	catch (const std::exception& e)
	{
		delete[] palette;
		std::cerr << "exception" << std::endl;
		return 0;
	}

/////GENERATE///////IMAGE///////////////////////////////

	std::ofstream	outfile("test.bmp", std::ios::binary);
	if (outfile.is_open() == 0)
	{
		std::cerr << "Cannot open file" << std::endl;
		delete[] palette;
		delete[] pixel_data;
		return 0;
	}

	// write file header
	outfile.write(reinterpret_cast<const char*>(&file_header), sizeof(struct BmpFileHeader));

	// write info header
	outfile.write(reinterpret_cast<const char*>(&info_header), sizeof(struct BmpInfoHeader));

	// write palette if needed
	if (info_header.color_number != 0)
	{
		outfile.write(reinterpret_cast<const char*>(palette), palette_size);
	}

	// write pixel data
	for (uint32_t i = padded_matrix_size - padded_row_size; i > 0; i -= padded_row_size)
	{
		outfile.write(reinterpret_cast<const char*>(&pixel_data[i]), padded_row_size);
	}
	outfile.write(reinterpret_cast<const char*>(&pixel_data[0]), padded_row_size);

	outfile.close();

	delete[] palette;
	delete[] pixel_data;

	return reinterpret_cast<uintptr_t>(ptr);
}

//Data*	Serializer::deserialize(uintptr_t raw)
//{
//}
