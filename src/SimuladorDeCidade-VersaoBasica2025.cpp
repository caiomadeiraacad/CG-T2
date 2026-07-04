// **********************************************************************
// PUCRS/Escola Politecnica
// COMPUTACAO GRAFICA
//
// Programa basico para criar aplicacoes 3D em OpenGL
//
// Marcio Sarroglia Pinho
// pinho@pucrs.br
// **********************************************************************

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <ctime>

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
#include <glut.h>
#endif

#include "Temporizador.h"
Temporizador T;
double AccumDeltaT=0;

#include "Ponto.h"
#include "ListaDeCoresRGB.h"
#include "Texture.h"

GLfloat AnguloDeVisao=90;
GLfloat AspectRatio, angulo=0;
GLfloat AlturaViewportDeMensagens = 0.2; // percentual em relacao � altura da tela


// Controle do modo de projecao
// 0: Projecao Paralela Ortografica; 1: Projecao Perspectiva
// A funcao "PosicUser" utiliza esta variavel. O valor dela eh alterado
// pela tecla 'p'

int ModoDeProjecao = 1;

// Controle do modo de projecao
// 0: Wireframe; 1: Faces preenchidas
// A funcao "Init" utiliza esta variavel. O valor dela eh alterado
// pela tecla 'w'
int ModoDeExibicao = 1;

double nFrames=0;
double TempoTotal=0;

// Qtd de ladrilhos do piso. Inicialzada na INIT
int QtdX;
int QtdZ;


// Representa o conteudo de uma celula do piso
class Elemento{
public:
    int tipo;
    int corDoObjeto;
    int corDoPiso;
    float altura;
    int textureID;
};

// codigos que definem o o tipo do elemento que esta em uma celula
#define VAZIO 0
#define PREDIO 10
#define RUA 20
#define COMBUSTIVEL 30
#define VEICULO 40

// Matriz que armazena informacoes sobre o que existe na cidade
Elemento Cidade[100][100];


Ponto Observador, Alvo, TerceiraPessoa, PosicaoVeiculo;

bool ComTextura = true;


// **********************************************************************
//
// **********************************************************************
void ImprimeCidade() {
    for (int i = 0; i < QtdZ; i++) {
        for (int j = 0; j < QtdX; j++) {
            cout << Cidade[i][j].corDoPiso << " ";
        }
        cout << endl;
    }
}

// **********************************************************************
// void InicializaCidade(int QtdX, int QtdZ)
// Esta funcao sera substituida por uma que le a matriz que representa
// a Cidade
// **********************************************************************
void InicializaCidade(int QtdX, int QtdZ)
{
    for (int i=0;i<QtdZ;i++)
        for (int j=0;j<QtdX;j++)
        {
            // a cor do piso vai depender do mapa. Aqui, eh aletoria.
            Cidade[i][j].corDoPiso = rand()%40;
            Cidade[i][j].corDoObjeto = White;
            Cidade[i][j].tipo = VAZIO;
        }
    //Cidade[9][19].corDoPiso = Black;
    //ImprimeCidade();
    
}

// **********************************************************************
// void PosicionaEmTerceiraPessoa()
//   Este metodo posiciona o obeservador fora do cenario, olhando para o
// centro do mapa
// As variaveis "TerceiraPessoa" e "PosicaoVeiculo" sao setadas na INIT
// **********************************************************************
void PosicionaEmTerceiraPessoa()
{
    Observador = TerceiraPessoa;   // Posicao do Observador
    Alvo = PosicaoVeiculo;         // Posicao do Alvo
    //Alvo.imprime("Posiciona - Veiculo:");
}

