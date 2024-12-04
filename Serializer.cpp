#include "Serializer.hpp"
#include "BmpFileFormat.hpp"
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <termios.h>
#include <unistd.h>

Data	Serializer::data;

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

void	Serializer::setRawMode(const bool enable)
{
	static struct termios oldt, newt;
	if (enable)
	{
	    // 터미널을 raw 모드로 설정
	    tcgetattr(0, &oldt);
	    newt = oldt;
	    newt.c_lflag &= ~(ICANON | ECHO);
	    tcsetattr(0, TCSANOW, &newt);
	}
	else
	{
	    // 원래 터미널 설정 복원
	    tcsetattr(0, TCSANOW, &oldt);
	}
}

uint8_t*	Serializer::generatePalette(uint32_t size, uint32_t color_number)
{
	uint8_t*	palette = NULL;

	try
	{
		palette = new uint8_t[size];

		for (uint16_t i = 0; i < color_number; i++)
		{
			uint16_t index = i << 2; // RGBA 인덱스 (i * 4)

			if (i < 216) {
				// 216색 RGB 조합 (6단계씩)
				uint8_t r = (i / 36) % 6;       // R 값 (0~5)
				uint8_t g = (i / 6) % 6;        // G 값 (0~5)
				uint8_t b = i % 6;              // B 값 (0~5)

				palette[index] = b * 51;        // B 채널
				palette[index + 1] = g * 51;    // G 채널
				palette[index + 2] = r * 51;    // R 채널
				palette[index + 3] = 0;         // Alpha 채널 (0)
			}
			else
			{
				// 39색 회색조 추가
				uint8_t gray = (i - 216) * 6;   // 회색 단계 (0~255)
				palette[index] = gray;          // R = G = B
				palette[index + 1] = gray;
				palette[index + 2] = gray;
				palette[index + 3] = 0;         // Alpha 채널 (0)
			}
		}
	}
	catch (const std::exception& e)
	{
	}

	return palette;
}

void	Serializer::clearPixel()
{
	for (uint32_t j = 0; j < data.terminal_height; j++)
	{
		for (uint32_t i = 0; i < data.terminal_width; i++)
		{
			data.terminal_pixel_data[j][i] = 0;
		}
	}
	initScreen();
}

uint32_t	Serializer::checkEscape(char* cptr)
{
	uint32_t	i = 0;
	for (; i < 2; i++)
	{
		std::cin.read(cptr, 1);
		if (*cptr != "\033["[i])
		{
			return 0;
		}
	}
	return 1;
}

// 유저가 픽셀을 입력하는 즉시 배열에 저장하고 화면에 띄운다.
// 0 <= ti < terminal_width
// 0 <= tj < terminal_height
uint32_t	Serializer::getPixel()
{
	char		c;
	uint32_t	ret = 1;
	if (checkEscape(&c) == 1) // move cursor if escape
	{
		std::cin.read(&c, 1);
		if (c == 'A' && data.tj > 0)
		{
			std::cout << CURSOR_UP;
			--(data.tj);
		}
		else if (c == 'B' && data.tj < data.terminal_height - 1)
		{
			std::cout << CURSOR_DOWN;
			++(data.tj);
		}
		else if (c == 'C' && data.ti < data.terminal_width)
		{
			std::cout << CURSOR_RIGHT;
			++(data.ti);
		}
		else if (c == 'D' && data.ti > 0)
		{
			std::cout << CURSOR_LEFT;
			--(data.ti);
		}
	}
	else // draw pixel or others
	{
		if (c == 'L')
		{
			clearPixel();
		}
		else if (c == '\n') // 입력 끝?
		{
			ret = chooseOption(FINISH_DRAWING);
		}
		else if ((c == '1' || c == '2' || c == '3' || c == '4') && data.ti < data.terminal_width)
		{
			data.terminal_pixel_data[data.tj][data.ti] = c - '0';
// 배열에 제대로 저장되고 있는지 테스트하기 위한 코드.
//			std::cout << data.terminal_pixel_data[data.tj][data.ti];
			++(data.ti);
			std::cout << c;
		}
		else if (c == 127 && (data.ti > 0 && data.ti <= data.terminal_width))
		{
			--(data.ti);
			data.terminal_pixel_data[data.tj][data.ti] = 0;
			std::cout << "\033[D" << c << "\033[D";
		}
	}
	return ret;
}

