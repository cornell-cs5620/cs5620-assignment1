/*
 * main.cpp
 * Blaze Game Engine v0.11
 *
 * Created by Ned Bingham on October 20, 2011.
 * Modified by Ned Bingham on October 20, 2011.
 * Copyright 2011 Sol Union. All rights reserved.
 *
 * Blaze Game Engine v0.11 is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Blaze Game Engine v0.11 is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Blaze Game Engine v0.11.
 * If not, see <http://www.gnu.org/licenses/>.
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

void init()
{
	for (int i = 0; i < 256; i++)
		keys[i] = false;

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
				bool is_camera = false;

				for (int j = 0; j < scene.cameras.size() && !is_camera; j++)
					if (scene.cameras[j] != NULL && scene.cameras[j]->model == scene.objects[i])
						is_camera = true;

				if (!is_camera || (is_camera && scene.render_cameras))
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
			bool is_camera = false;

			for (int i = 0; i < scene.cameras.size() && !is_camera; i++)
				if (scene.cameras[i] != NULL && scene.active_object_valid() && scene.cameras[i]->model == scene.objects[scene.active_object])
					is_camera = true;

			glutDetachMenu(GLUT_RIGHT_BUTTON);
			if (scene.active_object == -1)
				glutSetMenu(canvas_menu_id);
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
				/* TODO Assignment 2: clean up the lights as well when the associated object
				 * is deleted.
				 */

				for (int i = 0; i < scene.cameras.size(); )
				{
					if (scene.cameras[i] != NULL && scene.cameras[i]->model == scene.objects[scene.active_object])
					{
						delete scene.cameras[i];
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

}

void create_menu()
{
	/* TODO Assignment 2: Add menus for handling the lights. You should
	 * be able to enable/disable the drawing of the lights and create
	 * directional, point, or spot lights sources. You should also be able
	 * to change the emissive, ambient, diffuse, and specular colors and
	 * the attenuation of the lights. Finally, you should be able to rotate
	 * and translate the lights just like you would an object and have it
	 * affect the lighting of the scene.
	 */

	// TODO Assignment 2: Add menus to manipulate the shading model.

	int objects_id = glutCreateMenu(canvas_menu);
	glutAddMenuEntry(" Box         ", 1);
	glutAddMenuEntry(" Cylinder    ", 2);
    glutAddMenuEntry(" Sphere      ", 3);
    glutAddMenuEntry(" Pyramid     ", 4);
    glutAddMenuEntry(" Model       ", 5);

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
	glutAddMenuEntry(" Fill        ", 23);

	int culling_id = glutCreateMenu(canvas_menu);
	glutAddMenuEntry(" None        ", 28);
	glutAddMenuEntry(" Back        ", 29);
	glutAddMenuEntry(" Front       ", 30);

	int normal_id = glutCreateMenu(canvas_menu);
	glutAddMenuEntry(" None        ", 31);
	glutAddMenuEntry(" Face        ", 32);
	glutAddMenuEntry(" Vertex      ", 33);

    canvas_menu_id = glutCreateMenu(canvas_menu);
    glutAddSubMenu  (" Objects     ", objects_id);
    glutAddSubMenu  (" Cameras     ", camera_id);
    glutAddSubMenu  (" Polygon     ", mode_id);
    glutAddSubMenu  (" Culling     ", culling_id);
    glutAddSubMenu  (" Normals     ", normal_id);
    glutAddMenuEntry(" Quit        ", 0);


    object_menu_id = glutCreateMenu(object_menu);
    glutAddMenuEntry(" Set Focus   ", 5);
    glutAddMenuEntry(" Translate   ", 1);
    glutAddMenuEntry(" Rotate      ", 2);
    glutAddMenuEntry(" Scale       ", 3);
    glutAddMenuEntry(" Delete      ", 0);

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
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

	glutInitWindowSize(750, 750);
	glutInitWindowPosition(0, 0);
	window_id = glutCreateWindow("Assignment");

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "Error: " << glewGetErrorString(err) << endl;
		exit(1);
	}
	cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;
	cout << "Status: Using OpenGL " << glGetString(GL_VERSION) << endl;
	cout << "Status: Using GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	init();
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
