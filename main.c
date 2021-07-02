#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h> // Para usar strings

#ifdef WIN32
#include <windows.h> // Apenas para Windows
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>   // Funções da OpenGL
#include <GL/glu.h>  // Funções da GLU
#include <GL/glut.h> // Funções da FreeGLUT
#endif

// SOIL é a biblioteca para leitura das imagens
#include <SOIL.h>

// Um pixel RGB (24 bits)
typedef struct
{
    unsigned char r, g, b;
} RGB8;

// Uma imagem RGB
typedef struct
{
    int width, height;
    RGB8 *img;
} Img;

// Protótipos
void load(char *name, Img *pic);
void uploadTexture();
void seamcarve(int targetWidth); // executa o algoritmo
void freemem();                  // limpa memória (caso tenha alocado dinamicamente)

// Funções da interface gráfica e OpenGL
void init();
void draw();
void keyboard(unsigned char key, int x, int y);
void arrow_keys(int a_keys, int x, int y);

// Largura e altura da janela
int width, height;

// Largura desejada (selecionável)
int targetW;

// Identificadores de textura
GLuint tex[3];

// As 3 imagens
Img pic[3];
Img *source;
Img *mask;
Img *target;

// Imagem selecionada (0,1,2)
int sel;

// Carrega uma imagem para a struct Img
void load(char *name, Img *pic)
{
    int chan;
    pic->img = (RGB8 *)SOIL_load_image(name, &pic->width, &pic->height, &chan, SOIL_LOAD_RGB);
    if (!pic->img)
    {
        printf("SOIL loading error: '%s'\n", SOIL_last_result());
        exit(1);
    }
    printf("Load: %d x %d x %d\n", pic->width, pic->height, chan);
}

//
// Implemente AQUI o seu algoritmo
void seamcarve(int targetWidth)
{
    // Aplica o algoritmo e gera a saida em target->img...

    RGB8(*ptr)
    [target->width] = (RGB8(*)[target->width])target->img;

    RGB8(*ptr2)
    [source->width] = (RGB8(*)[source->width])source->img;

    int matrizGradientes[target->height][target->width];

    //Cria saída para imagem
    for (int i = 0; i < target->height; i++)
    {
        for (int j = 0; j < target->width; j++)
        {
            ptr[i][j].r = ptr2[i][j].r;
            ptr[i][j].g = ptr2[i][j].g;
            ptr[i][j].b = ptr2[i][j].b;
        }
    }
    
    montaMatrizGradiente(matrizGradientes, ptr);

    for (int i = 0; i < target->width - targetWidth; i++)
    {
        caminhoSeam(matrizGradientes, ptr, targetWidth);
    }

    // Chame uploadTexture a cada vez que mudar
    // a imagem (pic[2])
    uploadTexture();
    glutPostRedisplay();
}

int calculaGradiente(RGB8 pixelLeft, RGB8 pixelRight, RGB8 pixelUp, RGB8 pixelDown)
{
    int redGradX = pixelRight.r - pixelLeft.r;
    int greenGradX = pixelRight.g - pixelLeft.g;
    int blueGradX = pixelRight.b - pixelLeft.b;

    int redGradY = pixelDown.r - pixelUp.r;
    int greenGradY = pixelDown.g - pixelUp.g;
    int blueGradY = pixelDown.b - pixelUp.b;

    return ((pow(redGradX, 2) + pow(greenGradX, 2) + pow(blueGradX, 2)) + (pow(redGradY, 2) + pow(greenGradY, 2) + pow(blueGradY, 2)));
}

