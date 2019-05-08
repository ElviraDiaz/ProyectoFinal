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

Con las siguientes teclas se cambia el tipo de iluminación:
	1: Luz Direccional (Directional)
	2: Luz Posicional (Positional)
	3: Luz de Reflector (Spotlight)

Con las siguientes teclas se modifica el color de la luz:
	R: La luz se vuelve más roja
	G: La luz se vuelve más verde
	B: La luz se vuelve más azul

	H: La luz se vuelve completamente blanca
	T: La luz se apaga

Con las siguientes teclas se modifica la intensidad de la luz
	9: Disminuye la instensidad
	0: Aumenta la instensidad

---> NOTA IMPORTANTE: 
	Para modificar la intensidad de la luz y el color se debe presionar 
	varias veces la tecla para ir aumentando, si la tecla se deja presionada,
	ésta se tomará como un solo incremento.

Con el Mouse se puede mover la cámara desde la cual se observa el modelo
	Al mover el mouse a la derecha, la cámara gira a la derecha
	Al mover el mouse a la izquierda, la cámara gira a la izquierda
	Al mover el mouse hacia arriba, la cámara gira hacia arriba
	Al mover el mouse hacia abajo, la cámara gira hacia abajo
*/

//#define STB_IMAGE_IMPLEMENTATION
#include <glew.h>
#include <glfw3.h>
#include <stb_image.h>
#include <string.h>

#include "esfera.h"
#include "camera.h"
#include "Model.h"

// Other Libs
#include "SOIL2/SOIL2.h"

using namespace std;

Esfera esfera(1.0f);

