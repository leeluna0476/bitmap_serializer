#include "Config.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <termios.h>
#include <unistd.h>

// Orthodox Caninical Class Form
Config::Config()
{
};


Config::Config(const Config& _other)
{
};


void	Config::operator=(const Config& _other)
{
};

Config::~Config()
{
};

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
	for (int j = 0; j < real_height; j++)
	{
		for (int i = 0; i < real_width; i++)
		{
			real_pixel_data[j][i] = 0;
		}
	}
	initScreen();
}

// 유저가 픽셀을 입력하는 즉시 배열에 저장하고 화면에 띄운다.
// 0 <= ti < real_width
// 0 <= tj < real_height
void	Config::getPixel()
{
	char	c;
	int i = 0;
	for (; i < 2; i++)
	{
		std::cin.read(&c, 1);
		if (c != "\033["[i])
		{
			break;
		}
	}

	if (i == 2) // move cursor if escape
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
	for (int i = 0; i < real_width; i++)
	{
		// 특수문자여서 string 못 씀
		oss << "═";
	}
	std::cout << "╔" << oss.str() << "╗\n";

	for (int i = 0; i < real_height; i++)
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
		for (int i = 0; i < real_height; i++)
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
		getPixel();
		// CPU 부하 방지.
		usleep(10000);
	}
}
