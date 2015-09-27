/*
 * main.cpp
 *
 *  Created on: Dec 2, 2014
 *      Author: nbingham 
 */

#include "opengl.h"
#include "standard.h"
#include "canvas.h"
#include "scene.h"
#include "camera.h"
#include "model.h"
#include "primitive.h"
#include "tinyfiledialogs.h"
#include "light.h"

int window_id;

canvashdl canvas(750, 750);
scenehdl scene;

int mousex = 0, mousey = 0;
bool bound = false;
bool menu = false;

int canvas_menu_id;
int object_menu_id;
int light_menu_id;
int camera_menu_id;

namespace manipulate
{
	enum type
	{
		none = -1,
		translate = 0,
		rotate = 1,
		scale = 2,
		fovy = 3,
		aspect = 4,
		width = 5,
		height = 6,
		front = 7,
		back = 8
	};
}

manipulate::type manipulator;

bool keys[256];

void init(string working_directory)
{
	for (int i = 0; i < 256; i++)
		keys[i] = false;

	canvas.working_directory = working_directory;
	scene.canvas = &canvas;
	scene.cameras.push_back(new frustumhdl());
	scene.objects.push_back(new pyramidhdl(1.0, 1.0, 8));
	for (int k = 0; k < scene.objects.back()->rigid.size(); k++)
		for (int i = 0; i < scene.objects.back()->rigid[k].geometry.size(); i++)
		{
			swap(scene.objects.back()->rigid[k].geometry[i][1], scene.objects.back()->rigid[k].geometry[i][2]);
			scene.objects.back()->rigid[k].geometry[i][1] *= -1.0;
			swap(scene.objects.back()->rigid[k].geometry[i][4], scene.objects.back()->rigid[k].geometry[i][5]);
			scene.objects.back()->rigid[k].geometry[i][4] *= -1.0;
		}
	swap(scene.objects.back()->bound[2], scene.objects.back()->bound[4]);
	swap(scene.objects.back()->bound[3], scene.objects.back()->bound[5]);

	scene.cameras.back()->model = scene.objects.back();
	if (!scene.active_camera_valid())
	{
		scene.active_camera = scene.cameras.size()-1;
		scene.cameras[scene.active_camera]->project(&canvas);
	}
	scene.cameras[scene.active_camera]->position[2] = 10.0;
}

void displayfunc()
{
	canvas.clear_color_buffer();
	canvas.clear_depth_buffer();

	scene.draw();

	canvas.swap_buffers();
}

void reshapefunc(int w, int h)
{
	canvas.viewport(0, 0, w, h);
	glutPostRedisplay();
}

void pmotionfunc(int x, int y)
{
	if (bound)
	{
		glutSetMenu(canvas_menu_id);

		int deltax = x - mousex;
		int deltay = y - mousey;

		mousex = x;
		mousey = y;

		bool warp = false;
		if (mousex > 3*canvas.get_width()/4 || mousex < canvas.get_width()/4)
		{
			mousex = canvas.get_width()/2;
			warp = true;
		}

		if (mousey > 3*canvas.get_height()/4 || mousey < canvas.get_height()/4)
		{
			mousey = canvas.get_height()/2;
			warp = true;
		}

		if (warp)
			glutWarpPointer(mousex, mousey);

		if (scene.active_camera_valid())
		{
			scene.cameras[scene.active_camera]->orientation[1] -= (float)deltax/500.0;
			scene.cameras[scene.active_camera]->orientation[0] -= (float)deltay/500.0;
		}

		glutPostRedisplay();
	}
	else if (scene.active_camera_valid())
	{
		vec3f direction;
		vec3f position;

		if (scene.active_camera_valid())
		{
			if (scene.cameras[scene.active_camera]->type == "ortho")
			{
				position = canvas.unproject(canvas.to_window(vec2i(x, y)));
				direction = ror3(vec3f(0.0f, 0.0f, 1.0f), scene.cameras[scene.active_camera]->orientation);
			}
			else
			{
				position = scene.cameras[scene.active_camera]->position;
				direction = norm(canvas.unproject(canvas.to_window(vec2i(x, y))));
			}
		}

		int old_active_object = scene.active_object;
		scene.active_object = -1;
		for (int i = 0; i < scene.objects.size(); i++)
		{
			if (scene.objects[i] != NULL && scene.cameras[scene.active_camera]->model != scene.objects[i])
			{
				bool is_light = false;
				bool is_camera = false;

				// TODO Assignment 3: uncomment this
				//for (int j = 0; j < scene.lights.size() && !is_light; j++)
				//	if (scene.lights[j] != NULL && scene.lights[j]->model == scene.objects[i])
				//		is_light = true;

				for (int j = 0; j < scene.cameras.size() && !is_camera; j++)
					if (scene.cameras[j] != NULL && scene.cameras[j]->model == scene.objects[i])
						is_camera = true;

				if ((!is_light && !is_camera) || (is_light && scene.render_lights) || (is_camera && scene.render_cameras))
				{
					vec3f invdir = 1.0f/direction;
					vec3i sign((int)(invdir[0] < 0), (int)(invdir[1] < 0), (int)(invdir[2] < 0));
					vec3f origin = position - scene.objects[i]->position;
					float tmin, tmax, tymin, tymax, tzmin, tzmax;
					tmin = (scene.objects[i]->bound[0 + sign[0]]*scene.objects[i]->scale - origin[0])*invdir[0];
					tmax = (scene.objects[i]->bound[0 + 1-sign[0]]*scene.objects[i]->scale - origin[0])*invdir[0];
					tymin = (scene.objects[i]->bound[2 + sign[1]]*scene.objects[i]->scale - origin[1])*invdir[1];
					tymax = (scene.objects[i]->bound[2 + 1-sign[1]]*scene.objects[i]->scale - origin[1])*invdir[1];
					if ((tmin <= tymax) && (tymin <= tmax))
					{
						if (tymin > tmin)
							tmin = tymin;
						if (tymax < tmax)
							tmax = tymax;

						tzmin = (scene.objects[i]->bound[4 + sign[2]]*scene.objects[i]->scale - origin[2])*invdir[2];
						tzmax = (scene.objects[i]->bound[4 + 1-sign[2]]*scene.objects[i]->scale - origin[2])*invdir[2];

						if ((tmin <= tzmax) && (tzmin <= tmax))
						{
							scene.active_object = i;
							i = scene.objects.size();
						}
					}
				}
			}
		}

		if (scene.active_object != old_active_object)
		{
			bool is_light = false;
			bool is_camera = false;

			// TODO Assignment 3: uncomment this
			//for (int i = 0; i < scene.lights.size() && !is_light; i++)
			//	if (scene.lights[i] != NULL && scene.active_object_valid() && scene.lights[i]->model == scene.objects[scene.active_object])
			//		is_light = true;

			for (int i = 0; i < scene.cameras.size() && !is_camera; i++)
				if (scene.cameras[i] != NULL && scene.active_object_valid() && scene.cameras[i]->model == scene.objects[scene.active_object])
					is_camera = true;

			glutDetachMenu(GLUT_RIGHT_BUTTON);
			if (scene.active_object == -1)
				glutSetMenu(canvas_menu_id);
			else if (is_light)
				glutSetMenu(light_menu_id);
			else if (is_camera)
				glutSetMenu(camera_menu_id);
			else
				glutSetMenu(object_menu_id);
			glutAttachMenu(GLUT_RIGHT_BUTTON);
			glutPostRedisplay();
		}
	}
}

