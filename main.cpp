#include "Serializer.hpp"
#include <iostream>

int	main(void)
{
	Data*	data = Serializer::generateImgData();
	if (data != NULL)
	{
		Serializer::serialize(data);
		delete data;
	}
	else
	{
		std::cout << "Failed to generate image data." << std::endl;
	}

//	Serializer::deserialize(reinterpret_cast<uintptr_t>("test_draft.bmp"));
	return 0;
}
