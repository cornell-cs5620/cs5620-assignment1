/*
 * object.cpp
 *
 *  Created on: Jan 2, 2015
 *      Author: nbingham
 */

#include "object.h"
#include "canvas.h"

rigidhdl::rigidhdl()
{

}

rigidhdl::~rigidhdl()
{

}

/* draw
 *
 * Draw a rigid body.
 */
void rigidhdl::draw(canvashdl *canvas)
{
	canvas->draw_triangles(geometry, indices);
}

objecthdl::objecthdl()
{
	position = vec3f(0.0, 0.0, 0.0);
	orientation = vec3f(0.0, 0.0, 0.0);
	bound = vec6f(1.0e6, -1.0e6, 1.0e6, -1.0e6, 1.0e6, -1.0e6);
	scale = 1.0;
}

objecthdl::objecthdl(const objecthdl &o)
{
	position = o.position;
	orientation = o.orientation;
	bound = o.bound;
	scale = o.scale;
	rigid = o.rigid;
	for (map<string, materialhdl*>::const_iterator i = o.material.begin(); i != o.material.end(); i++)
		material.insert(pair<string, materialhdl*>(i->first, i->second->clone()));
}

objecthdl::~objecthdl()
{
	for (map<string, materialhdl*>::iterator i = material.begin(); i != material.end(); i++)
		if (i->second != NULL)
		{
			delete i->second;
			i->second = NULL;
		}

	material.clear();
}

/* draw
 *
 * Draw the model. Don't forget to apply the transformations necessary
 * for position, orientation, and scale.
 */
void objecthdl::draw(canvashdl *canvas)
{
	canvas->translate(position);
	canvas->rotate(orientation[0], vec3f(1.0, 0.0, 0.0));
	canvas->rotate(orientation[1], vec3f(0.0, 1.0, 0.0));
	canvas->rotate(orientation[2], vec3f(0.0, 0.0, 1.0));
	canvas->scale(vec3f(scale, scale, scale));

	for (int i = 0; i < rigid.size(); i++)
	{
		// TODO Assignment 3: Pass the material as a uniform into the renderer
		rigid[i].draw(canvas);
	}

	canvas->scale(vec3f(1.0/scale, 1.0/scale, 1.0/scale));
	canvas->rotate(-orientation[2], vec3f(0.0, 0.0, 1.0));
	canvas->rotate(-orientation[1], vec3f(0.0, 1.0, 0.0));
	canvas->rotate(-orientation[0], vec3f(1.0, 0.0, 0.0));
	canvas->translate(-position);
}

/* draw_bound
 *
 * Create a representation for the bounding box and
 * render it.
 */
void objecthdl::draw_bound(canvashdl *canvas)
{
	canvas->translate(position);
	canvas->rotate(orientation[0], vec3f(1.0, 0.0, 0.0));
	canvas->rotate(orientation[1], vec3f(0.0, 1.0, 0.0));
	canvas->rotate(orientation[2], vec3f(0.0, 0.0, 1.0));
	canvas->scale(vec3f(scale, scale, scale));
	vector<vec8f> bound_geometry;
	vector<int> bound_indices;
	bound_geometry.reserve(8);
	bound_geometry.push_back(vec8f(bound[0], bound[2], bound[4], 0.0, 0.0, 0.0, 0.0, 0.0));
	bound_geometry.push_back(vec8f(bound[1], bound[2], bound[4], 0.0, 0.0, 0.0, 0.0, 0.0));
	bound_geometry.push_back(vec8f(bound[1], bound[3], bound[4], 0.0, 0.0, 0.0, 0.0, 0.0));
	bound_geometry.push_back(vec8f(bound[0], bound[3], bound[4], 0.0, 0.0, 0.0, 0.0, 0.0));
	bound_geometry.push_back(vec8f(bound[0], bound[2], bound[5], 0.0, 0.0, 0.0, 0.0, 0.0));
	bound_geometry.push_back(vec8f(bound[1], bound[2], bound[5], 0.0, 0.0, 0.0, 0.0, 0.0));
	bound_geometry.push_back(vec8f(bound[1], bound[3], bound[5], 0.0, 0.0, 0.0, 0.0, 0.0));
	bound_geometry.push_back(vec8f(bound[0], bound[3], bound[5], 0.0, 0.0, 0.0, 0.0, 0.0));
	bound_indices.reserve(24);
	for (int i = 0; i < 4; i++)
	{
		bound_indices.push_back(i);
		bound_indices.push_back((i+1)%4);
		bound_indices.push_back(4+i);
		bound_indices.push_back(4+(i+1)%4);
		bound_indices.push_back(i);
		bound_indices.push_back(4+i);
	}
	// TODO Assignment 3: clear the material in the uniform list
	canvas->draw_lines(bound_geometry, bound_indices);
	canvas->scale(vec3f(1.0/scale, 1.0/scale, 1.0/scale));
	canvas->rotate(-orientation[2], vec3f(0.0, 0.0, 1.0));
	canvas->rotate(-orientation[1], vec3f(0.0, 1.0, 0.0));
	canvas->rotate(-orientation[0], vec3f(1.0, 0.0, 0.0));
	canvas->translate(-position);
}

