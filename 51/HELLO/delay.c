#include <reg52.h>
#include "delay.h"

//????1ms
void  delay(unsigned int uldata)
{
	unsigned int j  =  0;
	unsigned int g  =  0;
	for (j = 0; j < uldata; j++)
		for (g = 0; g < 110; g++);
}

