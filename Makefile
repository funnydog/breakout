CXXFLAGS := -O2 -std=c++14 -Wall \
	$(shell pkg-config --cflags glew) -DGLEW_STATIC \
	$(shell pkg-config --cflags glfw3) -DGLFW_INCLUDE_GLCOREARB \
	$(shell pkg-config --cflags freetype2)

LDFLAGS := $(shell pkg-config --libs glew) 	\
	$(shell pkg-config --libs glfw3) 	\
	$(shell pkg-config --libs freetype2) 	\
	-lSOIL

OBJECTS := game.o main.o shader.o texture.o resource.o sprite.o gameobject.o \
	level.o ball.o particle.o postprocess.o text.o

.PHONY: clean all

all: breakout

breakout: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	@rm -f *.o *~ breakout