void mousefunc(int button, int state, int x, int y)
{
	mousex = x;
	mousey = y;
}

void motionfunc(int x, int y)
{
	if (!bound && !menu)
	{
		int deltax = x - mousex;
		int deltay = mousey - y;

		mousex = x;
		mousey = y;

		vec3f direction;
		vec3f position;

		if (scene.active_camera_valid())
		{
			if (scene.cameras[scene.active_camera]->type == "ortho")
			{
				position = canvas.unproject(canvas.to_window(vec2i(x, y)));
				direction = ror3(vec3f(0.0f, 0.0f, 1.0f), scene.cameras[scene.active_camera]->orientation);
			}
			else
			{
				position = scene.cameras[scene.active_camera]->position;
				direction = norm(canvas.unproject(canvas.to_window(vec2i(x, y))));
			}
		}

		if (scene.active_object_valid() && scene.active_camera_valid())
		{
			if (manipulator == manipulate::translate)
			{
				float d = mag(scene.objects[scene.active_object]->position - position);
				scene.objects[scene.active_object]->position = d*direction + position;
			}
			else if (manipulator == manipulate::rotate)
				scene.objects[scene.active_object]->orientation += vec3f(-(float)deltay/100.0, (float)deltax/100.0, 0.0);
			else if (manipulator == manipulate::scale)
				scene.objects[scene.active_object]->scale += (float)deltay/100.0;

			for (int i = 0; i < scene.cameras.size(); i++)
				if (scene.cameras[i]->model == scene.objects[scene.active_object])
				{
					scene.cameras[i]->position = scene.objects[scene.active_object]->position;
					scene.cameras[i]->orientation = scene.objects[scene.active_object]->orientation;
				}
		}
		if (scene.active_camera_valid())
		{
			if (manipulator == manipulate::fovy && scene.cameras[scene.active_camera]->type == "perspective")
				((perspectivehdl*)scene.cameras[scene.active_camera])->fovy += (float)deltay/100.0;
			else if (manipulator == manipulate::aspect && scene.cameras[scene.active_camera]->type == "perspective")
				((perspectivehdl*)scene.cameras[scene.active_camera])->aspect += (float)deltay/100.0;
			else if (manipulator == manipulate::width && scene.cameras[scene.active_camera]->type == "ortho")
			{
				((orthohdl*)scene.cameras[scene.active_camera])->right += (float)deltay/200.0;
				((orthohdl*)scene.cameras[scene.active_camera])->left -= (float)deltay/200.0;
			}
			else if (manipulator == manipulate::width && scene.cameras[scene.active_camera]->type == "frustum")
			{
				((frustumhdl*)scene.cameras[scene.active_camera])->right += (float)deltay/200.0;
				((frustumhdl*)scene.cameras[scene.active_camera])->left -= (float)deltay/200.0;
			}
			else if (manipulator == manipulate::height && scene.cameras[scene.active_camera]->type == "ortho")
			{
				((orthohdl*)scene.cameras[scene.active_camera])->top += (float)deltay/200.0;
				((orthohdl*)scene.cameras[scene.active_camera])->bottom -= (float)deltay/200.0;
			}
			else if (manipulator == manipulate::height && scene.cameras[scene.active_camera]->type == "frustum")
			{
				((frustumhdl*)scene.cameras[scene.active_camera])->top += (float)deltay/200.0;
				((frustumhdl*)scene.cameras[scene.active_camera])->bottom -= (float)deltay/200.0;
			}
			else if (manipulator == manipulate::front && scene.cameras[scene.active_camera]->type == "ortho")
				((orthohdl*)scene.cameras[scene.active_camera])->front += (float)deltay/100.0;
			else if (manipulator == manipulate::front && scene.cameras[scene.active_camera]->type == "frustum")
				((frustumhdl*)scene.cameras[scene.active_camera])->front += (float)deltay/100.0;
			else if (manipulator == manipulate::front && scene.cameras[scene.active_camera]->type == "perspective")
				((perspectivehdl*)scene.cameras[scene.active_camera])->front += (float)deltay/100.0;
			else if (manipulator == manipulate::back && scene.cameras[scene.active_camera]->type == "ortho")
				((orthohdl*)scene.cameras[scene.active_camera])->back += (float)deltay/100.0;
			else if (manipulator == manipulate::back && scene.cameras[scene.active_camera]->type == "frustum")
				((frustumhdl*)scene.cameras[scene.active_camera])->back += (float)deltay/100.0;
			else if (manipulator == manipulate::back && scene.cameras[scene.active_camera]->type == "perspective")
				((perspectivehdl*)scene.cameras[scene.active_camera])->back += (float)deltay/100.0;

			if (manipulator == manipulate::fovy ||
				manipulator == manipulate::aspect ||
				manipulator == manipulate::width ||
				manipulator == manipulate::height ||
				manipulator == manipulate::front ||
				manipulator == manipulate::back)
				scene.cameras[scene.active_camera]->project(&canvas);
		}

		glutPostRedisplay();
	}
	else if (!bound)
	{
		menu = false;
		pmotionfunc(x, y);
	}
}

