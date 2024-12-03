#ifndef DATA_HPP
# define DATA_HPP

# include <cstdint>
# include <string>

# define CLEAR_SCREEN "\033[2J\033[H"
# define LEFT_TOP "\033[H\033[B\033[C"
# define CURSOR_UP "\033[A"
# define CURSOR_DOWN "\033[B"
# define CURSOR_RIGHT "\033[C"
# define CURSOR_LEFT "\033[D"

# define COLOR_1 0xB4
# define COLOR_2 0x12
# define COLOR_3 0x5

enum	plt_type
{
	GRAY,
	RGB
};

enum	button
{
	RIGHT = 0,
	LEFT
};

enum	optionDisplayMode
{
	BGCOLOR = 0,
	FINISH_DRAWING,
	CLEAR
};

typedef struct _Data
{
	uint32_t		raw_width;
	uint32_t		raw_height;

	uint32_t		terminal_width;
	uint32_t		terminal_height;

	std::string		filename;

	enum plt_type	palette_type;
	uint8_t			bgcolor;

	uint32_t		ti;
	uint32_t		tj;

	uint8_t**		terminal_pixel_data;
} Data;

#endif
