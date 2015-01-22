/*
 * model.h
 *
 *  Created on: Dec 3, 2014
 *      Author: nbingham
 */

#include "standard.h"
#include "scene.h"
#include "object.h"

#ifndef model_h
#define model_h

struct modelhdl : objecthdl
{
	modelhdl();
	modelhdl(string filename);
	~modelhdl();

	void load_obj(string filename);
	void load_mtl(string filename);
};

#endif