void keydownfunc(unsigned char key, int x, int y)
{
	keys[key] = true;

	if (key == 27) // Escape Key Pressed
	{
		glutDestroyWindow(window_id);
		exit(0);
	}
	else if (key == 'm' && bound)
	{
		bound = false;
		glutSetCursor(GLUT_CURSOR_INHERIT);
		glutAttachMenu(GLUT_RIGHT_BUTTON);
	}
	else if (key == 'm' && !bound)
	{
		bound = true;
		glutSetCursor(GLUT_CURSOR_NONE);
		glutDetachMenu(GLUT_RIGHT_BUTTON);
		mousex = x;
		mousey = y;
	}
}

void keyupfunc(unsigned char key, int x, int y)
{
	keys[key] = false;
}

void idlefunc()
{
	bool change = false;
	if (scene.active_camera_valid() && scene.cameras[scene.active_camera]->focus == NULL)
	{
		if (keys['w'])
		{
			scene.cameras[scene.active_camera]->position += -0.25f*ror3(vec3f(0.0, 0.0, 1.0), scene.cameras[scene.active_camera]->orientation);
			change = true;
		}
		if (keys['s'])
		{
			scene.cameras[scene.active_camera]->position += 0.25f*ror3(vec3f(0.0, 0.0, 1.0), scene.cameras[scene.active_camera]->orientation);
			change = true;
		}
		if (keys['a'])
		{
			scene.cameras[scene.active_camera]->position += -0.25f*ror3(vec3f(1.0, 0.0, 0.0), scene.cameras[scene.active_camera]->orientation);
			change = true;
		}
		if (keys['d'])
		{
			scene.cameras[scene.active_camera]->position += 0.25f*ror3(vec3f(1.0, 0.0, 0.0), scene.cameras[scene.active_camera]->orientation);
			change = true;
		}
		if (keys['q'])
		{
			scene.cameras[scene.active_camera]->position += -0.25f*ror3(vec3f(0.0, 1.0, 0.0), scene.cameras[scene.active_camera]->orientation);
			change = true;
		}
		if (keys['e'])
		{
			scene.cameras[scene.active_camera]->position += 0.25f*ror3(vec3f(0.0, 1.0, 0.0), scene.cameras[scene.active_camera]->orientation);
			change = true;
		}
	}
	else if (scene.active_camera_valid() && scene.cameras[scene.active_camera]->focus != NULL)
	{
		if (keys['w'])
		{
			scene.cameras[scene.active_camera]->radius -= 0.25;
			change = true;
		}
		if (keys['s'])
		{
			scene.cameras[scene.active_camera]->radius += 0.25;
			change = true;
		}
	}

	if (change)
		glutPostRedisplay();
}

