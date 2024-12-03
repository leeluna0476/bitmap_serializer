#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <cstdint>
# include <string>

# define CLEAR_SCREEN "\033[2J\033[H"
# define LEFT_TOP "\033[H\033[B\033[C"
# define CURSOR_UP "\033[A"
# define CURSOR_DOWN "\033[B"
# define CURSOR_RIGHT "\033[C"
# define CURSOR_LEFT "\033[D"

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

class	Interface
{
	private:
	// USER DEFINED
		uint32_t	raw_width;
		uint32_t	raw_height;
		// revised from raw_*
		uint32_t	terminal_width;
		uint32_t	terminal_height;

		std::string	filename;

		// palette type: GRAY, RGB
		enum plt_type	palette_type;
		uint8_t			bgcolor;

		// tui drawing board coordinate
		uint32_t	ti;
		uint32_t	tj;

		// map
		uint8_t**	terminal_pixel_data;

		// Orthodox Canonical Class Form
		Interface(const Interface& _other);
		void	operator=(const Interface& _other);

		void		setRawMode(bool enable);
		void		clearPixel();
		uint32_t	checkEscape(char* cptr);
		uint32_t	getPixel();
		void		displayOption(enum optionDisplayMode mode, enum button option);
		uint32_t	chooseOption(enum optionDisplayMode mode);
	public:
		// Orthodox Canonical Class Form
		Interface();
		~Interface();

		uint32_t		getRawWidth() const;
		uint32_t		getRawHeight() const;

		const std::string&	getFilename() const;

		uint32_t		getTerminalWidth() const;
		uint32_t		getTerminalHeight() const;
		const uint8_t**	getTerminalPixelData() const;

		enum plt_type	getPaletteType() const;
		uint8_t			getBgcolor() const;

		// clear screen, set tui box
		uint32_t		setConfig();
		void			initScreen();
		void			draw();
};

#endif
