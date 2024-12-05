#include "Serializer.hpp"

int	main(void)
{
	Data*	data = Serializer::generateImgData();
	if (data != NULL)
	{
		Serializer::serialize(data);
		delete data;
	}

//	Serializer::deserialize(reinterpret_cast<uintptr_t>("test_draft.bmp"));
	return 0;
}
