#include "Serializer.hpp"
#include "BmpFileFormat.hpp"
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <termios.h>
#include <unistd.h>

//Data	Serializer::data;
const uint8_t Serializer::palette[][3] =
{
	// GRAY
	{
		245,
		235,
		225
	},
	// RGB
	{
		180,
		18,
		5
	}
};


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

void	Serializer::clearPixel(Data& data)
{
	for (uint32_t j = 0; j < data.terminal_height; j++)
	{
		for (uint32_t i = 0; i < data.terminal_width; i++)
		{
			data.terminal_pixel_data[j][i] = 0;
		}
	}
	initScreen(data);
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
uint32_t	Serializer::getPixel(Data& data)
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
			clearPixel(data);
		}
		else if (c == '\n') // 입력 끝?
		{
			ret = chooseOption(data, FINISH_DRAWING, 3);
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
void	Serializer::displayOption(Data& data, enum optionDisplayMode mode, int8_t option)
{
	static const char*	option_box[][5] =
	{
		{
			"┌───────────────────────────┐",
			"│      Select  bgcolor      │",
			"│                           │",
			"│     [black]   [white]     │",
			"└───────────────────────────┘"
		},
		{
			"┌───────────────────────────┐",
			"│      Select  palette      │",
			"│                           │",
			"│      [GRAY]    [RGB]      │",
			"└───────────────────────────┘"
		},
		{
			"┌───────────────────────────┐",
			"│      Finish  drawing      │",
			"│                           │",
			"│ [Save] [Continue] [Draft] │",
			"└───────────────────────────┘"
		},
		{
			"                             ",
			"                             ",
			"                             ",
			"                             ",
			"                             "
		}
	};
	static const char*	option_highlight[][3] =
	{
		{
			"│     \033[44m[black]\033[0m   [white]     │",
			"│     [black]   \033[44m[white]\033[0m     │",
			""
		},
		{
			"│      \033[44m[GRAY]\033[0m    [RGB]      │",
			"│      [GRAY]    \033[44m[RGB]\033[0m      │",
			""
		},
		{
			"│ \033[44m[Save]\033[0m [Continue] [Draft] │",
			"│ [Save] \033[44m[Continue]\033[0m [Draft] │",
			"│ [Save] [Continue] \033[44m[Draft]\033[0m │"
		}
	};

	uint32_t	tab_vert = 5;
	uint32_t	tab_horiz = 0;
	if (mode == PALETTE_TYPE)
	{
		tab_vert *= 2;
	}
	else if (mode == FINISH_DRAWING || mode == CLEAR)
	{
		tab_vert = 13;
		tab_horiz = data.terminal_width + 5;
	}

	for (uint8_t i = 0; i < 5; i++)
	{
		std::cout << "\033[" << tab_vert++ << ";" << tab_horiz << "H";
		std::cout << option_box[mode][i] << "\n";
	}

	if (mode != CLEAR)
	{
		std::cout << "\033[" << tab_vert - 2 << ";" << tab_horiz << "H";
		std::cout << option_highlight[mode][option];
	}
}

uint8_t	Serializer::chooseOption(Data& data, enum optionDisplayMode mode, uint8_t button_number)
{
	// 커서 숨기기
	std::cout << "\033[?25l";

	displayOption(data, mode, FIRST);
	int8_t	option = FIRST;

	char	c;
	for (;;)
	{
		if (checkEscape(&c) == 1)
		{
			std::cin.read(&c, 1);
			if (c == 'C')
			{
				option = (option + 1) % button_number;
			}
			else if (c == 'D')
			{
				option = (option - 1 + button_number) % button_number;
			}
			displayOption(data, mode, option);
		}
		else if (c == '\n')
		{
			if (mode == FINISH_DRAWING)
			{
				displayOption(data, CLEAR, FIRST);
			}
			break;
		}
	}

	if (option == 2)
	{
		data.magic_number = 0x4A53;
		data.filename += "_draft";
		option = 0;
	}

	// 커서 위치 복원
	std::cout << "\033[" << data.tj + 2 << ";" << data.ti + 2 << "H";
	// 커서 보이기
	std::cout << "\033[?25h";

	return option;
}

void	Serializer::setColorIndex(Data& data)
{
	data.color_index[0] = data.bgcolor;
	data.color_index[1] = palette[data.palette_type][0];
	data.color_index[2] = palette[data.palette_type][1];
	data.color_index[3] = palette[data.palette_type][2];
	data.color_index[4] = ~(data.bgcolor);
}

// pixel 10의 배수로 반올림.
// 혼란을 줄 수 있으니 픽셀 입력 최소 10 이상
uint32_t	Serializer::setConfig(Data& data)
{
	std::cout << CLEAR_SCREEN << std::flush;

	data.magic_number = 0x4D42;

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

	setRawMode(true);

	data.bgcolor = chooseOption(data, BGCOLOR, 2) * 0xFF;
	data.palette_type = chooseOption(data, PALETTE_TYPE, 2);

	setColorIndex(data);

	return 1;
}

// 박스 크기 입력받고 박스 띄우기.
// 안내문 띄우기.
// bar 미리 초기화 해두기.
void	Serializer::initScreen(Data& data)
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
void	Serializer::draw(Data& data)
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
		if (getPixel(data) == 0)
		{
			break;
		}
	}
