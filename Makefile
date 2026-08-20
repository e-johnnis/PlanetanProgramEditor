CXX:=g++
INSTALL_DIR:=/usr/bin
RESOURCE_DIR:=/usr/share

CFLAGS=-std=c++17 -fopenmp
INCLUDES=-I./include -I/usr/include -I/usr/include/opencv4 -I/include
LIBS=-L/usr/lib -L/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_videoio
SRCS=src/main.cpp src/graphics.cpp src/graphics-render.cpp src/figure.cpp src/event.cpp src/math.cpp src/core.cpp

.PHONY: clean debug build install

build:
	mkdir -p build
	$(CXX) $(CFLAGS) -DRESOURCE_DIR="$(RESOURCE_DIR)" $(INCLUDES) -o build/ppe $(SRCS) $(LIBS)

debug:
	mkdir -p debug
	$(CXX) $(CFLAGS) -g -DDEBUG $(INCLUDES) -o debug/ppe-debug $(SRCS) $(LIBS)
	./debug/ppe-debug -v -o ./debug/debug-result.mkv example.ppc

install:
	mkdir -p $(RESOURCE_DIR)/planetan
	cp -u resources/* $(RESOURCE_DIR)/planetan/.
	cp -u build/ppe $(INSTALL_DIR)/.

clean:
	rm -rf debug/* build/*