/*
 * camera.cpp
 *
 *  Created on: Dec 3, 2014
 *      Author: nbingham
 */

#include "camera.h"
#include "object.h"

camerahdl::camerahdl()
{
	position = vec3f(0.0, 0.0, 0.0);
	orientation = vec3f(0.0, 0.0, 0.0);
	model = NULL;
	type = "camera";
	focus = NULL;
	radius = 10.0f;
}

camerahdl::~camerahdl()
{

}

void camerahdl::view(canvashdl *canvas)
{
	canvas->load_identity();
	if (focus == NULL)
	{
		canvas->rotate(-orientation[0], vec3f(1.0, 0.0, 0.0));
		canvas->rotate(-orientation[1], vec3f(0.0, 1.0, 0.0));
		canvas->rotate(-orientation[2], vec3f(0.0, 0.0, 1.0));
		canvas->translate(-position);
	}
	else
	{
		position = focus->position + ror3(vec3f(0.0, 0.0, radius), orientation);
		canvas->look_at(position,
						focus->position,
						ror3(vec3f(0.0, 1.0, 0.0), orientation));
	}

	if (model != NULL)
	{
		model->position = position;
		model->orientation = orientation;
		model->orientation[0] *= -1;
	}
}

orthohdl::orthohdl()
{
	left = -10.0;
	right = 10.0;
	bottom = -10.0;
	top = 10.0;
	front = 2.0;
	back = 101.0;
	type = "ortho";
}

orthohdl::~orthohdl()
{
}

void orthohdl::project(canvashdl *canvas)
{
	canvas->set_matrix(canvashdl::projection_matrix);
	canvas->load_identity();
	canvas->ortho(left, right, bottom, top, front, back);
	canvas->set_matrix(canvashdl::modelview_matrix);
}

frustumhdl::frustumhdl()
{
	left = -1.0;
	right = 1.0;
	bottom = -1.0;
	top = 1.0;
	front = 2.0;
	back = 101.0;
	type = "frustum";
}

frustumhdl::~frustumhdl()
{

}

void frustumhdl::project(canvashdl *canvas)
{
	canvas->set_matrix(canvashdl::projection_matrix);
	canvas->load_identity();
	canvas->frustum(left, right, bottom, top, front, back);
	canvas->set_matrix(canvashdl::modelview_matrix);
}

perspectivehdl::perspectivehdl()
{
	fovy = m_pi/4.0;
	aspect = 1.0;
	front = 2.0;
	back = 101.0;
	type = "perspective";
}

perspectivehdl::~perspectivehdl()
{

}

void perspectivehdl::project(canvashdl *canvas)
{
	canvas->set_matrix(canvashdl::projection_matrix);
	canvas->load_identity();
	canvas->perspective(fovy, aspect, front, back);
	canvas->set_matrix(canvashdl::modelview_matrix);
}