//Prototipos de funciones básicas
void resize(GLFWwindow* window, int width, int height);
void my_input(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

// ------------- Prototipos de las funciones de renderizado ---------
void montañaRusa(glm::mat4 model_loc, Shader lightingShader);
void riel(glm::mat4 model_loc, float escala_x, float rotacion, Shader lightingShader);
void carro(glm::mat4 model_loc, Shader lightingShader);
void silla(glm::mat4 model_loc, int lado, Shader lightingShader);


// settings
// Window size
int SCR_WIDTH = 3800;
int SCR_HEIGHT = 7600;

GLFWmonitor *monitors;
GLuint VBO, VAO, EBO;

//Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
double	lastX = 0.0f,
		lastY = 0.0f;
bool firstMouse = true;

//Timing
double	deltaTime = 0.0f,
		lastFrame = 0.0f;

//Lighting                                     //Luz tipo Reflector
glm::vec3 lightPosition(4.0f, 20.0f, 8.0f);    //Posición de la Luz. Original: (0.0f, 3.0f, 0.0f)
glm::vec3 lightDirection(0.0f, 0.0f, -3.0f);   //Dirección de la Luz

void myData(void);
void display(Shader, Model);
void getResolution(void);
void animate(void);
void LoadTextures(void);
unsigned int generateTextures(char*, bool);

//For Keyboard
float	movX = 0.0f,
		movY = 0.0f,
		movZ = -5.0f,
		rotX = 0.0f;

// Para controlar la intensidad de la luz
int CeroPresionado = 0;
int NuevePresionado = 0;

// Para controlar el color de la luz
int RPresionado = 0;
int GPresionado = 0;
int BPresionado = 0;

//Para controlar la fuente de iluminación --> Posicional por defecto
char Light_VertxShader[100] = "shaders/shader_texture_light_pos.vs";
char Light_FragShader[100] = "shaders/shader_texture_light_pos.fs";

//La fuente de luz se encuentra en la ubicación del sol o foco
glm::vec3 Light_Position = lightPosition;

//Estas variables son para cuando se usa luz de tipo reflector (Linterna)
// Esta define el tamaño del círculo de luz. Aumentar su valor para que el círculo sea más grande
float Light_CutOff = glm::cos(glm::radians(12.5f));
// Esta define la dirección de la luz en base a la posición de la cámara
glm::vec3 Light_Direction = camera.Front;



//-------- Texturas --------

//Difusas
unsigned int t_metal;
unsigned int t_amarillo;
unsigned int t_azul;

unsigned int t_piedra;
unsigned int t_terracota;

unsigned int t_negro;

//Especulares
unsigned int t_metal_brillo;
unsigned int t_amarillo_brillo;
unsigned int t_azul_brillo;

unsigned int t_piedra_brillo;
unsigned int t_terracota_brillo;

// ------- Propiedades de la Luz --------
float opacidad = 32.0f;

float R = 1.0f;
float G = 1.0f;
float B = 1.0f;

int Luz_Reflector = 0;  //Por defecto, la luz es de tipo Posicional, no de Reflector

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

void getResolution()
{
	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	SCR_WIDTH = mode->width;
	SCR_HEIGHT = (mode->height) - 80;

	lastX = SCR_WIDTH / 2.0f;
	lastY = SCR_HEIGHT / 2.0f;
}

//Valores a colocar si la extensión de la textura es:
// .jpg ----> 0
// .png ----> 1
void LoadTextures()
{
	//Texturas para primitivas
	t_metal = generateTextures("Texturas/Metal.png", 1);
	t_metal_brillo = generateTextures("Texturas/Metal_Especular.png", 1);

	t_amarillo = generateTextures("Texturas/Amarillo.png", 1);
	t_amarillo_brillo = generateTextures("Texturas/Amarillo_Especular.png", 1);

	t_azul = generateTextures("Texturas/Azul.png", 1);
	t_azul_brillo = generateTextures("Texturas/Azul_Especular.png", 1);

	t_piedra = generateTextures("Texturas/Piedra.png", 1);
	t_piedra_brillo = generateTextures("Texturas/Piedra_Especular.png", 1);

	t_terracota = generateTextures("Texturas/Terracota.png", 1);
	t_terracota_brillo = generateTextures("Texturas/Terracota_Especular.png", 1);

	t_negro = generateTextures("Texturas/Negro.png", 1);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, t_metal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, t_metal_brillo);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, t_amarillo);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, t_amarillo_brillo);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, t_azul);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, t_azul_brillo);
	
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, t_piedra);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, t_piedra_brillo);

	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, t_terracota);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, t_terracota_brillo);

	glActiveTexture(GL_TEXTURE11);
	glBindTexture(GL_TEXTURE_2D, t_negro);
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
		 0.5f,  0.25f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f,    // top right
		 0.5f, -0.3f,  -0.4f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f,    // bottom right
		-0.5f, -0.3f,  -0.4f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,    // bottom left
		-0.5f,  0.25f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f,    // top left 

		//Cara 2
		 0.5f,  0.45f, -0.4f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f,    // top right
		 0.5f,  0.25f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f,    // bottom right
		-0.5f,  0.25f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,    // bottom left
		-0.5f,  0.45f, -0.4f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f,    // top left 

		//Cara 3
		 0.5f,  0.45f, -0.35f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f,    // top right
	     0.5f,  0.45f, -0.4f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,    // bottom right
	    -0.5f,  0.45f, -0.4f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,    // bottom left
		-0.5f,  0.45f, -0.35f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,    // top left 

		//Cara 4
		 0.5f, -0.2f,  -0.3f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,    // top right
	     0.5f,  0.45f, -0.35f,  0.0f, 0.0f, 1.0f,   1.0f, 0.0f,    // bottom right
	    -0.5f,  0.45f, -0.35f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f,    // bottom left
		-0.5f, -0.2f,  -0.3f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,    // top left 

		//Cara 5
		 0.5f, -0.25f, -0.1f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,    // top right
	     0.5f, -0.2f,  -0.3f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,    // bottom right
	    -0.5f, -0.2f,  -0.3f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,    // bottom left
		-0.5f, -0.25f, -0.1f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,    // top left 

		//Cara 6
		 0.5f, -0.2f,   0.3f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,    // top right
	     0.5f, -0.25f, -0.1f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,    // bottom right
	    -0.5f, -0.25f, -0.1f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,    // bottom left
		-0.5f, -0.2f,   0.3f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,    // top left 

		//Cara 7
		 0.5f, -0.25f,  0.3f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,    // top right
	     0.5f, -0.2f,   0.3f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,    // bottom right
	    -0.5f, -0.2f,   0.3f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,    // bottom left
		-0.5f, -0.25f,  0.3f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,    // top left 

		//Cara 8
		 0.5f, -0.4f,   0.2f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,    // top right
	     0.5f, -0.25f,  0.3f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,    // bottom right
	    -0.5f, -0.25f,  0.3f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,    // bottom left
		-0.5f, -0.4f,   0.2f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,    // top left 

		//Cara 9
		 0.5f, -0.5f,  -0.2f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f,    // top right
	     0.5f, -0.4f,   0.2f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,    // bottom right
	    -0.5f, -0.4f,   0.2f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f,    // bottom left
		-0.5f, -0.5f,  -0.2f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,    // top left 

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
}

