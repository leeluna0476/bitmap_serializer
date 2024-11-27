#include "env.hpp"
#include <exception>
#include <cstdlib>

struct user	get_user_config()
{
	struct user	user_config;

	//
	//

	return user_config;
}

uint8_t*	generate_palette(enum plt_type type, uint32_t size, uint32_t color_number)
{
	uint8_t*	palette = NULL;

	try
	{
		palette = new uint8_t[size];

		if (type == GRAY)
		{
			uint8_t	gap = 255 / (color_number - 1);
			for (uint32_t i = 0; i < color_number; i++)
			{
				uint32_t	index = i << 2;
				uint8_t		gray = i * gap;
				palette[index] = gray;		// blue
				palette[index + 1] = gray;	// green
				palette[index + 2] = gray;	// red
				palette[index + 3] = 0x0;	// reserved
			}
		}
		else if (type == RGB)
		{
//			uint8_t	gap = 255 / (color_number - 1);
			for (uint32_t i = 0; i < color_number; i++)
			{
				uint32_t	index = i << 2;
				palette[index] = (uint8_t)(i % 256);
				palette[index + 1] = (uint8_t)(i * 2 % 256);
				palette[index + 2] = (uint8_t)(i * 4 % 256);
				palette[index + 3] = 0x0;
			}
		}
	}
	catch (const std::exception& e)
	{
	}

	return palette;
}
