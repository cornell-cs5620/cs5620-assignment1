/*
 * canvas.cpp
 *
 *  Created on: Dec 2, 2014
 *      Author: nbingham
 */

#include "canvas.h"
#include "core/geometry.h"

canvashdl::canvashdl(int w, int h)
{
	width = w;
	height = h;
	last_reshape_time = -1.0;
	reshape_width = w;
	reshape_height = h;
	initialized = false;

	color_buffer = new unsigned char[width*height*3];

	screen_texture = 0;
	screen_geometry = 0;
	screen_shader = 0;

	active_matrix = modelview_matrix;

	for (int i = 0; i < 3; i++)
		matrices[i] = identity<float, 4, 4>();

	polygon_mode = line;
	culling = backface;
}

canvashdl::~canvashdl()
{
	if (color_buffer != NULL)
	{
		delete [] color_buffer;
		color_buffer = NULL;
	}
}

void canvashdl::clear_color_buffer()
{
	memset(color_buffer, 0, width*height*3*sizeof(unsigned char));
}

void canvashdl::clear_depth_buffer()
{
}

void canvashdl::resize(int w, int h)
{
	last_reshape_time = -1.0;

	if (color_buffer != NULL)
	{
		delete [] color_buffer;
		color_buffer = NULL;
	}

	width = w;
	height = h;

	color_buffer = new unsigned char[w*h*3];

	glActiveTexture(GL_TEXTURE0);
	check_error(__FILE__, __LINE__);
	glBindTexture(GL_TEXTURE_2D, screen_texture);
	check_error(__FILE__, __LINE__);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, color_buffer);
	check_error(__FILE__, __LINE__);
}

/* set_matrix
 *
 * Change which matrix is active.
 */
void canvashdl::set_matrix(matrix_id matid)
{
	active_matrix = matid;
}

/* load_identity
 *
 * Set the active matrix to the identity matrix.
 */
void canvashdl::load_identity()
{
	matrices[active_matrix] = identity<float, 4, 4>();
}

/* rotate
 *
 * Multiply the active matrix by a rotation matrix.
 */
void canvashdl::rotate(float angle, vec3f axis)
{
	vec3f u = norm(axis);
	mat3f S(0.0, -u[2], u[1],
			u[2], 0.0, -u[0],
			-u[1], u[0], 0.0);
	mat3f uut(axis[0]*axis[0], axis[0]*axis[1], axis[0]*axis[2],
			  axis[0]*axis[1], axis[1]*axis[1], axis[1]*axis[2],
			  axis[0]*axis[2], axis[1]*axis[2], axis[2]*axis[2]);

	mat4f R = identity<float, 4, 4>();
	R.set(0,3,0,3, uut + (float)cos(angle)*(identity<float, 3, 3>() - uut) + (float)sin(angle)*S);

	matrices[active_matrix] = matrices[active_matrix]*R;
}

/* translate
 *
 * Multiply the active matrix by a translation matrix.
 */
void canvashdl::translate(vec3f direction)
{
	matrices[active_matrix] = matrices[active_matrix]*mat4f(1.0, 0.0, 0.0, direction[0],
									 0.0, 1.0, 0.0, direction[1],
									 0.0, 0.0, 1.0, direction[2],
									 0.0, 0.0, 0.0, 1.0);
}

/* scale
 *
 * Multiply the active matrix by a scaling matrix.
 */
void canvashdl::scale(vec3f size)
{
	matrices[active_matrix] = matrices[active_matrix]*mat4f(size[0], 0.0, 0.0, 0.0,
									 0.0, size[1], 0.0, 0.0,
									 0.0, 0.0, size[2], 0.0,
									 0.0, 0.0, 0.0, 1.0);
}

/* perspective
 *
 * Multiply the active matrix by a perspective projection matrix.
 */
void canvashdl::perspective(float fovy, float aspect, float n, float f)
{
	float x = tan(m_pi/2.0 - fovy/2);
	matrices[active_matrix] = mat4f(x/aspect, 0.0, 0.0, 0.0,
									0.0, x, 0.0, 0.0,
									0.0, 0.0, -(f+n)/(f-n), -2.0*f*n/(f-n),
									0.0, 0.0, -1.0, 0.0)*matrices[active_matrix];
}