// mode
// 0 -> choose bgcolor
// 1 -> finish drawing yes or no
//
// option
// 0 -> black, no
// 1 -> white, yes
void	Serializer::displayOption(enum optionDisplayMode mode, enum button option)
{
	const uint32_t	tab_horiz = data.terminal_width + 5;

	switch (mode)
	{
		case BGCOLOR:
			std::cout << "\033[5;H" \
				<< "┌────────────────────────┐\n" \
				<< "│     Select bgcolor     │\n" \
				<< "│                        │\n" \
				<< "│    [black]  [white]    │\n" \
				<< "└────────────────────────┘";

			switch (option)
			{
				case LEFT:
					std::cout << "\033[8;" << "H│    \033[44m[black]\033[0m  [white]    │";
					break;
				case RIGHT:
					std::cout << "\033[8;" << "H│    [black]  \033[44m[white]\033[0m    │";
					break;
				default:
					break;
			}
			break;
		case PALETTE_TYPE:
			std::cout << "\033[10H" \
				<< "┌────────────────────────┐\n" \
				<< "│     Select palette     │\n" \
				<< "│                        │\n" \
				<< "│     [GRAY]   [RGB]     │\n" \
				<< "└────────────────────────┘";

			switch (option)
			{
				case LEFT:
					std::cout << "\033[13;" << "H│     \033[44m[GRAY]\033[0m   [RGB]     │";
					break;
				case RIGHT:
					std::cout << "\033[13;" << "H│     [GRAY]   \033[44m[RGB]\033[0m     │";
					break;
				default:
					break;
			}
			break;
		case FINISH_DRAWING:
			std::cout \
				<< "\033[13;" << tab_horiz << "H┌────────────────────────┐" \
				<< "\033[14;" << tab_horiz << "H│     Finish drawing     │" \
				<< "\033[15;" << tab_horiz << "H│                        │" \
				<< "\033[16;" << tab_horiz << "H│      [yes]   [no]      │" \
				<< "\033[17;" << tab_horiz << "H└────────────────────────┘";
			switch (option)
			{
				case LEFT:
					std::cout << "\033[16;" << tab_horiz << "H│      \033[44m[yes]\033[0m   [no]      │";
					break;
				case RIGHT:
					std::cout << "\033[16;" << tab_horiz << "H│      [yes]   \033[44m[no]\033[0m      │";
					break;
				default:
					break;
			}
			break;
		case CLEAR:
			std::cout \
				<< "\033[13;" << tab_horiz << "H                          " \
				<< "\033[14;" << tab_horiz << "H                          " \
				<< "\033[15;" << tab_horiz << "H                          " \
				<< "\033[16;" << tab_horiz << "H                          " \
				<< "\033[17;" << tab_horiz << "H                          ";
			break;
		default:
			break;
	}
}

uint8_t	Serializer::chooseOption(enum optionDisplayMode mode)
{
	// 커서 숨기기
	std::cout << "\033[?25l";
	displayOption(mode, LEFT);
	enum button	option = LEFT;
	char	c;
	for (;;)
	{
		if (checkEscape(&c) == 1)
		{
			std::cin.read(&c, 1);
			if (c == 'C' || c == 'D')
			{
				option = option == LEFT ? RIGHT : LEFT;
				displayOption(mode, option);
			}
		}
		else if (c == '\n')
		{
			if (mode == FINISH_DRAWING)
			{
				displayOption(CLEAR, RIGHT);
			}
			break;
		}
		usleep(10000);
	}

	// 커서 위치 복원
	std::cout << "\033[" << data.tj + 2 << ";" << data.ti + 2 << "H";
	// 커서 보이기
	std::cout << "\033[?25h";

	return option;
}

void	Serializer::setColorIndex()
{
	data.color_index[0] = data.bgcolor;
	switch (data.palette_type)
	{
		case GRAY:
			data.color_index[1] = 245;
			data.color_index[2] = 235;
			data.color_index[3] = 225;
			break;
		case RGB:
			data.color_index[1] = 180;	// R
			data.color_index[2] = 18;	// G
			data.color_index[3] = 5;	// B
			break;
	}
	data.color_index[4] = ~(data.bgcolor);
}

// pixel 10의 배수로 반올림.
// 혼란을 줄 수 있으니 픽셀 입력 최소 10 이상
uint32_t	Serializer::setConfig()
{
	std::cout << CLEAR_SCREEN << std::flush;

	std::string	user_input;
	// get user input width
	std::cout << "[Enter image width]: ";
	getline(std::cin, user_input);
	std::istringstream(user_input) >> data.raw_width;
	// get user input height
	std::cout << "[Enter image height]: ";
	getline(std::cin, user_input);
	std::istringstream(user_input) >> data.raw_height;

	// 10자리로 반올림하고 10으로 나눈다.
	data.terminal_width = (data.raw_width % 10) >= 5 ? (data.raw_width + 10) / 10 : data.raw_width / 10;
	data.terminal_height = (data.raw_height % 10) >= 5 ? (data.raw_height + 10) / 10 : data.raw_height / 10;

	data.raw_width = data.terminal_width * 10;
	data.raw_height = data.terminal_height * 10;

	if (data.terminal_width == 0 || data.terminal_height == 0)
	{
		std::cerr << "Width and height of the image must be at least 10." << std::endl;
		return 0;
	}

	std::cout << "[Enter filename]: ";
	std::cout << "\n(output: <filename>.bmp)\033[A\033[6D";
	getline(std::cin, data.filename);
	data.filename += ".bmp";

	setRawMode(true);

	data.bgcolor = chooseOption(BGCOLOR) * 0xFF;
	data.palette_type = static_cast<enum paletteType>(chooseOption(PALETTE_TYPE));

	setColorIndex();

	return 1;
}

