CC = g++
INC_DIR = ./src/include
CFLAGS = -Wall -I$(INC_DIR) -I./src -I./src/SOIL

LIBS = -lglut -lGLU -lGL
TARGET = T2

SRCS = main.cpp \
       src/Texture.cpp \
       src/ImageClass.cpp \
       src/Temporizador.cpp \
       src/Ponto.cpp \
       src/Poligono.cpp \
       src/ListaDeCoresRGB.cpp \
       src/SOIL.c \
       src/image_DXT.c \
       src/image_helper.c \
       src/stb_image_aug.c

OBJS = $(SRCS:.cpp=.o)
OBJS := $(OBJS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean