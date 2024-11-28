#include "Config.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <termios.h>
#include <unistd.h>

// Orthodox Caninical Class Form
Config::Config() : ti(1), tj(1)
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

void	Config::moveCursor(const char c)
{
	switch (c)
	{
		case 'A':
			if (tj > 1)
			{
				std::cout << CURSOR_UP;
				--tj;
			}
			break;
		case 'B':
			if (tj < real_height)
			{
				std::cout << CURSOR_DOWN;
				++tj;
			}
			break;
		case 'C':
			if (ti < real_width)
			{
				std::cout << CURSOR_RIGHT;
				++ti;
			}
			break;
		case 'D':
			if (ti > 1)
			{
				std::cout << CURSOR_LEFT;
				--ti;
			}
			break;
		default:
			break;
	}
}

int	Config::initScreen()
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
		std::cout << "║" << std::string(real_width, ' ') << "║\n";
	}
	std::cout << "╚" << oss.str() << "╝" << std::endl;
	std::cout << LEFT_TOP << std::flush;

	setRawMode(true);

	return 1;
}

// 일단 커서 이동만 구현해둠.
void	Config::draw()
{
	int i;
	for(;;)
	{
		char	seq[3];
		i = 0;
		for (; i < 2; i++)
		{
			std::cin.read(seq + i, 1);
			if (seq[i] != "\033["[i])
			{
				break;
			}
		}
		if (i == 2)
		{
			std::cin.read(seq + 2, 1);
			moveCursor(seq[2]);
		}
		// CPU 부하 방지.
		usleep(10000);
	}
}
