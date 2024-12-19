/*
 * testClass.cpp
 *
 *  Created on: Feb 4, 2023
 *      Author: klonyyy
 */

#include "testClass.hpp"
#include "math.h"


float TestClass::getSin(float x)
{
	return sinf(x);
}

float TestClass::getCos(float x)
{
	return cosf(x);
}

float TestClass::getLissajousX(float x)
{
	return sinf(x);
}

float TestClass::getLissajousY1(float x)
{
	return sinf(4.0f * x);
}

float TestClass::getLissajousY2(float x)
{
	return sinf(7.0f * x);
}

float TestClass::getLissajousY3(float x)
{
	return sinf(15.0f * x);
}

void TestClass::spin()
{
	x += 0.02f;

	if(x > 2.0f*M_PI)
	{
		x = 0.0f;

		if(tri > 0)
			tri = -1;
		else
			tri = 1;
	}

	triangle += dir * triangleFrequency;

	if(std::abs(triangle) >= 1.0f)
		dir *= -1.0f;

	a = amplitude * sinf(x);
	b = amplitude * sinf(x + (1.0f/3.0f)*2.0f*M_PI);
	c = amplitude * sinf(x + (2.0f/3.0f)*2.0f*M_PI);
}