// **********************************************************************
//  void init(void)
//    Inicializa os parametros globais de OpenGL
// **********************************************************************
void init(void)
{
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f); // Fundo de tela preto
   
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glShadeModel(GL_SMOOTH); // (GL_FLAT);
    glColorMaterial ( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
    
    glEnable(GL_DEPTH_TEST);
    //glEnable (GL_CULL_FACE);
    
    if (ModoDeExibicao) // Faces Preenchidas??
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    glEnable(GL_NORMALIZE);
    
    // Quantidade de retangulos do piso vai depender do
    // mapa que sera' lido
    QtdX = 5;
    QtdZ = 5;
    
    InicializaCidade(QtdX, QtdZ);
    
    // Define a posicao do observador e do veiculo
    // com base no tamanho do mapa
    TerceiraPessoa = Ponto(QtdX/2, 10, QtdZ*1.1);
    PosicaoVeiculo = Ponto(QtdX/2, 0, QtdZ/2);
    
    PosicionaEmTerceiraPessoa();
    //glDisable(GL_TEXTURE_2D);
    
    AnguloDeVisao = 45;
    
    LoadTexture ("bricks.jpg"); // esta serah a textura 0
    LoadTexture ("Piso.jpg"); // esta serah a textura 1
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

    if (AccumDeltaT > 1.0/30) // fixa a atualizacao da tela em 30
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
}