void montaMatrizGradiente(int matrizGradiente[source->height][source->width], RGB8 pic2[target->height][target->width])
{
    for (int linha = 0; linha < source->height; linha++)
    {
        for (int coluna = 0; coluna < source->width; coluna++)
        {
                if (linha == 0)
                {
                    if (verificaCantoSuperiorEsquerdo(linha, coluna))
                    {
                        matrizGradiente[linha][coluna] = calculaGradiente(pic2[linha][source->width - 1], pic2[linha][coluna + 1], pic2[source->height - 1][coluna], pic2[linha + 1][coluna]);
                    }
                    else if (verificaCantoSuperiorDireito(linha, coluna))
                    {
                        matrizGradiente[linha][coluna] = calculaGradiente(pic2[linha][coluna], pic2[0][0], pic2[source->height - 1][source->width - 1], pic2[linha + 1][coluna]);
                    }
                    else
                    {
                        matrizGradiente[linha][coluna] = calculaGradiente(pic2[linha][coluna - 1], pic2[linha][coluna + 1], pic2[source->height - 1][coluna], pic2[linha + 1][coluna]);
                    }
                }
                //Verifica se é o pixel do canto inferior direito
                else if (verificaCantoInferiorDireito(linha, coluna))
                {
                    //Calcula valor do pixel e acumula com o valor do pixel de menor valor anterior
                    matrizGradiente[linha][coluna] = calculaGradiente(pic2[linha][coluna - 1], pic2[linha][0], pic2[linha - 1][coluna], pic2[0][coluna]) + valorGradientePixelAnterior(linha, coluna, matrizGradiente);
                }
                //Verifica se é o pixel do canto inferior esquerdo
                else if (verificaCantoInferiorEsquerdo(linha, coluna))
                {
                    //Calcula valor do pixel e acumula com o valor do pixel de menor valor anterior
                    matrizGradiente[linha][coluna] = calculaGradiente(pic2[linha][source->width - 1], pic2[linha][coluna + 1], pic2[source->height - 1][coluna], pic2[linha + 1][coluna]) + valorGradientePixelAnterior(linha, coluna, matrizGradiente);
                }
                else if (coluna == 0)
                {
                    //Calcula valor do pixel e acumula com o valor do pixel de menor valor anterior
                    matrizGradiente[linha][coluna] = calculaGradiente(pic2[linha][source->width - 1], pic2[linha][coluna + 1], pic2[linha - 1][coluna], pic2[linha + 1][coluna]) + valorGradientePixelAnterior(linha, coluna, matrizGradiente);
                }
                else if (coluna == source->width)
                {
                    //Calcula valor do pixel e acumula com o valor do pixel de menor valor anterior
                    matrizGradiente[linha][coluna] = calculaGradiente(pic2[linha][coluna - 1], pic2[linha][0], pic2[linha - 1][coluna], pic2[linha + 1][coluna]) + valorGradientePixelAnterior(linha, coluna, matrizGradiente);
                }
                else
                {
                    //Calcula valor do pixel e acumula com o valor do pixel de menor valor anterior
                    matrizGradiente[linha][coluna] = calculaGradiente(pic2[linha][coluna - 1], pic2[linha][coluna + 1], pic2[linha - 1][coluna], pic2[linha + 1][coluna]) + valorGradientePixelAnterior(linha, coluna, matrizGradiente);
                }
            
        }
    }
}

int verificaCantoSuperiorEsquerdo(int linha, int coluna)
{
    if (linha == 0 && coluna == 0)
    {
        return 1;
    }
    return 0;
}

int verificaCantoInferiorEsquerdo(int linha, int coluna)
{
    if (linha == source->height - 1 && coluna == 0)
    {
        return 1;
    }
    return 0;
}

int verificaCantoSuperiorDireito(int linha, int coluna)
{
    if (linha == 0 && coluna == source->width - 1)
    {
        return 1;
    }
    return 0;
}

int verificaCantoInferiorDireito(int linha, int coluna)
{
    if (linha == source->height - 1 && coluna == source->width - 1)
    {
        return 1;
    }
    return 0;
}

int valorGradientePixelAnterior(int linha, int coluna, int matrizGradiente[target->height][target->width])
{
    int valorPixel = 0;
    // Verifica se está na coluna mais a esquerda
    if (coluna == 0)
    {
        if (matrizGradiente[linha - 1][coluna] < matrizGradiente[linha - 1][coluna + 1])
        {
            valorPixel = matrizGradiente[linha - 1][coluna];
        }
        valorPixel = matrizGradiente[linha - 1][coluna + 1];
    }

    //verifica se está na coluna mais a direita
    else if (coluna == source->width)
    {
        if (matrizGradiente[linha - 1][coluna] < matrizGradiente[linha - 1][coluna - 1])
        {
            valorPixel = matrizGradiente[linha - 1][coluna];
        }
        valorPixel = matrizGradiente[linha - 1][coluna - 1];
    }
    else
    {
        valorPixel = retornaMenorNumero(matrizGradiente[linha - 1][coluna - 1], matrizGradiente[linha - 1][coluna], matrizGradiente[linha - 1][coluna + 1]);
    }

    return valorPixel;
}

int retornaMenorNumero(int primeiroNumero, int segundoNumero, int terceiroNumero)
{
    if (primeiroNumero < segundoNumero && primeiroNumero < terceiroNumero)
    {
        return primeiroNumero;
    }
    else if (segundoNumero < primeiroNumero && segundoNumero < terceiroNumero)
    {
        return segundoNumero;
    }
    return terceiroNumero;
}

