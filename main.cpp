#include "Serializer.hpp"

int	main(void)
{
	Data*	data = Serializer::generateImgData();
	if (data != NULL)
	{
		Serializer::serialize(data);
	}
	return 0;
}