// **********************************************************************
//  Desenha um predio no meio de uma celula
// **********************************************************************
void DesenhaPredio(float altura)
{
    glPushMatrix();
       // glTranslatef(0, -1, 0);
        glScalef(0.2, altura, 0.2);
        glTranslatef(0, 1, 0);
        glutSolidCube(1);
    glPopMatrix();
    
}
// **********************************************************************
// void DesenhaLadrilhoTex(int idTextura)
// **********************************************************************
void DesenhaLadrilhoTEX(int idTextura)
{
    UseTexture(idTextura); // Habilita a textura id_textura
    glBegin ( GL_QUADS );
        glNormal3f(0,1,0);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-0.5f,  0.0f, -0.5f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-0.5f,  0.0f,  0.5f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f( 0.5f,  0.0f,  0.5f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f( 0.5f,  0.0f, -0.5f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

}
// **********************************************************************
// void DesenhaPoligonosComTextura()
// **********************************************************************
void DesenhaPoligonosComTextura()
{
    glPushMatrix();
    glTranslatef(QtdX*0.2,1, QtdZ*0.8);
    glRotatef(angulo,1,0,0);
    DesenhaLadrilhoTEX(0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(QtdX*0.6,1,QtdZ*0.8);
    glRotatef(45,1,0,0);
    DesenhaLadrilhoTEX(1);
    glPopMatrix();

}

// **********************************************************************
// void DesenhaLadrilho(int corBorda, int corDentro)
// Desenha uma celula do piso.
// Eh possivel definir a cor da borda e do interior do piso
// O ladrilho tem largula 1, centro no (0,0,0) e esta sobre o plano XZ
// Para aqueles que forem usar texturas, esta funcao serah substituida
// pela funcao "DesenhaLadrilhoTex"
// **********************************************************************
void DesenhaLadrilho(int corBorda, int corDentro)
{
    defineCor(corDentro); // desenha QUAD preenchido
    glBegin ( GL_QUADS );
        glNormal3f(0,1,0);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-0.5f,  0.0f, -0.5f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-0.5f,  0.0f,  0.5f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f( 0.5f,  0.0f,  0.5f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f( 0.5f,  0.0f, -0.5f);
    glEnd();
    
    defineCor(corBorda);
    glBegin ( GL_LINE_STRIP ); // desenha borda do ladrilho
        glNormal3f(0,1,0);
        glVertex3f(-0.5f,  0.01f, -0.5f);
        glVertex3f(-0.5f,  0.01f,  0.5f);
        glVertex3f( 0.5f,  0.01f,  0.5f);
        glVertex3f( 0.5f,  0.01f, -0.5f);
    glEnd();
}
// **********************************************************************
//
//
// **********************************************************************
void DesenhaCidade(int QtdX, int QtdZ)
{
    srand(100); // usa uma semente fixa para gerar sempre as mesma cores no piso
    glPushMatrix();

    for(int x=0; x<QtdX;x++)
    {
        glPushMatrix();
        for(int z=0; z<QtdZ;z++)
        {
            DesenhaLadrilho(Yellow, Cidade[z][x].corDoPiso);
            glTranslated(0, 0, 1);
        }
        // Aqui, os predios devem ser desenhados
        
        glPopMatrix();
        glTranslated(1, 0, 0);
    }
    glPopMatrix();
}

// **********************************************************************
//  void DefineLuz(void)
// **********************************************************************
void DefineLuz(void)
{
  // Define cores para um objeto dourado
  GLfloat LuzAmbiente[]   = {0.4, 0.4, 0.4f } ;
  GLfloat LuzDifusa[]   = {0.7, 0.7, 0.7};
  GLfloat LuzEspecular[] = {0.9f, 0.9f, 0.9 };
    
  GLfloat PosicaoLuz0[]  = {Alvo.x, Alvo.y, Alvo.z };  // Posicao da Luz
  //GLfloat PosicaoLuz0[]  = {0.0f, 3.0f, 5.0f };  // Posicao da Luz
  GLfloat Especularidade[] = {1.0f, 1.0f, 1.0f};

   // ****************  Fonte de Luz 0

  glEnable ( GL_COLOR_MATERIAL );

   // Habilita o uso de iluminacao
  glEnable(GL_LIGHTING);

  // Ativa o uso da luz ambiente
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LuzAmbiente);
    
  // Define os parametros da luz numero Zero
  glLightfv(GL_LIGHT0, GL_AMBIENT, LuzAmbiente);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, LuzDifusa  );
  glLightfv(GL_LIGHT0, GL_SPECULAR, LuzEspecular  );
  glLightfv(GL_LIGHT0, GL_POSITION, PosicaoLuz0 );
  glEnable(GL_LIGHT0);

  // Ativa o "Color Tracking"
  glEnable(GL_COLOR_MATERIAL);

  // Define a reflectancia do material
  glMaterialfv(GL_FRONT,GL_SPECULAR, Especularidade);

  // Define a concentracaoo do brilho.
  // Quanto maior o valor do Segundo parametro, mais
  // concentrado sera o brilho. (Valores v�lidos: de 0 a 128)
  glMateriali(GL_FRONT,GL_SHININESS,51);

}

// **********************************************************************
//  void PosicUser()
// **********************************************************************
void PosicUser()
{

    // Define os parametros da projecao Perspectiva
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Define o volume de visualizacao sempre a partir da posicao do
    // observador
    if (ModoDeProjecao == 0)
        glOrtho(-10, 10, -10, 10, 0, 20); // Projecao paralela Orthografica
    else gluPerspective(AnguloDeVisao,AspectRatio,0.01,1500); // Projecao perspectiva

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    gluLookAt(Observador.x, Observador.y, Observador.z,   // Posicao do Observador
              Alvo.x, Alvo.y, Alvo.z,     // Posicao do Alvo
              0.0f,1.0f,0.0f); // UP


}
// **********************************************************************
//  void reshape( int w, int h )
//		trata o redimensionamento da janela OpenGL
//
// **********************************************************************
void reshape( int w, int h )
{

	// Evita divisao por zero, no caso de uma janela com largura 0.
	if(h == 0) h = 1;
    // Ajusta a relacao entre largura e altura para evitar distorcao na imagem.
    // Veja funcao "PosicUser".
    AspectRatio = (float)w / h; // recalcula o AspectRatio
    
	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    
	// Ajusta a viewport para ocupar toda a janela
    //glViewport(0, 0, w, h);
    
    // Seta a viewport para ocupar a parte superior da janela
    glViewport(0, h*AlturaViewportDeMensagens, w, h-h*AlturaViewportDeMensagens);
    
    gluPerspective(AnguloDeVisao,AspectRatio,0.01,1500); // Projecao perspectiva

}
// **********************************************************************
//
// **********************************************************************
void printString(string s, int posX, int posY, int cor)
{
    defineCor(cor);
    
    glRasterPos3i(posX, posY, 0); //define posicao na tela
    for (int i = 0; i < s.length(); i++)
    {
//GLUT_BITMAP_HELVETICA_10,
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s[i]);
    }
    
}
// **********************************************************************
// Esta funcao permite que se escreva mensagens na tela
//
// **********************************************************************
void DesenhaEm2D()
{
    int ativarLuz = false;
    if (glIsEnabled(GL_LIGHTING))
    {
        glDisable(GL_LIGHTING);
        ativarLuz = true;
    }
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Salva o tamanho da janela
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    
    // Define a area a ser ocupada pela area OpenGL dentro da Janela
    glViewport(0, 0, w, h*AlturaViewportDeMensagens); // a janela de mensagens fica na parte de baixo da janela

    // Define os limites logicos da area OpenGL dentro da Janela
    glOrtho(0,10, 0,10, 0,1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Desenha linha que Divide as �reas 2D e 3D
    defineCor(GreenCopper);
    glLineWidth(15);
    glBegin(GL_LINES);
        glVertex2f(0,10);
        glVertex2f(10,10);
    glEnd();
    
    printString("Esta area eh destinada a mensagens de texto. Veja a funcao DesenhaEm2D", 0,8, White);
    printString("Amarelo", 0, 0, Yellow);
    printString("Vermelho", 4, 2, Red);
    printString("Verde", 8, 4, Green);

    // Restaura os parametro que foram alterados
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, h*AlturaViewportDeMensagens, w, h-h*AlturaViewportDeMensagens);

    if (ativarLuz)
        glEnable(GL_LIGHTING);

}

// **********************************************************************
//  void display( void )
//
//
// **********************************************************************
void display( void )
{

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	DefineLuz();

	PosicUser();
    glLineWidth(2);
	
	glMatrixMode(GL_MODELVIEW);

    DesenhaCidade(QtdX,QtdZ);
    
    glPushMatrix();
    DesenhaPoligonosComTextura();
    glPopMatrix();

    DesenhaEm2D();

	glutSwapBuffers();
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
    case 27:        // Termina o programa qdo
      exit ( 0 );   // a tecla ESC for pressionada
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
    case 't':
        ComTextura = !ComTextura;
        break;
    default:
            cout << key;
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
			glutFullScreen ( ); // Go Into Full Screen Mode
			break;
	    case GLUT_KEY_DOWN:     // When Down Arrow Is Pressed...
			glutInitWindowSize  ( 700, 500 ); 
			break;
		default:
			break;
	}
}

