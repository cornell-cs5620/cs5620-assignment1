#include "opengl.h"
#include "standard.h"
#include "core/geometry.h"

using namespace std;
using namespace core;

#ifndef canvas_h
#define canvas_h

/* This is the renderer. Here you will implement
 * all of the basic OpenGL functionality.
 */
struct canvashdl
{
private:
	unsigned char *color_buffer;
	int width, height;
	int reshape_width, reshape_height;
	double last_reshape_time;
	bool initialized;

	// Interfacing with OpenGL
	GLuint screen_texture;
	GLuint screen_geometry;
	GLuint screen_shader;


	void load_texture();
	void load_geometry();
	void load_shader();
	void init_opengl();
	void check_error(const char *file, int line);
	double get_time();

	// Buffer Management
	void resize(int w, int h);

	// Bresenham's Algorithms
	void plot(vec2i xy, vec8f v);
	void plot_point(vec8f v);
	void plot_line(vec8f v1, vec8f v2);
	void plot_half_triangle(vec2i s1, vec2i s2, vec2i s3, vec8f v1, vec8f v2, vec8f v3, vec5f ave);
	void plot_triangle(vec8f v1, vec8f v2, vec8f v3);

public:
	canvashdl(int w, int h);
	~canvashdl();
    
    string working_directory;

	// Buffer Manipulation
	void clear_color_buffer();
	void clear_depth_buffer();
	void viewport(int w, int h);
	void swap_buffers();

	int get_width();
	int get_height();

	// Matrix Manipulation
	mat4f matrices[3];

	enum matrix_id
	{
		modelview_matrix = 0,
		projection_matrix = 1,
		normal_matrix = 2
	} active_matrix;

	void set_matrix(matrix_id matid);
	void load_identity();
	void rotate(float angle, vec3f axis);
	void translate(vec3f direction);
	void scale(vec3f size);
	void perspective(float fovy, float aspect, float n, float f);
	void frustum(float l, float r, float b, float t, float n, float f);
	void ortho(float l, float r, float b, float t, float n, float f);
	void look_at(vec3f eye, vec3f at, vec3f up);
	vec3f to_window(vec2i pixel);
	vec3f unproject(vec3f window);

	// Drawing and Shading
	enum
	{
		point = 0,
		line = 1,
	} polygon_mode;

	enum
	{
		disable = 0,
		backface = 1,
		frontface = 2
	} culling;

	// Values to pass into the shaders that are uniform across all vertices and fragments
	map<string, const void*> uniform;

	vec8f shade_vertex(vec8f v);
	vec3f shade_fragment(vec8f v);

	void draw_points(const vector<vec8f> &geometry);
	void draw_lines(const vector<vec8f> &geometry, const vector<int> &indices);
	void draw_triangles(const vector<vec8f> &geometry, const vector<int> &indices);
};

#endif
