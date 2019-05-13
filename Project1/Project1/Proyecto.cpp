/*----------------------------------------------------------------------------------*/
/*---------------------------------  Proyecto Final --------------------------------*/
/*-----------------------------------    2019-2   ----------------------------------*/
/*------------- Computación Gráfica e Interacción Humano Computadora ---------------*/
/*----------------------------------------------------------------------------------*/
/*--------------------------- Balcázar Rodríguez Adan Alí --------------------------*/
/*------------------------ Díaz Gutiérrez Elvira Angela Belem ----------------------*/
/*---------------------------- Visual Studio Versión 2017 --------------------------*/
/*----------------------------------------------------------------------------------*/
/*---------------------------------- Instructivo: ----------------------------------*/
/*
Con las siguientes teclas se puede mover todo el modelo:
	D: Mueve el modelo a la izquierda
	A: Mueve el modelo a la derecha
	W: Acerca el modelo
	S: Aleja el modelo

Con las siguientes teclas se cambia el modo de iluminación ambiental:
	1: Modo Día
	2: Modo Atardecer
	3: Modo Noche

Con la siguiente teclas se activan algunas animaciones:
	P: Montaña Rusa

Con el Mouse se puede mover la cámara desde la cual se observa el modelo
	Al mover el mouse a la derecha, la cámara gira a la derecha
	Al mover el mouse a la izquierda, la cámara gira a la izquierda
	Al mover el mouse hacia arriba, la cámara gira hacia arriba
	Al mover el mouse hacia abajo, la cámara gira hacia abajo
*/

// ------------------ Librerías ------------------

//#define STB_IMAGE_IMPLEMENTATION
#include <glew.h>
#include <glfw3.h>
#include <stb_image.h>
#include <string.h>
#include <iostream>
#include <fstream>

#include "esfera.h"
#include "camera.h"
#include "Model.h"

// Other Libs
#include "SOIL2/SOIL2.h"
using namespace std;

// ------------------ Prototipos de Funciones Básicas ------------------

void resize(GLFWwindow* window, int width, int height);
void my_input(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void myData(void);
void display(Shader, Model);
void getResolution(void);
void animate(void);
void LoadTextures(void);
unsigned int generateTextures(char*, bool);

void LoadKeyframes(string);

// ------------------ Prototipos de Funciones de Renderizado ------------------

void montañaRusa(glm::mat4 model_loc, Shader lightingShader, Shader lampShader, Shader modelShader, Model logo);
void riel(glm::mat4 model_loc, float escala_x, float rotacion, Shader lightingShader);
void carro(glm::mat4 model_loc, Shader lightingShader);
void silla(glm::mat4 model_loc, int lado, int seguridad, Shader lightingShader);

void ruedaDeLaFortuna(glm::mat4 model_loc, Shader lightingShader, Shader lampShader);
void cupula(glm::mat4 model_loc, Shader lightingShader);
void anillo(glm::mat4 model_loc, Shader lightingShader);

Esfera esfera(1.0f); // Objeto para renderizar esferas


// ------------------ Variables y funciones para la cámara y la pantalla ------------------

// Variables para el tamaño de la ventana
int SCR_WIDTH = 3800;
int SCR_HEIGHT = 7600;

GLFWmonitor *monitors;
GLuint VBO, VAO, EBO;

// Variables para cambiar el color del fondo
GLclampf bgColorRed = 1.0;  //Color de fondo (Rojo)
GLclampf bgColorGreen = 1.0;  //Color de fondo (Verde)
GLclampf bgColorBlue = 1.0;  //Color de fondo (Azul)

// Cámara
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
double	lastX = 0.0f,
		lastY = 0.0f;
bool firstMouse = true;

// Timing
double	deltaTime = 0.0f,
		lastFrame = 0.0f;

void getResolution()
{
	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	SCR_WIDTH = mode->width;
	SCR_HEIGHT = (mode->height) - 80;

	lastX = SCR_WIDTH / 2.0f;
	lastY = SCR_HEIGHT / 2.0f;
}


// ------------------ Variables para la Animación ------------------

// -------- Montaña Rusa --------

float giroSilla = 0.0f;    //Ángulo de giro de las sillas de la montaña rusa
int giroSillaInversa = 0;  //Sentido de giro de las sillas de la montaña rusa

// Variables para los Keyframes
float posCarroX = 0.0f;
float posCarroY = 0.0f;
float posCarroZ = 0.0f;

float rotCarro = 0.0f;


// -------- Rueda de la Fortuna --------

float giroRueda = 0.0f;    //Ángulo de giro de la rueda de la fortuna


// --------> Variables y funciones para Animación por Keyframes 

#define MAX_FRAMES 500   //Número máximo de keyframes
int i_max_steps = 7;    // Número de fotogramas entre keyframes
int i_curr_steps = 0;    // Contador para recorrer cada fotograma entre keyframes

bool play = false;       //Variable para dar inicio a la animación
int playIndex = 0;

int F1Presionado = 0;     //Variable para evitar flickering en el botón de guardado (Tecla L)
int PPresionado = 0;     //Variable para evitar flickering en el botón de play (Tecla P)

typedef struct _frame    //Por cada variable de control se debe crear su variable auxiliar de incremento
{
	//Variables para GUARDAR Key Frames

	float posCarroX;      //Posición en el eje X del carro de la montaña rusa
	float posCarroY;      //Posición en el eje Y del carro de la montaña rusa
    float posCarroZ;	  //Posición en el eje Z del carro de la montaña rusa (No se usa)

	float posCarroXInc;   //Variable de incremento para posCarroX
	float posCarroYInc;   //Variable de incremento para posCarroY
    float posCarroZInc;   //Variable de incremento para posCarroZ (No se usa)

	float rotCarro;       //Rotación del carro de la montaña rusa
	float rotCarroInc;    //Variable de incremento de rotCarro

}FRAME;

FRAME KeyFrame[MAX_FRAMES];   //Se crea el objeto del tipo keyframe
int FrameIndex = 0;			  // Contador para ir recorriendo cada keyframe

void saveFrame(string archivo)
{
	printf("FrameIndex = %d\n", FrameIndex);

	// ---- Se guardan los datos del keyframe ----

	KeyFrame[FrameIndex].posCarroX = posCarroX;
	KeyFrame[FrameIndex].posCarroY = posCarroY;
    KeyFrame[FrameIndex].posCarroZ = posCarroZ;
	KeyFrame[FrameIndex].rotCarro  = rotCarro;

	FrameIndex++;   // Aumenta la cuenta global de KayFrames

	// ---- Se guarda el KeyFrame en el archivo ----

	ofstream file(archivo, ios::app);    // Se abre el archivo
	file << posCarroX << 'f' << ' ' << posCarroY << 'f' << ' ' << posCarroZ << 'f' << ' ' << rotCarro << 'f' << endl;  // Se escriben los datos
	file.close();  // Se cierra el archivo
}

void resetElements(void)
{
	posCarroX = KeyFrame[0].posCarroX;
	posCarroY = KeyFrame[0].posCarroY;
    posCarroZ = KeyFrame[0].posCarroZ;

	rotCarro = KeyFrame[0].rotCarro;
}

void interpolation(void)
{
	KeyFrame[playIndex].posCarroXInc = (KeyFrame[playIndex + 1].posCarroX - KeyFrame[playIndex].posCarroX) / i_max_steps;
	KeyFrame[playIndex].posCarroYInc = (KeyFrame[playIndex + 1].posCarroY - KeyFrame[playIndex].posCarroY) / i_max_steps;
    KeyFrame[playIndex].posCarroZInc = (KeyFrame[playIndex + 1].posCarroZ - KeyFrame[playIndex].posCarroZ) / i_max_steps;

	KeyFrame[playIndex].rotCarroInc = (KeyFrame[playIndex + 1].rotCarro - KeyFrame[playIndex].rotCarro) / i_max_steps;
}

void LoadKeyFrames(string archivo) 
{
	ifstream file(archivo);  // Se abre el archivo

	if (!file.is_open())     // Si ocurre un error, se notifica
		std::cout << "No se encontro el archivo: " << archivo << '\n';

	// Se limpian todos los KeyFrames
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		KeyFrame[i].posCarroX = 0;
		KeyFrame[i].posCarroY = 0;
		KeyFrame[i].posCarroZ = 0;

		KeyFrame[i].posCarroXInc = 0;
		KeyFrame[i].posCarroYInc = 0;
		KeyFrame[i].posCarroZInc = 0;

		KeyFrame[i].rotCarro = 0;
		KeyFrame[i].rotCarroInc = 0;
	}

	//Variable para ir llenando los datos de cada KeyFrame
	int frameActual = 0; 

	//Variables para leer los datos de cada línea del archivo
	string posX;         
	string posY;
	string posZ;
	string rotacion;

	while (file.good()) 
	{
		getline(file, posX, ' ');       //Se lee cada dato
		getline(file, posY, ' ');
		getline(file, posZ, ' ');
		getline(file, rotacion, '\n');

		if (file.good())
		{
			KeyFrame[frameActual].posCarroX = stof(posX);      //Cast de string a float
			KeyFrame[frameActual].posCarroY = stof(posY);
			KeyFrame[frameActual].posCarroZ = stof(posZ);
			KeyFrame[frameActual].rotCarro = stof(rotacion);

			frameActual++;  //Aumenta el contador

			FrameIndex++;   //Aumenta la cuenta global de KayFrames
		}
	}

	file.close();  //Se cierra el archivo
}


// ------------------ Variables para la iluminación ------------------

// Coordenadas de Iluminación                    -- Luz tipo Posicional por Defecto --
glm::vec3 lightPosition(20.0f, 70.0f, 35.0f);    //Posición de la Luz
glm::vec3 lightDirection(0.0f, 0.0f, -3.0f);     //Dirección de la Luz

// Para controlar la intensidad de la luz
float opacidad = 32.0f;   //Variable de control

int CeroPresionado = 0;   //Variables para corroborar el estado de las teclas
int NuevePresionado = 0;

// Para controlar el color de la luz
float R = 1.0f;        //Variables de control
float G = 1.0f;
float B = 1.0f;

int RPresionado = 0;   //Variables para corroborar el estado de las teclas
int GPresionado = 0;
int BPresionado = 0;

//Para controlar la fuente de iluminación --> Posicional por defecto
char Light_VertxShader[100] = "shaders/shader_texture_light_pos.vs";
char Light_FragShader[100] = "shaders/shader_texture_light_pos.fs";

//La fuente de luz se encuentra en la ubicación del sol o foco
glm::vec3 Light_Position = lightPosition;

int modoNoche = 0;      //Variable para indicar cuando es de noche
int modoAtardecer = 0;  //Variable para indicar cuando es de tarde
int modoDia = 1;        //Variable para indicar cuando es de día

// --- Estas variables son para cuando se usa luz de tipo reflector (Linterna) ---
//  -> Esta define el tamaño del círculo de luz. Aumentar su valor para que el círculo sea más grande
float Light_CutOff = glm::cos(glm::radians(12.5f));
//  -> Esta define la dirección de la luz en base a la posición de la cámara
glm::vec3 Light_Direction = camera.Front;

int Luz_Reflector = 0;  //Por defecto, la luz es de tipo Posicional, no de Reflector


// ------------------ Variables y funciones para Texturas ------------------

// --- Difusas ---

// Materiales
unsigned int t_metal;
unsigned int t_metal_amarillo;
unsigned int t_piedra;
unsigned int t_terracota;

// Colores
unsigned int t_negro;
unsigned int t_cafe;
unsigned int t_rojo;
unsigned int t_naranja;
unsigned int t_amarillo;
unsigned int t_verde;
unsigned int t_azul;
unsigned int t_azul_rey;
unsigned int t_morado;
unsigned int t_rosa;
unsigned int t_blanco;

// --- Especulares ---

// Materiales
unsigned int t_metal_brillo;
unsigned int t_metal_amarillo_brillo;
unsigned int t_piedra_brillo;
unsigned int t_terracota_brillo;

// Colores
unsigned int t_negro_brillo;
unsigned int t_cafe_brillo;
unsigned int t_rojo_brillo;
unsigned int t_naranja_brillo;
unsigned int t_amarillo_brillo;
unsigned int t_verde_brillo;
unsigned int t_azul_brillo;
unsigned int t_azul_rey_brillo;
unsigned int t_morado_brillo;
unsigned int t_rosa_brillo;
unsigned int t_blanco_brillo;

unsigned int generateTextures(const char* filename, bool alfa)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	
	unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
	if (data)
	{
		if (alfa)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		return textureID;
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
		return 100;
	}

	stbi_image_free(data);
}