/* frustum
 *
 * Multiply the active matrix by a frustum projection matrix.
 */
void canvashdl::frustum(float l, float r, float b, float t, float n, float f)
{
	matrices[active_matrix] = mat4f(2.0*n/(r - l), 0.0, (r + l)/(r - l), 0.0,
					   0.0, 2.0*n/(t - b), (t + b)/(t - b), 0.0,
					   0.0, 0.0, -(f + n)/(f - n), -2.0*f*n/(f - n),
					   0.0, 0.0, -1.0, 0.0)*matrices[active_matrix];
}

/* ortho
 *
 * Multiply the active matrix by an orthographic projection matrix.
 */
void canvashdl::ortho(float l, float r, float b, float t, float n, float f)
{
	matrices[active_matrix] = mat4f(2.0/(r - l), 0.0, 0.0, -(r + l)/(r - l),
						   0.0, 2.0/(t - b), 0.0, -(t + b)/(t - b),
						   0.0, 0.0, -2.0/(f - n), -(f + n)/(f - n),
						   0.0, 0.0, 0.0, 1.0)*matrices[active_matrix];
}

void canvashdl::look_at(vec3f eye, vec3f at, vec3f up)
{
	vec3f f = norm(at - eye);
	up = norm(up);
	vec3f s = cross(f, up);
	vec3f u = cross(norm(s), f);
	matrices[active_matrix] = mat4f(s[0], s[1], s[2], 0.0,
									u[0], u[1], u[2], 0.0,
									-f[0], -f[1], -f[2], 0.0,
									0.0, 0.0, 0.0, 1.0)*matrices[active_matrix];
	translate(-eye);
}

/* to_window
 *
 * Given a pixel coordinate (x from 0 to width and y from 0 to height),
 * convert it into window coordinates (x from -1 to 1 and y from -1 to 1).
 */
vec3f canvashdl::to_window(vec2i pixel)
{
	vec3f result(2.0*(float)pixel[0]/(float)(width-1) - 1.0,
				 2.0*(float)(height - 1 - pixel[1])/(float)(height-1) - 1.0,
				 1.04); // <-- I don't actually understand why I need this magic number... Seems strange to me... Shouldn't it be 1.0?

	return result;
}

/* unproject
 *
 * Unproject a window coordinate into world coordinates.
 */
vec3f canvashdl::unproject(vec3f window)
{
	return inverse(matrices[projection_matrix]*matrices[modelview_matrix])*homogenize(window);
}

/* shade_vertex
 *
 * This is the vertex shader. All transformations (normal, projection, modelview, etc)
 * should happen here. Flat and Gouraud shading, those are done here as
 * well.
 */
