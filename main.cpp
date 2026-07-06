// **********************************************************************
// PUCRS/Escola Politecnica
// COMPUTACAO GRAFICA
//
// Programa basico para criar aplicacoes 3D em OpenGL
//
// Marcio Sarroglia Pinho
// pinho@pucrs.br
// **********************************************************************

#include <iostream>
#include <cmath>
#include <ctime>

#include <stdio.h>
#include <stdlib.h>
#include "src/loadTri.h"
using namespace std;


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

#include "src/Temporizador.h"
#include "src/ListaDeCoresRGB.h"
#include "src/Ponto.h"
#include "src/Texture.h"
#include "src/Bezier.h"

Temporizador T;
double AccumDeltaT=0;


GLfloat AspectRatio, angulo=0;

// Controle do modo de projecao
// 0: Projecao Paralela Ortografica; 1: Projecao Perspectiva
// A funcao "PosicUser" utiliza esta variavel. O valor dela eh alterado
// pela tecla 'p'
int ModoDeProjecao = 1;


// Controle do modo de projecao
// 0: Wireframe; 1: Faces preenchidas
// A funcao "Init" utiliza esta variavel. O valor dela eh alterado
// pela tecla 'e'
int ModoDeExibicao = 1;

float fuel = 100.0f;

float lookX = 0;
float lookY = 0;

bool firstMouse = true;

double nFrames=0;
double TempoTotal=0;
Ponto CantoEsquerdo = Ponto (-20,-1,-10);
Ponto ALVO, OBS, V1;

// Representa o conteudo de uma celula do piso
class Elemento{
public:
    int tipo;
    bool hasFuel;
    int corDoObjeto;
    int corDoPiso;
    float altura;
    int textureID;
};

class AABBBoundingBox {
    public:
        Ponto min;
        Ponto max;

        AABBBoundingBox() {}
        AABBBoundingBox(Ponto min, Ponto max) : min(min), max(max) {}

        bool CollideWith(AABBBoundingBox other) {
            return (min.x <= other.max.x && max.x >= other.min.x) &&
            (min.y <= other.max.y && max.y >= other.min.y) &&
            (min.z <= other.max.z && max.z >= other.min.z);
        }
};

// codigos que definem o o tipo do elemento que esta em uma celula
#define VAZIO 0
#define PREDIO 10
#define RUA 20
#define COMBUSTIVEL 30
#define VEICULO 40

bool isWarping = false;

// Matriz que armazena informacoes sobre o que existe na cidade
Elemento Cidade[100][100];

Ponto PosVehicle(14.0f, 0.0f, 1.0f);
float angleVehicle = 0.0f;
float speedVehicle = 0.0f;

// 3 p/ 3 pessoa; 1 p/ primeira;
int CameraMode = 3; 

int mapX = 0;
int mapZ = 0;

Objeto3D carModel;
Objeto3D buildingAModel;
Objeto3D buildingBModel;
Objeto3D buildingCModel;
Objeto3D barrelModel;
Objeto3D spaceshipModel; // ps: na falta de um aviao coloquei uma nave kkk 


Bezier planeRoute[4];
float t_plane = 0.0f;
int current_curve = 0;

int lastMouseX = -1;
int lastMouseY = -1;
float camera3RotX = 180.0f;
float camera3RotY = 30.0f;

void SpawnFuel() 
{
    for(int z = 0; z < mapZ; z++) {
        for(int x = 0; x < mapX; x++) {
            Cidade[z][x].hasFuel = false;
            if (Cidade[z][x].tipo >= 1 && Cidade[z][x].tipo <= 12 && (rand() % 20 == 0)) {
                Cidade[z][x].hasFuel = true;
            }
        }
    }
}

void DesenhaCubo(float tamAresta);

void LoadMap(const char* filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("ERROR: Nao foi possivel abrir o arquivo %s\n", filename);
        exit(1);
    }

    fscanf(fp, "%d %d", &mapX, &mapZ);

    for(int z = 0; z < mapZ; z++) {
        for(int x = 0; x < mapX; x++) {
            fscanf(fp, "%d", &Cidade[z][x].tipo);
            Cidade[z][x].textureID = -1;
            Cidade[z][x].altura = 0;
        }
    }

    fclose(fp);
    printf("mapa carregado: %dx%d\n", mapX, mapZ);
}

