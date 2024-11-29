#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <cstdint>

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

class	Config
{
	private:
	// USER DEFINED
		uint32_t	real_width;
		uint32_t	real_height;
		// revised from real_*
//		uint32_t	pseudo_width;
//		uint32_t	pseudo_height;

		// palette type: GRAY, RGB
		enum plt_type	palette_type;
		uint8_t			bgcolor;

		// tui drawing board coordinate
		uint32_t	ti;
		uint32_t	tj;

		// map
		uint8_t**	real_pixel_data;

		// Orthodox Canonical Class Form
		Config(const Config& _other);
		void	operator=(const Config& _other);

		void		setRawMode(bool enable);
		void		clearPixel();
		uint32_t	checkEscape(char* cptr);
		uint32_t	getPixel();
		void		displayOption(enum optionDisplayMode mode, enum button option);
		uint32_t	chooseOption(enum optionDisplayMode mode);
	public:
		// Orthodox Canonical Class Form
		Config();
		~Config();

		uint32_t		getRealWidth() const;
		uint32_t		getRealHeight() const;
		const uint8_t**	getRealPixelData() const;

		enum plt_type	getPaletteType() const;
		uint8_t			getBgcolor() const;

		// clear screen, set tui box
		uint32_t		setConfig();
		void			initScreen();
		void			draw();
};

#endif
