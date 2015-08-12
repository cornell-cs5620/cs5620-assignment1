The controls are as follows:

m - bind/unbind mouse
esc - quit

When the mouse is bound, then it is not visible and you may move the current camera around in first person or orbit using the following controls:
w - move forward
a - straif left
s - move backward
d - straif right
e - move up
q - move down
move mouse - look around as first person when not focused, orbit object when focused.

When the mouse is unbound, then it is visible and there are two controls
right click			- access menu
left click and drag	- adjust the current manipulator

The manipulators are as follows:
	translate	- Translate selected object
	rotate		- Rotate selected object
	scale		- Scale selected object
	fovy		- If the current camera is a Perspective camera, adjust its field of view
	aspect		- If the current camera is a Perspective camera, adjust its aspect ratio
	width		- If the current camera is an Ortho or a Frustum camera, adjust its viewport width
	height		- If the current camera is an Ortho or a Frustum camera, adjust its viewport height
	near		- Adjust the near plane distance from the current camera
	far		- Adjust the far plane distance from the current camera

If you right click without an object selected, the menu items are as follows:
Objects
	Box		- Add a box into the scene
	Cylinder	- Add a cylinder into the scene
	Sphere		- Add a sphere into the scene
	Pyramid		- Add a pyramid into the scene
	Model		- Load a .obj file into the scene
Lights
	Toggle Draw	- Change whether or not to render lights in the scene
	Directional	- Add a directional light to the scene
	Point		- Add a point light to the scene
	Spot		- Add a spot light to the scene
Cameras
	Fovy		- Set the current manipulator to fovy
	Aspect		- Set the current manipulator to aspect
	Width		- Set the current manipulator to width
	Height		- Set the current manipulator to height
	Near		- Set the current manipulator to near
	Far		- Set the current manipulator to far
	Toggle Draw	- Change whether or not to render cameras in the scene
	Clear Focus	- Clear the focus of the camera
	Ortho		- Add an Ortho camera to the scene
	Frustum		- Add a Frustum camera to the scene
	Perspective	- Add a Perspective camera to the scene
Polygon
	Point		- Render everything as a collection of points
	Line		- Render everything as a collection of lines (wireframe)
	Fill		- Render everything as a collection of filled polygons
Shading
	None		- Turn off shading
	Flat		- Set the shading model to flat
	Gouraud		- Set the shading model to gouraud
	Phong		- Set the shading model to phong
Culling
	None		- Don't cull any faces
	Front		- Set culling to cull the front face
	Back		- Set culling to cull the back face
Normals
	None		- Don't render the normals
	Face		- Render the face normals as lines perpendicular to the faces
	Vertex		- Render the vertex normals as lines from each vertex
Quit

If you right click on an object, then the menus are as follows:
Material
	White   - Set the shader of this object to ignore lighting and to be a solid white color
	Gouraud - Set the shader to render a lit color using gouraud shading
	Phong   - Set the shader to render a lit color using phong shading
	Custom  - Set the shader to render a a custom shader
	Texture - Set the shader to render a texture on the object
Set Focus	- Set this object as the camera focus
Translate	- Set the current manipulator to translate
Rotate		- Set the current manipulator to rotate
Scale		- Set the current manipulator to scale
Delete		- Delete this object from the scene

If you right click on a camera, then the menus are as follows:
Set Active	- Set this camera as the active camera
Set Focus	- Set this object as the camera focus
Translate	- Set the current manipulator to translate
Rotate		- Set the current manipulator to rotate
Scale		- Set the current manipulator to scale
Delete		- Delete this object from the scene

If you are compiling on Mac OS X using the makefile, then there are a few options for compiling the makefile.

make env=core2
    This is the default, it creates an OpenGL 2.1 instance for shaders 120 using the normal
    GLUT and OpenGL frameworks that come with your system.

make env=x11
    This is if you want to use the default X11 installation as your environment. You must
    install glew if you want to use this option. This most likely creates an OpenGL 2.1 instance
    with shader version 120.

make env=xquartz
    This is if you want to use XQuartz as your X11 environment. You must install glew if you
    want to use this option. This most likely creates an OpenGL 2.1 instance
    with shader version 120.

make env=core3
    This is not recommended, because you will have to rework the shaders to the newest version.
    This creates an OpenGL 3.2 instance for shaders 330. GLSL 330 is significantly different from
    120.
