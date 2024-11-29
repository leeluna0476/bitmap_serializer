#include "env.hpp"
#include <exception>
#include <cstdlib>

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
//				uint32_t	index = i << 2;
//				palette[index] = (uint8_t)(i % 256);
//				palette[index + 1] = (uint8_t)(i * 2 % 256);
//				palette[index + 2] = (uint8_t)(i * 4 % 256);
//				palette[index + 3] = 0x0;


				uint32_t index = i << 2; // RGBA 인덱스 (i * 4)

				if (i < 216) {
					// 216색 RGB 조합 (6단계씩)
					uint8_t r = (i / 36) % 6;       // R 값 (0~5)
					uint8_t g = (i / 6) % 6;        // G 값 (0~5)
					uint8_t b = i % 6;              // B 값 (0~5)

					palette[index] = b * 51;        // B 채널
					palette[index + 1] = g * 51;    // G 채널
					palette[index + 2] = r * 51;    // R 채널
					palette[index + 3] = 0;         // Alpha 채널 (0)
				} else {
					// 39색 회색조 추가
					uint8_t gray = (i - 216) * 6;   // 회색 단계 (0~255)
					palette[index] = gray;          // R = G = B
					palette[index + 1] = gray;
					palette[index + 2] = gray;
					palette[index + 3] = 0;         // Alpha 채널 (0)
				}
			}
		}
	}
	catch (const std::exception& e)
	{
	}

	return palette;
}