//Valores a colocar si la extensión de la textura es:
// .jpg ----> 0
// .png ----> 1
void LoadTextures()
{
	// Texturas de Materiales
	t_metal = generateTextures("Texturas/Metal.png", 1);
	t_metal_brillo = generateTextures("Texturas/Metal_Especular.png", 1);

	t_metal_amarillo = generateTextures("Texturas/Metal_Amarillo.png", 1);
	t_metal_amarillo_brillo = generateTextures("Texturas/Metal_Amarillo_Especular.png", 1);

	t_piedra = generateTextures("Texturas/Piedra.png", 1);
	t_piedra_brillo = generateTextures("Texturas/Piedra_Especular.png", 1);

	t_terracota = generateTextures("Texturas/Terracota.png", 1);
	t_terracota_brillo = generateTextures("Texturas/Terracota_Especular.png", 1);

	// Texturas de Colores
	t_negro = generateTextures("Texturas/Negro.png", 1);
	t_negro_brillo = generateTextures("Texturas/Negro_Especular.png", 1);

	t_cafe = generateTextures("Texturas/Cafe.png", 1);
	t_cafe_brillo = generateTextures("Texturas/Cafe_Especular.png", 1);

	t_rojo = generateTextures("Texturas/Rojo.png", 1);
	t_rojo_brillo = generateTextures("Texturas/Rojo_Especular.png", 1);

	t_naranja = generateTextures("Texturas/Naranja.png", 1);
	t_naranja_brillo = generateTextures("Texturas/Naranja_Especular.png", 1);

	t_amarillo = generateTextures("Texturas/Amarillo.png", 1);
	t_amarillo_brillo = generateTextures("Texturas/Amarillo_Especular.png", 1);

	t_verde = generateTextures("Texturas/Verde.png", 1);
	t_verde_brillo = generateTextures("Texturas/Verde_Especular.png", 1);

	t_azul = generateTextures("Texturas/Azul.png", 1);
	t_azul_brillo = generateTextures("Texturas/Azul_Especular.png", 1);

	t_azul_rey = generateTextures("Texturas/Azul_rey.png", 1);
	t_azul_rey_brillo = generateTextures("Texturas/Azul_rey_Especular.png", 1);

	t_morado = generateTextures("Texturas/Morado.png", 1);
	t_morado_brillo = generateTextures("Texturas/Morado_Especular.png", 1);

	t_rosa = generateTextures("Texturas/Rosa.png", 1);
	t_rosa_brillo = generateTextures("Texturas/Rosa_Especular.png", 1);

	t_blanco = generateTextures("Texturas/Blanco.png", 1);
	t_blanco_brillo = generateTextures("Texturas/Blanco_Especular.png", 1);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, t_metal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, t_metal_brillo);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, t_metal_amarillo);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, t_metal_amarillo_brillo);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, t_piedra);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, t_piedra_brillo);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, t_terracota);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, t_terracota_brillo);


	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, t_negro);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, t_negro_brillo);

	glActiveTexture(GL_TEXTURE11);
	glBindTexture(GL_TEXTURE_2D, t_cafe);
	glActiveTexture(GL_TEXTURE12);
	glBindTexture(GL_TEXTURE_2D, t_cafe_brillo);

	glActiveTexture(GL_TEXTURE13);
	glBindTexture(GL_TEXTURE_2D, t_rojo);
	glActiveTexture(GL_TEXTURE14);
	glBindTexture(GL_TEXTURE_2D, t_rojo_brillo);

	glActiveTexture(GL_TEXTURE15);
	glBindTexture(GL_TEXTURE_2D, t_naranja);
	glActiveTexture(GL_TEXTURE16);
	glBindTexture(GL_TEXTURE_2D, t_naranja_brillo);

	glActiveTexture(GL_TEXTURE17);
	glBindTexture(GL_TEXTURE_2D, t_amarillo);
	glActiveTexture(GL_TEXTURE18);
	glBindTexture(GL_TEXTURE_2D, t_amarillo_brillo);

	glActiveTexture(GL_TEXTURE19);
	glBindTexture(GL_TEXTURE_2D, t_verde);
	glActiveTexture(GL_TEXTURE20);
	glBindTexture(GL_TEXTURE_2D, t_verde_brillo);

	glActiveTexture(GL_TEXTURE21);
	glBindTexture(GL_TEXTURE_2D, t_azul);
	glActiveTexture(GL_TEXTURE22);
	glBindTexture(GL_TEXTURE_2D, t_azul_brillo);

	glActiveTexture(GL_TEXTURE23);
	glBindTexture(GL_TEXTURE_2D, t_azul_rey);
	glActiveTexture(GL_TEXTURE24);
	glBindTexture(GL_TEXTURE_2D, t_azul_rey_brillo);

	glActiveTexture(GL_TEXTURE25);
	glBindTexture(GL_TEXTURE_2D, t_morado);
	glActiveTexture(GL_TEXTURE26);
	glBindTexture(GL_TEXTURE_2D, t_morado_brillo);

	glActiveTexture(GL_TEXTURE27);
	glBindTexture(GL_TEXTURE_2D, t_rosa);
	glActiveTexture(GL_TEXTURE28);
	glBindTexture(GL_TEXTURE_2D, t_rosa_brillo);

	glActiveTexture(GL_TEXTURE29);
	glBindTexture(GL_TEXTURE_2D, t_blanco);
	glActiveTexture(GL_TEXTURE30);
	glBindTexture(GL_TEXTURE_2D, t_blanco_brillo);
}

