#ifndef ENV_HPP
# define ENV_HPP

# include <cstdint>

enum	plt_type
{
	GRAY,
	RGB
};

struct	user
{
	uint32_t		width;
	uint32_t		height;
	enum plt_type	palette_type;	// GRAY, RGB
	uint8_t			bgcolor; 		// 0x00 == BLACK, 0xFF == WHITE
};

struct user	get_user_config();
uint8_t*	generate_palette(enum plt_type type, uint32_t size, uint32_t color_number);

#endif