void caminhoSeam(int matrizGradiente[target->height][target->width], RGB8 ptr[target->height][target->width], int targetWidth)
{

    int posicao = posicaoPixelMenorValorAcumulado(matrizGradiente, targetWidth);
    removePixelPorPosicao(matrizGradiente, ptr, posicao);
    removeGradientePorPosicao(matrizGradiente, posicao);
}

int posicaoPixelMenorValorAcumulado(int matrizGradiente[target->height][target->width], int targetWidth)
{
    int posicao = 0;
    int menorValor = INT_MAX;
    for (int i = 0; i < targetWidth; i++)
    {
        if (matrizGradiente[target->height - 1][i] < menorValor)
        {
            menorValor = matrizGradiente[target->height - 1][i];
            posicao = i;
        }
    }

    return posicao;
}

void removePixelPorPosicao(int matrizGradiente[target->height][target->width], RGB8 ptr[target->height][target->width], int posicao)
{
    for (int linha = source->height - 1; linha >= 0; linha--)
    {
        //printf("Caminho seam: Linha %d Coluna %d Valor %d \n", linha, posicao, matrizGradiente[linha][posicao]);
        puxaPixel(linha, posicao, ptr);
        //puxaPixel(linha, posicao, ptr2);
        posicao = valorPosicaoPixelAnterior(linha, posicao, matrizGradiente);
    }
}

void removeGradientePorPosicao(int matrizGradiente[target->height][target->width], int posicao)
{
    for (int linha = source->height - 1; linha >= 0; linha--)
    {
        //printf("Caminho seam: Linha %d Coluna %d Valor %d \n", linha, posicao, matrizGradiente[linha][posicao]);
        puxaGradiente(linha, posicao, matrizGradiente);
        //puxaPixel(linha, posicao, ptr2);
        posicao = valorPosicaoPixelAnterior(linha, posicao, matrizGradiente);
    }
}

int valorPosicaoPixelAnterior(int linha, int coluna, int matrizGradiente[target->height][target->width])
{
    int posicao = 0;
    // Verifica se está na coluna mais a esquerda
    if (coluna == 0)
    {
        if (matrizGradiente[linha - 1][coluna] < matrizGradiente[linha - 1][coluna + 1])
        {
            posicao = coluna;
        }
        posicao = coluna + 1;
    }

    //verifica se está na coluna mais a direita
    else if (coluna == source->width)
    {
        if (matrizGradiente[linha - 1][coluna] < matrizGradiente[linha - 1][coluna - 1])
        {
            posicao = coluna;
        }
        posicao = coluna - 1;
    }
    else
    {
        posicao = retornaPosicaoMenorNumero(matrizGradiente[linha - 1][coluna - 1], matrizGradiente[linha - 1][coluna], matrizGradiente[linha - 1][coluna + 1], coluna);
    }

    return posicao;
}

int retornaPosicaoMenorNumero(int primeiroNumero, int segundoNumero, int terceiroNumero, int posicao)
{
    if (primeiroNumero < segundoNumero && primeiroNumero < terceiroNumero)
    {
        return posicao - 1;
    }
    else if (segundoNumero < primeiroNumero && segundoNumero < terceiroNumero)
    {
        return posicao;
    }
    return posicao + 1;
}

void puxaGradiente(int linha, int posicao, int matrizGradiente[target->height][target->width])
{
    for (int i = posicao; i < target->width; i++)
    {
        matrizGradiente[linha][i] = matrizGradiente[linha][i + 1];
    }
}

void puxaPixel(int linha, int posicao, RGB8 ptr[target->height][target->width])
{
    for (int i = posicao; i < target->width; i++)
    {
        ptr[linha][i].r = ptr[linha][i + 1].r;
        ptr[linha][i].g = ptr[linha][i + 1].g;
        ptr[linha][i].b = ptr[linha][i + 1].b;
    }
}





void freemem()
{
    // Libera a memória ocupada pelas 3 imagens
    free(pic[0].img);
    free(pic[1].img);
    free(pic[2].img);
}

