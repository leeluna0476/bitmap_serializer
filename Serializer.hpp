#ifndef SERIALIZER_HPP
# define SERIALIZER_HPP

# include "Data.hpp"
# include <cstdint>

class	Serializer
{
	private:
		// no instanciation
		// OCCF
		Serializer();
		Serializer(const Serializer& other);
		void	operator=(const Serializer& other);
		~Serializer();

	public:
		static uintptr_t	serialize(Data* ptr);
//		static Data*		deserialize(uintptr_t raw);
};

#endif