vec8f canvashdl::shade_vertex(vec8f v)
{
	vec8f point(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	vec4f position = matrices[projection_matrix]*matrices[modelview_matrix]*homogenize(v);
	position /= position[3];
	point.set(0,3,position(0,3));

	vec4f normal = matrices[normal_matrix]*homogenize(v, 3);
	point.set(3,6,normal(0,3));

	return point;
}

/* shade_fragment
 *
 * This is the fragment shader. The pixel color is determined here. Phong shading is also
 * done here.
 */
vec3f canvashdl::shade_fragment(vec8f v)
{
	return vec3f(1.0, 1.0, 1.0);
}

/* plot
 *
 * Plot a pixel and check it against the depth buffer.
 */
void canvashdl::plot(vec2i xy, vec8f v)
{
	int idx = xy[1]*width + xy[0];
	if (xy[0] >= 0 && xy[0] < width && xy[1] >= 0 && xy[1] < height)
	{
		vec3f color = shade_fragment(v);
		color_buffer[idx*3 + 0] = (int)(color[0]*255.0);
		color_buffer[idx*3 + 1] = (int)(color[1]*255.0);
		color_buffer[idx*3 + 2] = (int)(color[2]*255.0);
	}
}

/* plot_point
 *
 * Plot a point given in window coordinates.
 */
void canvashdl::plot_point(vec8f v)
{
	vec2i xy((v[0]+1.0)*0.5*(width-1), (v[1]+1.0)*0.5*(height-1));
	plot(xy, v);
}

/* plot_line
 *
 * Plot a line defined by two points in window coordinates. Use Bresenham's
 * Algorithm for this. Don't forget to interpolate the normals and texture
 * coordinates as well.
 */
void canvashdl::plot_line(vec8f v1, vec8f v2)
{
	vec2i s1((v1[0]+1.0)*0.5*(width-1), (v1[1]+1.0)*0.5*(height-1));
	vec2i s2((v2[0]+1.0)*0.5*(width-1), (v2[1]+1.0)*0.5*(height-1));

	int b = (abs(s2[1] - s1[1]) > abs(s2[0] - s1[0]));

	vec2i dv = s2 - s1;

	vec2i step((int)(dv[0] > 0) - (int)(dv[0] < 0),
			   (int)(dv[1] > 0) - (int)(dv[1] < 0));
	dv[0] *= step[0];
	dv[1] *= step[1];

	int D = 2*dv[1-b] - dv[b];

	plot(s1, v1);

	vec5f ave;
	ave = (v1(3,8) + v2(3,8))/2.0f;

	vec2i p = s1;
	p[b]+=step[b];
	while (step[b]*p[b] < step[b]*s2[b])
	{
		if (D > 0)
		{
			p[1-b] += step[1-b];
			D += 2*(dv[1-b] - dv[b]);
		}
		else
			D += 2*dv[1-b];

		float i12 = (float)(p[b] - s1[b])/(float)(s2[b] - s1[b]);
		vec8f l12;
		l12.set(0,3,(v2(0,3) - v1(0,3))*i12 + v1(0,3));
		l12.set(3,8,ave);

		plot(p, l12);
		p[b] += step[b];
	}
}

/* plot_half_triangle
 *
 * Plot half of a triangle defined by three points in window coordinates (v1, v2, v3).
 * The remaining inputs are as follows (s1, s2, s3) are the pixel coordinates of (v1, v2, v3),
 * and (ave) is the average value of the normal and texture coordinates for flat shading.
 * Use Bresenham's algorithm for this. You may plot the horizontal half or the vertical half.
 */
void canvashdl::plot_half_triangle(vec2i s1, vec2i s2, vec2i s3, vec8f v1, vec8f v2, vec8f v3, vec5f ave)
{
}

/* plot_triangle
 *
 * Use the above functions to plot a whole triangle. Don't forget to
 * take into account the polygon mode. You should be able to render the
 * triangle as 3 points, 3 lines, or a filled in triangle. (v1, v2, v3)
 * are given in window coordinates.
 */
void canvashdl::plot_triangle(vec8f v1, vec8f v2, vec8f v3)
{
	if (polygon_mode == point)
	{
		plot_point(v1);
		plot_point(v2);
		plot_point(v3);
	}
	else if (polygon_mode == line)
	{
		plot_line(v1, v2);
		plot_line(v2, v3);
		plot_line(v3, v1);
	}
}

/* draw_points
 *
 * Draw a set of 3D points on the canvas. Each point in geometry is
 * formatted (vx, vy, vz, nx, ny, nz, s, t). Don't forget to test the
 * points against the clipping plains of the projection. If you don't
 * you'll get weird behavior (especially when objects behind the camera
 * are being rendered).
 */
void canvashdl::draw_points(const vector<vec8f> &geometry)
{
	matrices[normal_matrix] = transpose(inverse(matrices[modelview_matrix]));
	mat4f transform = matrices[projection_matrix]*matrices[modelview_matrix];
	vec4f planes[6];
	for (int i = 0; i < 6; i++)
		planes[i] = transform.row(3) + (float)pow(-1.0, i)*(vec4f)transform.row(i/2);

	vector<vec8f> processed_geometry;
	processed_geometry.reserve(geometry.size());

	for (int i = 0; i < geometry.size(); i += 3)
	{
		bool keep = true;
		for (int j = 0; j < 6 && keep; j++)
			if (dot(homogenize((vec3f)geometry[i]), planes[j]) <= 0)
				keep = false;

		if (keep)
			processed_geometry.push_back(shade_vertex(geometry[i]));
	}

	for (int i = 0; i < processed_geometry.size(); i++)
		plot_point(processed_geometry[i]);
}

/* Draw a set of 3D lines on the canvas. Each point in geometry
 * is formatted (vx, vy, vz, nx, ny, nz, s, t). Don't forget to clip
 * the lines against the clipping planes of the projection. You can't
 * just not render them because you'll get some weird popping at the
 * edge of the view.
 */
void canvashdl::draw_lines(const vector<vec8f> &geometry, const vector<int> &indices)
{
	matrices[normal_matrix] = transpose(inverse(matrices[modelview_matrix]));
	mat4f transform = matrices[projection_matrix]*matrices[modelview_matrix];
	vec4f planes[6];
	for (int i = 0; i < 6; i++)
		planes[i] = transform.row(3) + (float)pow(-1.0, i)*(vec4f)transform.row(i/2);

	vector<vec8f> processed_geometry;
	vector<int> processed_indices;
	processed_geometry.reserve(geometry.size());
	processed_indices.reserve(indices.size());
	vector<int> index_map;
	index_map.resize(geometry.size(), -1);

	for (int i = 0; i < indices.size(); i += 2)
	{
		pair<vec8f, int> x0 = pair<vec8f, int>(geometry[indices[i]], indices[i]);
		pair<vec8f, int> x1 = pair<vec8f, int>(geometry[indices[i+1]], indices[i+1]);
		bool keep = true;

		for (int j = 0; j < 6 && keep; j++)
		{
			float d0 = dot(homogenize(x0.first), planes[j]);
			float d1 = dot(homogenize(x1.first), planes[j]);
			float del = dot(homogenize(x1.first) - homogenize(x0.first), planes[j]);
			float p = -d0/del;

			if (d0 > 0.0 && d1 <= 0.0)
			{
				x1.first = (1-p)*x0.first + p*x1.first;
				x1.second = -1;
			}
			else if (d0 <= 0.0 && d1 > 0.0)
			{
				x0.first = (1-p)*x0.first + p*x1.first;
				x0.second = -1;
			}
			else if (d0 <= 0.0 && d1 <= 0.0)
				keep = false;
		}

		if (keep)
		{
			if (x0.second == -1)
			{
				x0.second = (int)processed_geometry.size();
				processed_geometry.push_back(shade_vertex(x0.first));
			}
			else if (index_map[x0.second] == -1)
			{
				index_map[x0.second] = (int)processed_geometry.size();
				x0.second = (int)processed_geometry.size();
				processed_geometry.push_back(shade_vertex(x0.first));
			}
			else
				x0.second = index_map[x0.second];

			if (x1.second == -1)
			{
				x1.second = (int)processed_geometry.size();
				processed_geometry.push_back(shade_vertex(x1.first));
			}
			else if (index_map[x1.second] == -1)
			{
				index_map[x1.second] = (int)processed_geometry.size();
				x1.second = (int)processed_geometry.size();
				processed_geometry.push_back(shade_vertex(x1.first));
			}
			else
				x1.second = index_map[x1.second];

			processed_indices.push_back(x0.second);
			processed_indices.push_back(x1.second);
		}
	}

	for (int i = 1; i < processed_indices.size(); i+=2)
		plot_line(processed_geometry[processed_indices[i-1]],
				  processed_geometry[processed_indices[i]]);
}

/* Draw a set of 3D triangles on the canvas. Each point in geometry is
 * formatted (vx, vy, vz, nx, ny, nz, s, t). Don't forget to clip the
 * triangles against the clipping planes of the projection. You can't
 * just not render them because you'll get some weird popping at the
 * edge of the view. Also, this is where font/back face culling is implemented.
 */
void canvashdl::draw_triangles(const vector<vec8f> &geometry, const vector<int> &indices)
{
	matrices[normal_matrix] = transpose(inverse(matrices[modelview_matrix]));
	mat4f transform = matrices[projection_matrix]*matrices[modelview_matrix];
	vec4f planes[6];
	for (int i = 0; i < 6; i++)
		planes[i] = transform.row(3) + (float)pow(-1.0, i)*(vec4f)transform.row(i/2);
	vec3f eye = matrices[modelview_matrix].col(3)(0,3);

	vector<vec8f> processed_geometry;
	vector<int> processed_indices;
	processed_geometry.reserve(geometry.size());
	processed_indices.reserve(indices.size());
	vector<int> index_map;
	index_map.resize(geometry.size(), -1);

	for (int i = 0; i < indices.size(); i += 3)
	{
		vector<pair<vec8f, int> > polygon;
		for (int j = 0; j < 3; j++)
			polygon.push_back(pair<vec8f, int>(geometry[indices[i+j]], indices[i+j]));

		vector<pair<vec8f, int> > clipped;
		for (int j = 0; j < 6; j++)
		{
			pair<vec8f, int> x0 = polygon[polygon.size()-1];
			float d0 = dot(homogenize(x0.first), planes[j]);
			for (int k = 0; k < polygon.size(); k++)
			{
				pair<vec8f, int> x1 = polygon[k];
				float d1 = dot(homogenize(x1.first), planes[j]);
				float del = dot(homogenize(x1.first) - homogenize(x0.first), planes[j]);
				float p = -d0/del;

				if (d0 >= 0.0 && d1 >= 0.0)
					clipped.push_back(x1);
				else if (d0 >= 0.0 && d1 < 0.0)
					clipped.push_back(pair<vec8f, int>((1-(p+0.001f))*x0.first + (p+0.001f)*x1.first, -1));
				else if (d0 < 0.0 && d1 >= 0.0)
				{
					clipped.push_back(pair<vec8f, int>((1-(p-0.001f))*x0.first + (p-0.001f)*x1.first, -1));
					clipped.push_back(x1);
				}

				x0 = x1;
				d0 = d1;
			}

			polygon = clipped;
			clipped.clear();
		}

		if (polygon.size() > 2)
		{
			for (int i = 0; i < polygon.size(); i++)
			{
				if (polygon[i].second == -1)
				{
					polygon[i].second = (int)processed_geometry.size();
					polygon[i].first = shade_vertex(polygon[i].first);
					processed_geometry.push_back(polygon[i].first);
				}
				else if (index_map[polygon[i].second] == -1)
				{
					index_map[polygon[i].second] = (int)processed_geometry.size();
					polygon[i].second = (int)processed_geometry.size();
					polygon[i].first = shade_vertex(polygon[i].first);
					processed_geometry.push_back(polygon[i].first);
				}
				else
				{
					polygon[i].second = index_map[polygon[i].second];
					polygon[i].first = processed_geometry[polygon[i].second];
				}
			}

			for (int i = 2; i < polygon.size(); i++)
			{
				vec3f normal = cross(norm((vec3f)polygon[0].first - (vec3f)polygon[i-1].first),
								     norm((vec3f)polygon[i].first - (vec3f)polygon[i-1].first));

				if (culling == disable || (normal[2] >= 0.0 && culling == backface) || (normal[2] <= 0.0 && culling == frontface))
				{
					processed_indices.push_back(polygon[0].second);
					processed_indices.push_back(polygon[i-1].second);
					processed_indices.push_back(polygon[i].second);
				}
			}
		}
	}

	for (int i = 2; i < processed_indices.size(); i+=3)
		plot_triangle(processed_geometry[processed_indices[i-2]],
					  processed_geometry[processed_indices[i-1]],
					  processed_geometry[processed_indices[i]]);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Do not edit anything below here, that code just sets up OpenGL to render a single
 * quad that covers the whole screen, applies the color_buffer as a texture to it, and
 * keeps the buffer size and texture up to date.
 */
void canvashdl::load_texture()
{
	glGenTextures(1, &screen_texture);
	check_error(__FILE__, __LINE__);
	glActiveTexture(GL_TEXTURE0);
	check_error(__FILE__, __LINE__);
	glBindTexture(GL_TEXTURE_2D, screen_texture);
	check_error(__FILE__, __LINE__);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	check_error(__FILE__, __LINE__);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	check_error(__FILE__, __LINE__);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	check_error(__FILE__, __LINE__);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	check_error(__FILE__, __LINE__);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	check_error(__FILE__, __LINE__);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, color_buffer);
	check_error(__FILE__, __LINE__);
}

void canvashdl::load_geometry()
{
	// x, y, s, t
	const GLfloat geometry[] = {
			-1.0, -1.0, 0.0, 0.0,
			 1.0, -1.0, 1.0, 0.0,
			-1.0,  1.0, 0.0, 1.0,
			-1.0,  1.0, 0.0, 1.0,
			 1.0, -1.0, 1.0, 0.0,
			 1.0,  1.0, 1.0, 1.0
	};

	glGenBuffers(1, &screen_geometry);
	glBindBuffer(GL_ARRAY_BUFFER, screen_geometry);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*4*6, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat)*4*6, geometry);
}

