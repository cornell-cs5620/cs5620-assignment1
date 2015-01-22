This is an example implementation of assignment 1. The controls are as follows:

w - move forward
a - straif left
s - move backward
d - straif right
e - move up
q - move down
m - bind/unbind mouse
esc - quit

right click - access menu

If you are compiling on windows you will need to install a development environment

If you are compiling on linux you will need the following libraries
sudo apt-get install libglew1.10 libglew-dbg libglew-dev freeglut3 freeglut3-dev freeglut3-dbg

that should also pull in all of the other dependencies

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
