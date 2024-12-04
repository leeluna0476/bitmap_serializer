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

enum	paletteType
{
	GRAY = 0,
	RGB
};

enum	button
{
	LEFT = 0,
	RIGHT
};

enum	optionDisplayMode
{
	BGCOLOR = 0,
	PALETTE_TYPE,
	FINISH_DRAWING,
	CLEAR
};

typedef struct _Data
{
	// image 기준.
	uint32_t			raw_width;
	uint32_t			raw_height;

	// terminal 기준.
	uint32_t			terminal_width;
	uint32_t			terminal_height;

	std::string			filename;

	enum paletteType	palette_type;
	uint8_t				color_index[5];
	uint8_t				bgcolor;

	uint32_t			ti;
	uint32_t			tj;

	uint8_t**			terminal_pixel_data;
} Data;

#endif
