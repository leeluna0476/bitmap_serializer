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

class	Config
{
	private:
	// USER DEFINED
		uint32_t	real_width;
		uint32_t	real_height;
		// revised from real_*
		uint32_t	pseudo_width;
		uint32_t	pseudo_height;

		// palette type: GRAY, RGB
		enum plt_type	palette_type;
		// background color
		uint8_t			bgcolor;

		// tui drawing board coordinate
		int	ti;
		int	tj;

		// map
		int**	real_pixel_data;

		// Orthodox Canonical Class Form
		Config(const Config& _other);
		void	operator=(const Config& _other);

		void	setRawMode(const bool enable);
		void	clearPixel();
		int		checkEscape(char* cptr);
		int		getPixel();
		void	displayOption(const int display, const int option);
		int		finishDrawing();
	public:
		// Orthodox Canonical Class Form
		Config();
		~Config();

		// clear screen, set tui box
		int		getSize();
		void	initScreen();
		void	draw();
};

#endif
