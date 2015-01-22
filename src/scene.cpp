/*
 * scene.cpp
 *
 *  Created on: Dec 3, 2014
 *      Author: nbingham
 */

#include "scene.h"
#include "camera.h"
#include "object.h"

#include "primitive.h"
#include "model.h"

scenehdl::scenehdl()
{
	canvas = NULL;
	active_camera = -1;
	active_object = -1;
	render_normals = none;
	render_cameras = false;
}

scenehdl::~scenehdl()
{

}

/* draw
 *
 * Update the locations of all of the lights, draw all of the objects, and
 * if enabled, draw the normals, the lights, the cameras, etc.
 */
void scenehdl::draw()
{
	canvas->uniform.clear();

	if (active_camera_valid())
		cameras[active_camera]->view(canvas);

	for (int i = 0; i < objects.size(); i++)
		if (objects[i] != NULL)
		{
			bool is_camera = false;

			for (int j = 0; j < cameras.size() && !is_camera; j++)
				if (cameras[j] != NULL && cameras[j]->model == objects[i])
					is_camera = true;

			if (!is_camera || (is_camera && render_cameras))
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
