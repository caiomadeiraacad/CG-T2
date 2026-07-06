
// Estruturas para armazenar as texturas de uma cidade

// Constantes para indexar as texturas 
enum PISOS {
    CROSS,
    DL,
    DLR,
    DR,
    LR,
    None,
    UD,
    UDL,
    UDR,
    UL,
    ULR,
    UR,
    LAST_IMG
};

// Lista de nomes das texturas
string nomeTexturas[] = {
    "CROSS.png",
    "DL.png",
    "DLR.png",
    "DR.png",
    "LR.png",
    "None.png",
    "UD.png",
    "UDL.png",
    "UDR.png",
    "UL.png",
    "ULR.png",
    "UR.png"};

int idTexturaRua[LAST_IMG];  // vetor com os identificadores das texturas


// **********************************************************************
// void CarregaTexturas()
// **********************************************************************
void CarregaTexturas()
{
    for(int i=0;i<LAST_IMG;i++)
        idTexturaRua[i] = LoadTexture(nomeTexturas[i].c_str());
}