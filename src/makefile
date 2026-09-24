CC = C:/w64devkit/bin/gcc.exe
CFLAGS = -Wall -Wextra -std=c17 -O2 -IC:/w64devkit/include -I..
LIBS = -LC:/w64devkit/lib -lraylib -lopengl32 -lgdi32 -lwinmm

TARGET = main.exe
SOURCES = main.c
OBJECTS = ../core/scene.o ../core/math_utils.o ../sketch/sketch.o ../renderer/sdf.o ../renderer/raymarch_renderer.o ../features/extrude.o

all: $(TARGET)

$(TARGET): $(SOURCES) $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(OBJECTS) $(LIBS)

../core/scene.o: ../core/scene.c ../core/scene.h ../core/types.h
	$(CC) $(CFLAGS) -c ../core/scene.c -o ../core/scene.o

../core/math_utils.o: ../core/math_utils.c ../core/math_utils.h
	$(CC) $(CFLAGS) -c ../core/math_utils.c -o ../core/math_utils.o

../sketch/sketch.o: ../sketch/sketch.c ../sketch/sketch.h
	$(CC) $(CFLAGS) -c ../sketch/sketch.c -o ../sketch/sketch.o

../renderer/sdf.o: ../renderer/sdf.c ../renderer/sdf.h
	$(CC) $(CFLAGS) -c ../renderer/sdf.c -o ../renderer/sdf.o

../renderer/raymarch_renderer.o: ../renderer/raymarch_renderer.c ../renderer/raymarch_renderer.h
	$(CC) $(CFLAGS) -c ../renderer/raymarch_renderer.c -o ../renderer/raymarch_renderer.o

../features/extrude.o: ../features/extrude.c ../features/extrude.h
	$(CC) $(CFLAGS) -c ../features/extrude.c -o ../features/extrude.o

clean:
	-rm -f $(TARGET) ../core/*.o ../sketch/*.o ../renderer/*.o ../features/*.o 2>/dev/null || true

.PHONY: all clean
