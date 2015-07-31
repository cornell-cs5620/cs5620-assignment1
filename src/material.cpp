/*
 * material.cpp
 *
 *  Created on: Dec 18, 2014
 *      Author: nbingham
 */

#include "material.h"
#include "canvas.h"
#include "light.h"

materialhdl::materialhdl()
{
	type = "material";
}

materialhdl::~materialhdl()
{
}

uniformhdl::uniformhdl()
{
	type = "uniform";
	emission = vec3f(0.0, 0.0, 0.0);
	ambient = vec3f(0.1, 0.1, 0.1);
	diffuse = vec3f(1.0, 1.0, 1.0);
	specular = vec3f(1.0, 1.0, 1.0);
	shininess = 1.0;
}

uniformhdl::~uniformhdl()
{

}

/* This is the vertex shader for a uniform material. The vertex and normal are in world coordinates, and
 * you have write access to the varying array which is used to pass data to the fragment shader. Don't
 * forget that this data is interpolated before it reaches the fragment shader.
 */
vec3f uniformhdl::shade_vertex(canvashdl *canvas, vec3f vertex, vec3f normal, vector<float> &varying) const
{
	vec4f eye_space_vertex = canvas->matrices[canvashdl::modelview_matrix]*homogenize(vertex);

	/* TODO Assignment 3: Implement flat and gouraud shading: Get the lights from the canvas using get_uniform.
	 * Add up the results of the shade functions for the lights. Transform the vertex and normal from world
	 * coordinates to eye coordinates before passing them into the shade functions. Calculate the final color
	 * and pass that to the fragment shader through the varying array. Pass the necessary data for phong shading
	 * through the varying array.
	 */

	eye_space_vertex = canvas->matrices[canvashdl::projection_matrix]*eye_space_vertex;
	eye_space_vertex /= eye_space_vertex[3];
	return eye_space_vertex;
}

vec3f uniformhdl::shade_fragment(canvashdl *canvas, vector<float> &varying) const
{
	/* TODO Assignment 3: For flat and gouraud shading, just return the color you passed through the varying array.
	 * Implement phong shading, doing the same thing that you did to implement the gouraud and flat. The difference
	 * is that the normals have been interpolated. Implement the none shading model, this just returns the
	 * color of the material without lighting.
	 */

	return vec3f(1.0, 1.0, 1.0);
}

materialhdl *uniformhdl::clone() const
{
	uniformhdl *result = new uniformhdl();
	result->type = type;
	result->emission = emission;
	result->ambient = ambient;
	result->diffuse = diffuse;
	result->specular = specular;
	result->shininess = shininess;
	return result;
}

nonuniformhdl::nonuniformhdl()
{
	type = "non_uniform";
}

nonuniformhdl::~nonuniformhdl()
{

}

vec3f nonuniformhdl::shade_vertex(canvashdl *canvas, vec3f vertex, vec3f normal, vector<float> &varying) const
{
	vec4f eye_space_vertex = canvas->matrices[canvashdl::modelview_matrix]*homogenize(vertex);

	/* TODO Assignment 3: Implement the vertex shader for some non-uniform material here. You can actually
	 * implement as many as you want, just make sure to make the correct changes in model.cpp when you load
	 * the material library. Same thing goes if you decide to rename this class.
	 */

	eye_space_vertex = canvas->matrices[canvashdl::projection_matrix]*eye_space_vertex;
	eye_space_vertex /= eye_space_vertex[3];
	return eye_space_vertex;
}

vec3f nonuniformhdl::shade_fragment(canvashdl *canvas, vector<float> &varying) const
{
	/* TODO Assignment 3: Implement the fragment shader for some non-uniform material here. You can actually
	 * implement as many as you want, just make sure to make the correct changes in model.cpp when you load
	 * the material library. Same thing goes if you decide to rename this class.
	 */

	return vec3f(1.0, 1.0, 1.0);
}

materialhdl *nonuniformhdl::clone() const
{
	nonuniformhdl *result = new nonuniformhdl();
	result->type = type;
	return result;
}