void display(Shader modelShader, Model batarang)
{
	//Shader projectionShader("shaders/shader_light.vs", "shaders/shader_light.fs");
	//Shader projectionShader("shaders/shader_texture_color.vs", "shaders/shader_texture_color.fs");

	// ------ Aquí se selecciona el tipo de luz a utilizar, descomentar la línea que se va a ocupar -----

	//Shader lightingShader("shaders/shader_texture_light_dir.vs", "shaders/shader_texture_light_dir.fs");    //Directional
	//Shader lightingShader("shaders/shader_texture_light_pos.vs", "shaders/shader_texture_light_pos.fs");    //Positional
	//Shader lightingShader("shaders/shader_texture_light_spot.vs", "shaders/shader_texture_light_spot.fs");  //Spotlight

	Shader lightingShader(Light_VertxShader, Light_FragShader);
	Shader lampShader("shaders/shader_lamp.vs", "shaders/shader_lamp.fs");

	//To Use Lighting
	lightingShader.use();

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
	lightingShader.setFloat("light.quadratic", 0.032f);

	// Brillo del material
	lightingShader.setFloat("material_shininess", opacidad);

	//Matriz de modelo, vista t proyección
	glm::mat4 model = glm::mat4(1.0f);		//Initialize Matrix, Use this matrix for individual models
	glm::mat4 model_temp = glm::mat4(1.0f); //Maztriz auxiliar para la ubicación de cada elemento del modelo
	glm::mat4 model_loc = glm::mat4(1.0f);  //Matriz axuliar para la ubicación absoluta de todo el modelo

	glm::mat4 view = glm::mat4(1.0f);		//Use this matrix for ALL models
	glm::mat4 projection = glm::mat4(1.0f);	//This matrix is for Projection

	//Use "projection" to include Camera
	projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	view = camera.GetViewMatrix();

	// Se pasan las matrices al shader
	//lightingShader.setVec3("viewPos", camera.Position);
	lightingShader.setMat4("model", model);
	lightingShader.setMat4("view", view);
	lightingShader.setMat4("projection", projection);

	// Mientras la Luz sea de tipo reflector, ésta se estará actualizando constantemente en base a la posición de la cámara
	if (Luz_Reflector == 1)
	{
		Light_Position = camera.Position;
		Light_Direction = camera.Front;
		Light_CutOff = glm::cos(glm::radians(12.5f));
	}

	// ------------------------- Zona de Dibujo -------------------------

	glBindVertexArray(VAO);
	
	
	// ------------ Montaña Rusa  ------------

	model_loc = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)); //Ubicación de la montaña rusa
	model_loc = glm::scale(model_loc, glm::vec3(1.0f, 1.0f, 1.0f));           //Escala de toda la montaña
	montañaRusa(model_loc, lightingShader);                                   //Se dibuja la montaña

	// ------------- Luz ---------------
	lampShader.use();
	
	model = glm::mat4(1.0f);
	model = glm::translate(model, lightPosition);
	model = glm::scale(model, glm::vec3(1.5f));

	lampShader.setMat4("model", model);
	lampShader.setMat4("view", view);
	lampShader.setMat4("projection", projection);
	//glBindVertexArray(lightVAO);
	esfera.render();    //Sol redondo


	// ------------ Batarang ------------

	// ---- Fondo ----
	lightingShader.use();
	lightingShader.setMat4("view", view);
	lightingShader.setMat4("projection", projection);

	model = glm::translate(model_loc, glm::vec3(2.4f, 6.4f, 0.5f));
	model = glm::scale(model, glm::vec3(3.0f, 1.5f, 0.25f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_amarillo);
	lightingShader.setInt("material_specular", t_amarillo_brillo);
	esfera.render();

	model = glm::translate(model_loc, glm::vec3(2.4f, 6.4f, 0.5f));
	model = glm::scale(model, glm::vec3(3.15f, 1.65f, 0.15f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_negro);
	//lightingShader.setInt("material_specular", t_amarillo_brillo);
	esfera.render();


	// ---- Logo ----
	modelShader.use();		
	model = glm::translate(model_loc, glm::vec3(2.4f, 6.4f, 0.75f));
	model = glm::scale(model, glm::vec3(17.0f));
	modelShader.setMat4("model", model);
	modelShader.setMat4("view", view);
	modelShader.setMat4("projection", projection);
	batarang.Draw(modelShader);

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

	//Se crea el chader para cargar los modelos
	Shader modelShader("Shaders/modelLoading.vs", "Shaders/modelLoading.fs");
	
	//Se cargan los modelos a utilizar
	//Model batman = ((char *)"Models/Batarang/INJ_iOS_WEAPON_Batman's_Batarang_Dawn_Of_Justice.obj");
	Model batarang = ((char *)"Models/Batarang/INJ_iOS_WEAPON_Batman's_Batarang_Dawn_Of_Justice.obj");

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
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
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
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, (float)deltaTime * 5);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, (float)deltaTime * 5);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, (float)deltaTime * 5);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, (float)deltaTime * 5);


	// --------------- Tipo de Iluminación

	// Luz Posicional
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	{
		strcpy_s(Light_VertxShader, "shaders/shader_texture_light_pos.vs");
		strcpy_s(Light_FragShader, "shaders/shader_texture_light_pos.fs");

		Light_Position = lightPosition;

		Luz_Reflector = 0;  //La luz ya no es de tipo reflector
	}

	// Luz de Reflector
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	{
		strcpy_s(Light_VertxShader, "shaders/shader_texture_light_spot.vs");
		strcpy_s(Light_FragShader, "shaders/shader_texture_light_spot.fs");

		Luz_Reflector = 1;  //La luz es de tipo reflector
	}

	// Luz Direccional
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
	{
		strcpy_s(Light_VertxShader, "shaders/shader_texture_light_dir.vs");
		strcpy_s(Light_FragShader, "shaders/shader_texture_light_dir.fs");
		Light_Direction = lightDirection;

		Luz_Reflector = 0;   //La luz ya no es de tipo reflector
	}


	// --------------- Intensidad de la luz
	
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


	// ------- Color de la luz

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