void myData()
{	
	float vertices[] = {

		// ------------ CUBO ------------
		// Vértices: 0-24

		// Posición           //Normales            // UV's

		//Cara de enfrente
		 0.5f,  0.5f, 0.5f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,    // top right
		 0.5f, -0.5f, 0.5f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f,    // bottom right
		-0.5f, -0.5f, 0.5f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f,    // bottom left
		-0.5f,  0.5f, 0.5f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f,    // top left 

		//Cara de atrás
		-0.5f,  0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f,    // top right
		-0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f,    // bottom right
		 0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,    // bottom left
		 0.5f,  0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f,    // top left 

		//Cara de arriba
		 0.5f,  0.5f, -0.5f,   0.0f,  1.0f, 0.0f,   1.0f, 1.0f,    // top right
		 0.5f,  0.5f,  0.5f,   0.0f,  1.0f, 0.0f,   1.0f, 0.0f,    // bottom right
		-0.5f,  0.5f,  0.5f,   0.0f,  1.0f, 0.0f,   0.0f, 0.0f,    // bottom left
		-0.5f,  0.5f, -0.5f,   0.0f,  1.0f, 0.0f,   0.0f, 1.0f,    // top left 

		//Cara de abajo
		 0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f,    // top right
	     0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,    // bottom right
	    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f,    // bottom left
		-0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,    // top left 

		//Cara de la derecha
		 0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,    1.0f, 1.0f,    // top right
		 0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,    1.0f, 0.0f,    // bottom right
		 0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f,    // bottom left
		 0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f,    // top left 

		 //Cara de la izquierda
		-0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,    1.0f, 1.0f,    // top right
		-0.5f, -0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,    1.0f, 0.0f,    // bottom right
		-0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,    0.0f, 0.0f,    // bottom left
		-0.5f,  0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,    0.0f, 1.0f,    // top left 


		// ------------ Cilindro ------------
		// Vértices: 24-96 (72 vértices)

		// Posición                // Normales            // UV's

		//Cara 1
		0.0f,   0.5f, 0.45f,       0.0f, 0.0f, 1.0f,      1.0f, 1.0f,    // top right
		0.0f,  -0.5f, 0.45f,       0.0f, 0.0f, 1.0f,      1.0f, 0.0f,    // bottom right
	   -0.25f, -0.5f, 0.38f,       0.0f, 0.0f, 1.0f,      0.0f, 0.0f,    // bottom left
	   -0.25f,  0.5f, 0.38f,       0.0f, 0.0f, 1.0f,      0.0f, 1.0f,    // top left 

		//Cara 2
	   -0.25f,  0.5f, 0.38f,      -1.0f, 0.0f, 1.0f,      1.0f, 1.0f,    // top right
	   -0.25f, -0.5f, 0.38f,      -1.0f, 0.0f, 1.0f,      1.0f, 0.0f,    // bottom right
	   -0.38f, -0.5f, 0.25f,      -1.0f, 0.0f, 1.0f,      0.0f, 0.0f,    // bottom left
	   -0.38f,  0.5f, 0.25f,      -1.0f, 0.0f, 1.0f,      0.0f, 1.0f,    // top left 

		//Cara 3
	   -0.38f,  0.5f,  0.25f,     -1.0f, 0.0f, 0.0f,      1.0f, 1.0f,    // top right
	   -0.38f, -0.5f,  0.25f,     -1.0f, 0.0f, 0.0f,      1.0f, 0.0f,    // bottom right
	   -0.45f, -0.5f,  0.0f,      -1.0f, 0.0f, 0.0f,      0.0f, 0.0f,    // bottom left
	   -0.45f,  0.5f,  0.0f,      -1.0f, 0.0f, 0.0f,      0.0f, 1.0f,    // top left 

		//Cara 4
	   -0.45f,  0.5f,  0.0f,      -1.0f, 0.0f, 0.0f,      1.0f, 1.0f,    // top right
	   -0.45f, -0.5f,  0.0f,      -1.0f, 0.0f, 0.0f,      1.0f, 0.0f,    // bottom right
	   -0.38f, -0.5f, -0.25f,     -1.0f, 0.0f, 0.0f,      0.0f, 0.0f,    // bottom left
	   -0.38f,  0.5f, -0.25f,     -1.0f, 0.0f, 0.0f,      0.0f, 1.0f,    // top left 

		//Cara 5
	   -0.38f,  0.5f, -0.25f,     -1.0f, 0.0f, -1.0f,     1.0f, 1.0f,    // top right
	   -0.38f, -0.5f, -0.25f,     -1.0f, 0.0f, -1.0f,     1.0f, 0.0f,    // bottom right
	   -0.25f, -0.5f, -0.38f,     -1.0f, 0.0f, -1.0f,     0.0f, 0.0f,    // bottom left
	   -0.25f,  0.5f, -0.38f,     -1.0f, 0.0f, -1.0f,     0.0f, 1.0f,    // top left

		//Cara 6
	   -0.25f,  0.5f, -0.38f,      0.0f, 0.0f, -1.0f,     1.0f, 1.0f,    // top right
	   -0.25f, -0.5f, -0.38f,      0.0f, 0.0f, -1.0f,     1.0f, 0.0f,    // bottom right
		0.0f,  -0.5f, -0.45f,      0.0f, 0.0f, -1.0f,     0.0f, 0.0f,    // bottom left
		0.0f,   0.5f, -0.45f,      0.0f, 0.0f, -1.0f,     0.0f, 1.0f,    // top left

		//Cara 7
		0.0f,   0.5f, -0.45f,      0.0f, 0.0f, -1.0f,     1.0f, 1.0f,    // top right
		0.0f,  -0.5f, -0.45f,      0.0f, 0.0f, -1.0f,     1.0f, 0.0f,    // bottom right
		0.25f, -0.5f, -0.38f,      0.0f, 0.0f, -1.0f,     0.0f, 0.0f,    // bottom left
		0.25f,  0.5f, -0.38f,      0.0f, 0.0f, -1.0f,     0.0f, 1.0f,    // top left

		//Cara 8
		0.25f,  0.5f, -0.38f,      1.0f, 0.0f, -1.0f,     1.0f, 1.0f,    // top right
		0.25f, -0.5f, -0.38f,      1.0f, 0.0f, -1.0f,     1.0f, 0.0f ,   // bottom right
		0.38f, -0.5f, -0.25f,      1.0f, 0.0f, -1.0f,     0.0f, 0.0f,    // bottom left
		0.38f,  0.5f, -0.25f,      1.0f, 0.0f, -1.0f,     0.0f, 1.0f,    // top left

		//Cara 9
		0.38f,  0.5f, -0.25,       1.0f, 0.0f, 0.0f,      1.0f, 1.0f,    // top right
		0.38f, -0.5f, -0.25,       1.0f, 0.0f, 0.0f,      1.0f, 0.0f ,   // bottom right
		0.45f, -0.5f, 0.0f,        1.0f, 0.0f, 0.0f,      0.0f, 0.0f,    // bottom left
		0.45f,  0.5f, 0.0f,        1.0f, 0.0f, 0.0f,      0.0f, 1.0f,    // top left

		//Cara 10
		0.45f,  0.5f, 0.0f,        1.0f, 0.0f, 0.0f,      1.0f, 1.0f,    // top right
		0.45f, -0.5f, 0.0f,        1.0f, 0.0f, 0.0f,      1.0f, 0.0f ,   // bottom right
		0.38f, -0.5f, 0.25f,       1.0f, 0.0f, 0.0f,      0.0f, 0.0f,    // bottom left
		0.38f,  0.5f, 0.25f,       1.0f, 0.0f, 0.0f,      0.0f, 1.0f,    // top left

		//Cara 11
		0.38f,  0.5f, 0.25f,       1.0f, 0.0f, 1.0f,      1.0f, 1.0f,    // top right
		0.38f, -0.5f, 0.25f,       1.0f, 0.0f, 1.0f,      1.0f, 0.0f ,   // bottom right
		0.25f, -0.5f, 0.38f,       1.0f, 0.0f, 1.0f,      0.0f, 0.0f,    // bottom left
		0.25f,  0.5f, 0.38f,       1.0f, 0.0f, 1.0f,      0.0f, 1.0f,    // top left

		//Cara 12
		0.25f,  0.5f, 0.38f,       0.0f, 0.0f, 1.0f,      1.0f, 1.0f,    // top right
		0.25f, -0.5f, 0.38f,       0.0f, 0.0f, 1.0f,      1.0f, 0.0f ,   // bottom right
		0.0f,  -0.5f, 0.45f,       0.0f, 0.0f, 1.0f,      0.0f, 0.0f,    // bottom left
		0.0f,   0.5f, 0.45f,       0.0f, 0.0f, 1.0f,      0.0f, 1.0f,    // top left

		//Cara de arriba
		 0.0f,   0.5f,  0.45f,     0.0f, 1.0f, 0.0f,      0.5f,  0.0f,   // 1
		-0.25f,  0.5f,  0.38f,     0.0f, 1.0f, 0.0f,      0.25f, 0.12f,  // 2
		-0.38f,  0.5f,  0.25f,     0.0f, 1.0f, 0.0f,      0.12f, 0.25f,  // 3
		-0.45f,  0.5f,  0.0f,      0.0f, 1.0f, 0.0f,      0.0f,  0.5f,   // 4 
		-0.38f,  0.5f, -0.25f,     0.0f, 1.0f, 0.0f,      0.12f, 0.75f,  // 5
		-0.25f,  0.5f, -0.38f,     0.0f, 1.0f, 0.0f,      0.25f, 0.88f,  // 6
		 0.0f,   0.5f, -0.45f,     0.0f, 1.0f, 0.0f,      0.5f,  1.0f,   // 7 
		 0.25f,  0.5f, -0.38f,     0.0f, 1.0f, 0.0f,      0.75f, 0.88f,  // 8 
		 0.38f,  0.5f, -0.25f,     0.0f, 1.0f, 0.0f,      0.88f, 0.75f,  // 9 
		 0.45f,  0.5f,  0.0f,      0.0f, 1.0f, 0.0f,      1.0f,  0.5f,   // 10 
		 0.38f,  0.5f,  0.25f,     0.0f, 1.0f, 0.0f,      0.88f, 0.25f,  // 11
		 0.25f,  0.5f,  0.38f,     0.0f, 1.0f, 0.0f,      0.75f, 0.12f,  // 12

		//Cara de abajo
		 0.0f,   -0.5f,  0.45f,    0.0f, -1.0f, 0.0f,     0.5f,  0.0f,   // 1
		-0.25f,  -0.5f,  0.38f,    0.0f, -1.0f, 0.0f,     0.25f, 0.12f,  // 2
		-0.38f,  -0.5f,  0.25f,    0.0f, -1.0f, 0.0f,     0.12f, 0.25f,  // 3
		-0.45f,  -0.5f,  0.0f,     0.0f, -1.0f, 0.0f,     0.0f,  0.5f,   // 4 
		-0.38f,  -0.5f, -0.25f,    0.0f, -1.0f, 0.0f,     0.12f, 0.75f,  // 5
		-0.25f,  -0.5f, -0.38f,    0.0f, -1.0f, 0.0f,     0.25f, 0.88f,  // 6
		 0.0f,   -0.5f, -0.45f,    0.0f, -1.0f, 0.0f,     0.5f,  1.0f,   // 7 
		 0.25f,  -0.5f, -0.38f,    0.0f, -1.0f, 0.0f,     0.75f, 0.88f,  // 8 
		 0.38f,  -0.5f, -0.25f,    0.0f, -1.0f, 0.0f,     0.88f, 0.75f,  // 9 
		 0.45f,  -0.5f,  0.0f,     0.0f, -1.0f, 0.0f,     1.0f,  0.5f,   // 10 
		 0.38f,  -0.5f,  0.25f,    0.0f, -1.0f, 0.0f,     0.88f, 0.25f,  // 11
		 0.25f,  -0.5f,  0.38f,    0.0f, -1.0f, 0.0f,     0.75f, 0.12f,  // 12


		// ------------ SILLA ------------
		// Vértices: 96-156 (60 vértices) Tubo: 96-136 Costado Der: 136-146  Costado Izq: 146-156

		// Posición             // Normales           // UV's

		//Cara 1
		 0.5f,  0.25f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f,     // top right
		 0.5f, -0.3f,  -0.4f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f,     // bottom right
		-0.5f, -0.3f,  -0.4f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,     // bottom left
		-0.5f,  0.25f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f,     // top left 

		//Cara 2
		 0.5f,  0.45f, -0.4f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f,     // top right
		 0.5f,  0.25f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f,     // bottom right
		-0.5f,  0.25f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,     // bottom left
		-0.5f,  0.45f, -0.4f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f,     // top left 

		//Cara 3
		 0.5f,  0.45f, -0.35f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f,      // top right
	     0.5f,  0.45f, -0.4f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,      // bottom right
	    -0.5f,  0.45f, -0.4f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,      // bottom left
		-0.5f,  0.45f, -0.35f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,      // top left 

		//Cara 4
		 0.5f, -0.2f,  -0.3f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,      // top right
	     0.5f,  0.45f, -0.35f,  0.0f, 0.0f, 1.0f,   1.0f, 0.0f,      // bottom right
	    -0.5f,  0.45f, -0.35f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f,      // bottom left
		-0.5f, -0.2f,  -0.3f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,      // top left 

		//Cara 5
		 0.5f, -0.25f, -0.1f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,      // top right
	     0.5f, -0.2f,  -0.3f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,      // bottom right
	    -0.5f, -0.2f,  -0.3f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,      // bottom left
		-0.5f, -0.25f, -0.1f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,      // top left 

		//Cara 6
		 0.5f, -0.2f,   0.3f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,      // top right
	     0.5f, -0.25f, -0.1f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,      // bottom right
	    -0.5f, -0.25f, -0.1f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,      // bottom left
		-0.5f, -0.2f,   0.3f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,      // top left 

		//Cara 7
		 0.5f, -0.25f,  0.3f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,      // top right
	     0.5f, -0.2f,   0.3f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,      // bottom right
	    -0.5f, -0.2f,   0.3f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,      // bottom left
		-0.5f, -0.25f,  0.3f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,      // top left 

		//Cara 8
		 0.5f, -0.4f,   0.2f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,      // top right
	     0.5f, -0.25f,  0.3f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,      // bottom right
	    -0.5f, -0.25f,  0.3f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,      // bottom left
		-0.5f, -0.4f,   0.2f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,      // top left 

		//Cara 9
		 0.5f, -0.5f,  -0.2f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f,     // top right
	     0.5f, -0.4f,   0.2f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,     // bottom right
	    -0.5f, -0.4f,   0.2f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f,     // bottom left
		-0.5f, -0.5f,  -0.2f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,     // top left 

		//Cara 10
		 0.5f, -0.3f, -0.4f,    0.0f, -1.0f, 0.0f,    1.0f, 1.0f,    // top right
		 0.5f, -0.5f, -0.2f,    0.0f, -1.0f, 0.0f,    1.0f, 0.0f,    // bottom right
		-0.5f, -0.5f, -0.2f,    0.0f, -1.0f, 0.0f,    0.0f, 0.0f,    // bottom left
		-0.5f, -0.3f, -0.4f,    0.0f, -1.0f, 0.0f,    0.0f, 1.0f,    // top left 

		//Cara de la derecha
		 0.5f, -0.3f,  -0.4f,   1.0f, 0.0f, 0.0f,    0.9f,  0.1f,    // 1
		 0.5f,  0.25f, -0.5f,   1.0f, 0.0f, 0.0f,    1.0f,  0.75f,   // 2
		 0.5f,  0.45f, -0.4f,   1.0f, 0.0f, 0.0f,    0.9f,  0.95f,   // 3
		 0.5f,  0.45f, -0.35f,  1.0f, 0.0f, 0.0f,    0.85f, 0.95f,   // 4
		 0.5f, -0.2f,  -0.3f,   1.0f, 0.0f, 0.0f,    0.8f,  0.3f,    // 5
		 0.5f, -0.25f, -0.1f,   1.0f, 0.0f, 0.0f,    0.6f,  0.25f,   // 6
		 0.5f, -0.2f,   0.3f,   1.0f, 0.0f, 0.0f,    0.15f, 0.3f,    // 7
		 0.5f, -0.25f,  0.3f,   1.0f, 0.0f, 0.0f,    0.15f, 0.25f,   // 8
		 0.5f, -0.4f,   0.2f,   1.0f, 0.0f, 0.0f,    0.3f,  0.1f,    // 9
		 0.5f, -0.5f,  -0.2f,   1.0f, 0.0f, 0.0f,    0.7f,  0.0f,    // 10

		 //Cara de la izquierda
		-0.5f, -0.3f,  -0.4f,  -1.0f, 0.0f, 0.0f,    0.9f,  0.1f,    // 1
		-0.5f,  0.25f, -0.5f,  -1.0f, 0.0f, 0.0f,    1.0f,  0.75f,   // 2
		-0.5f,  0.45f, -0.4f,  -1.0f, 0.0f, 0.0f,    0.9f,  0.95f,   // 3
		-0.5f,  0.45f, -0.35f, -1.0f, 0.0f, 0.0f,    0.85f, 0.95f,   // 4
		-0.5f, -0.2f,  -0.3f,  -1.0f, 0.0f, 0.0f,    0.8f,  0.3f,    // 5
		-0.5f, -0.25f, -0.1f,  -1.0f, 0.0f, 0.0f,    0.6f,  0.25f,   // 6
		-0.5f, -0.2f,   0.3f,  -1.0f, 0.0f, 0.0f,    0.15f, 0.3f,    // 7
		-0.5f, -0.25f,  0.3f,  -1.0f, 0.0f, 0.0f,    0.15f, 0.25f,   // 8
		-0.5f, -0.4f,   0.2f,  -1.0f, 0.0f, 0.0f,    0.3f,  0.1f,    // 9
		-0.5f, -0.5f,  -0.2f,  -1.0f, 0.0f, 0.0f,    0.7f,  0.0f,    // 10


		// ------------ GUARDA ------------
		// Vértices: 156-186 (30 vértices) Tubo: 156-176   Costado Der: 176-181  Costado Izq: 181-186

		// Posición              // Normales          // UV's

		//Cara 1
		 0.5f,  0.25f, -0.45f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f,    // top right
		 0.5f, -0.3f,  -0.35f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,    // bottom right
		-0.5f, -0.3f,  -0.35f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f,    // bottom left
		-0.5f,  0.25f, -0.45f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,    // top left 

		//Cara 2
		 0.5f,  0.45f, -0.15f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f,    // top right
		 0.5f,  0.25f, -0.45f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f,    // bottom right
		-0.5f,  0.25f, -0.45f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,    // bottom left
		-0.5f,  0.45f, -0.15f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f,    // top left 

		//Cara 3
		 0.5f, -0.35f,  0.5f,    0.0f, 1.0f, -1.0f,   1.0f, 1.0f,    // top right
	     0.5f,  0.45f, -0.15f,   0.0f, 1.0f, -1.0f,   1.0f, 0.0f,    // bottom right
	    -0.5f,  0.45f, -0.15f,   0.0f, 1.0f, -1.0f,   0.0f, 0.0f,    // bottom left
		-0.5f, -0.35f,  0.5f,    0.0f, 1.0f, -1.0f,   0.0f, 1.0f,    // top left 

		//Cara 4
		 0.5f, -0.5f,   0.05f,   0.0f, 1.0f, 1.0f,    1.0f, 1.0f,    // top right
	     0.5f, -0.35f,  0.5f,    0.0f, 1.0f, 1.0f,    1.0f, 0.0f,    // bottom right
	    -0.5f, -0.35f,  0.5f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f,    // bottom left
		-0.5f, -0.5f,   0.05f,   0.0f, 1.0f, 1.0f,    0.0f, 1.0f,    // top left 

		//Cara 5
		 0.5f, -0.3f,  -0.35f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f,    // top right
	     0.5f, -0.5f,   0.05f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,    // bottom right
	    -0.5f, -0.5f,   0.05f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f,    // bottom left
		-0.5f, -0.3f,  -0.35f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,    // top left 

		//Cara de la derecha
		 0.5f, -0.3f,  -0.35f,   1.0f, 0.0f, 0.0f,    0.85f, 0.2f,    // 1
		 0.5f,  0.25f, -0.45f,   1.0f, 0.0f, 0.0f,    0.95f, 0.75f,   // 2
		 0.5f,  0.45f, -0.15f,   1.0f, 0.0f, 0.0f,    0.65f, 0.95f,   // 3
		 0.5f, -0.35f,  0.5f,    1.0f, 0.0f, 0.0f,    0.05f, 0.15f,   // 4
		 0.5f, -0.5f,   0.05f,   1.0f, 0.0f, 0.0f,    0.45f,  0.0f,   // 5

		 //Cara de la izquierda
		-0.5f, -0.3f,  -0.35f,  -1.0f, 0.0f, 0.0f,    0.85f, 0.2f,    // 1
		-0.5f,  0.25f, -0.45f,  -1.0f, 0.0f, 0.0f,    0.95f, 0.75f,   // 2
		-0.5f,  0.45f, -0.15f,  -1.0f, 0.0f, 0.0f,    0.65f, 0.95f,   // 3
		-0.5f, -0.35f,  0.5f,   -1.0f, 0.0f, 0.0f,    0.05f, 0.15f,   // 4
		-0.5f, -0.5f,   0.05f,  -1.0f, 0.0f, 0.0f,    0.45f,  0.0f,   // 5
	};

	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
}

void animate(void)
{
	// --------------- Animación de la Montaña Rusa ---------------

	if (play)
	{
		// Animación de las sillas 
		if (giroSillaInversa)  //Giro en sentido horario
		{
			giroSilla -= 7.5f;

			if (giroSilla <= 0.0f)
				giroSillaInversa = 0;
		}
		else  //Giro en sentido antihorario
		{
			giroSilla += 7.5f;

			if (giroSilla >= 1080.0f)  //3 vueltas: 360 * 3 = 1080
				giroSillaInversa = 1;
		}

		// Animación del carro
		if (i_curr_steps >= i_max_steps) //¿Fin de la animación entre cuadros (frames)?
		{
			playIndex++;
			if (playIndex > FrameIndex - 2)	//¿Fin de toda la animación?
			{
				printf("Termina la animacion\n");
				playIndex = 0;
				play = false;
			}
			else //Next frame interpolations
			{
				i_curr_steps = 0; //Se resetea el contador
				interpolation();  //Interpolación
			}
		}
		else
		{
			// Se dibuja la animación
			posCarroX += KeyFrame[playIndex].posCarroXInc;
			posCarroY += KeyFrame[playIndex].posCarroYInc;
		    posCarroZ += KeyFrame[playIndex].posCarroZInc;

			rotCarro += KeyFrame[playIndex].rotCarroInc;

			i_curr_steps++;
		}

		printf("playIndex = %d\n", playIndex);

	}

	// --------------- Animación de la Rueda de la Fortuna ---------------

	giroRueda += 1.0f;  //Giro en sentido antihorario
}

