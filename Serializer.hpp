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

//		static Data		data;
		static const uint8_t	palette[][3];

		static void			setRawMode(bool enable);
		static uint8_t*		generatePalette(uint32_t size, uint32_t color_number);
		static void			clearPixel(Data& data);
		static uint32_t		checkEscape(char* cptr);
		static uint32_t		getPixel(Data& data);
		static void			displayOption(Data& data, enum optionDisplayMode mode, int8_t option);
		static uint8_t		chooseOption(Data& data, enum optionDisplayMode mode, uint8_t button_number);

		static void			setColorIndex(Data& data);
		static uint32_t		setConfig(Data& data);
		static void			initScreen(Data& data);
		static void			draw(Data& data);

		static uint32_t		searchPalette(uint8_t c);
	public:
		static Data*		generateImgData();
		static uintptr_t	serialize(Data* ptr);
		static Data*		deserialize(uintptr_t raw);
};

#endif