/********************************************************************
 * 
 *  VOCÊ NÃO DEVE ALTERAR NADA NO PROGRAMA A PARTIR DESTE PONTO!
 *
 ********************************************************************/
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("seamcarving [origem] [mascara]\n");
        printf("Origem é a imagem original, mascara é a máscara desejada\n");
        exit(1);
    }
    glutInit(&argc, argv);

    // Define do modo de operacao da GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // pic[0] -> imagem original
    // pic[1] -> máscara desejada
    // pic[2] -> resultado do algoritmo

    // Carrega as duas imagens
    load(argv[1], &pic[0]);
    load(argv[2], &pic[1]);

    if (pic[0].width != pic[1].width || pic[0].height != pic[1].height)
    {
        printf("Imagem e máscara com dimensões diferentes!\n");
        exit(1);
    }

    // A largura e altura da janela são calculadas de acordo com a maior
    // dimensão de cada imagem
    width = pic[0].width;
    height = pic[0].height;

    // A largura e altura da imagem de saída são iguais às da imagem original (1)
    pic[2].width = pic[1].width;
    pic[2].height = pic[1].height;

    // Ponteiros para as structs das imagens, para facilitar
    source = &pic[0];
    mask = &pic[1];
    target = &pic[2];

    // Largura desejada inicialmente é a largura da janela
    targetW = target->width;

    // Especifica o tamanho inicial em pixels da janela GLUT
    glutInitWindowSize(width, height);

    // Cria a janela passando como argumento o titulo da mesma
    glutCreateWindow("Seam Carving");

    // Registra a funcao callback de redesenho da janela de visualizacao
    glutDisplayFunc(draw);

    // Registra a funcao callback para tratamento das teclas ASCII
    glutKeyboardFunc(keyboard);

    // Registra a funcao callback para tratamento das setas
    glutSpecialFunc(arrow_keys);

    // Cria texturas em memória a partir dos pixels das imagens
    tex[0] = SOIL_create_OGL_texture((unsigned char *)pic[0].img, pic[0].width, pic[0].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    tex[1] = SOIL_create_OGL_texture((unsigned char *)pic[1].img, pic[1].width, pic[1].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Exibe as dimensões na tela, para conferência
    printf("Origem  : %s %d x %d\n", argv[1], pic[0].width, pic[0].height);
    printf("Máscara : %s %d x %d\n", argv[2], pic[1].width, pic[0].height);
    sel = 0; // pic1

    // Define a janela de visualizacao 2D
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, width, height, 0.0);
    glMatrixMode(GL_MODELVIEW);

    // Aloca memória para a imagem de saída
    pic[2].img = malloc(pic[1].width * pic[1].height * 3); // W x H x 3 bytes (RGB)
    // Pinta a imagem resultante de preto!
    memset(pic[2].img, 0, width * height * 3);

    // Cria textura para a imagem de saída
    tex[2] = SOIL_create_OGL_texture((unsigned char *)pic[2].img, pic[2].width, pic[2].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Entra no loop de eventos, não retorna
    glutMainLoop();
}

// Gerencia eventos de teclado
void keyboard(unsigned char key, int x, int y)
{
    if (key == 27)
    {
        // ESC: libera memória e finaliza
        freemem();
        exit(1);
    }
    if (key >= '1' && key <= '3')
        // 1-3: seleciona a imagem correspondente (origem, máscara e resultado)
        sel = key - '1';
    if (key == 's')
    {
        seamcarve(targetW);
    }
    glutPostRedisplay();
}

void arrow_keys(int a_keys, int x, int y)
{
    switch (a_keys)
    {
    case GLUT_KEY_RIGHT:
        if (targetW <= pic[2].width - 10)
            targetW += 10;
        seamcarve(targetW);
        break;
    case GLUT_KEY_LEFT:
        if (targetW > 10)
            targetW -= 10;
        seamcarve(targetW);
        break;
    default:
        break;
    }
}
// Faz upload da imagem para a textura,
// de forma a exibi-la na tela
void uploadTexture()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 target->width, target->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, target->img);
    glDisable(GL_TEXTURE_2D);
}

// Callback de redesenho da tela
void draw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Preto
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Para outras cores, veja exemplos em /etc/X11/rgb.txt

    glColor3ub(255, 255, 255); // branco

    // Ativa a textura corresponde à imagem desejada
    glBindTexture(GL_TEXTURE_2D, tex[sel]);
    // E desenha um retângulo que ocupa toda a tela
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2f(0, 0);

    glTexCoord2f(1, 0);
    glVertex2f(pic[sel].width, 0);

    glTexCoord2f(1, 1);
    glVertex2f(pic[sel].width, pic[sel].height);

    glTexCoord2f(0, 1);
    glVertex2f(0, pic[sel].height);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Exibe a imagem
    glutSwapBuffers();
}
