/*
 * material.h
 *
 *  Created on: Dec 18, 2014
 *      Author: nbingham
 */

#include "core/geometry.h"
#include "standard.h"

using namespace core;

#ifndef material_h
#define material_h

struct canvashdl;

struct materialhdl
{
	materialhdl();
	virtual ~materialhdl();

	string type;

	virtual vec3f shade_vertex(canvashdl *canvas, vec3f vertex, vec3f normal, vector<float> &varying) const = 0;
	virtual vec3f shade_fragment(canvashdl *canvas, vector<float> &varying) const = 0;
	virtual materialhdl *clone() const = 0;
};

struct whitehdl : materialhdl
{
	whitehdl();
	~whitehdl();

	vec3f shade_vertex(canvashdl *canvas, vec3f vertex, vec3f normal, vector<float> &varying) const;
	vec3f shade_fragment(canvashdl *canvas, vector<float> &varying) const;
	materialhdl *clone() const;
};

struct gouraudhdl : materialhdl
{
	gouraudhdl();
	~gouraudhdl();

	vec3f emission;
	vec3f ambient;
	vec3f diffuse;
	vec3f specular;
	float shininess;

	vec3f shade_vertex(canvashdl *canvas, vec3f vertex, vec3f normal, vector<float> &varying) const;
	vec3f shade_fragment(canvashdl *canvas, vector<float> &varying) const;
	materialhdl *clone() const;
};

struct phonghdl : materialhdl
{
	phonghdl();
	~phonghdl();

	vec3f emission;
	vec3f ambient;
	vec3f diffuse;
	vec3f specular;
	float shininess;

	vec3f shade_vertex(canvashdl *canvas, vec3f vertex, vec3f normal, vector<float> &varying) const;
	vec3f shade_fragment(canvashdl *canvas, vector<float> &varying) const;
	materialhdl *clone() const;
};

struct customhdl : materialhdl
{
	customhdl();
	~customhdl();

	vec3f shade_vertex(canvashdl *canvas, vec3f vertex, vec3f normal, vector<float> &varying) const;
	vec3f shade_fragment(canvashdl *canvas, vector<float> &varying) const;
	materialhdl *clone() const;
};

#endif