// **********************************************************************
//  void main ( int argc, char** argv )
// **********************************************************************
int  main ( int argc, char** argv )
{
    cout << "Simulador de Cidade" << endl;
    
    
    system("pwd");

    glutInit            ( &argc, argv );
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    glutInitWindowPosition (0,0);

    // Define o tamanho inicial da janela grafica do programa
    glutInitWindowSize  ( 900, 700);
   
    // Cria a janela na tela, definindo o nome da
    // que aparecera na barra de titulo da janela.
    glutCreateWindow    ( "Simulador de Cidades" );

    // executa algumas inicializacoes
    init ();

    // Define que o tratador de evento para
    // o redesenho da tela. A funcao "display"
    // sera' chamada automaticamente quando
    // for necessario redesenhar a janela
    glutDisplayFunc ( display );

    // Define que o tratador de evento para
    // o invalidacao da tela. A funcao "display"
    // sera' chamada automaticamente sempre que a
    // maquina estiver ociosa (idle)
    glutIdleFunc(animate);

    // Define que o tratador de evento para
    // o redimensionamento da janela. A funcao "reshape"
    // sera chamada automaticamente quando
    // o usuario alterar o tamanho da janela
    glutReshapeFunc ( reshape );

    // Define que o tratador de evento para
    // as teclas. A funcao "keyboard"
    // sera chamada automaticamente sempre
    // o usuario pressionar uma tecla comum
    glutKeyboardFunc ( keyboard );

    // Define que o tratador de evento para
    // as teclas especiais(F1, F2,... ALT-A,
    // ALT-B, Teclas de Seta, ...).
    // A funcao "arrow_keys" sera chamada
    // automaticamente sempre o usuario
    // pressionar uma tecla especial
    glutSpecialFunc ( arrow_keys );

    // inicia o tratamento dos eventos
    glutMainLoop ( );

    return 0;
}





