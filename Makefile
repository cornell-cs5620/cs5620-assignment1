CXXFLAGS = -O2 -g -fmessage-length=0
SOURCES	:= $(shell find src -name '*.cpp') $(shell find src -name '*.c')
OBJECTS	:= $(subst .c,.o,$(subst .cpp,.o,$(subst src/,build/,$(SOURCES))))
DIRECTORIES := $(sort $(dir $(OBJECTS)))
SEARCH_PATHS = 
LDFLAGS	= 
TARGET	= assignment

ifeq ($(OS),Windows_NT)
    CXXFLAGS += -static-libgcc -static-libstdc++ -D WIN32
    LDFLAGS += -lfreeglut -lglu32 -lglew32 -lopengl32 -lcomdlg32
else
    UNAME := $(shell uname -s)
    ifeq ($(UNAME),Linux)
        CXXFLAGS += -D LINUX
        LDFLAGS += -lglut -lGL -lGLU -lGLEW
    endif
    ifeq ($(UNAME),Darwin)
        ifeq ($(env),core3)
            CXXFLAGS += -D OSX_CORE3 -Wno-deprecated
            LDFLAGS += -framework GLUT -framework OpenGL
        else ifeq ($(env),x11)
            CXXFLAGS += -D OSX_X11 -Wno-deprecated -I/usr/X11/include
            LDFLAGS += -L/usr/X11/lib -lglut -lGL -lGLU -lGLEW
        else ifeq ($(env),xquartz)
            CXXFLAGS += -D OSX_XQUARTZ -Wno-deprecated -I/opt/X11/include
            LDFLAGS += -L/opt/X11/lib -lglut -lGL -lGLU -lGLEW
        else # default to the frameworks for version 2.1 because they are something everyone has
            CXXFLAGS += -D OSX_CORE2 -Wno-deprecated
            LDFLAGS += -framework GLUT -framework OpenGL
        endif
    endif
endif

all: build $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(SEARCH_PATHS) $(CXXFLAGS) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

build/%.o: src/%.cpp
	$(CXX) $(SEARCH_PATHS) $(CXXFLAGS) -c -o $@ $<

build/%.o: src/%.c
	$(CC) $(SEARCH_PATHS) $(CXXFLAGS) -c -o $@ $<

build:
	mkdir $(DIRECTORIES)

clean:
	rm -f $(OBJECTS) $(TARGET) $(TARGET)
