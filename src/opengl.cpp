/*
 * opengl.cpp
 *
 *  Created on: Jan 5, 2015
 *      Author: nbingham
 */

#include "opengl.h"

GLuint load_shader_file(string filename, GLuint type)
{
	GLuint handle = 0;

	ifstream fin(filename.c_str());
	if (!fin.is_open())
	{
		cerr << "Error: Could not find file: " << filename << endl;
		return 0;
	}
	string source((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
	fin.close();

	const char *buffer = source.c_str();
	int length = source.length();

	handle = glCreateShader(type);
	glShaderSource(handle, 1, &buffer, &length);
	glCompileShader(handle);

	char temp[1024];
	length = 1023;
	glGetShaderInfoLog(handle, 1023, &length, temp);
	if (length > 0)
		cerr << temp;

	return handle;
}

GLuint load_shader_source(string source, GLuint type)
{
	GLuint handle = 0;
	const char* data = source.c_str();
	GLint length = source.length();

	if (length == 0)
		return 0;

	handle = glCreateShader(type);
	glShaderSource(handle, 1, &data, &length);
	glCompileShader(handle);

	char temp[1024];
	length = 1023;
	glGetShaderInfoLog(handle, 1023, &length, temp);
	if (length > 0)
		cerr << temp;

	return handle;
}