void canvashdl::load_shader()
{
	GLuint vertex = load_shader_file(working_directory + "res/canvas.vx", GL_VERTEX_SHADER);
	GLuint fragment = load_shader_file(working_directory + "res/canvas.ft", GL_FRAGMENT_SHADER);

	screen_shader = glCreateProgram();
	glAttachShader(screen_shader, vertex);
	glAttachShader(screen_shader, fragment);
	glLinkProgram(screen_shader);
}

void canvashdl::init_opengl()
{
	glEnable(GL_TEXTURE_2D);
	glViewport(0, 0, width, height);

	load_texture();
	load_geometry();
	load_shader();
	initialized = true;
}

void canvashdl::check_error(const char *file, int line)
{
	GLenum error_code;
	error_code = glGetError();
	if (error_code != GL_NO_ERROR)
		cerr << "error: " << file << ":" << line << ": " << gluErrorString(error_code) << endl;
}

double canvashdl::get_time()
{
	timeval gtime;
	gettimeofday(&gtime, NULL);
	return gtime.tv_sec + gtime.tv_usec*1.0E-6;
}

void canvashdl::viewport(int w, int h)
{
	glViewport(0, 0, w, h);
	last_reshape_time = get_time();
	reshape_width = w;
	reshape_height = h;
}

