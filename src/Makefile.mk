# Makefile para Windows

PROG = ProgOpenGL.exe
#FONTES = Ponto.cpp Poligono.cpp Temporizador.cpp ListaDeCoresRGB.cpp ProgramaBasicoOpenGL.cpp

#FONTES = ExemploDeManipulacaoDeImagens.cpp ImageClass.cpp Temporizador.cpp SOIL/SOIL.cpp SOIL/image_DXT.cpp SOIL/image_helper.cpp SOIL/stb_image_aug.cpp

#FONTES = Ponto.cpp Poligono.cpp Temporizador.cpp ListaDeCoresRGB.cpp ProgramaBasico3D.cpp
FONTES = Ponto.cpp Poligono.cpp Temporizador.cpp ListaDeCoresRGB.cpp \
			ImageClass.cpp Texture.cpp \
			SOIL/SOIL.cpp SOIL/image_DXT.cpp SOIL/image_helper.cpp SOIL/stb_image_aug.cpp \
			ProgramaBasico3D-NAV-TEX.cpp


OBJETOS = $(FONTES:.cpp=.cpp)

# Old
#CFLAGS = -g -Iinclude\GL  -Wno-write-strings -Wno-narrowing # -Wall -g  # Todas as warnings, infos de debug
# New
CFLAGS = -std=c++14 -g -Iinclude\GL  -Wno-write-strings -Wno-narrowing -DWIN32 # -Wall -g  # Todas as warnings, infos de debug

# Old
#LDFLAGS = -Llib -lfreeglut -lopengl32 -lglu32 -lm
# New
LDFLAGS = -Llib\x64 -lfreeglut -lopengl32 -lglu32 -lm

CC = g++

$(PROG): $(OBJETOS)
	g++ $(CFLAGS) $(OBJETOS) -o $@ $(LDFLAGS)

clean:
	rm $(OBJETOS) $(PROG)