void montañaRusa(glm::mat4 model_loc, Shader lightingShader)
{
	glm::mat4 model;  //Matriz para ir dibujando cada elemento

	//  ------------- Estructura Tubos -------------

	// --- Base 1 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(-9.7f, -2.05f, 0.0f));
	model = glm::rotate(model, glm::radians(24.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 18.0f, 1.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(-9.7f, -2.05f, -8.0f));
	model = glm::rotate(model, glm::radians(24.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 18.0f, 1.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// Horizontal Superior
	model = glm::translate(model_loc, glm::vec3(-13.4f, 6.4f, -4.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 8.90, 1.10f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Base 2 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(-5.5f, -0.5f, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 20.25f, 1.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(-5.5f, -0.5f, -8.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 20.25f, 1.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// Horizontal Superior
	model = glm::translate(model_loc, glm::vec3(-5.5f, 9.75f, -4.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 8.90, 1.10f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Horizontal Inferior
	model = glm::translate(model_loc, glm::vec3(-5.5f, 2.25f, -4.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.90f, 8.0f, 0.90f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Base 3 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(-1.25f, -2.05f, 0.0f));
	model = glm::rotate(model, glm::radians(-24.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 18.0f, 1.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(-1.25f, -2.05f, -8.0f));
	model = glm::rotate(model, glm::radians(-24.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 18.0f, 1.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// Horizontal Superior
	model = glm::translate(model_loc, glm::vec3(2.4f, 6.4f, -4.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 8.90, 1.10f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Base 4 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(3.0f, -6.0, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 9.0f, 1.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(3.0f, -6.0f, -8.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 9.0f, 1.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// Horizontal Superior
	model = glm::translate(model_loc, glm::vec3(3.0f, -1.5f, -4.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 8.90, 1.10f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Horizontal Inferior
	model = glm::translate(model_loc, glm::vec3(3.0f, -6.0f, -4.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.90f, 8.0f, 0.90f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Base 5 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(6.0f, -2.05f, 0.0f));
	model = glm::rotate(model, glm::radians(24.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 18.0f, 1.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(6.0f, -2.05f, -8.0f));
	model = glm::rotate(model, glm::radians(24.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 18.0f, 1.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// Horizontal Inferior
	model = glm::translate(model_loc, glm::vec3(6.90f, -4.25f, -4.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.75f, 8.0f, 0.75f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Base 6 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(10.0f, -0.5f, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 20.25f, 1.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(10.0f, -0.5f, -8.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 20.25f, 1.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// Horizontal Superior
	model = glm::translate(model_loc, glm::vec3(10.0f, 9.75f, -4.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 8.90, 1.10f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Horizontal Intermedio
	model = glm::translate(model_loc, glm::vec3(10.0f, 3.15f, -4.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.90f, 8.0f, 0.90f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Horizontal Inferior
	model = glm::translate(model_loc, glm::vec3(10.0f, -4.90f, -4.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.90f, 8.0f, 0.90f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Base 7 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(15.25f, 2.5f, 0.0f));
	model = glm::rotate(model, glm::radians(-19.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 27.0f, 1.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(15.25f, 2.5f, -8.0f));
	model = glm::rotate(model, glm::radians(-19.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 27.0f, 1.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// Horizontal Superior
	model = glm::translate(model_loc, glm::vec3(19.7f, 15.5f, -4.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 8.90, 1.10f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Horizontal Intermedio
	model = glm::translate(model_loc, glm::vec3(16.85f, 7.10f, -4.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.75f, 8.0f, 0.75f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Horizontal Inferior
	model = glm::translate(model_loc, glm::vec3(12.75f, -4.90f, -4.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.90f, 8.0f, 0.90f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Base 8 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(22.0f, -3.125f, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 14.75f, 1.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(22.0f, -3.125f, -8.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 14.75f, 1.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// Horizontal Superior
	model = glm::translate(model_loc, glm::vec3(22.0f, 4.25f, -4.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.10f, 8.90, 1.10f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Horizontal Inferior
	model = glm::translate(model_loc, glm::vec3(22.0f, -2.25f, -4.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.90f, 8.0f, 0.90f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);


	//  ------------- Estructura Uniones -------------

	// --- Unión Base 1, 2 y 3 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(-5.5f, -10.5f, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(3.0f, 1.0f, 2.5f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(-5.5f, -10.5f, -8.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(3.0f, 1.0f, 2.5f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Unión Base 4 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(3.0f, -10.5f, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.5f, 1.0f, 1.5f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(3.0f, -10.5f, -8.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.5f, 1.0f, 1.5f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Unión Base 5, 6 y 7 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(10.0f, -10.5f, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(3.5f, 1.0f, 3.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(10.0f, -10.5f, -8.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(3.5f, 1.0f, 3.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Unión Base 8 ---
	// Frontal
	model = glm::translate(model_loc, glm::vec3(22.0f, -10.5f, 0.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.5f, 1.0f, 1.5f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// Trasera
	model = glm::translate(model_loc, glm::vec3(22.0f, -10.5f, -8.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.5f, 1.0f, 1.5f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);


	//  ------------- Estructura Rieles -------------

	glm::mat4 temp = glm::mat4(1.0f);

	// Riel 1
	model = glm::translate(model_loc, glm::vec3(19.75f, 16.5f, -4.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 2
	model = glm::translate(model_loc, glm::vec3(14.875f, 13.625f, -4.0f)); //Ubicación del Riel
	riel(model, 11.0f, 32.65f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 3
	model = glm::translate(model_loc, glm::vec3(10.0f, 10.75f, -4.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 4
	model = glm::translate(model_loc, glm::vec3(8.6f, 11.625f, -4.0f)); //Ubicación del Riel
	riel(model, 3.2f, -38.5f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 5
	model = glm::translate(model_loc, glm::vec3(7.2f, 12.5f, -4.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 6
	model = glm::translate(model_loc, glm::vec3(4.8f, 9.95f, -4.0f)); //Ubicación del Riel
	riel(model, 7.15f, 50.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 7
	model = glm::translate(model_loc, glm::vec3(2.4f, 7.4f, -4.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 8
	model = glm::translate(model_loc, glm::vec3(0.0f, 9.95f, -4.0f)); //Ubicación del Riel
	riel(model, 7.15f, -50.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 9
	model = glm::translate(model_loc, glm::vec3(-2.4f, 12.5f, -4.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 10
	model = glm::translate(model_loc, glm::vec3(-3.95f, 11.625f, -4.0f)); //Ubicación del Riel
	riel(model, 3.35f, 36.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 11
	model = glm::translate(model_loc, glm::vec3(-5.5f, 10.75f, -4.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 12
	model = glm::translate(model_loc, glm::vec3(-8.25f, 11.25, -4.0f)); //Ubicación del Riel
	riel(model, 4.85f, -12.40f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 13
	model = glm::translate(model_loc, glm::vec3(-11.0f, 11.75, -4.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 14
	model = glm::translate(model_loc, glm::vec3(-12.80f, 9.35f, -4.0f)); //Ubicación del Riel
	riel(model, 6.3f, 56.7f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 15
	model = glm::translate(model_loc, glm::vec3(-14.45f, 6.3f, -4.0f)); //Ubicación del Riel
	riel(model, 1.5f, 90.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z
	
	// Riel 16
	model = glm::translate(model_loc, glm::vec3(-11.35f, 2.90f, -4.0f)); //Ubicación del Riel
	riel(model, 8.90f, -43.65f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 17
	model = glm::translate(model_loc, glm::vec3(-8.0f, 0.0f, -4.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 18
	model = glm::translate(model_loc, glm::vec3(-2.8f, 2.7f, -4.0f)); //Ubicación del Riel
	riel(model, 11.35f, 29.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 19
	model = glm::translate(model_loc, glm::vec3(2.4f, 5.4f, -4.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 20
	model = glm::translate(model_loc, glm::vec3(6.2f, 3.825f, -4.0f)); //Ubicación del Riel
	riel(model, 7.75f, -25.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 21
	model = glm::translate(model_loc, glm::vec3(10.0f, 2.25f, -4.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 22
	model = glm::translate(model_loc, glm::vec3(13.25f, 4.25f, -4.0f)); //Ubicación del Riel
	riel(model, 7.40f, 34.50f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 23
	model = glm::translate(model_loc, glm::vec3(16.5f, 6.25f, -4.0f)); //Ubicación del Riel
	riel(model, 1.0f, 0.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 24 
	model = glm::translate(model_loc, glm::vec3(19.06f, 5.375f, -4.0f)); //Ubicación del Riel
	riel(model, 4.90f, -22.25f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 25
	model = glm::translate(model_loc, glm::vec3(21.0f, 1.125f, -4.0f)); //Ubicación del Riel
	riel(model, 7.5f, 90.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 26
	model = glm::translate(model_loc, glm::vec3(20.05f, -3.175f, -4.0f)); //Ubicación del Riel
	riel(model, 2.95f, 40.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 27
	model = glm::translate(model_loc, glm::vec3(13.5f, -4.0f, -4.0f)); //Ubicación del Riel
	riel(model, 11.5f, 0.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	//Riel 28 
	model = glm::translate(model_loc, glm::vec3(5.75f, -2.25f, -4.0f)); //Ubicación del Riel
	riel(model, 6.025f, -38.5f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	//Riel 29
	model = glm::translate(model_loc, glm::vec3(2.5f, -0.5f, -4.0f)); //Ubicación del Riel
	riel(model, 2.5f, 0.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 30
	model = glm::translate(model_loc, glm::vec3(-0.40f, -1.225f, -4.0f)); //Ubicación del Riel
	riel(model, 4.0f, 22.5f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	//Riel 31
	model = glm::translate(model_loc, glm::vec3(-1.95f, -3.5f, -4.0f)); //Ubicación del Riel
	riel(model, 4.0f, 90.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 32
	model = glm::translate(model_loc, glm::vec3(0.675f, -6.0f, -4.0f)); //Ubicación del Riel
	riel(model, 6.25f, -18.5f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 33
	model = glm::translate(model_loc, glm::vec3(12.5f, -7.0f, -4.0f)); //Ubicación del Riel
	riel(model, 18.0f, 0.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 34
	model = glm::translate(model_loc, glm::vec3(22.10f, -5.95f, -4.0f)); //Ubicación del Riel
	riel(model, 3.20f, 52.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z

	// Riel 35
	model = glm::translate(model_loc, glm::vec3(23.0f, 4.5f, -4.0f)); //Ubicación del Riel
	riel(model, 19.0f, 90.0f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z
	
	// Riel 36
	model = glm::translate(model_loc, glm::vec3(21.55f, 15.15f, -4.0f)); //Ubicación del Riel
	riel(model, 4.425f, -42.5f, lightingShader);   //Modelo de referencia, Escala en X, Rotación en Z


	// ------------- Piso -------------
	model = glm::translate(model_loc, glm::vec3(4.5f, -11.5f, -4.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	model = glm::scale(model, glm::vec3(40.0f, 1.0f, 14.0f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_terracota);
	lightingShader.setInt("material_specular", t_terracota_brillo);
	glDrawArrays(GL_QUADS, 0, 24);

	glm::mat4 ubicacion_carro = glm::mat4(1.0f);

	// ------------- Carro -------------
	model = glm::translate(model_loc, glm::vec3(19.75f, 17.5f, -4.0f));           //Se parte de Riel 1
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));  //Rotación en X
	ubicacion_carro = model;                                                      //Obtenemos las coordenadas del carro a lo largo del recorrido
	//model = glm::scale(model, glm::vec3(40.0f, 1.0f, 14.0f));                   //Escalado del carro
	carro(model, lightingShader);                                                 //Se manda a dibujar todo el carro

	// ------------ Sillas  ------------
	// Frontal Izquierda
	model = glm::translate(ubicacion_carro, glm::vec3(-1.25f, 0.0f, 2.325f));        //Ubicación de la silla sobre el carro
	//model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));   //Rotación en X
	model = glm::scale(model, glm::vec3(0.40f));                                     //Escalado de la silla
	silla(model, -1, lightingShader);                                                //Se dibuja la silla. -1: Guarda Derecha  1: Guarda Izquierda

	// Frontal Derecha
	model = glm::translate(ubicacion_carro, glm::vec3(-1.25f, 0.0f, -2.325f));       //Ubicación de la silla sobre el carro
	//model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));   //Rotación en X
	model = glm::scale(model, glm::vec3(0.40f));                                     //Escalado de la silla
	silla(model, 1, lightingShader);                                                 //Se dibuja la silla. -1: Guarda Derecha  1: Guarda Izquierda

	// Trasera Izquierda
	model = glm::translate(ubicacion_carro, glm::vec3(1.25f, 0.0f, 2.325f));         //Ubicación de la silla sobre el carro
	//model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));   //Rotación en X
	model = glm::scale(model, glm::vec3(0.40f));                                     //Escalado de la silla
	silla(model, -1, lightingShader);                                                //Se dibuja la silla. -1: Guarda Derecha  1: Guarda Izquierda

	// Trasera Izquierda
	model = glm::translate(ubicacion_carro, glm::vec3(1.25f, 0.0f, -2.325f));        //Ubicación de la silla sobre el carro
	//model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));   //Rotación en X
	model = glm::scale(model, glm::vec3(0.40f));                                     //Escalado de la silla
	silla(model, 1, lightingShader);                                                 //Se dibuja la silla. -1: Guarda Derecha  1: Guarda Izquierda
}

void riel(glm::mat4 model_loc, float escala_x, float rotacion, Shader lightingShader)
{
	glm::mat4 model;

	//Se aplica la rotación a todo el riel
	model_loc = glm::rotate(model_loc, glm::radians(rotacion), glm::vec3(0.0f, 0.0f, 1.0f));

	// ------------- Riel Frontal -------------
	// Centro
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(escala_x, 1.0f, 0.25f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_amarillo);
	lightingShader.setInt("material_specular", t_amarillo_brillo);
	glDrawArrays(GL_QUADS, 0, 24);

	// Superior
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.5f, 1.0f));
	model = glm::scale(model, glm::vec3(escala_x, 0.05f, 0.5f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_amarillo);
	lightingShader.setInt("material_specular", t_amarillo_brillo);
	glDrawArrays(GL_QUADS, 0, 24);

	//Inferior
	model = glm::translate(model_loc, glm::vec3(0.0f, -0.5f, 1.0f));
	model = glm::scale(model, glm::vec3(escala_x, 0.05f, 0.5f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_amarillo);
	lightingShader.setInt("material_specular", t_amarillo_brillo);
	glDrawArrays(GL_QUADS, 0, 24);

	// ------------- Riel Posterior -------------
	// Centro
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, -1.0f));
	model = glm::scale(model, glm::vec3(escala_x, 1.0f, 0.25f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_amarillo);
	lightingShader.setInt("material_specular", t_amarillo_brillo);
	glDrawArrays(GL_QUADS, 0, 24);

	// Superior
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.5f, -1.0f));
	model = glm::scale(model, glm::vec3(escala_x, 0.05f, 0.5f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_amarillo);
	lightingShader.setInt("material_specular", t_amarillo_brillo);
	glDrawArrays(GL_QUADS, 0, 24);

	//Inferior
	model = glm::translate(model_loc, glm::vec3(0.0f, -0.5f, -1.0f));
	model = glm::scale(model, glm::vec3(escala_x, 0.05f, 0.5f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_amarillo);
	lightingShader.setInt("material_specular", t_amarillo_brillo);
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
	lightingShader.setInt("material_diffuse", t_azul);
	lightingShader.setInt("material_specular", t_azul_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// --- Tubo Frontal ---
	model = glm::translate(model_loc, glm::vec3(-1.25f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f, 3.25f, 1.0f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_azul);
	lightingShader.setInt("material_specular", t_azul_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Anillo Frontal del Centro ---
	model = glm::translate(model_loc, glm::vec3(-1.25f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.25f, 0.25f, 1.25f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_azul);
	lightingShader.setInt("material_specular", t_azul_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Anillo Frontal Izquierdo ---
	model = glm::translate(model_loc, glm::vec3(-1.25f, 0.0f, 1.35f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.25f, 0.25f, 1.25f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_azul);
	lightingShader.setInt("material_specular", t_azul_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Anillo Frontal Derecho ---
	model = glm::translate(model_loc, glm::vec3(-1.25f, 0.0f, -1.35f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.25f, 0.25f, 1.25f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_azul);
	lightingShader.setInt("material_specular", t_azul_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Unión Frontal Izquierda ---
	model = glm::translate(model_loc, glm::vec3(-0.85f, 0.0f, 0.30f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.95f, 1.0f, 0.95f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_azul);
	lightingShader.setInt("material_specular", t_azul_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// --- Unión Frontal Derecha ---
	model = glm::translate(model_loc, glm::vec3(-0.85f, 0.0f, -0.30f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.95f, 1.0f, 0.95f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_azul);
	lightingShader.setInt("material_specular", t_azul_brillo);
	glDrawArrays(GL_QUADS, 24, 48);


	// --- Tubo Posterior ---
	model = glm::translate(model_loc, glm::vec3(1.25f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f, 3.25f, 1.0f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_azul);
	lightingShader.setInt("material_specular", t_azul_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Anillo Posterior del Centro ---
	model = glm::translate(model_loc, glm::vec3(1.25f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.25f, 0.25f, 1.25f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_azul);
	lightingShader.setInt("material_specular", t_azul_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Anillo Posterior Izquierdo ---
	model = glm::translate(model_loc, glm::vec3(1.25f, 0.0f, 1.35f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.25f, 0.25f, 1.25f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_azul);
	lightingShader.setInt("material_specular", t_azul_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Anillo Posterior Derecho ---
	model = glm::translate(model_loc, glm::vec3(1.25f, 0.0f, -1.35f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.25f, 0.25f, 1.25f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_azul);
	lightingShader.setInt("material_specular", t_azul_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
	glDrawArrays(GL_POLYGON, 72, 12);
	glDrawArrays(GL_POLYGON, 84, 12);

	// --- Unión Posterior Izquierda ---
	model = glm::translate(model_loc, glm::vec3(0.85f, 0.0f, 0.30f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.95f, 1.0f, 0.95f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_azul);
	lightingShader.setInt("material_specular", t_azul_brillo);
	glDrawArrays(GL_QUADS, 24, 48);

	// --- Unión Posterior Derecha ---
	model = glm::translate(model_loc, glm::vec3(0.85f, 0.0f, -0.30f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.95f, 1.0f, 0.95f));
	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_azul);
	lightingShader.setInt("material_specular", t_azul_brillo);
	glDrawArrays(GL_QUADS, 24, 48);
}

void silla(glm::mat4 model_loc, int lado, Shader lightingShader)
{
	glm::mat4 model;

	model_loc = glm::rotate(model_loc, glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));;

	// ------------- Asiento -------------
	model = glm::translate(model_loc, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(2.5f, 5.0f, 5.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_azul);
	lightingShader.setInt("material_specular", t_azul_brillo);
	glDrawArrays(GL_QUADS, 96, 40);      //Asiento
	glDrawArrays(GL_POLYGON, 136, 10);   //Costado Derecho
	glDrawArrays(GL_POLYGON, 146, 10);   //Costado Izquierdo


	// ------------- Guarda ----------------
	model = glm::translate(model_loc, glm::vec3(lado * 1.5f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.45f, 5.0f, 5.0f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_metal);
	lightingShader.setInt("material_specular", t_metal_brillo);
	glDrawArrays(GL_QUADS, 156, 20);    //Contorno
	glDrawArrays(GL_POLYGON, 176, 5);   //Costado Derecho
	glDrawArrays(GL_POLYGON, 181, 5);   //Costado Izquierdo


	// ---------------- Tubos de Seguridad ----------------
	// --- Superior Derecho ---
	model = glm::translate(model_loc, glm::vec3(1.0f, 1.5f, -1.125f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.25f, 1.25f, 0.25f));

	lightingShader.setMat4("model", model);
	lightingShader.setInt("material_diffuse", t_amarillo);
	lightingShader.setInt("material_specular", t_amarillo_brillo);
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
	lightingShader.setInt("material_diffuse", t_amarillo);
	lightingShader.setInt("material_specular", t_amarillo_brillo);
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