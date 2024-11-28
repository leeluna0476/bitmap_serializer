#include "Config.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <termios.h>
#include <unistd.h>

// Orthodox Canonical Class Form
Config::Config()
{
}


Config::Config(const Config& _other)
{
	// never
	std::cout << _other.palette_type;
}


void	Config::operator=(const Config& _other)
{
	// never
	std::cout << _other.palette_type;
}

Config::~Config()
{
}

// 프로세스 종료 시 원상태로 복원. false
void	Config::setRawMode(const bool enable)
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

void	Config::clearPixel()
{
	for (uint32_t j = 0; j < real_height; j++)
	{
		for (uint32_t i = 0; i < real_width; i++)
		{
			real_pixel_data[j][i] = 0;
		}
	}
	initScreen();
}

int	Config::checkEscape(char* cptr)
{
	int	i = 0;
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
// 0 <= ti < real_width
// 0 <= tj < real_height
int	Config::getPixel()
{
	char	c;
	int		ret = 1;
	if (checkEscape(&c) == 1) // move cursor if escape
	{
		std::cin.read(&c, 1);
		if (c == 'A' && tj > 0)
		{
			std::cout << CURSOR_UP;
			--tj;
		}
		else if (c == 'B' && tj < real_height - 1)
		{
			std::cout << CURSOR_DOWN;
			++tj;
		}
		else if (c == 'C' && ti < real_width - 1)
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
			ret = !finishDrawing();
		}
		else if ((c == '1' || c == '2' || c == '3') && ti < real_width)
		{
			real_pixel_data[tj][ti] = c - '0';
// 배열에 제대로 저장되고 있는지 테스트하기 위한 코드.
//			std::cout << real_pixel_data[tj][ti];
			ti++;
			std::cout << c;
		}
		else if (c == 127 && (ti > 0 && ti <= real_width))
		{
			--ti;
			real_pixel_data[tj][ti] = 0;
			std::cout << "\033[D" << c << "\033[D";
		}
	}
	return ret;
}

// display: box = 1, erase = 0
// option: yes = 1, no = 0
void	Config::displayOption(const int display, const int option)
{
	const int	tab = real_width + 6;

	switch (display)
	{
		case 1:
			std::cout \
				<< "\033[8;"  << tab << "H┌────────────────────────┐\n" \
				<< "\033[9;"  << tab << "H│     Finish Drawing     │\n" \
				<< "\033[10;" << tab << "H│                        │\n" \
				<< "\033[11;" << tab << "H│      [yes]   [no]      │\n" \
				<< "\033[12;" << tab << "H└────────────────────────┘";

			switch (option)
			{
				case 1:
					std::cout << "\033[11;" << tab << "H│      \033[44m[yes]\033[0m   [no]      │\n";
					break;
				case 0:
					std::cout << "\033[11;" << tab << "H│      [yes]   \033[44m[no]\033[0m      │\n";
					break;
				default:
					break;
			}
			break;
		case 0:
			std::cout \
				<< "\033[8;"  << tab << "H                          \n" \
				<< "\033[9;"  << tab << "H                          \n" \
				<< "\033[10;" << tab << "H                          \n" \
				<< "\033[11;" << tab << "H                          \n" \
				<< "\033[12;" << tab << "H                          ";
			break;
		default:
			break;
	}
}

int	Config::finishDrawing()
{
	// 커서 숨기기
	std::cout << "\033[?25l";
	displayOption(1, 1);
	int		option = 1;
	char	c;
	for (;;)
	{
		if (checkEscape(&c) == 1)
		{
			std::cin.read(&c, 1);
			if (c == 'C') // no
			{
				option = 0;
				displayOption(1, 0);
			}
			else if (c == 'D') // yes
			{
				option = 1;
				displayOption(1, 1);
			}
		}
		else if (c == '\n')
		{
			displayOption(0, 0);
			break;
		}
		usleep(10000);
	}

	std::cout << "\033[" << tj + 2 << ";" << ti + 2 << "H";
	// 커서 보이기
	std::cout << "\033[?25h";
	return option;
}

uint32_t	Config::getRealWidth() const
{
	return real_width;
}

uint32_t	Config::getRealHeight() const
{
	return real_height;
}

const int**	Config::getRealPixelData() const
{
	return const_cast<const int**>(real_pixel_data);
}

enum plt_type	Config::getPaletteType() const
{
	return palette_type;
}

int	Config::getSize()
{
	std::string	user_input;
	// get user input width
	std::cout << "[Enter image width]: " << std::flush;
	getline(std::cin, user_input);
	std::istringstream(user_input) >> real_width;
	// get user input height
	std::cout << "[Enter image height]: " << std::flush;
	getline(std::cin, user_input);
	std::istringstream(user_input) >> real_height;

	if (real_width == 0 || real_height == 0)
	{
		return 0;
	}

	return 1;
}

// 박스 크기 입력받고 박스 띄우기.
// 안내문 띄우기.
void	Config::initScreen()
{
	ti = 0;
	tj = 0;
	std::cout << CLEAR_SCREEN << std::flush;

	std::ostringstream	oss;
	for (uint32_t i = 0; i < real_width; i++)
	{
		// 특수문자여서 string 못 씀
		oss << "═";
	}
	std::cout << "╔" << oss.str() << "╗\n";

	for (uint32_t i = 0; i < real_height; i++)
	{
		std::cout << "║\033[" << real_width << "C║\n";
	}
	std::cout << "╚" << oss.str() << "╝";

	// 안내사항
	std::cout << LEFT_TOP << "\033[" << real_width + 5 << "C" \
		<< "[ USAGE ]\n" << "\033[" << real_width + 6 << "C" \
		<< "1. Move the cursor by the arrow keys.\n" << "\033[" << real_width + 6 << "C" \
		<< "2. Enter the color by { 1, 2, 3 }.\n" << "\033[" << real_width + 6 << "C" \
		<< "3. Enter 'L' to clear the screen.\n";
	std::cout << LEFT_TOP << std::flush;

	setRawMode(true);
}

// 지정된 문자 외에는 무시.
void	Config::draw()
{
	try
	{
		real_pixel_data = new int*[real_height];
		for (uint32_t i = 0; i < real_height; i++)
		{
			real_pixel_data[i] = new int[real_width];
		}
	}
	catch (const std::exception& e)
	{
		return;
		// exception 어디서 잡을지 고민...
	}

	for(;;)
	{
		if (getPixel() == 0)
		{
			break;
		}
		// CPU 부하 방지.
		usleep(10000);
	}
	std::cout << CLEAR_SCREEN << std::flush;
}