void InitBezier() {
    float W = (float)mapX;
    float H = (float)mapZ;
    float Y = 8.0f;

    planeRoute[0] = Bezier(Ponto(W/2, Y, 0), Ponto(0, Y, 0), Ponto(0, Y, H/2));
    planeRoute[1] = Bezier(Ponto(0, Y, H/2), Ponto(0, Y, H), Ponto(W/2, Y, H));
    planeRoute[2] = Bezier(Ponto(W/2, Y, H), Ponto(W, Y, H), Ponto(W, Y, H/2));
    planeRoute[3] = Bezier(Ponto(W, Y, H/2), Ponto(W, Y, 0), Ponto(W/2, Y, 0));
}

void UpdateCamera() {
    float angleRads = angleVehicle * M_PI / 180.0f;
    float dirX = sin(angleRads);
    float dirZ = cos(angleRads);

    float lookRads = (angleVehicle + lookX) * M_PI / 180.0f;
    
    float targetDirX = sin(lookRads);
    float targetDirZ = cos(lookRads);
    float targetDirY =  sin(lookY * M_PI / 180.0f); 

    if (CameraMode == 3) {
        float radX = camera3RotX * M_PI / 180.0f;
        float radY = camera3RotY * M_PI / 180.0f;
        float dist = 10.0f;

        OBS.x = PosVehicle.x + dist * sin(radY) * sin(radX);
        OBS.y = PosVehicle.y + dist * cos(radY);
        OBS.z = PosVehicle.z + dist * sin(radY) * cos(radX);

        ALVO = PosVehicle;
    }
    else if (CameraMode == 1) {
        float offsetFront = 0.6f;
        OBS.x = PosVehicle.x + (dirX * offsetFront);
        OBS.y = PosVehicle.y + 0.4f;
        OBS.z = PosVehicle.z + (dirZ * offsetFront);

        float radYaw = (angleVehicle + lookX) * M_PI / 180.0f;
        float radPitch = lookY * M_PI / 180.0f;

        float targetDirX = cos(radPitch) * sin(radYaw);
        float targetDirY = sin(radPitch);
        float targetDirZ = cos(radPitch) * cos(radYaw);

        ALVO.x = OBS.x + (targetDirX * 5.0f);
        ALVO.y = OBS.y + (targetDirY * 5.0f);
        ALVO.z = OBS.z + (targetDirZ * 5.0f);
    }
}

void DrawAirplane() {
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(1.0f);
    for(int i=0; i<4; i++) {
        glBegin(GL_LINE_STRIP);
        for(float t=0; t<=1.0f; t+=0.1f) {
            Ponto P = planeRoute[i].Calcula(t);
            glVertex3f(P.x, P.y, P.z);
        }
        glEnd();
    }
    glEnable(GL_LIGHTING);

    Ponto pos = planeRoute[current_curve].Calcula(t_plane);
    
    float t_next = t_plane + 0.05f;
    int next_curva = current_curve;
    if (t_next > 1.0f) {
        t_next -= 1.0f;
        next_curva = (current_curve + 1) % 4;
    }
    Ponto posNext = planeRoute[next_curva].Calcula(t_next);
    
    float dx = posNext.x - pos.x;
    float dz = posNext.z - pos.z;
    float airplaneAngle = atan2(dx, dz) * 180.0f / M_PI;

    glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);
        glRotatef(airplaneAngle, 0, 1, 0);
        glScalef(2.0f, 2.0f, 2.0f); 
        glDisable(GL_TEXTURE_2D);
        glColor3f(1.0f, 0.0f, 0.0f);
        spaceshipModel.desenha();
    glPopMatrix();
}

void DrawVehicle() {
    glPushMatrix();
        glTranslatef(PosVehicle.x, 0.0f, PosVehicle.z);
        glRotatef(angleVehicle, 0, 1, 0);
        glScalef(0.2f, 0.2f, 0.2f);
        
        glEnable(GL_TEXTURE_2D);
        glColor3f(1.0f, 1.0f, 1.0f);
        UseTexture(12);

        carModel.desenha();

        UseTexture(-1);
        glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}