void menustatusfunc(int status, int x, int y)
{
	if (status == GLUT_MENU_IN_USE)
		menu = true;
}

void canvas_menu(int num)
{
	if (num == 0)
		exit(0);
	else if (num == 1)
		scene.objects.push_back(new boxhdl(1.0, 1.0, 1.0));
	else if (num == 2)
		scene.objects.push_back(new cylinderhdl(1.0, 1.0, 20));
	else if (num == 3)
		scene.objects.push_back(new spherehdl(1.0, 10, 20));
	else if (num == 4)
		scene.objects.push_back(new pyramidhdl(1.0, 1.0, 20));
	else if (num == 5)
	{
		const char* filters[1];
		filters[0] = "*.obj";
		const char *path = tinyfd_openFileDialog("Load a Model", "", 1, filters, 0);
		if (path != NULL && strlen(path) > 0)
			scene.objects.push_back(new modelhdl(path));
	}
	// TODO Assignment 3: uncomment this
	/*else if (num == 6)
		scene.render_lights = !scene.render_lights;
	else if (num == 7)
	{
		scene.lights.push_back(new directionalhdl());
		scene.objects.push_back(new cylinderhdl(0.25, 1.0, 8));
		for (int k = 0; k < scene.objects.back()->rigid.size(); k++)
			for (int i = 0; i < scene.objects.back()->rigid[k].geometry.size(); i++)
			{
				swap(scene.objects.back()->rigid[k].geometry[i][1], scene.objects.back()->rigid[k].geometry[i][2]);
				scene.objects.back()->rigid[k].geometry[i][1] *= -1.0;
				swap(scene.objects.back()->rigid[k].geometry[i][4], scene.objects.back()->rigid[k].geometry[i][5]);
				scene.objects.back()->rigid[k].geometry[i][4] *= -1.0;
			}
		swap(scene.objects.back()->bound[2], scene.objects.back()->bound[4]);
		swap(scene.objects.back()->bound[3], scene.objects.back()->bound[5]);
		scene.lights.back()->model = scene.objects.back();
	}
	else if (num == 8)
	{
		scene.lights.push_back(new pointhdl());
		scene.objects.push_back(new spherehdl(0.25, 4, 8));
		scene.lights.back()->model = scene.objects.back();
	}
	else if (num == 9)
	{
		scene.lights.push_back(new spothdl());
		scene.objects.push_back(new pyramidhdl(0.25, 1.0, 8));
		for (int k = 0; k < scene.objects.back()->rigid.size(); k++)
			for (int i = 0; i < scene.objects.back()->rigid[k].geometry.size(); i++)
			{
				swap(scene.objects.back()->rigid[k].geometry[i][1], scene.objects.back()->rigid[k].geometry[i][2]);
				scene.objects.back()->rigid[k].geometry[i][1] *= -1.0;
				swap(scene.objects.back()->rigid[k].geometry[i][4], scene.objects.back()->rigid[k].geometry[i][5]);
				scene.objects.back()->rigid[k].geometry[i][4] *= -1.0;
			}
		swap(scene.objects.back()->bound[2], scene.objects.back()->bound[4]);
		swap(scene.objects.back()->bound[3], scene.objects.back()->bound[5]);
		scene.lights.back()->model = scene.objects.back();
	}*/
	else if (num == 10)
		manipulator = manipulate::fovy;
	else if (num == 11)
		manipulator = manipulate::aspect;
	else if (num == 12)
		manipulator = manipulate::width;
	else if (num == 13)
		manipulator = manipulate::height;
	else if (num == 14)
		manipulator = manipulate::front;
	else if (num == 15)
		manipulator = manipulate::back;
	else if (num == 16)
		scene.render_cameras = !scene.render_cameras;
	else if (num == 17 && scene.active_camera_valid())
		scene.cameras[scene.active_camera]->focus = NULL;
	else if (num == 18)
	{
		scene.cameras.push_back(new orthohdl());
		scene.objects.push_back(new pyramidhdl(1.0, 1.0, 8));
		for (int k = 0; k < scene.objects.back()->rigid.size(); k++)
			for (int i = 0; i < scene.objects.back()->rigid[k].geometry.size(); i++)
			{
				swap(scene.objects.back()->rigid[k].geometry[i][1], scene.objects.back()->rigid[k].geometry[i][2]);
				scene.objects.back()->rigid[k].geometry[i][1] *= -1.0;
				swap(scene.objects.back()->rigid[k].geometry[i][4], scene.objects.back()->rigid[k].geometry[i][5]);
				scene.objects.back()->rigid[k].geometry[i][4] *= -1.0;
			}
		swap(scene.objects.back()->bound[2], scene.objects.back()->bound[4]);
		swap(scene.objects.back()->bound[3], scene.objects.back()->bound[5]);

		scene.cameras.back()->model = scene.objects.back();
		if (!scene.active_camera_valid())
		{
			scene.active_camera = scene.cameras.size()-1;
			scene.cameras[scene.active_camera]->project(&canvas);
		}
	}
	else if (num == 19)
	{
		scene.cameras.push_back(new frustumhdl());
		scene.objects.push_back(new pyramidhdl(1.0, 1.0, 8));
		for (int k = 0; k < scene.objects.back()->rigid.size(); k++)
			for (int i = 0; i < scene.objects.back()->rigid[k].geometry.size(); i++)
			{
				swap(scene.objects.back()->rigid[k].geometry[i][1], scene.objects.back()->rigid[k].geometry[i][2]);
				scene.objects.back()->rigid[k].geometry[i][1] *= -1.0;
				swap(scene.objects.back()->rigid[k].geometry[i][4], scene.objects.back()->rigid[k].geometry[i][5]);
				scene.objects.back()->rigid[k].geometry[i][4] *= -1.0;
			}
		swap(scene.objects.back()->bound[2], scene.objects.back()->bound[4]);
		swap(scene.objects.back()->bound[3], scene.objects.back()->bound[5]);

		scene.cameras.back()->model = scene.objects.back();
		if (!scene.active_camera_valid())
		{
			scene.active_camera = scene.cameras.size()-1;
			scene.cameras[scene.active_camera]->project(&canvas);
		}
	}
	else if (num == 20)
	{
		scene.cameras.push_back(new perspectivehdl());
		scene.objects.push_back(new pyramidhdl(1.0, 1.0, 8));
		for (int k = 0; k < scene.objects.back()->rigid.size(); k++)
			for (int i = 0; i < scene.objects.back()->rigid[k].geometry.size(); i++)
			{
				swap(scene.objects.back()->rigid[k].geometry[i][1], scene.objects.back()->rigid[k].geometry[i][2]);
				scene.objects.back()->rigid[k].geometry[i][1] *= -1.0;
				swap(scene.objects.back()->rigid[k].geometry[i][4], scene.objects.back()->rigid[k].geometry[i][5]);
				scene.objects.back()->rigid[k].geometry[i][4] *= -1.0;
			}
		swap(scene.objects.back()->bound[2], scene.objects.back()->bound[4]);
		swap(scene.objects.back()->bound[3], scene.objects.back()->bound[5]);

		scene.cameras.back()->model = scene.objects.back();
		if (!scene.active_camera_valid())
		{
			scene.active_camera = scene.cameras.size()-1;
			scene.cameras[scene.active_camera]->project(&canvas);
		}
	}
	else if (num == 21)
		canvas.polygon_mode = canvashdl::point;
	else if (num == 22)
		canvas.polygon_mode = canvashdl::line;
	else if (num == 23)
		canvas.polygon_mode = canvashdl::fill;
	// TODO Assignment 3: uncomment this
	//else if (num == 25)
	//	canvas.shade_model = canvashdl::flat;
	//else if (num == 26)
	//	canvas.shade_model = canvashdl::smooth;
	else if (num == 28)
		canvas.culling = canvashdl::disable;
	else if (num == 29)
		canvas.culling = canvashdl::backface;
	else if (num == 30)
		canvas.culling = canvashdl::frontface;
	else if (num == 31)
		scene.render_normals = scenehdl::none;
	else if (num == 32)
		scene.render_normals = scenehdl::face;
	else if (num == 33)
		scene.render_normals = scenehdl::vertex;

	glutPostRedisplay();
}