//	std::cout << CLEAR_SCREEN << std::flush;
}

Data*	Serializer::generateImgData()
{

	Data*	data = NULL;
	try
	{
		data = new Data;

		if (setConfig(*data) == 0)
		{
			throw std::exception();
		}
		initScreen(*data);
		draw(*data);
		data->filename += ".bmp";
	}
	catch (const std::exception& e)
	{
		delete data;
	}
	return data;
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
	info_header.image_size = 0;
	// resolution = 미터당 픽셀 밀집도. 기본값 = 0.
	info_header.horizontal_resolution = 0;
	info_header.vertical_resolution = 0;
	// palette를 사용하지 않을 때(bits_per_pixel > 8)는 0.
	info_header.color_number = info_header.bits_per_pixel > 8 ? 0 : (1 << info_header.bits_per_pixel);
	info_header.color_number_important = 0;

	// 색상 개수 * 4. 바이트 단위.
	uint32_t	palette_size = info_header.color_number << 2;
/////FILE//////HEADER///////////////////////////////////
	file_header.type = ptr->magic_number;
	// file header size + info header size + palette size + pixel data
	// 각 픽셀 크기는 bits_per_pixel에 따름, 픽셀 개수는 너비와 동, 패딩 사이즈 = (4 - (pixel % 4)) % 4
	// palette 크기는 조건부. 비트 깊이가 8비트 이하일 때만 적용.
	// 사이즈는 모두 바이트 단위.
	// 픽셀 하나에 할당되는 바이트 수. // 8로 올림을 한 뒤 잘랐는데 지금 시점에서는 의미 없어보임.
	uint16_t	pixel_size = (info_header.bits_per_pixel + 7) >> 3;
	// 행 하나에 할당되는 바이트 수.
	uint32_t	row_size_byte = info_header.width * pixel_size;
	// 각 행을 4의 배수로 패딩.
	uint32_t	padding = (4 - (row_size_byte % 4)) % 4;
	// 패딩 처리한 행의 바이트 수.
	uint32_t	padded_row_size = row_size_byte + padding;
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
			uint32_t	line_gap = j * padded_row_size;
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

	if (ptr->magic_number == 0x4A53)
	{
		outfile.write(reinterpret_cast<const char*>(&(ptr->palette_type)), sizeof(uint8_t));
		outfile.write(reinterpret_cast<const char*>(&(ptr->bgcolor)), sizeof(uint8_t));
	}

	outfile.close();

	delete[] palette;
	delete[] pixel_data;

	return reinterpret_cast<uintptr_t>(ptr->filename.c_str());
}