// 박스 크기 입력받고 박스 띄우기.
// 안내문 띄우기.
// bar 미리 초기화 해두기.
void	Serializer::initScreen()
{
	const char*	palette_name[2] = { "GRAY", "RGB" };
	const char*	background_name[2] = { "BLACK", "WHITE" };

	data.ti = 0;
	data.tj = 0;

	std::cout << CLEAR_SCREEN << std::flush;

	std::ostringstream	oss;
	for (uint32_t i = 0; i < data.terminal_width; i++)
	{
		oss << "═";
	}
	std::cout << "╔" << oss.str() << "╗\n";

	for (uint32_t i = 0; i < data.terminal_height; i++)
	{
		std::cout << "║\033[" << data.terminal_width << "C║\n";
	}
	std::cout << "╚" << oss.str() << "╝";

	// 현재 세팅
	std::cout << LEFT_TOP << "\033[" << data.terminal_width + 5 << "C" \
		<< "[ PROPERTIES ]\n\033["<< data.terminal_width + 6 << "C" \
		<< "Palette: " << palette_name[data.palette_type] << "\n\033[" << data.terminal_width + 6 << "C" \
		<< "Bgcolor: " << background_name[data.bgcolor / 0xFF] << "\n\n";

	// 안내사항
	std::cout << "\033[" << data.terminal_width + 6 << "C" \
		<< "[ USAGE ]\n" << "\033[" << data.terminal_width + 6 << "C" \
		<< "1. Move the cursor by the arrow keys.\n" << "\033[" << data.terminal_width + 6 << "C" \
		<< "2. Enter the color by { 1, 2, 3, 4 }.\n" << "\033[" << data.terminal_width + 6 << "C" \
		<< "  - { 1, 2, 3 } are the three palette colors and 4 is the color opposite to the background color.\n"  << "\033[" << data.terminal_width + 6 << "C" \
		<< "3. Press <backspace> to erase the pixel\n" << "\033[" << data.terminal_width + 6 << "C" \
		<< "4. Enter 'L' to clear the screen.\n";
	std::cout << LEFT_TOP << std::flush;
}

// 지정된 문자 외에는 무시.
void	Serializer::draw()
{
	try
	{
		data.terminal_pixel_data = new uint8_t*[data.terminal_height];
		for (uint32_t i = 0; i < data.terminal_height; i++)
		{
			data.terminal_pixel_data[i] = new uint8_t[data.terminal_width]();
		}
	}
	catch (const std::exception& e)
	{
		return;
		// exception 어디서 잡을지 고민...
	}

	for (;;)
	{
		if (getPixel() == 0)
		{
			break;
		}
		// CPU 부하 방지.
		usleep(10000);
	}
//	std::cout << CLEAR_SCREEN << std::flush;
}

Data*	Serializer::generateImgData()
{
	if (setConfig() == 0)
	{
		return NULL;
	}

	initScreen();
	draw();
	return &data;
}

// uintptr_t의 정체?
uintptr_t	Serializer::serialize(Data* ptr)
{
	struct BmpFileHeader	file_header;
	struct BmpInfoHeader	info_header;

/////INFO//////HEADER///////////////////////////////////
	info_header.size = sizeof(struct BmpInfoHeader);
	// width, height user decision
	info_header.width = ptr->raw_width;
	info_header.height = ptr->raw_height;
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
		if ((palette = generatePalette(palette_size, info_header.color_number)) == NULL)
		{
			std::cerr << "exception" << std::endl;
			return 0;
		}
	}

/////PIXEL//////DATA////////////////////////////////////

	uint8_t*	pixel_data = 0;

	try
	{
		pixel_data = new uint8_t[padded_matrix_size];

		for (uint32_t j = 0; j < info_header.height; j++)
		{
			uint32_t	line_gap = j * padded_row_size;;
			uint32_t	_j = j / 10;
			uint32_t	i = 0;
			for ( ; i < info_header.width; i++)
			{
				pixel_data[line_gap + i] = ptr->color_index[ptr->terminal_pixel_data[_j][i / 10]];
			}

			for ( ; i < padded_row_size; i++)
			{
				pixel_data[line_gap + i] = 0;
			}
		}
	}
	catch (const std::exception& e)
	{
		delete[] palette;
		std::cerr << "exception" << std::endl;
		return 0;
	}

/////GENERATE///////IMAGE///////////////////////////////

	std::ofstream	outfile(ptr->filename, std::ios::binary);
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
