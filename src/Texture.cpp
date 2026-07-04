

#include <vector>
#include "Texture.h"

std::vector<int> Texturas;

// **********************************************************************
//  GLuint LoadTexture (const char *nomeTex)
// **********************************************************************

GLuint LoadTextureAUX (const char *nomeTex)
{
    GLenum errorCode;
    GLuint IdTEX;
    // Habilita o uso de textura
    glEnable ( GL_TEXTURE_2D );

    // Define a forma de armazenamento dos pixels na textura (1= alihamento por byte)
    glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );

    // Gera um identificar para a textura
    glGenTextures (1, &IdTEX); //  vetor que guardas os números das texturas
    errorCode =  glGetError();
    if (errorCode == GL_INVALID_OPERATION)
    {
        cout << "Erro: glGenTextures chamada entre glBegin/glEnd." << endl;
        return -1;
    }

    // Define que tipo de textura será usada
    // GL_TEXTURE_2D ==> define que será usada uma textura 2D (bitmaps)
    // texture_id[OBJETO_ESQUERDA]  ==> define o número da textura
    glBindTexture ( GL_TEXTURE_2D, IdTEX );

    // Carrega a imagem
    ImageClass Img;

    int r = Img.Load(nomeTex);
    if (!r)
    {
        cout << "Erro lendo imagem " << nomeTex << endl;
        exit(1);
    }

    int level = 0;
    int border = 0;

    // Envia a textura para OpenGL, usando o formato apropriado
    int formato;
    formato = GL_RGB;
    if (Img.Channels() == 4)
        formato = GL_RGBA;
    //if (Img.Channels() == 3)
    //   formato = GL_RGB;

    glTexImage2D (GL_TEXTURE_2D, level, formato,
                  Img.SizeX(), Img.SizeY(),
                  border, formato,
                  GL_UNSIGNED_BYTE, Img.GetImagePtr());
    errorCode = glGetError();
    if (errorCode == GL_INVALID_OPERATION)
    {
        cout << "Erro: glTexImage2D chamada entre glBegin/glEnd." << endl;
        return -1;
    }
    if (errorCode != GL_NO_ERROR)
    {
        cout << "Houve algum erro na criacao da textura." << endl;
        return -1;
    }
    // Ajusta os filtros iniciais para a textura
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Img.Delete();
    
    cout << "Carga de textura OK." << endl;
    return IdTEX;
}

GLuint LoadTexture (const char *nomeTex)
{
    int aux = LoadTextureAUX (nomeTex);
    Texturas.push_back(aux);
    return(aux);
    
}



// **********************************************************************
//    Define a textura a ser usada
// **********************************************************************
void UseTexture(int NroDaTextura)
{
    //cout << "Qtd de Texturas: " << Texturas.size() << endl;
    if (NroDaTextura < 0)
    {
        glDisable (GL_TEXTURE_2D); // desabilita texturas
        return;
    }
    if (NroDaTextura >= Texturas.size())
    {
        cout << "Numero invalido da textura: " << NroDaTextura << endl;
        glDisable (GL_TEXTURE_2D);
        return;
    }
    // habilta o uso de texturas e ativa a textura 'Texturas[NroDaTextura]'
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, Texturas[NroDaTextura]);
}