void display(Shader modelShader, Model batarang)
{
	//Matriz de modelo, vista t proyección
	glm::mat4 model = glm::mat4(1.0f);		//Initialize Matrix, Use this matrix for individual models
	glm::mat4 model_temp = glm::mat4(1.0f); //Maztriz auxiliar para la ubicación de cada elemento del modelo
	glm::mat4 model_loc = glm::mat4(1.0f);  //Matriz axuliar para la ubicación absoluta de todo el modelo

	glm::mat4 view = glm::mat4(1.0f);		//Use this matrix for ALL models
	glm::mat4 projection = glm::mat4(1.0f);	//This matrix is for Projection

	//Use "projection" to include Camera
	projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
	view = camera.GetViewMatrix();

	//Se habilita el shader de modelos
	modelShader.use();

	// Se pasan las matrices al shader de modelos
	modelShader.setMat4("model", model);
	modelShader.setMat4("view", view);
	modelShader.setMat4("projection", projection);

	//Se abre el shader de focos de luz y se habilita
	Shader lampShader("shaders/shader_lamp.vs", "shaders/shader_lamp.fs");  
	lampShader.use();

	// Se pasan las matrices al shader de focos de luz
	lampShader.setMat4("model", model);
	lampShader.setMat4("view", view);
	lampShader.setMat4("projection", projection);


	//Shader projectionShader("shaders/shader_light.vs", "shaders/shader_light.fs");
	//Shader projectionShader("shaders/shader_texture_color.vs", "shaders/shader_texture_color.fs");

	// ------ Aquí se selecciona el tipo de luz a utilizar, descomentar la línea que se va a ocupar -----

	//Shader lightingShader("shaders/shader_texture_light_dir.vs", "shaders/shader_texture_light_dir.fs");    //Directional
	//Shader lightingShader("shaders/shader_texture_light_pos.vs", "shaders/shader_texture_light_pos.fs");    //Positional
	//Shader lightingShader("shaders/shader_texture_light_spot.vs", "shaders/shader_texture_light_spot.fs");  //Spotlight

	Shader lightingShader(Light_VertxShader, Light_FragShader);             //Light_VertxShader y Light_FragShader son variables que pueden tener cualquiera de las rutas anteriores

	//Se habilita el Shader de iluminación
	lightingShader.use();

	// Se pasan las matrices al shader de iluminación
	lightingShader.setMat4("model", model);
	lightingShader.setMat4("view", view);
	lightingShader.setMat4("projection", projection);

	// ------ Aquí se selecciona el tipo de luz a utilizar, descomentar la línea que se va a ocupar -----

	//	Directional - Luz direccional, se manda el sentido en el que apunta la fuente de luz
	//lightingShader.setVec3("light.direction", lightDirection);

	//	Positional - Luz posicional, se manda la ubicación de la fuente de luz
	//lightingShader.setVec3("light.position", lightPosition);

	//	Spotlight - Luz de reflector, la luz se encuentra en la cámara
	//lightingShader.setVec3("light.position", camera.Position);
	//lightingShader.setVec3("light.direction", camera.Front);
	//lightingShader.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));

	lightingShader.setVec3("light.position", Light_Position);
	lightingShader.setVec3("light.direction", Light_Direction);
	lightingShader.setFloat("light.cutOff", Light_CutOff);

	lightingShader.setVec3("viewPos", camera.Position);

	// Propiedades de la luz --- Luz Variable
	lightingShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
	lightingShader.setVec3("light.diffuse", R, G, B);
	lightingShader.setVec3("light.specular", R, G, B);

	//PAra la Luz Posicional y de Reflector
	lightingShader.setFloat("light.constant", 1.0f);
	lightingShader.setFloat("light.linear", 0.09f);
	//lightingShader.setFloat("light.quadratic", 0.032f);
	lightingShader.setFloat("light.quadratic", 0.032f);

	// Brillo del material
	lightingShader.setFloat("material_shininess", opacidad);

	// Mientras la Luz sea de tipo reflector, ésta se estará actualizando constantemente en base a la posición de la cámara
	if (Luz_Reflector == 1)
	{
		Light_Position = camera.Position;
		Light_Direction = camera.Front;
		Light_CutOff = glm::cos(glm::radians(12.5f));
	}

	// ------------------------- Zona de Dibujo -------------------------

	// ------------ Montaña Rusa ------------
	glBindVertexArray(VAO);
	model_loc = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, .0f));    //Rotación de la montaña rusa
	model_loc = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, 0.0f));                          //Ubicación de la montaña rusa
	model_loc = glm::scale(model_loc, glm::vec3(1.0f, 1.0f, 1.0f));                              //Escala de toda la montaña
	montañaRusa(model_loc, lightingShader, lampShader, modelShader, batarang);                   //Se dibuja la montaña

	
	// ------------ Rueda de la Fortuna ------------
	glBindVertexArray(VAO);
	lightingShader.use();
	model_loc = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));    //Rotación de la rueda de la fortuna
	model_loc = glm::translate(model_loc, glm::vec3(50.0f, 10.5f, -1.5f));                        //Ubicación de la rueda de la fortuna
	model_loc = glm::scale(model_loc, glm::vec3(1.0f, 1.0f, 1.0f));                               //Escala de toda la rueda
	ruedaDeLaFortuna(model_loc, lightingShader, lampShader);                                      //Se dibuja la rueda

	// ------------- Sol / Luna ---------------
	lampShader.use();

	if (modoDia)
		lampShader.setVec3("color", 1.00000f, 1.00000f, 0.20000f);  //Amarillo (Sol de medio día)
	if (modoAtardecer)
		lampShader.setVec3("color", 1.00000f, 0.53333f, 0.06667f);  //Naranja (Sol de la tarde)
	if (modoNoche)
		lampShader.setVec3("color", 1.00000f, 1.00000f, 0.79216f);  //Blanco (Luna)

	model = glm::translate(glm::mat4(1.0f), lightPosition);
	model = glm::scale(model, glm::vec3(1.5f));
	lampShader.setMat4("model", model);
	esfera.render();       //Sol redondo

	glBindVertexArray(0);
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    /*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
	monitors = glfwGetPrimaryMonitor();
	getResolution();

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Proyecto", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
	glfwSetWindowPos(window, 0, 30);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, resize);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//To Enable capture of our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	glewInit();

	//Mis funciones
	//Datos a utilizar
	LoadTextures();
	myData();
	esfera.init();
	glEnable(GL_DEPTH_TEST);

	// Se crea el shader para cargar los modelos
	Shader modelShader("Shaders/modelLoading.vs", "Shaders/modelLoading.fs");

	// Se cargan los modelos a utilizar
	Model batarang((GLchar *)"Models/Batarang/INJ_iOS_WEAPON_Batman's_Batarang_Dawn_Of_Justice.obj", 0);


	// Inicialización de KeyFrames
	LoadKeyFrames("KeyFramesCarro.txt");

    // render loop
    // While the windows is not closed
    while (!glfwWindowShouldClose(window))
    {
		// per-frame time logic
		// --------------------
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
        // input
        // -----
        my_input(window);
		animate();

        // render
        // Backgound color
        glClearColor(bgColorRed, bgColorGreen, bgColorBlue, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Mi función de dibujo
		display(modelShader, batarang);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void my_input(GLFWwindow *window)
{
	// Cerrar la ventana
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Movimiento de la cámara
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, (float)deltaTime * 5);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, (float)deltaTime * 5);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, (float)deltaTime * 5);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, (float)deltaTime * 5);

	// --------------- Animación por keyframes ---------------

	// Movimiento 
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		posCarroX -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		posCarroX += 0.1f;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		posCarroY -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		posCarroY += 0.1f;

	// Rotación de la Rodilla Izquierda
	if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
		rotCarro--;
	if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
		rotCarro++;

	// Para iniciar la animación de la montaña rusa
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && (PPresionado == 0))
	{
		if (play == false && (FrameIndex > 1))
		{
			resetElements();
			interpolation(); //Primer Interpolation	

			play = true;
			playIndex = 0;
			i_curr_steps = 0;
		}
		else
			play = false;

		PPresionado = 1;   //La tecla ya está presionada
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
		PPresionado = 0;

	// Para guardar el keyframe del carro de la montaña rusa
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS && (F1Presionado == 0))
	{
		if (FrameIndex < MAX_FRAMES)
			saveFrame("KeyFramesCarro.txt");

		F1Presionado = 1;   //La tecla ya está presionada
	}
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_RELEASE)
		F1Presionado = 0;


	// --------------- Tipo de Iluminación ---------------

	// Luz Posicional - Modo día
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	{
		strcpy_s(Light_VertxShader, "shaders/shader_texture_light_pos.vs");
		strcpy_s(Light_FragShader, "shaders/shader_texture_light_pos.fs");

		Light_Position = lightPosition = glm::vec3(20.0f, 70.0f, 35.0f);

		Luz_Reflector = 0;  //La luz ya no es de tipo reflector

		//Se cambia el color de fondo (Blanco)
		bgColorRed   = 1.0f;
		bgColorGreen = 1.0f;
		bgColorBlue  = 1.0f;

		//Se cambia el color de la luz (Blanco)
		R = 1.0f;
		G = 1.0f;
		B = 1.0f;

		modoDia = 1;         //Modo dia On
		modoAtardecer = 0;   //Modo atardecer Off
		modoNoche = 0;       //Modo noche Off
	}

	// Luz Posicional - Modo atardecer
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	{
		strcpy_s(Light_VertxShader, "shaders/shader_texture_light_pos.vs");
		strcpy_s(Light_FragShader, "shaders/shader_texture_light_pos.fs");

		Light_Position = lightPosition = glm::vec3(65.0f, 60.0f, 30.0f);

		Luz_Reflector = 0;  //La luz ya no es de tipo reflector

		//Se cambia el color de fondo (Blanco)
		bgColorRed = 1.0f;
		bgColorGreen = 1.0f;
		bgColorBlue = 1.0f;

		//Se cambia el color de la luz
		R = 1.00000f;
		G = 0.53333f;
		B = 0.06667f;

		modoDia = 0;         //Modo dia Off
		modoAtardecer = 1;   //Modo atardecer On
		modoNoche = 0;       //Modo noche Off
	}

	// Luz Posicional - Modo noche
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
	{
		strcpy_s(Light_VertxShader, "shaders/shader_texture_light_pos.vs");
		strcpy_s(Light_FragShader, "shaders/shader_texture_light_pos.fs");

		Light_Position = lightPosition = glm::vec3(-25.0f, 60.0f, 30.0f);

		Luz_Reflector = 0;  //La luz ya no es de tipo reflector

		//Se cambia el color de fondo (Negro)
		bgColorRed = 0.0f;
		bgColorGreen = 0.0f;
		bgColorBlue = 0.0f;

		//Se cambia el color de la luz
		R = 0.92549f;
		G = 0.92941f;
		B = 0.67843f;

		modoDia = 0;         //Modo dia Off
		modoAtardecer = 0;   //Modo atardecer Off
		modoNoche = 1;       //Modo noche On
	}

	/*
	// Luz de Reflector
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	{
		strcpy_s(Light_VertxShader, "shaders/shader_texture_light_spot.vs");
		strcpy_s(Light_FragShader, "shaders/shader_texture_light_spot.fs");

		Luz_Reflector = 1;  //La luz es de tipo reflector

		//Light_Position = glm::vec3(20.0f, 10.0f, 0.0f);

		//Se cambia el color de fondo (Noche)
		bgColorRed   = 0.0f;
		bgColorGreen = 0.0f;
		bgColorBlue  = 0.0f;

		modoNoche = 1;   //Modo noche on
	}

	// Luz Direccional
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
	{
		strcpy_s(Light_VertxShader, "shaders/shader_texture_light_dir.vs");
		strcpy_s(Light_FragShader, "shaders/shader_texture_light_dir.fs");
		Light_Direction = lightDirection;

		Luz_Reflector = 0;   //La luz ya no es de tipo reflector

		//Se cambia el color de fondo (Día)
		bgColorRed   = 1.0f;
		bgColorGreen = 1.0f;
		bgColorBlue  = 1.0f;

		modoNoche = 0;   //Modo noche off
	}
	

	// Instructivo:
		Con las siguientes teclas se modifica el color de la luz :
		R: La luz se vuelve más roja
		G : La luz se vuelve más verde
		B : La luz se vuelve más azul

		H : La luz se vuelve completamente blanca
		T : La luz se apaga

		Con las siguientes teclas se modifica la intensidad de la luz
		9 : Disminuye la instensidad
		0 : Aumenta la instensidad

		---> NOTA IMPORTANTE:
		Para modificar la intensidad de la luz y el color se debe presionar
		varias veces la tecla para ir aumentando, si la tecla se deja presionada,
		ésta se tomará como un solo incremento.

	// --------------- Intensidad de la Luz ---------------
	
	// Aumenta
	if ((glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) && (CeroPresionado == 0))
	{
		opacidad -= 1;
		CeroPresionado = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE)
		CeroPresionado = 0;

	// Disminuye
	if ((glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) && (NuevePresionado == 0))
	{
		opacidad += 1;
		NuevePresionado = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_RELEASE)
		NuevePresionado = 0;


	// --------------- Color de la luz ---------------

	// Aumenta el Rojo
	if ((glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) && (RPresionado == 0))
	{
		R += 0.1f;
		RPresionado = 1;
		
		if (R > 1.1f)
			R = 0.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
		RPresionado = 0;

	// Aumenta el Verde
	if ((glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) && (GPresionado == 0))
	{
		G += 0.1f;
		GPresionado = 1;

		if (G > 1.1f)
			G = 0.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE)
		GPresionado = 0;

	// Aumenta el Azul
	if ((glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) && (BPresionado == 0))
	{
		B += 0.1f;
		BPresionado = 1;

		if (B > 1.1f)
			B = 0.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
		BPresionado = 0;

	// Luz Blanca
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
	{
		R = 1.0f;
		G = 1.0f;
		B = 1.0f;
	}

	// Luz Apagada
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
	{
		R = 0.0f;
		G = 0.0f;
		B = 0.0f;
	}
	*/
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void resize(GLFWwindow* window, int width, int height)
{
    // Set the Viewport to the size of the created window
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset*3, yoffset*3);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}


