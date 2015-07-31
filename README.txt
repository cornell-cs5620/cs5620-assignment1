For assignment 1, implement everything specified by the comments marked:

	TODO Assignment 1
	
There are 36 of them. Good luck.

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