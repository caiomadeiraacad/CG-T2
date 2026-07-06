#ifndef OBJETO3D_H
#define OBJETO3D_H

#include <iostream>
#include <fstream>
#include <cmath>

#ifdef WIN32
#include <windows.h>
#include <glut.h>
#else
#include <sys/time.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif

#ifdef __linux__
#include <GL/glut.h>
#endif

using namespace std;

// Struct para armazenar um ponto (AGORA COM TEXTURA U e V)
typedef struct {
    float X, Y, Z;
    float U, V; // Coordenadas de Textura (Magica do colormap.png)
    
    void Set(float x, float y, float z, float u = 0.0f, float v = 0.0f) {
        X = x; Y = y; Z = z; U = u; V = v;
    }
} TPoint;

// Struct para armazenar um triângulo
typedef struct {
    TPoint P1, P2, P3;
} TTriangle;

// Rotina que faz um produto vetorial (Fornecida pelo Professor)
void ProdVetorial(TPoint v1, TPoint v2, TPoint &vresult) {
    vresult.X = v1.Y * v2.Z - (v1.Z * v2.Y);
    vresult.Y = v1.Z * v2.X - (v1.X * v2.Z);
    vresult.Z = v1.X * v2.Y - (v1.Y * v2.X);
}

// Rotina para calcular um vetor unitario (Fornecida pelo Professor)
void VetUnitario(TPoint &vet) {
    float modulo = sqrt(vet.X * vet.X + vet.Y * vet.Y + vet.Z * vet.Z);
    if (modulo == 0.0) return;
    vet.X /= modulo;
    vet.Y /= modulo;
    vet.Z /= modulo;
}

// Classe para armazenar um objeto 3D
class Objeto3D {
    TTriangle *faces; // vetor de faces
    unsigned int nFaces; // Variavel que armazena o numero de faces do objeto
    GLuint displayList;  // <--- NOVA VARIÁVEL PARA O FPS!

public:
    Objeto3D() {
        nFaces = 0;
        faces = NULL;
        displayList = 0;
    }

    unsigned int getNFaces() {
        return nFaces;
    }

    // --- LEITOR ADAPTADO PARA O SEU FORMATO .TRI INDEXADO ---
    void leObjeto(const char *Nome) {
        FILE *fp = fopen(Nome, "r");
        if (fp == NULL) {
            cout << "ERRO: Nao foi possivel abrir o modelo " << Nome << endl;
            return;
        }

        // 1. Lê a quantidade de Vértices (ex: 1394)
        int numVertices = 0;
        if (fscanf(fp, "%d", &numVertices) != 1) return;

        // 2. Lê os vértices e as texturas (X, Y, Z, U, V)
        TPoint* tempVertices = new TPoint[numVertices];
        for (int i = 0; i < numVertices; i++) {
            float x, y, z, u, v;
            fscanf(fp, "%f %f %f %f %f", &x, &y, &z, &u, &v);
            tempVertices[i].Set(x, y, z, u, v);
        }

        // 3. Lê a quantidade de Faces/Triângulos (ex: 2032)
        fscanf(fp, "%u", &nFaces);
        faces = new TTriangle[nFaces];

        // 4. Lê quais vértices formam cada triângulo (Índices)
        for (unsigned int i = 0; i < nFaces; i++) {
            int id1, id2, id3;
            fscanf(fp, "%d %d %d", &id1, &id2, &id3);

            // Monta o triângulo puxando os dados do vetor temporário
            faces[i].P1 = tempVertices[id1];
            faces[i].P2 = tempVertices[id2];
            faces[i].P3 = tempVertices[id3];
        }

        delete[] tempVertices; // Limpa a memória temporária
        fclose(fp);
        cout << "Modelo " << Nome << " carregado com sucesso! (" << nFaces << " faces)" << endl;

        // --- COMPILA O OBJETO NA PLACA DE VÍDEO (DISPLAY LIST) PARA O FPS IR AOS CÉUS ---
        displayList = glGenLists(1);
        glNewList(displayList, GL_COMPILE);
        
        glBegin(GL_TRIANGLES);
        for (unsigned int i = 0; i < nFaces; i++) {
            // Calcula a Normal (Para iluminação)
            TPoint v1, v2, normal;
            v1.Set(faces[i].P2.X - faces[i].P1.X, faces[i].P2.Y - faces[i].P1.Y, faces[i].P2.Z - faces[i].P1.Z);
            v2.Set(faces[i].P3.X - faces[i].P1.X, faces[i].P3.Y - faces[i].P1.Y, faces[i].P3.Z - faces[i].P1.Z);
            
            ProdVetorial(v1, v2, normal);
            VetUnitario(normal);
            glNormal3f(normal.X, normal.Y, normal.Z);

            glTexCoord2f(faces[i].P1.U, faces[i].P1.V); glVertex3f(faces[i].P1.X, faces[i].P1.Y, faces[i].P1.Z);
            glTexCoord2f(faces[i].P2.U, faces[i].P2.V); glVertex3f(faces[i].P2.X, faces[i].P2.Y, faces[i].P2.Z);
            glTexCoord2f(faces[i].P3.U, faces[i].P3.V); glVertex3f(faces[i].P3.X, faces[i].P3.Y, faces[i].P3.Z);
        }
        glEnd();
        
        glEndList();
        // ---------------------------------------------------------------------------------
    }

    // --- FUNÇÃO DE DESENHO INSTANTÂNEA ---
    void desenha() {
        if (nFaces == 0 || faces == NULL) return;
        // Desenha usando a memória da placa de vídeo (MUITO RÁPIDO)
        glCallList(displayList); 
    }
};

#endif