void object_menu(int num)
{
	if (num == 0)
	{
		if (scene.active_object >= 0 && scene.active_object < scene.objects.size())
		{
			if (scene.objects[scene.active_object] != NULL)
			{
				// TODO Assignment 3: uncomment this
				/*for (int i = 0; i < scene.lights.size(); )
				{
					if (scene.lights[i] != NULL && scene.lights[i]->model == scene.objects[scene.active_object])
					{
						delete scene.lights[i];
						scene.lights.erase(scene.lights.begin() + i);
					}
					else
						i++;
				}*/

				for (int i = 0; i < scene.cameras.size(); )
				{
					if (scene.cameras[i] != NULL && scene.cameras[i]->model == scene.objects[scene.active_object])
					{
						delete scene.cameras[i];
						if (scene.active_camera > i)
							scene.active_camera--;
						scene.cameras.erase(scene.cameras.begin() + i);
					}
					else
						i++;
				}
				delete scene.objects[scene.active_object];
			}
			scene.objects.erase(scene.objects.begin() + scene.active_object);
			glutPostRedisplay();
		}
	}
	else if (num == 4)
	{
		scene.active_camera = -1;
		for (int i = 0; i < scene.cameras.size(); i++)
			if (scene.cameras[i] != NULL && scene.active_object_valid() && scene.cameras[i]->model == scene.objects[scene.active_object])
				scene.active_camera = i;

		if (scene.active_camera_valid())
			scene.cameras[scene.active_camera]->project(&canvas);

		glutPostRedisplay();
	}
	else if (num == 1)
		manipulator = manipulate::translate;
	else if (num == 2)
		manipulator = manipulate::rotate;
	else if (num == 3)
		manipulator = manipulate::scale;
	else if (num == 5 && scene.active_object_valid() && scene.active_camera_valid())
	{
		scene.cameras[scene.active_camera]->focus = scene.objects[scene.active_object];
		scene.cameras[scene.active_camera]->radius = dist(scene.objects[scene.active_object]->position, scene.cameras[scene.active_camera]->position);
	}
	// TODO Assignment 4: uncomment this
	/*else if (num == 6 && scene.active_object_valid())
	{
		for (map<string, materialhdl*>::iterator i = scene.objects[scene.active_object]->material.begin(); i != scene.objects[scene.active_object]->material.end(); i++)
		{
			if (i->second != NULL)
				delete i->second;
			i->second = new texturehdl();
		}
		glutPostRedisplay();
	}*/
	// TODO Assignment 3: uncomment this
	/*else if (num == 7 && scene.active_object_valid())
	{
		for (map<string, materialhdl*>::iterator i = scene.objects[scene.active_object]->material.begin(); i != scene.objects[scene.active_object]->material.end(); i++)
		{
			if (i->second != NULL)
				delete i->second;
			i->second = new customhdl();
		}
		glutPostRedisplay();
	}
	else if (num == 8 && scene.active_object_valid())
	{
		for (map<string, materialhdl*>::iterator i = scene.objects[scene.active_object]->material.begin(); i != scene.objects[scene.active_object]->material.end(); i++)
		{
			if (i->second != NULL)
				delete i->second;
			i->second = new phonghdl();
		}
		glutPostRedisplay();
	}
	else if (num == 9 && scene.active_object_valid())
	{
		for (map<string, materialhdl*>::iterator i = scene.objects[scene.active_object]->material.begin(); i != scene.objects[scene.active_object]->material.end(); i++)
		{
			if (i->second != NULL)
				delete i->second;
			i->second = new gouraudhdl();
		}
		glutPostRedisplay();
	}
	else if (num == 10 && scene.active_object_valid())
	{
		for (map<string, materialhdl*>::iterator i = scene.objects[scene.active_object]->material.begin(); i != scene.objects[scene.active_object]->material.end(); i++)
		{
			if (i->second != NULL)
				delete i->second;
			i->second = new whitehdl();
		}
		glutPostRedisplay();
	}*/
}

