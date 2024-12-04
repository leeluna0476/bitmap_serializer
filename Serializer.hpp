#ifndef SERIALIZER_HPP
# define SERIALIZER_HPP

# include "Data.hpp"

class	Serializer
{
	private:
		// no instanciation
		// OCCF
		Serializer();
		Serializer(const Serializer& other);
		void	operator=(const Serializer& other);
		~Serializer();

		static Data		data;
		static const char*	palette_index[2][5];

		static void			setRawMode(bool enable);
		static uint8_t*		generatePalette(uint32_t size, uint32_t color_number);
		static void			clearPixel();
		static uint32_t		checkEscape(char* cptr);
		static uint32_t		getPixel();
		static void			displayOption(enum optionDisplayMode mode, enum button option);
		static uint8_t		chooseOption(enum optionDisplayMode mode);

		static void			setColorIndex();
		static uint32_t		setConfig();
		static void			initScreen();
		static void			draw();
	public:
		static Data*		generateImgData();
		static uintptr_t	serialize(Data* ptr);
//		static Data*		deserialize(uintptr_t raw);
};

#endif
