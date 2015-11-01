/*
 * scene.cpp
 *
 *  Created on: Dec 3, 2014
 *      Author: nbingham
 */

#include "scene.h"
#include "camera.h"
#include "object.h"
#include "light.h"

#include "primitive.h"
#include "model.h"

scenehdl::scenehdl()
{
	canvas = NULL;
	active_camera = -1;
	active_object = -1;
	render_normals = none;
	render_lights = false;
	render_cameras = false;
}

scenehdl::~scenehdl()
{
	for (int i = 0; i < (int)objects.size(); i++)
		delete objects[i];
	objects.clear();

	for (int i = 0; i < (int)cameras.size(); i++)
		delete cameras[i];
	cameras.clear();

	for (int i = 0; i < (int)lights.size(); i++)
		delete lights[i];
	lights.clear();
}

/* draw
 *
 * Update the locations of all of the lights, draw all of the objects, and
 * if enabled, draw the normals, the lights, the cameras, etc.
 */
void scenehdl::draw()
{
	/* TODO Assignment 3: Clear the uniform variables and pass the vector of
	 * lights into the renderer as a uniform variable.
	 */

	if (active_camera_valid())
		cameras[active_camera]->view(canvas);

	// TODO Assignment 3: Update the light positions and directions

	for (int i = 0; i < objects.size(); i++)
		if (objects[i] != NULL)
		{
			bool is_camera = false;

			// TODO Assignment 3: Render the lights;

			for (int j = 0; j < cameras.size() && !is_camera; j++)
				if (cameras[j] != NULL && cameras[j]->model == objects[i])
					is_camera = true;

			if (!is_camera || (is_camera && render_cameras && (!active_camera_valid() || objects[i] != cameras[active_camera]->model)))
			{
				objects[i]->draw(canvas);

				if (render_normals == vertex || render_normals == face)
					objects[i]->draw_normals(canvas, render_normals == face);

				if (i == active_object)
					objects[i]->draw_bound(canvas);
			}
		}
}

bool scenehdl::active_camera_valid()
{
	return (active_camera >= 0 && active_camera < cameras.size() && cameras[active_camera] != NULL);
}

bool scenehdl::active_object_valid()
{
	return (active_object >= 0 && active_object < objects.size() && objects[active_object] != NULL);
}
