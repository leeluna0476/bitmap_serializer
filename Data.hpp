#ifndef DATA_HPP
# define DATA_HPP

# include <stdint.h>
# include <string>

# define CLEAR_SCREEN "\033[2J\033[H"
# define LEFT_TOP "\033[H\033[B\033[C"
# define CURSOR_UP "\033[A"
# define CURSOR_DOWN "\033[B"
# define CURSOR_RIGHT "\033[C"
# define CURSOR_LEFT "\033[D"

enum	option
{
	FIRST = 0,
	SECOND,
	THIRD
};

union	Option
{
	uint8_t		raw;
	enum option	enumerate;
};

enum	optionDisplayMode
{
	BGCOLOR = 0,
	PALETTE_TYPE,
	FINISH_DRAWING,
	CHOOSE_SD,
	CLEAR
};

typedef struct _Data
{
	uint16_t	magic_number; // BM, SJ
	// image 기준.
	uint32_t	image_width;
	uint32_t	image_height;

	// terminal 기준.
	uint32_t	terminal_width;
	uint32_t	terminal_height;

	std::string	filename;

	uint8_t		bgcolor;
	enum option	palette_type;
	uint8_t		color_index[5];

	uint32_t	ti;
	uint32_t	tj;

	uint8_t**	terminal_pixel_data;
} Data;

#endif
