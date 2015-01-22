/*
 * camera.h
 *
 *  Created on: Dec 3, 2014
 *      Author: nbingham
 */

#include "core/geometry.h"
#include "canvas.h"

using namespace core;

#ifndef camera_h
#define camera_h

struct objecthdl;

struct camerahdl
{
	camerahdl();
	virtual ~camerahdl();

	string type;

	objecthdl *model;
	vec3f position;
	vec3f orientation;

	objecthdl *focus;
	float radius;

	virtual void project(canvashdl *canvas) = 0;
	void view(canvashdl *canvas);
};

struct orthohdl : camerahdl
{
	orthohdl();
	~orthohdl();

	float left, right, bottom, top, front, back;

	void project(canvashdl *canvas);
};

struct frustumhdl : camerahdl
{
	frustumhdl();
	~frustumhdl();

	float left, right, bottom, top, front, back;

	void project(canvashdl *canvas);
};

struct perspectivehdl : camerahdl
{
	perspectivehdl();
	~perspectivehdl();

	float fovy, aspect, front, back;

	void project(canvashdl *canvas);
};

#endif