void canvashdl::swap_buffers()
{
	if (!initialized)
		init_opengl();

	if (last_reshape_time > 0.0 && get_time() - last_reshape_time > 0.125)
		resize(reshape_width, reshape_height);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(screen_shader);
	check_error(__FILE__, __LINE__);

	glActiveTexture(GL_TEXTURE0);
	check_error(__FILE__, __LINE__);
	glBindTexture(GL_TEXTURE_2D, screen_texture);
	check_error(__FILE__, __LINE__);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, color_buffer);
	check_error(__FILE__, __LINE__);
	glUniform1i(glGetUniformLocation(screen_shader, "tex"), 0);
	check_error(__FILE__, __LINE__);

	glBindBuffer(GL_ARRAY_BUFFER, screen_geometry);
	check_error(__FILE__, __LINE__);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	check_error(__FILE__, __LINE__);
	glEnableClientState(GL_VERTEX_ARRAY);
	check_error(__FILE__, __LINE__);

	glTexCoordPointer(2, GL_FLOAT, sizeof(GLfloat)*4, (float*)(sizeof(GLfloat)*2));
	check_error(__FILE__, __LINE__);
	glVertexPointer(2, GL_FLOAT, sizeof(GLfloat)*4, NULL);
	check_error(__FILE__, __LINE__);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	check_error(__FILE__, __LINE__);

	glDisableClientState(GL_VERTEX_ARRAY);
	check_error(__FILE__, __LINE__);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	check_error(__FILE__, __LINE__);

	glutSwapBuffers();
	check_error(__FILE__, __LINE__);
}

int canvashdl::get_width()
{
	return width;
}

int canvashdl::get_height()
{
	return height;
}