// **********************************************************************
//  void init(void)
//        Inicializa os parametros globais de OpenGL
// **********************************************************************
void init(void)
{
    glClearColor(0.3f, 0.4f, 0.0f, 1.0f); 

    carModel.leObjeto("assets/tri/sedan.tri");
    buildingAModel.leObjeto("assets/tri/building-sample-tower-a.tri");
    buildingBModel.leObjeto("assets/tri/building-sample-tower-b.tri");
    buildingCModel.leObjeto("assets/tri/building-sample-tower-c.tri");
    barrelModel.leObjeto("assets/tri/barrel.tri");
    spaceshipModel.leObjeto("assets/tri/craft_racer.tri");

    LoadMap("Mapa1.txt");
    InitBezier();

    LoadTexture("assets/CROSS.png"); // 1
    LoadTexture("assets/DL.png");    // 2
    LoadTexture("assets/DLR.png");   // 3
    LoadTexture("assets/DR.png");    // 4
    LoadTexture("assets/LR.png");    // 5
    LoadTexture("assets/None.png");  // 6
    LoadTexture("assets/UD.png");    // 7
    LoadTexture("assets/UDL.png");   // 8
    LoadTexture("assets/UDR.png");   // 9
    LoadTexture("assets/UL.png");    // 10
    LoadTexture("assets/ULR.png");   // 11
    LoadTexture("assets/UR.png");    // 12

    LoadTexture("assets/tri/colormap.png");  // 13
    LoadTexture("assets/tri/variation-a.png"); // 14
    LoadTexture("assets/tri/variation-b.png"); // 15

    UseTexture(-1);

    srand(time(NULL));
    SpawnFuel();

    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glEnable (GL_CULL_FACE );
    glEnable(GL_NORMALIZE);
    //glShadeModel(GL_SMOOTH);
    glShadeModel(GL_FLAT);

    glColorMaterial ( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
    if (ModoDeExibicao) // Faces Preenchidas??
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    ALVO = Ponto(2, 0, 2);
    OBS = Ponto(2, 5, 8);
    V1 = ALVO - OBS;
    
    LoadTexture ("assets/bricks.jpg"); // esta serah a textura 0
    LoadTexture ("assets/Piso.jpg"); // esta serah a textura 1
    LoadTexture ("assets/CROSS.jpg"); // esta serah a textura 2
    UseTexture (-1); // desabilita o uso de textura, inicialmente
 
}

// **********************************************************************
//
// **********************************************************************
void animate()
{
    double dt;
    dt = T.getDeltaT();
    AccumDeltaT += dt;
    TempoTotal += dt;
    nFrames++;

    t_plane += 0.15f * dt;
    if (t_plane >= 1.0f) {
        t_plane -= 1.0f;
        current_curve = (current_curve + 1) % 4;
    }

    if (AccumDeltaT > 1.0/30) 
    {
        AccumDeltaT = 0;
        angulo+= 1;
        glutPostRedisplay();
    }
    if (TempoTotal > 5.0)
    {
        cout << "Tempo Acumulado: "  << TempoTotal << " segundos. " ;
        cout << "Nros de Frames sem desenho: " << nFrames << endl;
        cout << "FPS(sem desenho): " << nFrames/TempoTotal << endl;
        TempoTotal = 0;
        nFrames = 0;
    }

    if (speedVehicle > 0.0f && fuel > 0.0f) {
        float angleRads = angleVehicle * M_PI / 180.0f;
        float dist = speedVehicle * dt;

        fuel -= dist * 0.5f; 
        if (fuel <= 0.0f) {
            fuel = 0.0f;
            speedVehicle = 0.0f;
            cout << "sem combustivel" << endl;
        }

        float nextX = PosVehicle.x + dist * sin(angleRads);
        float nextZ = PosVehicle.z + dist * cos(angleRads);

        int gridX = (int)round(nextX);
        int gridZ = (int)round(nextZ);

        if (gridX >= 0 && gridX < mapX && gridZ >= 0 && gridZ < mapZ) {
            int floorType = Cidade[gridZ][gridX].tipo;

            if ((floorType >= 1 && floorType <= 12) || floorType == COMBUSTIVEL || floorType == 40) {
                PosVehicle.x = nextX;
                PosVehicle.z = nextZ;

                if (floorType == 30) {
                    fuel += 20.0f;
                    if (fuel > 100.0f) fuel = 100.0f; 
                    
                    Cidade[gridZ][gridX].tipo = 7; 
                    cout << "pegou combustivel. tanque: " << fuel << endl;
                }
            } else {
                speedVehicle = 0.0f;
                cout << "bateu fora da rua" << endl;
            }
        } else {
            speedVehicle = 0.0f;
        }


        if (Cidade[gridZ][gridX].hasFuel) {
            fuel += 20.0f;
            if (fuel > 100.0f) 
                fuel = 100.0f;
            Cidade[gridZ][gridX].hasFuel = false;
            printf("combustivel coletado. tanque=%.2f", fuel);
        }

        glutPostRedisplay();
    }
}

// **********************************************************************
//  void DesenhaCubo()
// **********************************************************************
void DesenhaCubo(float tamAresta)
{
    glBegin ( GL_QUADS );
    // Front Face
    glNormal3f(0,0,1);
    glVertex3f(-tamAresta/2, -tamAresta/2,  tamAresta/2);
    glVertex3f( tamAresta/2, -tamAresta/2,  tamAresta/2);
    glVertex3f( tamAresta/2,  tamAresta/2,  tamAresta/2);
    glVertex3f(-tamAresta/2,  tamAresta/2,  tamAresta/2);
    // Back Face
    glNormal3f(0,0,-1);
    glVertex3f(-tamAresta/2, -tamAresta/2, -tamAresta/2);
    glVertex3f(-tamAresta/2,  tamAresta/2, -tamAresta/2);
    glVertex3f( tamAresta/2,  tamAresta/2, -tamAresta/2);
    glVertex3f( tamAresta/2, -tamAresta/2, -tamAresta/2);
    // Top Face
    glNormal3f(0,1,0);
    glVertex3f(-tamAresta/2,  tamAresta/2, -tamAresta/2);
    glVertex3f(-tamAresta/2,  tamAresta/2,  tamAresta/2);
    glVertex3f( tamAresta/2,  tamAresta/2,  tamAresta/2);
    glVertex3f( tamAresta/2,  tamAresta/2, -tamAresta/2);
    // Bottom Face
    glNormal3f(0,-1,0);
    glVertex3f(-tamAresta/2, -tamAresta/2, -tamAresta/2);
    glVertex3f( tamAresta/2, -tamAresta/2, -tamAresta/2);
    glVertex3f( tamAresta/2, -tamAresta/2,  tamAresta/2);
    glVertex3f(-tamAresta/2, -tamAresta/2,  tamAresta/2);
    // Right face
    glNormal3f(1,0,0);
    glVertex3f( tamAresta/2, -tamAresta/2, -tamAresta/2);
    glVertex3f( tamAresta/2,  tamAresta/2, -tamAresta/2);
    glVertex3f( tamAresta/2,  tamAresta/2,  tamAresta/2);
    glVertex3f( tamAresta/2, -tamAresta/2,  tamAresta/2);
    // Left Face
    glNormal3f(-1,0,0);
    glVertex3f(-tamAresta/2, -tamAresta/2, -tamAresta/2);
    glVertex3f(-tamAresta/2, -tamAresta/2,  tamAresta/2);
    glVertex3f(-tamAresta/2,  tamAresta/2,  tamAresta/2);
    glVertex3f(-tamAresta/2,  tamAresta/2, -tamAresta/2);
    glEnd();

}
void DesenhaParalelepipedo()
{
    glPushMatrix();
        glTranslatef(0,0,-1);
        glScalef(1,1,2);
        glutSolidCube(2);
        //DesenhaCubo(1);
    glPopMatrix();
}

// **********************************************************************
// void DesenhaLadrilho(int corBorda, int corDentro)
// Desenha uma c�lula do piso.
// Eh possivel definir a cor da borda e do interior do piso
// O ladrilho tem largula 1, centro no (0,0,0) e est� sobre o plano XZ
// **********************************************************************
void DesenhaLadrilho(int corBorda, int corDentro)
{
    defineCor(corDentro);// desenha QUAD preenchido
    //glColor3f(0.5,0,0.5);
    glBegin ( GL_QUADS );
        glNormal3f(0,1,0);
        glVertex3f(-0.5f,  0.0f, -0.5f);
        glVertex3f(-0.5f,  0.0f,  0.5f);
        glVertex3f( 0.5f,  0.0f,  0.5f);
        glVertex3f( 0.5f,  0.0f, -0.5f);
    glEnd();

    defineCor(corBorda);
    //glColor3f(0,1,0);

    glBegin ( GL_LINE_LOOP );
        glNormal3f(0,1,0);
        glVertex3f(-0.5f,  0.0f, -0.5f);
        glVertex3f(-0.5f,  0.0f,  0.5f);
        glVertex3f( 0.5f,  0.0f,  0.5f);
        glVertex3f( 0.5f,  0.0f, -0.5f);
    glEnd();
}
// **********************************************************************
// void DesenhaLadrilhoTex(int idTextura)
// **********************************************************************
void DesenhaLadrilhoTEX(int idTextura)
{
    //glColor3f(1,1,1);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);

    UseTexture(idTextura); // Habilita a textura id_textura
    glBegin ( GL_QUADS );
        glNormal3f(0,1,0);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.0f, -0.5f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f,  0.0f,  0.5f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f,  0.0f,  0.5f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.0f, -0.5f);
    glEnd();
    
    UseTexture(-1); // Desabilita o uso de texturas
    
    // glBegin ( GL_LINE_LOOP );
    //     glNormal3f(0,1,0);
    //     glVertex3f(-0.5f,  0.0f, -0.5f);
    //     glVertex3f(-0.5f,  0.0f,  0.5f);
    //     glVertex3f( 0.5f,  0.0f,  0.5f);
    //     glVertex3f( 0.5f,  0.0f, -0.5f);
    // glEnd();
    glDisable(GL_TEXTURE_2D);

}
// **********************************************************************
//
// **********************************************************************
void DesenhaPlacas()
{
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    DesenhaLadrilhoTEX(1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1, 1, 0);
    glRotatef(90, 1, 0, 0);
    DesenhaLadrilhoTEX(0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-1, 1, 0);
    glRotatef(90, 1, 0, 0);
    glRotatef(angulo, 0, 1, 0);
    DesenhaLadrilhoTEX(2);
    glPopMatrix();

}
// **********************************************************************
//
//
// **********************************************************************
void DesenhaPiso()
{
    srand(100); // usa uma semente fixa para gerar sempre as mesma cores no piso
    glPushMatrix();
    glTranslated(CantoEsquerdo.x, CantoEsquerdo.y, CantoEsquerdo.z);
    for(int x=-20; x<20;x++)
    {
        glPushMatrix();
        for(int z=-20; z<20;z++)
        {
            DesenhaLadrilho(MediumGoldenrod, rand()%40);
            glTranslated(0, 0, 1);
        }
        glPopMatrix();
        glTranslated(1, 0, 0);
    }
    glPopMatrix();
}
//GLfloat LuzAmbiente[]   = {0.4, 0.4, 0.4 } ;
GLfloat LuzAmbiente[]   = {0.4, 0.4, 0.4 } ;
GLfloat LuzDifusa[]   = {0.7, 0.7, 0.7};
GLfloat LuzEspecular[] = {0.9f, 0.9f, 0.0 };
GLfloat PosicaoLuz0[]  = {0.0f, 3.0f, 5.0f };  // Posi��o da Luz
GLfloat Especularidade[] = {1.0f, 1.0f, 1.0f};
//GLfloat Especularidade[] = {0.5f, 0.5f, 1.0f};