// ------------------> Implementación de funciones de Renderizado <------------------

void montañaRusa(glm::mat4 model_loc, Shader lightingShader, Shader lampShader, Shader modelShader, Model logo)
{
	glm::mat4 model;                   //Matriz para ir dibujando cada elemento
	glm::mat4 temp = glm::mat4(1.0f);  //Matriz auxiliar

	Shader shader("", "");   //Shader vacío para elegir entre cuál utilizar


	//  ------------- Rieles -------------

	// Modo de iluminación
	if (modoNoche)
	{
		lampShader.use();                               //Se activa este shader        
		lampShader.setVec3("color", 1.0f, 1.0f, 0.0f);  //Color de la estructura en modo noche  
		shader = lampShader;                            //Se selecciona este shader
	}
	else
	{
		lightingShader.use();                                                //Se activa este shader  
		lightingShader.setInt("material_diffuse", t_metal_amarillo);         //Texturas de la estructura en modo día
		lightingShader.setInt("material_specular", t_metal_amarillo_brillo);
		shader = lightingShader;                                             //Se selecciona este shader
	}

	// Riel 1
	model = glm::translate(model_loc, glm::vec3(19.75f, 16.5f, -5.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, shader);       //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 2
	model = glm::translate(model_loc, glm::vec3(14.875f, 13.625f, -5.0f)); //Ubicación del Riel
	riel(model, 11.0f, 32.65f, shader);    //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 3
	model = glm::translate(model_loc, glm::vec3(10.0f, 10.75f, -5.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, shader);       //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 4
	model = glm::translate(model_loc, glm::vec3(8.6f, 11.625f, -5.0f)); //Ubicación del Riel
	riel(model, 3.2f, -38.5f, shader);     //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 5
	model = glm::translate(model_loc, glm::vec3(7.2f, 12.5f, -5.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, shader);       //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 6
	model = glm::translate(model_loc, glm::vec3(4.8f, 9.95f, -5.0f)); //Ubicación del Riel
	riel(model, 7.15f, 50.0f, shader);     //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 7
	model = glm::translate(model_loc, glm::vec3(2.4f, 7.4f, -5.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, shader);       //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 8
	model = glm::translate(model_loc, glm::vec3(0.0f, 9.95f, -5.0f)); //Ubicación del Riel
	riel(model, 7.15f, -50.0f, shader);    //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 9
	model = glm::translate(model_loc, glm::vec3(-2.4f, 12.5f, -5.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, shader);       //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 10
	model = glm::translate(model_loc, glm::vec3(-3.95f, 11.625f, -5.0f)); //Ubicación del Riel
	riel(model, 3.35f, 36.0f, shader);     //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 11
	model = glm::translate(model_loc, glm::vec3(-5.5f, 10.75f, -5.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, shader);       //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 12
	model = glm::translate(model_loc, glm::vec3(-8.25f, 11.25, -5.0f)); //Ubicación del Riel
	riel(model, 4.85f, -12.40f, shader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 13
	model = glm::translate(model_loc, glm::vec3(-11.0f, 11.75, -5.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, shader);       //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 14
	model = glm::translate(model_loc, glm::vec3(-12.80f, 9.35f, -5.0f)); //Ubicación del Riel
	riel(model, 6.3f, 56.7f, shader);      //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 15
	model = glm::translate(model_loc, glm::vec3(-14.45f, 6.3f, -5.0f)); //Ubicación del Riel
	riel(model, 1.5f, 90.0f, shader);      //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 16
	model = glm::translate(model_loc, glm::vec3(-11.35f, 2.90f, -5.0f)); //Ubicación del Riel
	riel(model, 8.90f, -43.65f, shader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 17
	model = glm::translate(model_loc, glm::vec3(-8.0f, 0.0f, -5.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, shader);       //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 18
	model = glm::translate(model_loc, glm::vec3(-2.8f, 2.7f, -5.0f)); //Ubicación del Riel
	riel(model, 11.35f, 29.0f, shader);    //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 19
	model = glm::translate(model_loc, glm::vec3(2.4f, 5.4f, -5.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, shader);       //Modelo de referencia, Escala en X, Rotación en Z
	 
	// Riel 20
	model = glm::translate(model_loc, glm::vec3(6.2f, 3.825f, -5.0f)); //Ubicación del Riel
	riel(model, 7.75f, -25.0f, shader);    //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 21
	model = glm::translate(model_loc, glm::vec3(10.0f, 2.25f, -5.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, shader);       //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 22
	model = glm::translate(model_loc, glm::vec3(13.25f, 4.25f, -5.0f)); //Ubicación del Riel
	riel(model, 7.40f, 34.50f, shader);    //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 23
	model = glm::translate(model_loc, glm::vec3(16.5f, 6.25f, -5.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, shader);       //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 24 
	model = glm::translate(model_loc, glm::vec3(19.06f, 5.375f, -5.0f)); //Ubicación del Riel
	riel(model, 4.90f, -22.25f, shader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 25
	model = glm::translate(model_loc, glm::vec3(21.0f, 1.125f, -5.0f)); //Ubicación del Riel
	riel(model, 7.5f, 90.0f, shader);      //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 26
	model = glm::translate(model_loc, glm::vec3(20.05f, -3.175f, -5.0f)); //Ubicación del Riel
	riel(model, 2.95f, 40.0f, shader);     //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 27
	model = glm::translate(model_loc, glm::vec3(13.5f, -4.0f, -5.0f)); //Ubicación del Riel
	riel(model, 11.5f, 0.0f, shader);      //Modelo de referencia, Escala en X, Rotación en Z

	//Riel 28 
	model = glm::translate(model_loc, glm::vec3(5.75f, -2.25f, -5.0f)); //Ubicación del Riel
	riel(model, 6.025f, -38.5f, shader);   //Modelo de referencia, Escala en X, Rotación en Z

	//Riel 29
	model = glm::translate(model_loc, glm::vec3(2.5f, -0.5f, -5.0f)); //Ubicación del Riel
	riel(model, 2.5f, 0.0f, shader);       //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 30
	model = glm::translate(model_loc, glm::vec3(-0.40f, -1.225f, -5.0f)); //Ubicación del Riel
	riel(model, 4.0f, 22.5f, shader);      //Modelo de referencia, Escala en X, Rotación en Z

	//Riel 31
	model = glm::translate(model_loc, glm::vec3(-1.95f, -3.5f, -5.0f)); //Ubicación del Riel
	riel(model, 4.0f, 90.0f, shader);      //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 32
	model = glm::translate(model_loc, glm::vec3(0.675f, -6.0f, -5.0f)); //Ubicación del Riel
	riel(model, 6.25f, -18.5f, shader);    //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 33
	model = glm::translate(model_loc, glm::vec3(12.5f, -7.0f, -5.0f)); //Ubicación del Riel
	riel(model, 18.0f, 0.0f, shader);      //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 34
	model = glm::translate(model_loc, glm::vec3(22.10f, -5.95f, -5.0f)); //Ubicación del Riel
	riel(model, 3.20f, 52.0f, shader);     //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 35
	model = glm::translate(model_loc, glm::vec3(23.0f, 4.5f, -5.0f)); //Ubicación del Riel
	riel(model, 19.0f, 90.0f, shader);     //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 36
	model = glm::translate(model_loc, glm::vec3(21.55f, 15.15f, -5.0f)); //Ubicación del Riel
	riel(model, 4.425f, -42.5f, shader);   //Modelo de referencia, Escala en X, Rotación en Z


	lightingShader.use();

	// ------------- Carro -------------

	glm::mat4 ubicacion_carro = glm::mat4(1.0f);  //Variable auxiliar para saber la posición del carro en todo momento

	lightingShader.setInt("material_diffuse", t_azul);           //Texturas del chasis
	lightingShader.setInt("material_specular", t_azul_brillo);

	model = glm::translate(model_loc, glm::vec3(19.75f, 17.5f, -5.0f));                //Se parte de Riel 1
	model = glm::translate(model, glm::vec3(posCarroX, posCarroY, posCarroZ));         //Se desplaza al keyframe correspondiente
	model = glm::rotate(model, glm::radians(rotCarro), glm::vec3(0.0f, 0.0f, 1.0f));   //Rotación en X
	ubicacion_carro = model;                                                           //Obtenemos las coordenadas del carro a lo largo del recorrido
	//model = glm::scale(model, glm::vec3(40.0f, 1.0f, 14.0f));                        //Escalado del carro
	carro(model, lightingShader);                                                      //Se manda a dibujar todo el carro


	// ------------ Sillas del Carro  ------------
	
	// Frontal Izquierda 1
	lightingShader.setInt("material_diffuse", t_azul);         //Texturas de la silla
	lightingShader.setInt("material_specular", t_azul_brillo);

	model = glm::translate(ubicacion_carro, glm::vec3(-1.25f, 0.0f, 2.325f));           //Ubicación de la silla sobre el carro
	model = glm::rotate(model, glm::radians(giroSilla), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de giro
	model = glm::scale(model, glm::vec3(0.40f));                                        //Escalado de la silla
	silla(model, -1, 1, lightingShader);                                                //Se dibuja la silla. -1: Guarda Derecha  1: Guarda Izquierda

	// Frontal Izquierda 2
	lightingShader.setInt("material_diffuse", t_azul);         //Texturas de la silla
	lightingShader.setInt("material_specular", t_azul_brillo);

	model = glm::translate(ubicacion_carro, glm::vec3(-1.25f, 0.0f, 3.35f));            //Ubicación de la silla sobre el carro
	model = glm::rotate(model, glm::radians(giroSilla), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de giro
	model = glm::scale(model, glm::vec3(0.40f));                                        //Escalado de la silla
	silla(model, 0, 1, lightingShader);                                                 //Se dibuja la silla. -1: Guarda Derecha  1: Guarda Izquierda
	
	// Frontal Derecha 1
	lightingShader.setInt("material_diffuse", t_azul);         //Texturas de la silla
	lightingShader.setInt("material_specular", t_azul_brillo);

	model = glm::translate(ubicacion_carro, glm::vec3(-1.25f, 0.0f, -2.325f));          //Ubicación de la silla sobre el carro
	model = glm::rotate(model, glm::radians(giroSilla), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de giro
	model = glm::scale(model, glm::vec3(0.40f));                                        //Escalado de la silla
	silla(model, 1, 1, lightingShader);                                                 //Se dibuja la silla. -1: Guarda Derecha  1: Guarda Izquierda

	// Frontal Derecha 2
	lightingShader.setInt("material_diffuse", t_azul);         //Texturas de la silla
	lightingShader.setInt("material_specular", t_azul_brillo);

	model = glm::translate(ubicacion_carro, glm::vec3(-1.25f, 0.0f, -3.35f));           //Ubicación de la silla sobre el carro
	model = glm::rotate(model, glm::radians(giroSilla), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de giro
	model = glm::scale(model, glm::vec3(0.40f));                                        //Escalado de la silla
	silla(model, 0, 1, lightingShader);                                                 //Se dibuja la silla. -1: Guarda Derecha  1: Guarda Izquierda
	
	// Trasera Izquierda 1
	lightingShader.setInt("material_diffuse", t_azul);         //Texturas de la silla
	lightingShader.setInt("material_specular", t_azul_brillo);

	model = glm::translate(ubicacion_carro, glm::vec3(1.25f, 0.0f, 2.325f));            //Ubicación de la silla sobre el carro
	model = glm::rotate(model, glm::radians(giroSilla), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de giro
	model = glm::scale(model, glm::vec3(0.40f));                                        //Escalado de la silla
	silla(model, -1, 1, lightingShader);                                                //Se dibuja la silla. -1: Guarda Derecha  1: Guarda Izquierda

	// Trasera Izquierda 2
	lightingShader.setInt("material_diffuse", t_azul);         //Texturas de la silla
	lightingShader.setInt("material_specular", t_azul_brillo);

	model = glm::translate(ubicacion_carro, glm::vec3(1.25f, 0.0f, 3.35f));             //Ubicación de la silla sobre el carro
	model = glm::rotate(model, glm::radians(giroSilla), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de giro
	model = glm::scale(model, glm::vec3(0.40f));                                        //Escalado de la silla
	silla(model, 0, 1, lightingShader);                                                 //Se dibuja la silla. -1: Guarda Derecha  1: Guarda Izquierda
	
	// Trasera Derecha 1
	lightingShader.setInt("material_diffuse", t_azul);         //Texturas de la silla
	lightingShader.setInt("material_specular", t_azul_brillo);

	model = glm::translate(ubicacion_carro, glm::vec3(1.25f, 0.0f, -2.325f));           //Ubicación de la silla sobre el carro
	model = glm::rotate(model, glm::radians(giroSilla), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de giro
	model = glm::scale(model, glm::vec3(0.40f));                                        //Escalado de la silla
	silla(model, 1, 1, lightingShader);                                                 //Se dibuja la silla. -1: Guarda Derecha  1: Guarda Izquierda

	// Trasera Derecha 2
	lightingShader.setInt("material_diffuse", t_azul);         //Texturas de la silla
	lightingShader.setInt("material_specular", t_azul_brillo);

	model = glm::translate(ubicacion_carro, glm::vec3(1.25f, 0.0f, -3.35f));            //Ubicación de la silla sobre el carro
	model = glm::rotate(model, glm::radians(giroSilla), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de giro
	model = glm::scale(model, glm::vec3(0.40f));                                        //Escalado de la silla
	silla(model, 0, 1, lightingShader);                                                 //Se dibuja la silla. -1: Guarda Derecha  1: Guarda Izquierda


	//  ------------- Bases de los Rieles -------------

	lightingShader.setInt("material_diffuse", t_metal);           //Texturas de todas las bases
	lightingShader.setInt("material_specular", t_metal_brillo);

	// --- Base 1 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(-9.7f, -2.05f, 0.0f));
	model = glm::rotate(model, glm::radians(24.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 18.0f, 1.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(-9.7f, -2.05f, -10.0f));
	model = glm::rotate(model, glm::radians(24.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 18.0f, 1.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// Horizontal Superior
	model = glm::translate(model_loc, glm::vec3(-13.4f, 6.4f, -5.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 10.90f, 1.10f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// -> Anillo Frontal Izquierdo 
	model = glm::translate(model_loc, glm::vec3(-13.4f, 6.4f, -1.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.30f, 0.25f, 1.30f));
	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// ->  Anillo Frontal Derecho 
	model = glm::translate(model_loc, glm::vec3(-13.4f, 6.4f, -9.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.30f, 0.25f, 1.30f));
	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Base 2 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(-5.5f, -0.5f, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 20.25f, 1.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(-5.5f, -0.5f, -10.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 20.25f, 1.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// Horizontal Superior
	model = glm::translate(model_loc, glm::vec3(-5.5f, 9.75f, -5.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 10.90f, 1.10f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// -> Anillo Frontal Izquierdo 
	model = glm::translate(model_loc, glm::vec3(-5.5f, 9.75f, -1.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.30f, 0.25f, 1.30f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// ->  Anillo Frontal Derecho 
	model = glm::translate(model_loc, glm::vec3(-5.5f, 9.75f, -9.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.30f, 0.25f, 1.30f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Horizontal Inferior
	model = glm::translate(model_loc, glm::vec3(-5.5f, 2.25f, -5.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.90f, 10.0f, 0.90f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// -> Anillo Frontal Izquierdo 
	model = glm::translate(model_loc, glm::vec3(-5.5f, 2.25f, -1.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 0.25f, 1.10f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// ->  Anillo Frontal Derecho 
	model = glm::translate(model_loc, glm::vec3(-5.5f, 2.25f, -9.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.30f, 0.25f, 1.30f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Base 3 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(-1.25f, -2.05f, 0.0f));
	model = glm::rotate(model, glm::radians(-24.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 18.0f, 1.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(-1.25f, -2.05f, -10.0f));
	model = glm::rotate(model, glm::radians(-24.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 18.0f, 1.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// Horizontal Superior
	model = glm::translate(model_loc, glm::vec3(2.4f, 6.4f, -5.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 10.90f, 1.10f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// -> Anillo Frontal Izquierdo 
	model = glm::translate(model_loc, glm::vec3(2.4f, 6.4f, -1.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.30f, 0.25f, 1.30f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// ->  Anillo Frontal Derecho 
	model = glm::translate(model_loc, glm::vec3(2.4f, 6.4f, -9.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.30f, 0.25f, 1.30f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Base 4 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(3.0f, -6.0, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 9.0f, 1.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(3.0f, -6.0f, -10.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 9.0f, 1.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// Horizontal Superior
	model = glm::translate(model_loc, glm::vec3(3.0f, -1.5f, -5.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 10.90f, 1.10f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// -> Anillo Frontal Izquierdo 
	model = glm::translate(model_loc, glm::vec3(3.0f, -1.5f, -1.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.30f, 0.25f, 1.30f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// ->  Anillo Frontal Derecho 
	model = glm::translate(model_loc, glm::vec3(3.0f, -1.5f, -9.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.30f, 0.25f, 1.30f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Horizontal Inferior
	model = glm::translate(model_loc, glm::vec3(3.0f, -6.0f, -5.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.90f, 10.0f, 0.90f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// -> Anillo Frontal Izquierdo 
	model = glm::translate(model_loc, glm::vec3(3.0f, -6.0f, -1.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 0.25f, 1.10f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// ->  Anillo Frontal Derecho 
	model = glm::translate(model_loc, glm::vec3(3.0f, -6.0f, -9.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 0.25f, 1.10f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Base 5 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(6.0f, -2.05f, 0.0f));
	model = glm::rotate(model, glm::radians(24.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 18.0f, 1.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(6.0f, -2.05f, -10.0f));
	model = glm::rotate(model, glm::radians(24.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 18.0f, 1.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// Horizontal Inferior
	model = glm::translate(model_loc, glm::vec3(6.90f, -4.25f, -5.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.75f, 10.0f, 0.75f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// -> Anillo Frontal Izquierdo 
	model = glm::translate(model_loc, glm::vec3(6.90f, -4.25f, -1.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.90f, 0.25f, 0.90f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// ->  Anillo Frontal Derecho 
	model = glm::translate(model_loc, glm::vec3(6.90f, -4.25f, -9.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.90f, 0.25f, 0.90f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Base 6 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(10.0f, -0.5f, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 20.25f, 1.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(10.0f, -0.5f, -10.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 20.25f, 1.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// Horizontal Superior
	model = glm::translate(model_loc, glm::vec3(10.0f, 9.75f, -5.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 10.90f, 1.10f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// -> Anillo Frontal Izquierdo 
	model = glm::translate(model_loc, glm::vec3(10.0f, 9.75f, -1.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.30f, 0.25f, 1.30f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// ->  Anillo Frontal Derecho 
	model = glm::translate(model_loc, glm::vec3(10.0f, 9.75f, -9.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.30f, 0.25f, 1.30f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Horizontal Intermedio
	model = glm::translate(model_loc, glm::vec3(10.0f, 3.15f, -5.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.90f, 10.0f, 0.90f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// -> Anillo Frontal Izquierdo 
	model = glm::translate(model_loc, glm::vec3(10.0f, 3.15f, -1.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 0.25f, 1.10f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// ->  Anillo Frontal Derecho 
	model = glm::translate(model_loc, glm::vec3(10.0f, 3.15f, -9.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 0.25f, 1.10f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Horizontal Inferior
	model = glm::translate(model_loc, glm::vec3(10.0f, -4.90f, -5.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.90f, 10.0f, 0.90f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// -> Anillo Frontal Izquierdo 
	model = glm::translate(model_loc, glm::vec3(10.0f, -4.90f, -1.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 0.25f, 1.10f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// ->  Anillo Frontal Derecho 
	model = glm::translate(model_loc, glm::vec3(10.0f, -4.90f, -9.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 0.25f, 1.10f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Base 7 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(15.25f, 2.5f, 0.0f));
	model = glm::rotate(model, glm::radians(-19.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 27.0f, 1.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(15.25f, 2.5f, -10.0f));
	model = glm::rotate(model, glm::radians(-19.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 27.0f, 1.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// Horizontal Superior
	model = glm::translate(model_loc, glm::vec3(19.7f, 15.5f, -5.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 10.90f, 1.10f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// -> Anillo Frontal Izquierdo 
	model = glm::translate(model_loc, glm::vec3(19.7f, 15.5f, -1.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.30f, 0.25f, 1.30f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// ->  Anillo Frontal Derecho 
	model = glm::translate(model_loc, glm::vec3(19.7f, 15.5f, -9.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.30f, 0.25f, 1.30f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Horizontal Intermedio
	model = glm::translate(model_loc, glm::vec3(16.85f, 7.10f, -5.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.75f, 10.0f, 0.75f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// -> Anillo Frontal Izquierdo 
	model = glm::translate(model_loc, glm::vec3(16.85f, 7.10f, -1.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.90f, 0.25f, 0.90f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// ->  Anillo Frontal Derecho 
	model = glm::translate(model_loc, glm::vec3(16.85f, 7.10f, -9.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.90f, 0.25f, 0.90f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Horizontal Inferior
	model = glm::translate(model_loc, glm::vec3(12.75f, -4.90f, -5.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.90f, 10.0f, 0.90f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// -> Anillo Frontal Izquierdo 
	model = glm::translate(model_loc, glm::vec3(12.75f, -4.90f, -1.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 0.25f, 1.10f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// ->  Anillo Frontal Derecho 
	model = glm::translate(model_loc, glm::vec3(12.75f, -4.90f, -9.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 0.25f, 1.10f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Base 8 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(22.0f, -3.125f, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 14.75f, 1.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(22.0f, -3.125f, -10.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 14.75f, 1.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// Horizontal Superior
	model = glm::translate(model_loc, glm::vec3(22.0f, 4.25f, -5.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 10.90f, 1.10f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// -> Anillo Frontal Izquierdo 
	model = glm::translate(model_loc, glm::vec3(22.0f, 4.25f, -1.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.30f, 0.25f, 1.30f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// ->  Anillo Frontal Derecho 
	model = glm::translate(model_loc, glm::vec3(22.0f, 4.25f, -9.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.30f, 0.25f, 1.30f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Horizontal Inferior
	model = glm::translate(model_loc, glm::vec3(22.0f, -2.25f, -5.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.90f, 10.0f, 0.90f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// -> Anillo Frontal Izquierdo 
	model = glm::translate(model_loc, glm::vec3(22.0f, -2.25f, -1.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 0.25f, 1.10f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// ->  Anillo Frontal Derecho 
	model = glm::translate(model_loc, glm::vec3(22.0f, -2.25f, -9.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 0.25f, 1.10f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);


	//  ------------- Soportes de las Bases -------------

	lightingShader.setInt("material_diffuse", t_metal);           //Texturas de todas las bases
	lightingShader.setInt("material_specular", t_metal_brillo);

	// --- Soporte Base 1, 2 y 3 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(-5.5f, -10.25f, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(3.0f, 0.5f, 2.5f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Trasero
	model = glm::translate(model_loc, glm::vec3(-5.5f, -10.25f, -10.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(3.0f, 0.5f, 2.5f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Soporte Base 4 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(3.0f, -10.25f, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.5f, 0.5f, 1.5f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Trasero
	model = glm::translate(model_loc, glm::vec3(3.0f, -10.25f, -10.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.5f, 0.5f, 1.5f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Soporte Base 5, 6 y 7 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(10.25f, -10.25f, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(3.5f, 0.5f, 3.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Trasero
	model = glm::translate(model_loc, glm::vec3(10.25f, -10.25f, -10.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(3.5f, 0.5f, 3.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Soporte Base 8 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(22.0f, -10.25f, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.5f, 0.5f, 1.5f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Trasero
	model = glm::translate(model_loc, glm::vec3(22.0f, -10.25f, -10.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.5f, 0.5f, 1.5f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);


	// ------------- Piso -------------

	lightingShader.setInt("material_diffuse", t_terracota);           //Textura del piso
	lightingShader.setInt("material_specular", t_terracota_brillo);

	model = glm::translate(model_loc, glm::vec3(4.5f, -11.0f, -5.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(40.0f, 1.0f, 16.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 0, 24);


	// ------------- Logo Batman -------------

	// Modo de iluminación
	if (modoNoche)
	{
		lampShader.use();                               //Se activa este shader        
		lampShader.setVec3("color", 1.0f, 1.0f, 0.0f);  //Color de la estructura en modo noche  
		shader = lampShader;                            //Se selecciona este shader
	}
	else
	{
		lightingShader.use();                                                //Se activa este shader  
		lightingShader.setInt("material_diffuse", t_metal_amarillo);         //Texturas de la estructura en modo día
		lightingShader.setInt("material_specular", t_metal_amarillo_brillo);
		shader = lightingShader;                                             //Se selecciona este shader
	}

	// Óvalo Amarrillo
	model = glm::translate(model_loc, glm::vec3(2.4f, 6.4f, 0.5f));
	model = glm::scale(model, glm::vec3(3.0f, 1.5f, 0.25f));
	shader.setMat4("model", model);
	esfera.render();

	// Modo de iluminación
	if (modoNoche)
	{
		lampShader.use();                               //Se activa este shader        
		lampShader.setVec3("color", 0.0f, 0.0f, 0.0f);  //Color de la estructura en modo noche  
		shader = lampShader;                            //Se selecciona este shader
	}
	else
	{
		lightingShader.use();                                        //Se activa este shader  
		lightingShader.setInt("material_diffuse", t_negro);          //Texturas de la estructura en modo día
		lightingShader.setInt("material_specular", t_negro_brillo);
		shader = lightingShader;                                     //Se selecciona este shader
	}

	// Óvalo Negro
	model = glm::translate(model_loc, glm::vec3(2.4f, 6.4f, 0.5f));
	model = glm::scale(model, glm::vec3(3.15f, 1.65f, 0.15f));
	shader.setMat4("model", model);
	esfera.render();

	// Batarang
	modelShader.use();
	model = glm::translate(model_loc, glm::vec3(2.4f, 6.4f, 0.75f));
	model = glm::scale(model, glm::vec3(17.0f));
	modelShader.setMat4("model", model);
	logo.Draw(modelShader);
}

void riel(glm::mat4 model_loc, float escala_x, float rotacion, Shader shader)
{
	glm::mat4 model;

	//Se aplica la rotación a todo el riel
	model_loc = glm::rotate(model_loc, glm::radians(rotacion), glm::vec3(0.0f, 0.0f, 1.0f));
	
	// ------------- Riel Frontal -------------

	// Centro
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(escala_x, 1.0f, 0.25f));
	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 0, 24);

	// Superior
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.5f, 1.0f));
	model = glm::scale(model, glm::vec3(escala_x, 0.05f, 0.5f));
	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 0, 24);

	//Inferior
	model = glm::translate(model_loc, glm::vec3(0.0f, -0.5f, 1.0f));
	model = glm::scale(model, glm::vec3(escala_x, 0.05f, 0.5f));
	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 0, 24);

	// ------------- Riel Posterior -------------
	// Centro
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, -1.0f));
	model = glm::scale(model, glm::vec3(escala_x, 1.0f, 0.25f));
	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 0, 24);

	// Superior
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.5f, -1.0f));
	model = glm::scale(model, glm::vec3(escala_x, 0.05f, 0.5f));
	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 0, 24);

	//Inferior
	model = glm::translate(model_loc, glm::vec3(0.0f, -0.5f, -1.0f));
	model = glm::scale(model, glm::vec3(escala_x, 0.05f, 0.5f));
	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 0, 24);
}

void carro(glm::mat4 model_loc, Shader lightingShader)
{
	glm::mat4 model;

	// ------------- Chasis-------------

	// --- Tubo Central ---
	model = glm::rotate(model_loc, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 2.5f, 1.0f));
	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// --- Tubo Frontal ---
	model = glm::translate(model_loc, glm::vec3(-1.25f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f, 3.25f, 1.0f));
	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Anillo Frontal del Centro ---
	model = glm::translate(model_loc, glm::vec3(-1.25f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.25f, 0.25f, 1.25f));
	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);
	
	// --- Anillo Frontal Izquierdo ---
	model = glm::translate(model_loc, glm::vec3(-1.25f, 0.0f, 1.35f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.25f, 0.25f, 1.25f));
	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);
	
	// --- Anillo Frontal Derecho ---
	model = glm::translate(model_loc, glm::vec3(-1.25f, 0.0f, -1.35f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.25f, 0.25f, 1.25f));
	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Unión Frontal Izquierda ---
	model = glm::translate(model_loc, glm::vec3(-0.85f, 0.0f, 0.30f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.95f, 1.0f, 0.95f));
	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// --- Unión Frontal Derecha ---
	model = glm::translate(model_loc, glm::vec3(-0.85f, 0.0f, -0.30f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.95f, 1.0f, 0.95f));
	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);


	// --- Tubo Posterior ---
	model = glm::translate(model_loc, glm::vec3(1.25f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f, 3.25f, 1.0f));
	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Anillo Posterior del Centro ---
	model = glm::translate(model_loc, glm::vec3(1.25f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.25f, 0.25f, 1.25f));
	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);
	
	// --- Anillo Posterior Izquierdo ---
	model = glm::translate(model_loc, glm::vec3(1.25f, 0.0f, 1.35f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.25f, 0.25f, 1.25f));
	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);
	
	// --- Anillo Posterior Derecho ---
	model = glm::translate(model_loc, glm::vec3(1.25f, 0.0f, -1.35f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.25f, 0.25f, 1.25f));
	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Unión Posterior Izquierda ---
	model = glm::translate(model_loc, glm::vec3(0.85f, 0.0f, 0.30f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.95f, 1.0f, 0.95f));
	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);

	// --- Unión Posterior Derecha ---
	model = glm::translate(model_loc, glm::vec3(0.85f, 0.0f, -0.30f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.95f, 1.0f, 0.95f));
	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
}

void silla(glm::mat4 model_loc, int lado, int seguridad, Shader lightingShader)
{
	glm::mat4 model;

	model_loc = glm::rotate(model_loc, glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));;

	// ------------- Asiento -------------
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(2.5f, 5.0f, 5.0f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 96, 40);      //Asiento
	glDrawArrays(GL_POLYGON, 136, 10);   //Costado Derecho
	glDrawArrays(GL_POLYGON, 146, 10);   //Costado Izquierdo


	// ------------- Guarda ----------------
	if (lado == 1 || lado == -1)
	{
		model = glm::translate(model_loc, glm::vec3(lado * 1.5f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.45f, 5.0f, 5.0f));

		lightingShader.setMat4("model", model);
		lightingShader.setInt("material_diffuse", t_metal);
		lightingShader.setInt("material_specular", t_metal_brillo);
		glDrawArrays(GL_QUADS, 156, 20);    //Contorno
		glDrawArrays(GL_POLYGON, 176, 5);   //Costado Derecho
		glDrawArrays(GL_POLYGON, 181, 5);   //Costado Izquierdo
	}


	// ---------------- Tubos de Seguridad ----------------
	
	if (seguridad != 0)
	{
		lightingShader.setInt("material_diffuse", t_metal_amarillo);           //Texturas de los tubos
		lightingShader.setInt("material_specular", t_metal_amarillo_brillo);

		// --- Superior Derecho ---
		model = glm::translate(model_loc, glm::vec3(1.0f, 1.5f, -1.125f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.25f, 1.25f, 0.25f));

		lightingShader.setMat4("model", model);
		glDrawArrays(GL_QUADS, 24, 48);
		glDrawArrays(GL_POLYGON, 72, 12);
		glDrawArrays(GL_POLYGON, 84, 12);

		// --- Superior Derecho Inclinado ---
		model = glm::translate(model_loc, glm::vec3(0.82f, 1.290f, -0.375f));
		model = glm::rotate(model, glm::radians(-50.0f), glm::vec3(1.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(0.23f, 0.75f, 0.23f));

		lightingShader.setMat4("model", model);
		glDrawArrays(GL_QUADS, 24, 48);
		glDrawArrays(GL_POLYGON, 72, 12);
		glDrawArrays(GL_POLYGON, 84, 12);

		// --- Frontal Derecho ---
		model = glm::translate(model_loc, glm::vec3(0.630f, 0.625f, -0.175f));
		model = glm::scale(model, glm::vec3(0.25f, 1.0f, 0.25f));

		lightingShader.setMat4("model", model);
		glDrawArrays(GL_QUADS, 24, 48);
		glDrawArrays(GL_POLYGON, 72, 12);
		glDrawArrays(GL_POLYGON, 84, 12);

		// --- Inferior Derecho Inclinado ---
		model = glm::translate(model_loc, glm::vec3(0.63f, -0.61f, -0.975f));
		model = glm::rotate(model, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.23f, 2.30f, 0.23f));

		lightingShader.setMat4("model", model);
		glDrawArrays(GL_QUADS, 24, 48);
		glDrawArrays(GL_POLYGON, 72, 12);
		glDrawArrays(GL_POLYGON, 84, 12);

		// --- Superior Izquierdo ---
		model = glm::translate(model_loc, glm::vec3(-1.0f, 1.5f, -1.125f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.25f, 1.25f, 0.25f));

		lightingShader.setMat4("model", model);
		glDrawArrays(GL_QUADS, 24, 48);
		glDrawArrays(GL_POLYGON, 72, 12);
		glDrawArrays(GL_POLYGON, 84, 12);

		// --- Superior Izquierdo Inclinado ---
		model = glm::translate(model_loc, glm::vec3(-0.82f, 1.290f, -0.375f));
		model = glm::rotate(model, glm::radians(-50.0f), glm::vec3(1.0f, 0.0f, -1.0f));
		model = glm::scale(model, glm::vec3(0.23f, 0.75f, 0.23f));

		lightingShader.setMat4("model", model);
		glDrawArrays(GL_QUADS, 24, 48);
		glDrawArrays(GL_POLYGON, 72, 12);
		glDrawArrays(GL_POLYGON, 84, 12);

		// --- Frontal Izquierdo ---
		model = glm::translate(model_loc, glm::vec3(-0.630f, 0.625f, -0.175f));
		model = glm::scale(model, glm::vec3(0.25f, 1.0f, 0.25f));

		lightingShader.setMat4("model", model);
		glDrawArrays(GL_QUADS, 24, 48);
		glDrawArrays(GL_POLYGON, 72, 12);
		glDrawArrays(GL_POLYGON, 84, 12);

		// --- Inferior Izquierdo Inclinado ---
		model = glm::translate(model_loc, glm::vec3(-0.63f, -0.61f, -0.975f));
		model = glm::rotate(model, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.23f, 2.30f, 0.23f));

		lightingShader.setMat4("model", model);
		glDrawArrays(GL_QUADS, 24, 48);
		glDrawArrays(GL_POLYGON, 72, 12);
		glDrawArrays(GL_POLYGON, 84, 12);
	}
}

void ruedaDeLaFortuna(glm::mat4 model_loc, Shader lightingShader, Shader lampShader)
{
	glm::mat4 model;  //Matriz para ir dibujando cada elemento

	//  ------------- Estructura Circular -------------
	
	Shader shader("","");   //Shader vacío para elegir entre cuál utilizar

	// Modo de iluminación
	if (modoNoche)
	{
		lampShader.use();                               //Se activa este shader        
		lampShader.setVec3("color", 0.0f, 0.0f, 1.0f);  //Color de la estructura en modo noche  
		shader = lampShader;                            //Se selecciona este shader
	}
	else
	{
		lightingShader.use();                                      //Se activa este shader  
		lightingShader.setInt("material_diffuse", t_azul);         //Texturas de la estructura en modo día
		lightingShader.setInt("material_specular", t_azul_brillo);
		shader = lightingShader;                                   //Se selecciona este shader
	}

	// --- Rueda Frontal ---
	
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, 0.0f));                     //Ubicación del anillo
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));                             //Escala del anillo
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de Giro
	anillo(model, shader);                                                              //Se dibuja el anillo
	
	// --- Rueda Trasera ---

	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, -7.0f));                    //Ubicación del anillo
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));                             //Escala del anillo
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de Giro
	anillo(model, shader);                                                              //Se dibuja el anillo


	//  ------------- Soportes de los Anillos -------------

	// Modo de iluminación
	if (modoNoche)
	{
		lampShader.use();                                //Se activa este shader  
		lampShader.setVec3("color", 1.0f, 1.0f, 1.0f);   //Color de la estructura en modo noche  
		shader = lampShader;                             //Se selecciona este shader
	}
	else
	{
		lightingShader.use();                                         //Se activa este shader  
		lightingShader.setInt("material_diffuse", t_blanco);          //Texturas de la estructura en modo día
		lightingShader.setInt("material_specular", t_blanco_brillo);
		shader = lightingShader;                                      //Se selecciona este shader
	}

	// --- Soportes Frontales ---
	// 1
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de Giro
	model = glm::scale(model, glm::vec3(0.5f, 30.0f, 0.5f));
	
	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 2
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(22.5f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de Giro
	model = glm::scale(model, glm::vec3(0.5f, 30.0f, 0.5f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 3
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de Giro
	model = glm::scale(model, glm::vec3(0.5f, 30.0f, 0.5f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 4
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(67.5f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de Giro
	model = glm::scale(model, glm::vec3(0.5f, 30.0f, 0.5f)); 

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 5
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de Giro
	model = glm::scale(model, glm::vec3(0.5f, 30.0f, 0.5f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 6
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(112.5f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de Giro
	model = glm::scale(model, glm::vec3(0.5f, 30.0f, 0.5f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 7
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(135.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de Giro
	model = glm::scale(model, glm::vec3(0.5f, 30.0f, 0.5f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 8
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(157.5f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de Giro
	model = glm::scale(model, glm::vec3(0.5f, 30.0f, 0.5f)); 

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Soportes Traseros ---
	// 1
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, -7.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de Giro
	model = glm::scale(model, glm::vec3(0.5f, 30.0f, 0.5f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 2
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, -7.0f));
	model = glm::rotate(model, glm::radians(22.5f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de Giro
	model = glm::scale(model, glm::vec3(0.5f, 30.0f, 0.5f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 3
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, -7.0f));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de Giro
	model = glm::scale(model, glm::vec3(0.5f, 30.0f, 0.5f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 4
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, -7.0f));
	model = glm::rotate(model, glm::radians(67.5f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de Giro
	model = glm::scale(model, glm::vec3(0.5f, 30.0f, 0.5f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 5
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, -7.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de Giro
	model = glm::scale(model, glm::vec3(0.5f, 30.0f, 0.5f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 6
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, -7.0f));
	model = glm::rotate(model, glm::radians(112.5f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de Giro
	model = glm::scale(model, glm::vec3(0.5f, 30.0f, 0.5f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 7
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, -7.0f));
	model = glm::rotate(model, glm::radians(135.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));    //Animación de Giro
	model = glm::scale(model, glm::vec3(0.5f, 30.0f, 0.5f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 8
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, -7.0f));
	model = glm::rotate(model, glm::radians(157.5f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));    //Animación de Giro
	model = glm::scale(model, glm::vec3(0.5f, 30.0f, 0.5f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);


	//  ------------- Eje de la Rueda -------------

	// Modo de iluminación
	if (modoNoche)
	{
		lampShader.use();                                //Se activa este shader  
		lampShader.setVec3("color", 0.0f, 0.0f, 1.0f);   //Color de la estructura en modo noche  
		shader = lampShader;                             //Se selecciona este shader
	}
	else
	{
		lightingShader.use();                                       //Se activa este shader  
		lightingShader.setInt("material_diffuse", t_azul);          //Texturas de la estructura en modo día
		lightingShader.setInt("material_specular", t_azul_brillo);
		shader = lightingShader;                                      //Se selecciona este shader
	}

	// Tubo central
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, -3.5f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 1.0f, .0f));  //Animación de Giro
	model = glm::scale(model, glm::vec3(1.25f, 10.0f, 1.25f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Anillo Frontal
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 1.0f, 0.0f));   //Animación de Giro
	model = glm::scale(model, glm::vec3(2.0f, 1.0f, 2.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Anillo Trasero
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, -7.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(giroRueda), glm::vec3(0.0f, 1.0f, 0.0f));   //Animación de Giro
	model = glm::scale(model, glm::vec3(2.0f, 1.0f, 2.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);


	//  ------------- Soportes de la Base -------------

	// Modo de iluminación
	if (modoNoche)
	{
		lampShader.use();                                //Se activa este shader  
		lampShader.setVec3("color", 0.0f, 0.0f, 1.0f);   //Color de la estructura en modo noche  
		shader = lampShader;                             //Se selecciona este shader
	}
	else
	{
		lightingShader.use();                                       //Se activa este shader  
		lightingShader.setInt("material_diffuse", t_azul);          //Texturas de la estructura en modo día
		lightingShader.setInt("material_specular", t_azul_brillo);
		shader = lightingShader;                                      //Se selecciona este shader
	}

	// -- Soportes Frontales ---

	// Soporte Izquierdo
	model = glm::translate(model_loc, glm::vec3(-6.0f, -10.5f, 1.0f));
	model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 24.0f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Soporte Derecho
	model = glm::translate(model_loc, glm::vec3(6.0f, -10.5f, 1.0f));
	model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 24.0f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	//Unión Soporte-Piso Izquierdo
	model = glm::translate(model_loc, glm::vec3(-11.75f, -20.75f, 1.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(2.0f, 0.5f, 2.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	//Unión Soporte-Piso Derecho
	model = glm::translate(model_loc, glm::vec3(11.75f, -20.75f, 1.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(2.0f, 0.5f, 2.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// -- Soportes Traseros ---

	// Soporte Izquierdo
	model = glm::translate(model_loc, glm::vec3(-6.0f, -10.5f, -8.0f));
	model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 24.0f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Soporte Derecho
	model = glm::translate(model_loc, glm::vec3(6.0f, -10.5f, -8.0f));
	model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 24.0f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	//Unión Soporte-Piso Izquierdo
	model = glm::translate(model_loc, glm::vec3(-11.75f, -20.75f, -8.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(2.0f, 0.5f, 2.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	//Unión Soporte-Piso Derecho
	model = glm::translate(model_loc, glm::vec3(11.75f, -20.75f, -8.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(2.0f, 0.5f, 2.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);


	// ------------- Piso -------------

	lightingShader.use();

	model = glm::translate(model_loc, glm::vec3(0.0f, -21.5f, -3.5f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(40.0f, 1.0f, 16.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_terracota);          //Textura del piso
	lightingShader.setInt("material_specular", t_terracota_brillo);
	glDrawArrays(GL_QUADS, 0, 24);


	//  ------------- Cúpulas-------------

	// 1
	lightingShader.setInt("material_diffuse", t_cafe);           //Texturas de la cúpula
	lightingShader.setInt("material_specular", t_cafe_brillo);

	model = glm::rotate(model_loc, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de Giro
	model = glm::translate(model, glm::vec3(15.0f, 0.0f, -3.5f));                           //Ubicación de la cúpula
	model = glm::rotate(model, glm::radians(-giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));      //Rotación inversa para preservar la verticalidad
	model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f));                            //Offset hacia abajo
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));                                 //Escala de la cúpula
	cupula(model, lightingShader);                                                          //Se dibuja la cúpula

	// 2
	lightingShader.setInt("material_diffuse", t_morado);          //Texturas de la cúpula
	lightingShader.setInt("material_specular", t_morado_brillo);

	model = glm::rotate(model_loc, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));   //Animación de Giro
	model = glm::translate(model, glm::vec3(-15.0f, 0.0f, -3.5f));                          //Ubicación de la cúpula
	model = glm::rotate(model, glm::radians(-giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));      //Rotación inversa para preservar la verticalidad
	model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f));                            //Offset hacia abajo
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));                                 //Escala de la cúpula
	cupula(model, lightingShader);                                                          //Se dibuja la cúpula

	// 3
	lightingShader.setInt("material_diffuse", t_azul_rey);       //Texturas de la cúpula
	lightingShader.setInt("material_specular", t_azul_rey);

	model = glm::rotate(model_loc, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));  //Animación de Giro
	model = glm::translate(model, glm::vec3(0.0f, 15.0f, -3.5f));                          //Ubicación de la cúpula
	model = glm::rotate(model, glm::radians(-giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));     //Rotación inversa para preservar la verticalidad
	model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f));                           //Offset hacia abajo
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));                                //Escala de la cúpula
	cupula(model, lightingShader);                                                         //Se dibuja la cúpula

	// 4
	lightingShader.setInt("material_diffuse", t_rojo);          //Texturas de la cúpula
	lightingShader.setInt("material_specular", t_rojo_brillo);

	model = glm::rotate(model_loc, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));  //Animación de Giro
	model = glm::translate(model, glm::vec3(0.0f, -15.0f, -3.5f));                         //Ubicación de la cúpula
	model = glm::rotate(model, glm::radians(-giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));     //Rotación inversa para preservar la verticalidad
	model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f));                           //Offset hacia abajo
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));                                //Escala de la cúpula
	cupula(model, lightingShader);                                                         //Se dibuja la cúpula

	// 5
	lightingShader.setInt("material_diffuse", t_rosa);          //Texturas de la cúpula
	lightingShader.setInt("material_specular", t_rosa_brillo);

	model = glm::rotate(model_loc, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));  //Animación de Giro
	model = glm::translate(model, glm::vec3(10.675f, 10.675f, -3.5f));                     //Ubicación de la cúpula
	model = glm::rotate(model, glm::radians(-giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));     //Rotación inversa para preservar la verticalidad
	model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f));                           //Offset hacia abajo
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));                                //Escala de la cúpula
	cupula(model, lightingShader);                                                         //Se dibuja la cúpula

	// 6
	lightingShader.setInt("material_diffuse", t_naranja);         //Texturas de la cúpula
	lightingShader.setInt("material_specular", t_naranja_brillo);

	model = glm::rotate(model_loc, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));  //Animación de Giro
	model = glm::translate(model, glm::vec3(-10.675f, 10.675f, -3.5f));                    //Ubicación de la cúpula
	model = glm::rotate(model, glm::radians(-giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));     //Rotación inversa para preservar la verticalidad
	model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f));                           //Offset hacia abajo
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));                                //Escala de la cúpula
	cupula(model, lightingShader);                                                         //Se dibuja la cúpula

	// 7
	lightingShader.setInt("material_diffuse", t_amarillo);        //Texturas de la cúpula
	lightingShader.setInt("material_specular", t_amarillo_brillo);
	 
	model = glm::rotate(model_loc, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));  //Animación de Giro
	model = glm::translate(model, glm::vec3(10.675f, -10.675f, -3.5f));                    //Ubicación de la cúpula
	model = glm::rotate(model, glm::radians(-giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));     //Rotación inversa para preservar la verticalidad
	model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f));                           //Offset hacia abajo
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));                                //Escala de la cúpula
	cupula(model, lightingShader);                                                         //Se dibuja la cúpula

	// 8
	lightingShader.setInt("material_diffuse", t_verde);          //Texturas de la cúpula
	lightingShader.setInt("material_specular", t_verde_brillo);

	model = glm::rotate(model_loc, glm::radians(giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));  //Animación de Giro
	model = glm::translate(model, glm::vec3(-10.675f, -10.675f, -3.5f));                   //Ubicación de la cúpula
	model = glm::rotate(model, glm::radians(-giroRueda), glm::vec3(0.0f, 0.0f, 1.0f));     //Rotación inversa para preservar la verticalidad
	model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f));                           //Offset hacia abajo
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));                                //Escala de la cúpula
	cupula(model, lightingShader);                                                         //Se dibuja la cúpula
}

void cupula(glm::mat4 model_loc, Shader lightingShader)
{
	glm::mat4 model;  //Matriz para ir dibujando cada elemento


	// ------------- Techo -------------
	// Barrera
	model = glm::translate(model_loc, glm::vec3(0.0f, 2.5f, 0.0f));
	model = glm::scale(model, glm::vec3(6.75f, 0.3f, 6.75f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Adorno 1
	model = glm::translate(model_loc, glm::vec3(0.0f, 2.625f, 0.0f));
	model = glm::scale(model, glm::vec3(4.5f, 0.5f, 4.5f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Adorno 2
	model = glm::translate(model_loc, glm::vec3(0.0f, 2.75f, 0.0f));
	model = glm::scale(model, glm::vec3(2.5f, 0.5f, 2.5f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);
	

	// ------------- Contorno -------------
	// Barrera
	model = glm::translate(model_loc, glm::vec3(0.0f, -0.625f, 0.0f));              //Ubicación del anillo
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));   //El anillo va acostado
	model = glm::scale(model, glm::vec3(0.2f, 0.2f, 1.75f));                        //Escala del anillo
	anillo(model, lightingShader);                                                  //Se dibuja el anillo

	// Borde
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.05f, 0.0f));                //Ubicación del anillo
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));   //El anillo va acostado
	model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.225f));                       //Escala del anillo
	anillo(model, lightingShader);                                                  //Se dibuja el anillo


	//  ------------- Sillas -------------
	// Izquierda
	model = glm::translate(model_loc, glm::vec3(0.0f, -0.25f, 0.55f));    //Ubicación de la silla sobre la cúpula
	model = glm::scale(model, glm::vec3(0.425f));                         //Escalado de la silla
	silla(model, 0, 0, lightingShader);                                   //Se dibuja la silla. -1: Guarda Derecha  1: Guarda Izquierda

	// Derecha
	model = glm::translate(model_loc, glm::vec3(0.0f, -0.25f, -0.55f));   //Ubicación de la silla sobre la cúpula
	model = glm::scale(model, glm::vec3(0.425f));                         //Escalado de la silla
	silla(model, 0, 0, lightingShader);                                   //Se dibuja la silla. -1: Guarda Derecha  1: Guarda Izquierda


	//  ------------- Base -------------
	// Piso
	model = glm::translate(model_loc, glm::vec3(0.0f, -1.25f, 0.0f));
	model = glm::scale(model, glm::vec3(6.75f, 0.3f, 6.75f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Adorno
	model = glm::translate(model_loc, glm::vec3(0.0f, -1.5f, 0.0f));
	model = glm::scale(model, glm::vec3(4.5f, 0.5f, 4.5f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);


	//  ------------- Uniones del Techo con la Base -------------

	lightingShader.setInt("material_diffuse", t_metal);          //Textura de las uniones
	lightingShader.setInt("material_specular", t_metal_brillo);

	// --- Barras Verticales ---
	
	// Unión Frontal Izquierda
	model = glm::translate(model_loc, glm::vec3(-1.25f, 1.25f, 2.75f));
	model = glm::scale(model, glm::vec3(0.15f, 2.5f, 0.15f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Unión Frontal Derecha
	model = glm::translate(model_loc, glm::vec3(-1.25f, 1.25f, -2.75f));
	model = glm::scale(model, glm::vec3(0.15f, 2.5f, 0.15f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Unión Trasera Izquierda
	model = glm::translate(model_loc, glm::vec3(1.25f, 1.25f, 2.75f));
	model = glm::scale(model, glm::vec3(0.15f, 2.5f, 0.15f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Unión Trasera Derecha
	model = glm::translate(model_loc, glm::vec3(1.25f, 1.25f, -2.75f));
	model = glm::scale(model, glm::vec3(0.15f, 2.5f, 0.15f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Remaches ---

	// Remache Frontal Izquierdo
	model = glm::translate(model_loc, glm::vec3(-1.25f, 2.5f, 2.75f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.25f, 0.15f, 0.25f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Remache Frontal Derecho
	model = glm::translate(model_loc, glm::vec3(-1.25f, 2.5f, -2.75f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(-70.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.25f, 0.15f, 0.25f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Remache Trasero Izquierdo
	model = glm::translate(model_loc, glm::vec3(1.25f, 2.5f, 2.75f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(-70.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.25f, 0.15f, 0.25f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Remache Trasero Derecho
	model = glm::translate(model_loc, glm::vec3(1.25f, 2.5f, -2.75f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.25f, 0.15f, 0.25f));

	lightingShader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);	
}

void anillo(glm::mat4 model_loc, Shader shader)
{
	glm::mat4 model;  //Matriz para ir dibujando cada elemento

	// ------------ Partes del Anillo ------------
	// 1
	model = glm::translate(model_loc, glm::vec3(0.0f, 15.0f, 0.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 6.25f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 2
	model = glm::translate(model_loc, glm::vec3(0.0f, -15.0f, 0.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 6.25f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 3
	model = glm::translate(model_loc, glm::vec3(15.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 6.25f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 4
	model = glm::translate(model_loc, glm::vec3(-15.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 6.25f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 5
	model = glm::translate(model_loc, glm::vec3(10.675f, 10.675f, 0.0f));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 5.90f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 6
	model = glm::translate(model_loc, glm::vec3(-10.675f, -10.675f, 0.0f));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 5.90f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 7
	model = glm::translate(model_loc, glm::vec3(10.675f, -10.675f, 0.0f));
	model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 5.90f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 8
	model = glm::translate(model_loc, glm::vec3(-10.675f, 10.675f, 0.0f));
	model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 5.90f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 9
	model = glm::translate(model_loc, glm::vec3(5.85f, 13.85f, 0.0f));
	model = glm::rotate(model, glm::radians(67.75f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 6.25f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 10
	model = glm::translate(model_loc, glm::vec3(-5.85f, -13.85f, 0.0f));
	model = glm::rotate(model, glm::radians(67.75f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 6.25f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 11
	model = glm::translate(model_loc, glm::vec3(5.85f, -13.85f, 0.0f));
	model = glm::rotate(model, glm::radians(-67.75f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 6.25f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 12
	model = glm::translate(model_loc, glm::vec3(-5.85f, 13.85f, 0.0f));
	model = glm::rotate(model, glm::radians(-67.75f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 6.25f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 13
	model = glm::translate(model_loc, glm::vec3(13.85f, 5.85f, 0.0f));
	model = glm::rotate(model, glm::radians(22.25f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 6.25f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 14
	model = glm::translate(model_loc, glm::vec3(-13.85f, -5.85f, 0.0f));
	model = glm::rotate(model, glm::radians(22.25f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 6.25f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 15
	model = glm::translate(model_loc, glm::vec3(13.85f, -5.85f, 0.0f));
	model = glm::rotate(model, glm::radians(-22.25f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 6.25f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// 16
	model = glm::translate(model_loc, glm::vec3(-13.85f, 5.85f, 0.0f));
	model = glm::rotate(model, glm::radians(-22.25f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 6.25f, 1.0f));

	shader.setMat4("model", model);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);
}