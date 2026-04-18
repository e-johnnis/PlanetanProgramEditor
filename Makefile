CXX:=g++
INSTALL_DIR:=${HOME}/.local/bin

CFLAGS=-std=c++17 -fopenmp
INCLUDES=-I./include -I/usr/include -I/usr/include/opencv4 -I/include
LIBS=-L/usr/lib -L/lib -lopencv_core -lopencv_imgcodecs -lopencv_videoio
SRCS=src/main.cpp src/graphics.cpp src/graphics-render.cpp src/event.cpp src/math.cpp src/core.cpp

.PHONY: clean debug build install

build:
	mkdir -p build

debug:
	mkdir -p debug
	$(CXX) $(CFLAGS) -g -DDEBUG $(INCLUDES) -o debug/ppe $(SRCS) $(LIBS)
	./debug/ppe -v -o ./debug/debug-result.avi example.ppc

install:
	mkdir -p ${HOME}/.local ${HOME}/.local/share/planetan $(INSTALL_DIR)

clean: obj debug build
	rm -r obj/* debug/* build/*