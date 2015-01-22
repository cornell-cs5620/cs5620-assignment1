/*
 * primitive.h
 *
 *  Created on: Dec 21, 2014
 *      Author: nbingham
 */

#include "object.h"

#ifndef primitive_h
#define primitive_h

struct boxhdl : objecthdl
{
	boxhdl(float width, float height, float depth);
	~boxhdl();
};

struct spherehdl : objecthdl
{
	spherehdl(float radius, int levels, int slices);
	~spherehdl();
};

struct cylinderhdl : objecthdl
{
	cylinderhdl(float radius, float height, int slices);
	~cylinderhdl();
};

struct pyramidhdl : objecthdl
{
	pyramidhdl(float radius, float height, int slices);
	~pyramidhdl();
};

#endif