// **********************************************************************
//  void DefineLuz(void)
// **********************************************************************
void DefineLuz(void)
{

   // ****************  Fonte de Luz 0

 glEnable ( GL_COLOR_MATERIAL );

   // Habilita o uso de ilumina��o
  glEnable(GL_LIGHTING);

  // Ativa o uso da luz ambiente
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LuzAmbiente);
  // Define os parametros da luz n�mero Zero
  glLightfv(GL_LIGHT0, GL_AMBIENT, LuzAmbiente);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, LuzDifusa  );
  glLightfv(GL_LIGHT0, GL_SPECULAR, LuzEspecular  );
  glLightfv(GL_LIGHT0, GL_POSITION, PosicaoLuz0 );
  glEnable(GL_LIGHT0);

  // Ativa o "Color Tracking"
  glEnable(GL_COLOR_MATERIAL);

  // Define a reflectancia do material
  glMaterialfv(GL_FRONT,GL_SPECULAR, Especularidade);

  // Define a concentra��oo do brilho.
  // Quanto maior o valor do Segundo parametro, mais
  // concentrado ser� o brilho. (Valores v�lidos: de 0 a 128)
  glMateriali(GL_FRONT,GL_SHININESS,51);

}
// **********************************************************************
//
// **********************************************************************
void MygluPerspective(float fieldOfView, float aspect, float zNear, float zFar )
{
    //https://stackoverflow.com/questions/2417697/gluperspective-was-removed-in-opengl-3-1-any-replacements/2417756#2417756
    // The following code is a fancy bit of math that is equivilant to calling:
    // gluPerspective( fieldOfView/2.0f, width/height , 0.1f, 255.0f )
    // We do it this way simply to avoid requiring glu.h
    //GLfloat zNear = 0.1f;
    //GLfloat zFar = 255.0f;
    //GLfloat aspect = float(width)/float(height);
    GLfloat fH = tan( float(fieldOfView / 360.0f * 3.14159f) ) * zNear;
    GLfloat fW = fH * aspect;
    glFrustum( -fW, fW, -fH, fH, zNear, zFar );
}
// **********************************************************************
//  void PosicUser()
// **********************************************************************
void PosicUser()
{

    // Define os par�metros da proje��o Perspectiva
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Define o volume de visualiza��o sempre a partir da posicao do
    // observador
    if (ModoDeProjecao == 0)
        glOrtho(-10, 10, -10, 10, 0, 7); // Projecao paralela Orthografica
    //else MygluPerspective(60,AspectRatio,0.01,50); // Projecao perspectiva
    else MygluPerspective(60, AspectRatio, 0.1f, 500.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(OBS.x,OBS.y, OBS.z,   // Posi��o do Observador
              ALVO.x,ALVO.y, ALVO.z,     // Posi��o do Alvo
              0.0f,1.0f,0.0f);

}
// **********************************************************************
//  void reshape( int w, int h )
//		trata o redimensionamento da janela OpenGL
//
// **********************************************************************
void reshape( int w, int h )
{

	// Evita divis�o por zero, no caso de uam janela com largura 0.
	if(h == 0) h = 1;
    // Ajusta a rela��o entre largura e altura para evitar distor��o na imagem.
    // Veja fun��o "PosicUser".
	AspectRatio = 1.0f * w / h;
	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Seta a viewport para ocupar toda a janela
    glViewport(0, 0, w, h);
    //cout << "Largura" << w << endl;

	PosicUser();

}
// **********************************************************************
//  void DesenhaCentro()
// **********************************************************************
void DesenhaCentro()
{
    glColor3f(0.7,0,0);
    glutSolidSphere(0.3, 20, 20);
}

void DrawMap() 
{
    AABBBoundingBox vehicleBox(Ponto(PosVehicle.x - 0.15, PosVehicle.y, PosVehicle.z - 0.3), 
                               Ponto(PosVehicle.x + 0.15, PosVehicle.y + 0.3, PosVehicle.z + 0.3)); 
    
    glPushMatrix();

    for(int z = 0; z < mapZ; z++) {
        for(int x = 0; x < mapX; x++) {
            
            glPushMatrix(); 
            
            glTranslatef((float)x, 0.0f, (float)z); 
            
            int type = Cidade[z][x].tipo;

            if (type == 30 || (type >= 1 && type <= 12)) {
                glEnable(GL_TEXTURE_2D);
                glColor3f(1,1,1);
                int texID = (type == 30) ? 6 : (type - 1);
                DesenhaLadrilhoTEX(texID);
                glDisable(GL_TEXTURE_2D);
            } else { 
                DesenhaLadrilho(White, Green); 
            } 

            if (type == 30) {
                glPushMatrix();
                    glTranslatef(0.0f, 0.5f, 0.0f);
                    glRotatef(angulo * 5.0f, 0, 1, 0);
                    glScalef(0.8f, 0.8f, 0.8f);
                    // glColor3f(1.0f, 0.5f, 0.0f);
                    barrelModel.desenha();
                glPopMatrix();
            }

            if (Cidade[z][x].hasFuel) {
                glPushMatrix();
                    glTranslatef(0.0, 0.5f, 0.0f);
                    glRotatef(angulo * 5.0f, 0, 1, 0);
                    glScalef(1.0f, 1.0f, 1.0f);
                    glDisable(GL_TEXTURE_2D);
                    glColor3f(1.0f, 0.5f, 0.0f);
                    barrelModel.desenha();
                glPopMatrix();
            }

            
            if (type == 0) {
                if ((x + z) % 2 == 0) {
                    int seed = (x * 13) + (z * 17);
                    float buldingHeight = 2.0f + (seed % 6);

                    // float r = ((x * 3) % 10) / 10.0f;
                    // float g = ((z * 5) % 10) / 10.0f;
                    // float b = ((x + z) % 10) / 10.0f;
                    
                    AABBBoundingBox buildingBox { Ponto(x - 0.2f, 0.0f, z - 0.2f), Ponto(x + 0.2f, 2.0f, z + 0.2f)};
                    
                    if (vehicleBox.CollideWith(buildingBox)) {
                        speedVehicle = 0.0f;
                        PosVehicle.x -= sin(angleVehicle * M_PI / 180.0f) * 0.1f;
                        PosVehicle.z -= cos(angleVehicle * M_PI / 180.0f) * 0.1f;
                    }

                    glPushMatrix();
                        glTranslatef(0.0f, 0.0f, 0.0f); 
                        
                        int buildignRot = (seed % 4) * 90;
                        glRotatef((float)buildignRot, 0, 1, 0);
                        
                        // int sortedBuildingHeight = seed % 2;
                        // if (sortedBuildingHeight == 0) glScalef(0.4f, 1.0f, 0.4f);
                        // else glScalef(0.4f, 1.2f, 0.4f);
                         glScalef(0.5f, 0.5f, 0.5f);
                        
                        glEnable(GL_TEXTURE_2D);
                        glColor3f(1.0f, 1.0f, 1.0f); 
                        UseTexture(13);
                        
                        int sortedBuildingType = seed % 3;
                        // printf("sortedbuildtype: %d\n", sortedBuildingType);
                        if (sortedBuildingType == 0) buildingAModel.desenha();
                        else if (sortedBuildingType == 1) buildingBModel.desenha();
                        else if (sortedBuildingType == 2) buildingCModel.desenha();
                        else buildingAModel.desenha();
                        
                        UseTexture(-1);
                        glDisable(GL_TEXTURE_2D);
                        
                    glPopMatrix();
                }
            }

            glPopMatrix(); 
        }
    }

    glPopMatrix();
}

void DrawHUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, glutGet(GLUT_WINDOW_WIDTH), 0.0, glutGet(GLUT_WINDOW_HEIGHT));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
        glVertex2f(20, 20);
        glVertex2f(220, 20);
        glVertex2f(220, 40);
        glVertex2f(20, 40);
    glEnd();

    if (fuel > 20.0f) glColor3f(0.0f, 0.8f, 0.0f);
    else glColor3f(0.8f, 0.0f, 0.0f);
    
    glBegin(GL_QUADS);
        glVertex2f(20, 20);
        glVertex2f(20 + (fuel * 2.0f), 20); // fuel * 2 = 200px max
        glVertex2f(20 + (fuel * 2.0f), 40);
        glVertex2f(20, 40);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(25, 45);
    char fuelText[50];
    sprintf(fuelText, "Combustivel: %.1f%%", fuel);
    for (char* c = fuelText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    glRasterPos2f(20, glutGet(GLUT_WINDOW_HEIGHT) - 30);
    const char* ctrl1 = "Controles: Esq/Dir (Direção) | ESPACO (Acelerar/Parar)";
    for (const char* c = ctrl1; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    
    glRasterPos2f(20, glutGet(GLUT_WINDOW_HEIGHT) - 50);
    const char* ctrl2 = "W,A,S,D (Mover Camera) | R (Resetar Camera) | C (Mudar Camera)";
    for (const char* c = ctrl2; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

}

// **********************************************************************
//  void display( void )
// **********************************************************************
void display( void )
{

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    UpdateCamera();
	PosicUser();
    DefineLuz();

    DrawMap();
    DrawVehicle();
    DrawAirplane();
    DrawHUD();
    
	glutSwapBuffers();
}

void passiveMouseMotion(int x, int y) {
    if (CameraMode == 3) {
        int centerX = glutGet(GLUT_WINDOW_WIDTH) / 2;
        int centerY = glutGet(GLUT_WINDOW_HEIGHT) / 2;

        if (x == centerX && y == centerY) {
            return;
        }

        if (firstMouse) {
            firstMouse = false;
            glutWarpPointer(centerX, centerY);
            return;
        }

        float dx = (float)(x - centerX);
        float dy = (float)(y - centerY);

        camera3RotX += dx * 0.02f; 
        camera3RotY += dy * 0.02f;

        if (camera3RotY < 5.0f) camera3RotY = 5.0f;
        if (camera3RotY > 175.0f) camera3RotY = 175.0f;

        glutWarpPointer(centerX, centerY);
        glutPostRedisplay();
    }
}

void mouseClick(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        lastMouseX = x;
        lastMouseY = y;
    } else {
        lastMouseX = -1;
        lastMouseY = -1;
    }
}

// **********************************************************************
//  void keyboard ( unsigned char key, int x, int y )
//
//
// **********************************************************************
void keyboard ( unsigned char key, int x, int y )
{
    switch ( key )
    {
    case 27: 
      exit ( 0 );   
      break;
    case 32:
        if (speedVehicle == 0.0f) {
            speedVehicle = (float)mapX / 10.0f;
        } else {
            speedVehicle = 0.0f;
        }
        break;
    case 'c':
    case 'C':
        if (CameraMode == 3) CameraMode = 1;
        else CameraMode = 3;
        break;
    case 'p':
            ModoDeProjecao = !ModoDeProjecao;
            glutPostRedisplay();
            break;
    case 'e':
            ModoDeExibicao = !ModoDeExibicao;
            init();
            glutPostRedisplay();
            break;
    case 'a': case 'A': 
        if (CameraMode == 3) {
            camera3RotX -= 5.0f; 
        } else {
            lookX += 5.0f;     
        }
        break;
    case 'd': case 'D': 
        if (CameraMode == 3) {
            camera3RotX += 5.0f; 
        } else {
            lookX -= 5.0f;      
        }
        break;
    case 'w': case 'W':
        if (CameraMode == 3) {
            camera3RotY -= 5.0f; 
            if (camera3RotY < 5.0f) camera3RotY = 5.0f; 
        } else {
            lookY += 5.0f;      
            if (lookY > 80.0f) lookY = 80.0f; 
        }
        break;
    case 's': case 'S':
        if (CameraMode == 3) {
            camera3RotY += 5.0f; 
            if (camera3RotY > 175.0f) camera3RotY = 175.0f;
        } else {
            lookY -= 5.0f;      
            if (lookY < -80.0f) lookY = -80.0f;
        }
        break;
    case 'r': case 'R':
        lookX = 0.0f; 
        lookY = 0.0f;
        camera3RotX = 180.0f;
        camera3RotY = 30.0f;
        break;
  }
}

// **********************************************************************
//  void arrow_keys ( int a_keys, int x, int y )
//
//
// **********************************************************************
void arrow_keys ( int a_keys, int x, int y )
{
	switch ( a_keys )
	{
		case GLUT_KEY_UP:       // When Up Arrow Is Pressed...
            V1 = ALVO-OBS; // Go Into Full Screen Mode
            V1.versor();
            OBS = OBS + V1;
            ALVO = ALVO + V1;
			break;
	    case GLUT_KEY_DOWN:     // When Down Arrow Is Pressed...
            V1 = ALVO-OBS; // Go Into Full Screen Mode
            V1.versor();
            OBS = OBS - V1;
            ALVO = ALVO - V1;
			break;
        case GLUT_KEY_LEFT:   // When Up Arrow Is Pressed...
            // V1 = ALVO-OBS;    // Go Into Full Screen Mode
            // V1.rotacionaY(5);
            // ALVO = OBS + V1;
            angleVehicle += 5.0f;
            break;
        case GLUT_KEY_RIGHT:   // When Up Arrow Is Pressed...
            // V1 = ALVO-OBS;    // Go Into Full Screen Mode
            // V1.rotacionaY(-5);
            // ALVO = OBS + V1;
            angleVehicle -= 5.0f;
            break;
		default:
			break;
	}
}

// **********************************************************************
//  void main ( int argc, char** argv )
//
//
// **********************************************************************
int main ( int argc, char** argv )
{
	glutInit            ( &argc, argv );
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
	glutInitWindowPosition (0,0);
	glutInitWindowSize  ( 1800, 900 );
	glutCreateWindow    ( "Computacao Grafica - Exemplo Basico 3D" );

	init ();
    //system("pwd");

	glutDisplayFunc ( display );
	glutReshapeFunc ( reshape );
	glutKeyboardFunc ( keyboard );
	glutSpecialFunc ( arrow_keys );

    // glutPassiveMotionFunc(passiveMouseMotion);
    //glutSetCursor(GLUT_CURSOR_NONE);
    // glutMouseFunc(mouseClick);

	glutIdleFunc ( animate );

	glutMainLoop ( );
	return 0;
}



