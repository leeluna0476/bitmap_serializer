#include "Interface.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <termios.h>
#include <unistd.h>

// Orthodox Canonical Class Form
Interface::Interface()
{
}


Interface::Interface(const Interface& _other)
{
	// never
	std::cout << _other.palette_type;
}


void	Interface::operator=(const Interface& _other)
{
	// never
	std::cout << _other.palette_type;
}

Interface::~Interface()
{
}

// 프로세스 종료 시 원상태로 복원. false
void	Interface::setRawMode(const bool enable)
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

void	Interface::clearPixel()
{
	for (uint32_t j = 0; j < terminal_height; j++)
	{
		for (uint32_t i = 0; i < terminal_width; i++)
		{
			terminal_pixel_data[j][i] = 0;
		}
	}
	initScreen();
}

uint32_t	Interface::checkEscape(char* cptr)
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
uint32_t	Interface::getPixel()
{
	char		c;
	uint32_t	ret = 0;
	if (checkEscape(&c) == 1) // move cursor if escape
	{
		std::cin.read(&c, 1);
		if (c == 'A' && tj > 0)
		{
			std::cout << CURSOR_UP;
			--tj;
		}
		else if (c == 'B' && tj < terminal_height - 1)
		{
			std::cout << CURSOR_DOWN;
			++tj;
		}
		else if (c == 'C' && ti < terminal_width - 1)
		{
			std::cout << CURSOR_RIGHT;
			++ti;
		}
		else if (c == 'D' && ti > 0)
		{
			std::cout << CURSOR_LEFT;
			--ti;
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
		else if ((c == '1' || c == '2' || c == '3' || c == '4') && ti < terminal_width)
		{
			terminal_pixel_data[tj][ti] = c - '0';
// 배열에 제대로 저장되고 있는지 테스트하기 위한 코드.
//			std::cout << terminal_pixel_data[tj][ti];
			++ti;
			std::cout << c;
		}
		else if (c == 127 && (ti > 0 && ti <= terminal_width))
		{
			--ti;
			terminal_pixel_data[tj][ti] = 0;
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
void	Interface::displayOption(enum optionDisplayMode mode, enum button option)
{
	const uint32_t	tab = terminal_width + 6;

	switch (mode)
	{
		case BGCOLOR:
			std::cout \
				<< "\033[5;" << "H┌────────────────────────┐\n" \
				<< "\033[6;" << "H│     Select bgcolor     │\n" \
				<< "\033[7;" << "H│                        │\n" \
				<< "\033[8;" << "H│    [white]  [black]    │\n" \
				<< "\033[9;" << "H└────────────────────────┘";

			switch (option)
			{
				case LEFT:
					std::cout << "\033[8;" << "H│    \033[44m[white]\033[0m  [black]    │\n";
					break;
				case RIGHT:
					std::cout << "\033[8;" << "H│    [white]  \033[44m[black]\033[0m    │\n";
					break;
				default:
					break;
			}
			break;
		case FINISH_DRAWING:
			std::cout \
				<< "\033[9;"  << tab << "H┌────────────────────────┐\n" \
				<< "\033[10;" << tab << "H│     Finish Drawing     │\n" \
				<< "\033[11;" << tab << "H│                        │\n" \
				<< "\033[12;" << tab << "H│      [yes]   [no]      │\n" \
				<< "\033[13;" << tab << "H└────────────────────────┘";

			switch (option)
			{
				case LEFT:
					std::cout << "\033[12;" << tab << "H│      \033[44m[yes]\033[0m   [no]      │\n";
					break;
				case RIGHT:
					std::cout << "\033[12;" << tab << "H│      [yes]   \033[44m[no]\033[0m      │\n";
					break;
				default:
					break;
			}
			break;
		case CLEAR:
			std::cout \
				<< "\033[9;"  << tab << "H                          \n" \
				<< "\033[10;" << tab << "H                          \n" \
				<< "\033[11;" << tab << "H                          \n" \
				<< "\033[12;" << tab << "H                          \n" \
				<< "\033[13;" << tab << "H                          ";
			break;
		default:
			break;
	}
}

uint32_t	Interface::chooseOption(enum optionDisplayMode mode)
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
			if (c == 'C') // black, no
			{
				option = RIGHT;
				displayOption(mode, RIGHT);
			}
			else if (c == 'D') // white, yes
			{
				option = LEFT;
				displayOption(mode, LEFT);
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
	std::cout << "\033[" << tj + 2 << ";" << ti + 2 << "H";
	// 커서 보이기
	std::cout << "\033[?25h";

	return option;
}

uint32_t	Interface::getRawWidth() const
{
	return raw_width;
}

uint32_t	Interface::getRawHeight() const
{
	return raw_height;
}

const std::string&	Interface::getFilename() const
{
	return filename;
}

uint32_t	Interface::getTerminalWidth() const
{
	return terminal_width;
}

uint32_t	Interface::getTerminalHeight() const
{
	return terminal_height;
}

const uint8_t**	Interface::getTerminalPixelData() const
{
	return const_cast<const uint8_t**>(terminal_pixel_data);
}

enum plt_type	Interface::getPaletteType() const
{
	return palette_type;
}

uint8_t	Interface::getBgcolor() const
{
	return bgcolor;
}

// pixel 10의 배수로 반올림.
// 혼란을 줄 수 있으니 픽셀 입력 최소 10 이상
uint32_t	Interface::setConfig()
{
	std::cout << CLEAR_SCREEN << std::flush;

	std::string	user_input;
	// get user input width
	std::cout << "[Enter image width]: ";
	getline(std::cin, user_input);
	std::istringstream(user_input) >> raw_width;
	// get user input height
	std::cout << "[Enter image height]: ";
	getline(std::cin, user_input);
	std::istringstream(user_input) >> raw_height;

	// 10자리로 반올림하고 10으로 나눈다.
	terminal_width = (raw_width % 10) >= 5 ? (raw_width + 10) / 10 : raw_width / 10;
	terminal_height = (raw_height % 10) >= 5 ? (raw_height + 10) / 10 : raw_height / 10;

	raw_width = terminal_width * 10;
	raw_height = terminal_height * 10;

	if (terminal_width == 0 || terminal_height == 0)
	{
		std::cerr << "Width and height of the image must be at least 10." << std::endl;
		return 0;
	}

	std::cout << "[Enter filename]: ";
	std::cout << "\n(output: <filename>.bmp)\033[A\033[6D";
	getline(std::cin, filename);
	filename += ".bmp";

	setRawMode(true);

	bgcolor = static_cast<int>(chooseOption(BGCOLOR)) * 0xFF;

	return 1;
}

// 박스 크기 입력받고 박스 띄우기.
// 안내문 띄우기.
void	Interface::initScreen()
{
	ti = 0;
	tj = 0;
	std::cout << CLEAR_SCREEN << std::flush;

	std::ostringstream	oss;
	for (uint32_t i = 0; i < terminal_width; i++)
	{
		// 특수문자여서 string 못 씀
		oss << "═";
	}
	std::cout << "╔" << oss.str() << "╗\n";

	for (uint32_t i = 0; i < terminal_height; i++)
	{
		std::cout << "║\033[" << terminal_width << "C║\n";
	}
	std::cout << "╚" << oss.str() << "╝";

	// 안내사항
	std::cout << LEFT_TOP << "\033[" << terminal_width + 5 << "C" \
		<< "[ USAGE ]\n" << "\033[" << terminal_width + 6 << "C" \
		<< "1. Move the cursor by the arrow keys.\n" << "\033[" << terminal_width + 6 << "C" \
		<< "2. Enter the color by { 1, 2, 3, 4 }.\n" << "\033[" << terminal_width + 6 << "C" \
		<< "  - { 1, 2, 3 } are the three palette colors and 4 is the color opposite to the background color.\n"  << "\033[" << terminal_width + 6 << "C" \
		<< "3. Press <backspace> to erase the pixel\n" << "\033[" << terminal_width + 6 << "C" \
		<< "4. Enter 'L' to clear the screen.\n";
	std::cout << LEFT_TOP << std::flush;
}

// 지정된 문자 외에는 무시.
void	Interface::draw()
{
	try
	{
		terminal_pixel_data = new uint8_t*[terminal_height];
		for (uint32_t i = 0; i < terminal_height; i++)
		{
			terminal_pixel_data[i] = new uint8_t[terminal_width]();
		}
	}
	catch (const std::exception& e)
	{
		return;
		// exception 어디서 잡을지 고민...
	}

	for (;;)
	{
		if (getPixel() == 1)
		{
			break;
		}
		// CPU 부하 방지.
		usleep(10000);
	}
//	std::cout << CLEAR_SCREEN << std::flush;
}