/* draw_normals
 *
 * create a representation of the normals for this object.
 * If face is false, render the vertex normals. Otherwise,
 * calculate the normals for each face and render those.
 */
void objecthdl::draw_normals(canvashdl *canvas, bool face)
{
	float radius = 0.0;
	for (int i = 0; i < 6; i++)
		if (abs(bound[i]) > radius)
			radius = abs(bound[i]);

	vector<vec8f> normal_geometry;
	vector<int> normal_indices;

	canvas->translate(position);
	canvas->rotate(orientation[0], vec3f(1.0, 0.0, 0.0));
	canvas->rotate(orientation[1], vec3f(0.0, 1.0, 0.0));
	canvas->rotate(orientation[2], vec3f(0.0, 0.0, 1.0));
	canvas->scale(vec3f(scale, scale, scale));
	for (int i = 0; i < rigid.size(); i++)
	{
		if (!face)
		{
			for (int j = 0; j < rigid[i].geometry.size(); j++)
			{
				normal_indices.push_back(normal_geometry.size());
				normal_geometry.push_back(rigid[i].geometry[j]);
				normal_geometry.back().set(3,6,vec3f(0.0, 0.0, 0.0));
				normal_indices.push_back(normal_geometry.size());
				normal_geometry.push_back(rigid[i].geometry[j]);
				normal_geometry.back().set(0,3,(vec3f)(normal_geometry.back()(0,3) + radius*0.1f*normal_geometry.back()(3,6)));
				normal_geometry.back().set(3,6,vec3f(0.0, 0.0, 0.0));
			}
		}
		else
		{
			for (int j = 0; j < rigid[i].indices.size(); j+=3)
			{
				vec3f normal = norm((vec3f)rigid[i].geometry[rigid[i].indices[j + 0]](3,6) +
									(vec3f)rigid[i].geometry[rigid[i].indices[j + 1]](3,6) +
									(vec3f)rigid[i].geometry[rigid[i].indices[j + 2]](3,6));
				vec3f center = ((vec3f)rigid[i].geometry[rigid[i].indices[j + 0]](0,3) +
								(vec3f)rigid[i].geometry[rigid[i].indices[j + 1]](0,3) +
								(vec3f)rigid[i].geometry[rigid[i].indices[j + 2]](0,3))/3.0f;
				normal_indices.push_back(normal_geometry.size());
				normal_geometry.push_back(center);
				normal_geometry.back().set(3,8,vec5f(0.0, 0.0, 0.0, 0.0, 0.0));
				normal_indices.push_back(normal_geometry.size());
				normal_geometry.push_back(center + radius*0.1f*normal);
				normal_geometry.back().set(3,8,vec5f(0.0, 0.0, 0.0, 0.0, 0.0));
			}
		}

		// TODO Assignment 3: clear the material in the uniform list
		canvas->draw_lines(normal_geometry, normal_indices);
		normal_geometry.clear();
		normal_indices.clear();
	}
	canvas->scale(vec3f(1.0/scale, 1.0/scale, 1.0/scale));
	canvas->rotate(-orientation[2], vec3f(0.0, 0.0, 1.0));
	canvas->rotate(-orientation[1], vec3f(0.0, 1.0, 0.0));
	canvas->rotate(-orientation[0], vec3f(1.0, 0.0, 0.0));
	canvas->translate(-position);
}