// raw == draft filename
Data*	Serializer::deserialize(uintptr_t raw)
{
	char*	_filename = reinterpret_cast<char*>(raw);
	struct BmpFileHeader	file_header;
	struct BmpInfoHeader	info_header;

	std::ifstream	infile(_filename, std::ios::binary);
	if (infile.is_open() == 0)
	{
		std::cerr << "Cannot open file" << std::endl;
		return NULL;
	}

// CHECK IF IT IS A DRAFT FILE
	infile.read(reinterpret_cast<char*>(&(file_header.type)), 2);
	if (file_header.type != 0x4A53)
	{
		// is not a draft file
		return NULL;
	}

	infile.seekg(16, std::ios::cur);
	infile.read(reinterpret_cast<char*>(&(info_header.width)), 4);
	infile.read(reinterpret_cast<char*>(&(info_header.height)), 4);
	std::cout << info_header.width << std::endl;

	if (info_header.width % 10 != 0 || info_header.height % 10 != 0)
	{
		// is not a draft file
		return NULL;
	}

	infile.seekg(8, std::ios::cur);
	infile.read(reinterpret_cast<char*>(&(info_header.bits_per_pixel)), 2);

	infile.seekg(10, std::ios::beg);
	infile.read(reinterpret_cast<char*>(&(file_header.offbits)), 4);
	infile.seekg(file_header.offbits, std::ios::beg);

// READ PIXEL DATA
	uint16_t	pixel_size = (info_header.bits_per_pixel + 7) >> 3;
	// 행 하나에 할당되는 바이트 수.
	uint32_t	row_size_byte = info_header.width * pixel_size;
	// 각 행을 4의 배수로 패딩.
	uint32_t	padding = (4 - (row_size_byte % 4)) % 4;
	uint32_t	matrix_size = info_header.height * info_header.width;

	uint8_t*	pixel_data = NULL;
	Data*	ptr = NULL;
	try
	{
		pixel_data = new uint8_t[matrix_size];
		// 뒤집어서 저장.
		for (uint32_t j = info_header.height - 1; j > 0; j--)
		{
			uint32_t line_gap = j * info_header.width;
			infile.read(reinterpret_cast<char*>(pixel_data + line_gap), info_header.width);
			infile.seekg(padding, std::ios::cur);
		}
		infile.read(reinterpret_cast<char*>(pixel_data), info_header.width);

		// generate charset

		uint8_t	_palette_type = 0;
		infile.read(reinterpret_cast<char*>(&_palette_type), sizeof(uint8_t));

		uint32_t	charset = (palette[_palette_type][2] << 16) | (palette[_palette_type][1] << 8) | palette[_palette_type][0];
		for (uint32_t i = 0; i < matrix_size; i++)
		{
			uint8_t c = pixel_data[i];
			if ((c == 0x0 || c == 0xFF || (charset & c)) == false)
			{
				throw std::exception();
			}
		}

// DESERIALIZE
		ptr = new Data();
		ptr->magic_number = 0x424D;
		ptr->raw_width = info_header.width;
		ptr->raw_height = info_header.height;
		ptr->terminal_width = info_header.width / 10;
		ptr->terminal_height = info_header.height / 10;
		ptr->filename = _filename;

		size_t	pos = ptr->filename.rfind("_draft.bmp");
		if (pos != std::string::npos)
		{
			ptr->filename[pos] = '\0';
		}

		ptr->palette_type = _palette_type;

		infile.read(reinterpret_cast<char*>(&(ptr->bgcolor)), sizeof(uint8_t));

		ptr->terminal_pixel_data = new uint8_t*[ptr->terminal_height]();
		for (uint32_t j = 0; j < ptr->terminal_height; j++)
		{
			ptr->terminal_pixel_data[j] = new uint8_t[ptr->terminal_width]();
			uint32_t	line_gap = j * 10 * ptr->raw_width;
			for (uint32_t i = 0; i < ptr->terminal_width; i++)
			{
				ptr->terminal_pixel_data[j][i] = pixel_data[line_gap + i * 10];
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "exception" << std::endl;

		infile.close();

		delete[] pixel_data;
		if (ptr->terminal_pixel_data != NULL)
		{
			for (uint32_t i = 0; i < ptr->terminal_height; i++)
			{
				delete ptr->terminal_pixel_data[i];
			}
		}
		delete[] ptr->terminal_pixel_data;
		delete ptr;

		return NULL;
	}

	infile.close();

	delete[] pixel_data;

	return ptr;
}