// TODO Assignment 3: uncomment this
/*void color_menu(int num)
{
	for (int i = 0; i < scene.lights.size(); i++)
		if (scene.lights[i] != NULL && scene.lights[i]->model == scene.objects[scene.active_object])
		{
			switch (num)
			{
			case 0: scene.lights[i]->ambient = red*0.2f; break;
			case 1: scene.lights[i]->ambient = orange*0.2f; break;
			case 2: scene.lights[i]->ambient = yellow*0.2f; break;
			case 3: scene.lights[i]->ambient = green*0.2f; break;
			case 4: scene.lights[i]->ambient = blue*0.2f; break;
			case 5: scene.lights[i]->ambient = indigo*0.2f; break;
			case 6: scene.lights[i]->ambient = violet*0.2f; break;
			case 7: scene.lights[i]->ambient = black*0.2f; break;
			case 8: scene.lights[i]->ambient = white*0.2f; break;
			case 9: scene.lights[i]->ambient = brown*0.2f; break;
			case 10: scene.lights[i]->diffuse = red; break;
			case 11: scene.lights[i]->diffuse = orange; break;
			case 12: scene.lights[i]->diffuse = yellow; break;
			case 13: scene.lights[i]->diffuse = green; break;
			case 14: scene.lights[i]->diffuse = blue; break;
			case 15: scene.lights[i]->diffuse = indigo; break;
			case 16: scene.lights[i]->diffuse = violet; break;
			case 17: scene.lights[i]->diffuse = black; break;
			case 18: scene.lights[i]->diffuse = white; break;
			case 19: scene.lights[i]->diffuse = brown; break;
			case 20: scene.lights[i]->specular = red; break;
			case 21: scene.lights[i]->specular = orange; break;
			case 22: scene.lights[i]->specular = yellow; break;
			case 23: scene.lights[i]->specular = green; break;
			case 24: scene.lights[i]->specular = blue; break;
			case 25: scene.lights[i]->specular = indigo; break;
			case 26: scene.lights[i]->specular = violet; break;
			case 27: scene.lights[i]->specular = black; break;
			case 28: scene.lights[i]->specular = white; break;
			case 29: scene.lights[i]->specular = brown; break;
			}
		}
}

void attenuation_menu(int num)
{
	for (int i = 0; i < scene.lights.size(); i++)
	{
		if (scene.lights[i] != NULL && scene.lights[i]->model == scene.objects[scene.active_object] && scene.lights[i]->type == "point")
		{
			switch (num)
			{
			case 0: ((pointhdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.7, 1.8); break;
			case 1: ((pointhdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.35, 0.44); break;
			case 2: ((pointhdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.22, 0.20); break;
			case 3: ((pointhdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.14, 0.07); break;
			case 4: ((pointhdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.09, 0.032); break;
			case 5: ((pointhdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.07, 0.017); break;
			case 6: ((pointhdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.045, 0.0075); break;
			case 7: ((pointhdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.027, 0.0028); break;
			case 8: ((pointhdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.022, 0.0019); break;
			case 9: ((pointhdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.014, 0.0007); break;
			case 10: ((pointhdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.0007, 0.0002); break;
			case 11: ((pointhdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.0014, 0.000007); break;
			}
		}
		else if (scene.lights[i] != NULL && scene.lights[i]->model == scene.objects[scene.active_object] && scene.lights[i]->type == "spot")
		{
			switch (num)
			{
			case 0: ((spothdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.7, 1.8); break;
			case 1: ((spothdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.35, 0.44); break;
			case 2: ((spothdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.22, 0.20); break;
			case 3: ((spothdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.14, 0.07); break;
			case 4: ((spothdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.09, 0.032); break;
			case 5: ((spothdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.07, 0.017); break;
			case 6: ((spothdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.045, 0.0075); break;
			case 7: ((spothdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.027, 0.0028); break;
			case 8: ((spothdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.022, 0.0019); break;
			case 9: ((spothdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.014, 0.0007); break;
			case 10: ((spothdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.0007, 0.0002); break;
			case 11: ((spothdl*)scene.lights[i])->attenuation = vec3f(1.0, 0.0014, 0.000007); break;
			}
		}
	}
}*/

