#ifndef ENV_HPP
# define ENV_HPP

# include <cstdint>
# include "Config.hpp"

# define COLOR_1 0xB4
# define COLOR_2 0x12
# define COLOR_3 0x5

struct	user
{
	uint32_t		width;
	uint32_t		height;
	enum plt_type	palette_type;	// GRAY, RGB
	uint8_t			bgcolor; 		// 0x00 == BLACK, 0xFF == WHITE
};

uint8_t*	generate_palette(enum plt_type type, uint32_t size, uint32_t color_number);

#endif
