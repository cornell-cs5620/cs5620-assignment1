/*
 * light.h
 *
 *  Created on: Dec 3, 2014
 *      Author: nbingham
 */

#include "core/geometry.h"
#include "core/color.h"
#include "standard.h"
using namespace core;

#ifndef light_h
#define light_h

struct objecthdl;
struct canvashdl;

struct lighthdl
{
	lighthdl();
	lighthdl(const vec3f &ambient, const vec3f &diffuse, const vec3f &specular);
	virtual ~lighthdl();

	string type;

	// Constant
	objecthdl *model;
	vec3f ambient;
	vec3f diffuse;
	vec3f specular;

	virtual void update(canvashdl *canvas) = 0;
	virtual void shade(vec3f &ambient, vec3f &diffuse, vec3f &specular, vec3f vertex, vec3f normal, float shininess) const = 0;
};

struct directionalhdl : lighthdl
{
	directionalhdl();
	directionalhdl(const vec3f &direction, const vec3f &ambient, const vec3f &diffuse, const vec3f &specular);
	~directionalhdl();

	// Updated
	vec3f direction;

	void update(canvashdl *canvas);
	void shade(vec3f &ambient, vec3f &diffuse, vec3f &specular, vec3f vertex, vec3f normal, float shininess) const;
};

struct pointhdl : lighthdl
{
	pointhdl();
	pointhdl(const vec3f &position, const vec3f &attenuation, const vec3f &ambient, const vec3f &diffuse, const vec3f &specular);
	~pointhdl();

	// Constant
	vec3f attenuation;

	// Updated
	vec3f position;

	void update(canvashdl *canvas);
	void shade(vec3f &ambient, vec3f &diffuse, vec3f &specular, vec3f vertex, vec3f normal, float shininess) const;
};

struct spothdl : lighthdl
{
	spothdl();
	spothdl(const vec3f &attenuation, const float &cutoff, const float &exponent, const vec3f &ambient, const vec3f &diffuse, const vec3f &specular);
	~spothdl();

	// Constant
	vec3f attenuation;
	float cutoff;
	float exponent;

	// Updated
	vec3f position;
	vec3f direction;

	void update(canvashdl *canvas);
	void shade(vec3f &ambient, vec3f &diffuse, vec3f &specular, vec3f vertex, vec3f normal, float shininess) const;
};

#endif