void create_menu()
{
	int objects_id = glutCreateMenu(canvas_menu);
	glutAddMenuEntry(" Box         ", 1);
	glutAddMenuEntry(" Cylinder    ", 2);
    glutAddMenuEntry(" Sphere      ", 3);
    glutAddMenuEntry(" Pyramid     ", 4);
    glutAddMenuEntry(" Model       ", 5);


    // TODO Assignment 3: uncomment this
    /*int lights_id = glutCreateMenu(canvas_menu);
    glutAddMenuEntry(" Toggle Draw ", 6);
	glutAddMenuEntry(" Directional ", 7);
	glutAddMenuEntry(" Point       ", 8);
	glutAddMenuEntry(" Spot        ", 9);*/

	int camera_id = glutCreateMenu(canvas_menu);
	glutAddMenuEntry(" Fovy        ", 10);
	glutAddMenuEntry(" Aspect      ", 11);
	glutAddMenuEntry(" Width       ", 12);
	glutAddMenuEntry(" Height      ", 13);
	glutAddMenuEntry(" Near        ", 14);
	glutAddMenuEntry(" Far         ", 15);
	glutAddMenuEntry(" Toggle Draw ", 16);
	glutAddMenuEntry(" Clear Focus ", 17);
	glutAddMenuEntry(" Ortho       ", 18);
	glutAddMenuEntry(" Frustum     ", 19);
	glutAddMenuEntry(" Perspective ", 20);

	int mode_id = glutCreateMenu(canvas_menu);
	glutAddMenuEntry(" Point       ", 21);
	glutAddMenuEntry(" Line        ", 22);
	// TODO Assignment 2: uncomment this
	//glutAddMenuEntry(" Fill        ", 23);


	// TODO Assignment 3: uncomment this
	/*int ambient_id = glutCreateMenu(color_menu);
	glutAddMenuEntry(" Red         ", 0);
	glutAddMenuEntry(" Orange      ", 1);
	glutAddMenuEntry(" Yellow      ", 2);
	glutAddMenuEntry(" Green       ", 3);
	glutAddMenuEntry(" Blue        ", 4);
	glutAddMenuEntry(" Indigo      ", 5);
	glutAddMenuEntry(" Violet      ", 6);
	glutAddMenuEntry(" Black       ", 7);
	glutAddMenuEntry(" White       ", 8);
	glutAddMenuEntry(" Brown       ", 9);

	int diffuse_id = glutCreateMenu(color_menu);
	glutAddMenuEntry(" Red         ", 10);
	glutAddMenuEntry(" Orange      ", 11);
	glutAddMenuEntry(" Yellow      ", 12);
	glutAddMenuEntry(" Green       ", 13);
	glutAddMenuEntry(" Blue        ", 14);
	glutAddMenuEntry(" Indigo      ", 15);
	glutAddMenuEntry(" Violet      ", 16);
	glutAddMenuEntry(" Black       ", 17);
	glutAddMenuEntry(" White       ", 18);
	glutAddMenuEntry(" Brown       ", 19);

	int specular_id = glutCreateMenu(color_menu);
	glutAddMenuEntry(" Red         ", 20);
	glutAddMenuEntry(" Orange      ", 21);
	glutAddMenuEntry(" Yellow      ", 22);
	glutAddMenuEntry(" Green       ", 23);
	glutAddMenuEntry(" Blue        ", 24);
	glutAddMenuEntry(" Indigo      ", 25);
	glutAddMenuEntry(" Violet      ", 26);
	glutAddMenuEntry(" Black       ", 27);
	glutAddMenuEntry(" White       ", 28);
	glutAddMenuEntry(" Brown       ", 29);

	int attenuation_id = glutCreateMenu(attenuation_menu);
	glutAddMenuEntry(" Range 7     ",  0);
	glutAddMenuEntry(" Range 13    ",  1);
	glutAddMenuEntry(" Range 20    ",  2);
	glutAddMenuEntry(" Range 32    ",  3);
	glutAddMenuEntry(" Range 50    ",  4);
	glutAddMenuEntry(" Range 65    ",  5);
	glutAddMenuEntry(" Range 100   ",  6);
	glutAddMenuEntry(" Range 160   ",  7);
	glutAddMenuEntry(" Range 200   ",  8);
	glutAddMenuEntry(" Range 325   ",  9);
	glutAddMenuEntry(" Range 600   ",  10);
	glutAddMenuEntry(" Range 3250  ",  11);


	int shading_id = glutCreateMenu(canvas_menu);
	glutAddMenuEntry(" Flat        ", 25);
	glutAddMenuEntry(" Smooth      ", 26);*/

	// TODO Assignment 2: uncomment this
	/*int culling_id = glutCreateMenu(canvas_menu);
	glutAddMenuEntry(" None        ", 28);
	glutAddMenuEntry(" Back        ", 29);
	glutAddMenuEntry(" Front       ", 30);*/

	int normal_id = glutCreateMenu(canvas_menu);
	glutAddMenuEntry(" None        ", 31);
	glutAddMenuEntry(" Face        ", 32);
	glutAddMenuEntry(" Vertex      ", 33);

    canvas_menu_id = glutCreateMenu(canvas_menu);
    glutAddSubMenu  (" Objects     ", objects_id);
    // TODO Assignment 3: uncomment this
    //glutAddSubMenu  (" Lights      ", lights_id);
    glutAddSubMenu  (" Cameras     ", camera_id);
    glutAddSubMenu  (" Polygon     ", mode_id);
    // TODO Assignment 3: uncomment this
    //glutAddSubMenu  (" Shading     ", shading_id);
    // TODO Assignment 2: uncomment this
    //glutAddSubMenu  (" Culling     ", culling_id);
    glutAddSubMenu  (" Normals     ", normal_id);
    glutAddMenuEntry(" Quit        ", 0);

    // TODO Assignment 3: uncomment this
    /*int material_menu_id = glutCreateMenu(object_menu);
	glutAddMenuEntry(" White       ", 10);
	glutAddMenuEntry(" Gouraud     ", 9);
	glutAddMenuEntry(" Phong       ", 8);
	glutAddMenuEntry(" Custom      ", 7);*/
    // TODO Assignment 4: uncomment this
	//glutAddMenuEntry(" Texture     ", 6);

    object_menu_id = glutCreateMenu(object_menu);
    // TODO Assignment 3: uncomment this
    //glutAddSubMenu  (" Material    ", material_menu_id);
    glutAddMenuEntry(" Set Focus   ", 5);
    glutAddMenuEntry(" Translate   ", 1);
    glutAddMenuEntry(" Rotate      ", 2);
    glutAddMenuEntry(" Scale       ", 3);
    glutAddMenuEntry(" Delete      ", 0);

    // TODO Assignment 3: uncomment this
    /*light_menu_id = glutCreateMenu(object_menu);
    glutAddSubMenu  (" Ambient     ", ambient_id);
	glutAddSubMenu  (" Diffuse     ", diffuse_id);
	glutAddSubMenu  (" Specular    ", specular_id);
	glutAddSubMenu  (" Attenuation ", attenuation_id);
	glutAddMenuEntry(" Set Focus   ", 5);
	glutAddMenuEntry(" Translate   ", 1);
	glutAddMenuEntry(" Rotate      ", 2);
	glutAddMenuEntry(" Delete      ", 0);*/

	camera_menu_id = glutCreateMenu(object_menu);
	glutAddMenuEntry(" Set Active  ", 4);
	glutAddMenuEntry(" Set Focus   ", 5);
	glutAddMenuEntry(" Translate   ", 1);
	glutAddMenuEntry(" Rotate      ", 2);
	glutAddMenuEntry(" Scale       ", 3);
	glutAddMenuEntry(" Delete      ", 0);

    glutSetMenu(canvas_menu_id);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMenuStatusFunc(menustatusfunc);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	int display_mode = GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE;
#ifdef OSX_CORE3
	display_mode |= GLUT_3_2_CORE_PROFILE;
#endif
	glutInitDisplayMode(display_mode);

	glutInitWindowSize(750, 750);
	glutInitWindowPosition(0, 0);
	window_id = glutCreateWindow("Assignment");

#ifdef __GLEW_H__
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "Error: " << glewGetErrorString(err) << endl;
		exit(1);
	}
	cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;
#endif

	cout << "Status: Using OpenGL " << glGetString(GL_VERSION) << endl;
	cout << "Status: Using GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;


	init(string(argv[0]).substr(0, string(argv[0]).find_last_of("/\\")) + "/");
	create_menu();

	glutReshapeFunc(reshapefunc);
	glutDisplayFunc(displayfunc);
	glutIdleFunc(idlefunc);

	glutPassiveMotionFunc(pmotionfunc);
	glutMotionFunc(motionfunc);
	glutMouseFunc(mousefunc);

	glutKeyboardFunc(keydownfunc);
	glutKeyboardUpFunc(keyupfunc);

	glutMainLoop();
}
