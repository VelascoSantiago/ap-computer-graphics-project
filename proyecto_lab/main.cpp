#include <iostream>
#include <cmath>
#include <cstdio>
#include <vector>
#include <string>
#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include "SOIL2/SOIL2.h"

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow *window, double xPos, double yPos);
void DoMovement();
void Animation();

const GLuint WIDTH = 1280, HEIGHT = 720;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// ---------------------------------------------------------------------
//  AJUSTES de la escena
// ---------------------------------------------------------------------
const float PLAZA_ESCALA = 1.0f;
// Escalas iniciales (se cambian en vivo con Z / X sobre el grupo seleccionado)
float escGrupo[3] = { 0.35f, 0.35f, 0.35f };   // 0 CUAU, 1 SENTINEL, 2 MAC

// Angulo (grados) para que el CUAU mire hacia donde corre. Con 180 el modelo correria de espaldas.
const float CUAU_YAW_OFFSET = 0.0f;

// Boca del railgun en coordenadas locales del SENTINEL (OBJ). Se calibra observando el destello.
// (el railgun_aim.obj ocupa x -1.76..-0.53, y 11.7..13.82, z 1.2..8.85: la boca es el extremo +Z)
glm::vec3 bocaLocal(-1.15f, 12.8f, 8.9f);
const float RETROCESO_MAX = 0.8f;

// El MAC original esta centrado en x=16 y su base queda ~5.8 por debajo del origen
const glm::vec3 MAC_CENTRO(16.0f, 0.0f, 0.0f);
const float MAC_SUELO = 5.8f;
// Centros de las aspas dentro del OBJ (de aspas.txt): x=16, y, z=0
const float ASPA_Y[3] = { 0.30f, 2.85f, 5.40f };

// ---------------------------------------------------------------------
//  Estado de la escena
// ---------------------------------------------------------------------
glm::vec3 posCuau(0.0f, 0.0f, 0.0f);
glm::vec3 posSentinel(0.0f, 0.0f, -30.0f);   // centro del grupo de 3 SENTINEL
glm::vec3 posMac(12.0f, 0.0f, 6.0f);
float yawGrupo[3] = { 0.0f, 0.0f, 0.0f };   // giro manual en grados (Q / E)

// Fila de SENTINEL del relato (escena 4): a lo largo de z, como salen caminando
const glm::vec3 offLinea[3] = { glm::vec3(0.0f, 0.0f, -4.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 4.5f) };
const glm::vec3 offSentinel[3] = {
	glm::vec3(-11.0f, 0.0f, 0.0f),
	glm::vec3(  0.0f, 0.0f, -5.0f),
	glm::vec3( 11.0f, 0.0f, 0.0f)
};

bool animSierras = true;   // N
bool animScarface = true;  // B
bool animEsquive = true;   // V
bool modoRig = false;      // G: dibuja CUAU y SENTINEL por partes (jerarquia) en vez de las mallas de pose
bool rigQuieto = false;    // R: (con G) pose T y todo detenido, para revisar modelos y luces
float tRig = 0.0f;

float tScar = 0.0f, tEsq = 0.0f;
float rotAspa = 0.0f;                 // grados
glm::vec3 cuauOffset(0.0f);
float cuauYaw = 0.0f, cuauTilt = 0.0f; // grados

int grupoSel = 0;                     // 0 CUAU, 1 SENTINEL, 2 MAC
glm::vec3* grupos[3] = { &posCuau, &posSentinel, &posMac };
const char* nombreGrupo[3] = { "CUAU", "SENTINEL", "MAC" };

Camera camera(glm::vec3(0.0f, 10.0f, 30.0f));
GLfloat lastX = WIDTH / 2.0, lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

GLfloat deltaTime = 0.0f, lastFrame = 0.0f;

static void SetModel(GLuint P, const glm::mat4& m)
{
	glUniformMatrix4fv(glGetUniformLocation(P, "model"), 1, GL_FALSE, glm::value_ptr(m));
}

// ---------------------------------------------------------------------
//  MODELADO JERARQUICO POR PARTES (CUAU y SENTINEL)
//  Cada pieza (OBJ en Models/*/partes) tiene su origen en el pivote de la articulacion.
//  'offset' = pivote de la pieza respecto al pivote de su padre (partes/pivotes.txt, coordenadas OBJ).
//  Convenciones (coordenadas OBJ): el personaje mira a +Z (el -Y de Blender), +X es su IZQUIERDA. Rotaciones en grados.
//  Una rotacion +X de una extremidad que cuelga la lleva hacia ATRAS (-Z); -X hacia delante.
//  Matriz de cada pieza = matriz(padre) * T(offset) * Rx * Ry * Rz   (Rz se aplica primero)
// ---------------------------------------------------------------------
struct PiezaDef { const char* archivo; int padre; glm::vec3 offset; bool luz; };
struct Pieza { Model* modelo; int padre; glm::vec3 offset; glm::vec3 rot; bool luz; bool visible; };
// altura = pivote de la pelvis sobre el suelo; luces 0 apagado / 1 encendido; M = matrices de mundo de cada pieza (de la ultima vez que se dibujo)
struct Rig { std::vector<Pieza> p; float altura; float luces; std::vector<glm::mat4> M; };

// Indices comunes a CUAU y SENTINEL (mismo orden en ambas tablas)
enum { PELVIS, TORSO, BRAZO_I, ANTEBRAZO_I, BRAZO_D, ANTEBRAZO_D,
       MUSLO_I, PANTORRILLA_I, PIE_I, MUSLO_D, PANTORRILLA_D, PIE_D, N_BASE };
enum { C_CABEZA = N_BASE, C_VISOR, C_LUCES, C_LUZ68, N_CUAU };   // extras de CUAU
enum { S_RAILGUN = N_BASE, N_SENT };                              // extra del SENTINEL

const PiezaDef DEF_CUAU[N_CUAU] = {
	{ "pelvis",          -1,            glm::vec3( 0.00f,  0.00f,  0.00f), false },
	{ "torso",           PELVIS,        glm::vec3( 0.00f,  1.94f, -0.01f), false },
	{ "brazo_sup_izq",   TORSO,         glm::vec3( 1.86f,  0.89f, -0.34f), false },
	{ "antebrazo_izq",   BRAZO_I,       glm::vec3( 3.16f, -0.06f, -0.31f), false },
	{ "brazo_sup_der",   TORSO,         glm::vec3(-1.86f,  0.89f, -0.34f), false },
	{ "antebrazo_der",   BRAZO_D,       glm::vec3(-3.16f, -0.06f, -0.31f), false },
	{ "muslo_izq",       PELVIS,        glm::vec3( 0.97f, -0.45f, -0.20f), false },
	{ "pantorrilla_izq", MUSLO_I,       glm::vec3( 0.34f, -4.42f, -0.29f), false },
	{ "pie_izq",         PANTORRILLA_I, glm::vec3( 0.19f, -5.18f, -0.69f), false },
	{ "muslo_der",       PELVIS,        glm::vec3(-0.97f, -0.45f, -0.20f), false },
	{ "pantorrilla_der", MUSLO_D,       glm::vec3(-0.34f, -4.42f, -0.29f), false },
	{ "pie_der",         PANTORRILLA_D, glm::vec3(-0.19f, -5.18f, -0.69f), false },
	{ "cabeza",          TORSO,         glm::vec3( 0.00f,  2.12f,  0.02f), false },
	{ "visor",           C_CABEZA,      glm::vec3( 0.00f,  0.00f,  0.00f), true  },
	{ "luces_casco",     C_CABEZA,      glm::vec3( 0.00f,  0.00f,  0.00f), true  },
	{ "luz68",           TORSO,         glm::vec3( 0.00f,  0.00f,  0.00f), true  },
};

const PiezaDef DEF_SENT[N_SENT] = {
	{ "pelvis",          -1,            glm::vec3( 0.00f,  0.00f,  0.00f), false },
	{ "torso",           PELVIS,        glm::vec3( 0.00f,  1.92f,  0.00f), false },   // incluye cabeza y mochila
	{ "brazo_sup_izq",   TORSO,         glm::vec3( 1.86f,  1.11f, -0.35f), false },
	{ "antebrazo_izq",   BRAZO_I,       glm::vec3( 3.00f, -0.08f, -0.31f), false },
	{ "brazo_sup_der",   TORSO,         glm::vec3(-1.86f,  1.11f, -0.35f), false },
	{ "antebrazo_der",   BRAZO_D,       glm::vec3(-3.00f, -0.08f, -0.31f), false },
	{ "muslo_izq",       PELVIS,        glm::vec3( 1.11f, -0.48f, -0.20f), false },
	{ "pantorrilla_izq", MUSLO_I,       glm::vec3( 0.20f, -3.32f, -0.29f), false },
	{ "pie_izq",         PANTORRILLA_I, glm::vec3( 0.19f, -5.18f, -0.69f), false },
	{ "muslo_der",       PELVIS,        glm::vec3(-1.11f, -0.48f, -0.20f), false },
	{ "pantorrilla_der", MUSLO_D,       glm::vec3(-0.20f, -3.32f, -0.29f), false },
	{ "pie_der",         PANTORRILLA_D, glm::vec3(-0.19f, -5.18f, -0.69f), false },
	{ "railgun",         ANTEBRAZO_D,   glm::vec3(-3.70f,  0.00f,  0.00f), false },   // en la mano derecha (centro del arma = mano)
};

// El railgun se mueve con la mano. Su eje largo es Z. Con 180 grados el cañon apuntaria hacia atras.
const float RAILGUN_GIRO_Y = 0.0f;   // el perfil del railgun del rig coincide con railgun_aim.obj (apunta a +Z): sin giro

// Obrero minimalista (unidades = metros, 1.8 de alto): cuerpo + brazo derecho con el martillo en la mano (pivote en el hombro)
// Dos variantes de color (solo colores, sin escudos): UNAM = azul y dorado, IPN = blanco y vino
enum { OB_CUERPO, OB_BRAZO, N_OBRERO };
const PiezaDef DEF_OBRERO_UNAM[N_OBRERO] = {
	{ "obrero_unam_cuerpo", -1,        glm::vec3( 0.00f, 0.00f, 0.00f), false },
	{ "obrero_unam_brazo",  OB_CUERPO, glm::vec3(-0.30f, 1.45f, 0.00f), false },
};
const PiezaDef DEF_OBRERO_IPN[N_OBRERO] = {
	{ "obrero_ipn_cuerpo",  -1,        glm::vec3( 0.00f, 0.00f, 0.00f), false },
	{ "obrero_ipn_brazo",   OB_CUERPO, glm::vec3(-0.30f, 1.45f, 0.00f), false },
};

// Altura de la cadera sobre el suelo (pivote de la pelvis; los pies llegan a y = -9.0 en el OBJ)
const float ALTURA_CUAU = 11.85f, ALTURA_SENT = 10.78f;
const float CUAU_RIG_YAW = 0.0f;     // el rig mira a +Z, igual que la malla de sprint
const float SENT_RIG_YAW = 0.0f;     // la malla aim y el rig miran a +Z: no hace falta girar

Rig rigCuau, rigSent[3];   // un rig de CUAU y uno por SENTINEL (los 3 comparten los modelos, cada uno tiene su pose)
Rig rigObrero[2];          // 0 = UNAM, 1 = IPN: cada rig se dibuja varias veces cambiando el angulo del brazo

bool verObreros = false;   // F (prueba): circulo de obreros alrededor de CUAU; ESPACIO = martillazo
float tGolpe = 10.0f;      // segundos desde el ultimo martillazo

// Martillazo (grados del brazo; -X = hacia delante/arriba): levanta 0-0.40 s, golpea 0.40-0.50, rebota 0.50-0.60, regresa 0.60-0.95
static float Suave(float u) { return u * u * (3.0f - 2.0f * u); }
static float AnguloMartillo(float t)
{
	if (t <= 0.0f || t >= 0.95f) return 0.0f;
	if (t < 0.40f) return -165.0f * Suave(t / 0.40f);
	if (t < 0.50f) { float u = (t - 0.40f) / 0.10f; return -165.0f + 125.0f * u * u; }   // el golpe acelera
	if (t < 0.60f) return -40.0f - 22.0f * Suave((t - 0.50f) / 0.10f);                    // rebote
	return -62.0f * (1.0f - Suave((t - 0.60f) / 0.35f));                                   // vuelve al reposo
}

static void CargarRig(Rig& r, const std::string& carpeta, const PiezaDef* defs, int n, float altura)
{
	r.altura = altura; r.luces = 0.0f;
	for (int i = 0; i < n; i++)
	{
		std::string ruta = carpeta + "/" + defs[i].archivo + ".obj";
		Pieza p;
		p.modelo = new Model((GLchar*)ruta.c_str());
		p.padre = defs[i].padre; p.offset = defs[i].offset; p.rot = glm::vec3(0.0f);
		p.luz = defs[i].luz; p.visible = true;
		r.p.push_back(p);
	}
}

// Dibuja el rig: 'base' ya trae posicion, giro y escala del personaje. Los padres van antes que los hijos.
static void DibujarRig(Rig& r, glm::mat4 base, Shader& sh)
{
	GLuint P = sh.Program;
	std::vector<glm::mat4> M(r.p.size());
	base = glm::translate(base, glm::vec3(0.0f, r.altura, 0.0f));
	for (size_t i = 0; i < r.p.size(); i++)
	{
		const Pieza& p = r.p[i];
		glm::mat4 m = (p.padre < 0) ? base : M[p.padre];
		m = glm::translate(m, p.offset);
		m = glm::rotate(m, glm::radians(p.rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
		m = glm::rotate(m, glm::radians(p.rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
		m = glm::rotate(m, glm::radians(p.rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
		M[i] = m;
		if (!p.visible) continue;
		// piezas que se encienden: apagadas = oscuras; encendidas = brillan con su propio color
		glUniform1f(glGetUniformLocation(P, "emissive"), p.luz ? r.luces : 0.0f);
		glUniform1f(glGetUniformLocation(P, "brillo"),   p.luz ? (0.15f + 0.85f * r.luces) : 1.0f);
		SetModel(P, m);
		p.modelo->Draw(sh);
	}
	glUniform1f(glGetUniformLocation(P, "emissive"), 0.0f);
	glUniform1f(glGetUniformLocation(P, "brillo"), 1.0f);
	r.M = M;   // para anclar objetos a las piezas (p. ej. la MAC a la mano)
}

// Pose T de reposo (todas las rotaciones en 0): para revisar los modelos tal como salieron de Blender
static void PoseT(Rig& r)
{
	for (size_t i = 0; i < r.p.size(); i++) r.p[i].rot = glm::vec3(0.0f);
}

// El SENTINEL lleva el railgun en la mano derecha: se contrarresta el giro del brazo para que el arma quede horizontal
// (con el balanceo del brazo se cabecea un poco, como al caminar con ella)
static void SujetarRailgun(Rig& r)
{
	r.p[S_RAILGUN].rot = glm::vec3(0.0f, RAILGUN_GIRO_Y, -r.p[BRAZO_D].rot.z);
}

// Ciclo de caminata para las 12 piezas comunes (prueba de pivotes; es la base de las caminatas del relato)
static void PoseCaminar(Rig& r, float t, float amp, float frec)
{
	float s = sin(frec * t), c = cos(frec * t);
	r.p[MUSLO_I].rot.x = -amp * s;                    // -X lleva la pierna hacia delante (+Z)
	r.p[MUSLO_D].rot.x =  amp * s;
	r.p[PANTORRILLA_I].rot.x = 1.3f * amp * std::max(0.0f,  c);    // la rodilla flexiona (pie hacia atras) mientras la pierna avanza
	r.p[PANTORRILLA_D].rot.x = 1.3f * amp * std::max(0.0f, -c);
	r.p[PIE_I].rot.x = -0.7f * (r.p[MUSLO_I].rot.x + r.p[PANTORRILLA_I].rot.x);   // el pie intenta quedar plano
	r.p[PIE_D].rot.x = -0.7f * (r.p[MUSLO_D].rot.x + r.p[PANTORRILLA_D].rot.x);
	r.p[BRAZO_I].rot = glm::vec3( 0.7f * amp * s, 0.0f, -78.0f);   // brazos colgando, balanceo contrario a la pierna del mismo lado
	r.p[BRAZO_D].rot = glm::vec3(-0.7f * amp * s, 0.0f,  78.0f);
	r.p[TORSO].rot.y = 4.0f * s;                      // contragiro del torso
}

// =====================================================================
//  RELATO: MAQUINA DE ESTADOS (10 escenas)   (misma idea del ejemplo de maquina de estados: un entero de estado + Animation() por cuadro)
//  Cada escena recorre las fases:  TEXTO -> CAMARA -> ESPERA (tecla de accion) -> ANIMA -> FIN (Enter)
//  En las escenas 0 y 9 la camara viaja primero y el texto sale despues.
//  Teclas: Enter avanza, Espacio / flechas = accion de la escena. Modo desarrollador: F1 camara libre, F2 relato/banco de pruebas,
//  [ y ] cambian de escena, C imprime el punto de camara.
// =====================================================================
enum Fase { F_TEXTO, F_CAMARA, F_ESPERA, F_ANIMA, F_FIN };
struct Escena
{
	const char* paginas[3];     // PNG de la historia en UI/ (Enter avanza); lo que no se usa queda en nullptr
	bool textoDespues;          // true: primero viaja la camara y despues sale el texto
	glm::vec3 cam, mira;        // destino de la camara
	float durCam;               // segundos del viaje de camara
};

// Puntos de la plaza (de plaza.obj: el suelo es plano en y = 0)
// ---- Posiciones y giros finales de cada escena (obtenidos con el modo edicion, F3) ----
const glm::vec3 ZP(-12.72f, 1.90f, 8.31f);          // escena 1: centro del corro de obreros (sobre la plataforma de los vestigios, suelo y = 1.85)
const glm::vec3 E1_CUAU(-12.91f, 1.61f, 5.10f);     // escena 1: pies de CUAU acostado (su cabeza queda hacia +z)
const float     E1_CUAU_YAW = -179.1f;
const glm::vec3 E3_CUAU(-12.58f, 2.00f, 7.70f);     // escena 3
const float     E3_CUAU_YAW = 125.0f;
const float     E2_Z_OFF[3] = { 5.31f, 4.81f, 4.40f };   // escena 2: desplazamiento en z de cada SENTINEL (la fila queda mas cerca de la puerta)
const glm::vec3 E4_INICIO(-11.71f, 0.0f, 3.62f);    // escena 4: CUAU sale de la pirámide ...
const glm::vec3 E4_FIN(7.0f, 0.0f, 5.2f);           // ... y corre hacia la camara, frente a la fila de SENTINEL
const glm::vec3 E4_SENT(13.71f, 0.28f, 1.31f);      // escena 4: centro de la fila de SENTINEL (a lo largo de z)
const glm::vec3 E5_SENT(16.13f, 0.0f, 5.21f);       // escenas 5 a 7: el SENTINEL que cae
const float     E5_SENT_YAW = -94.1f;
const glm::vec3 E5_CUAU(13.22f, 0.60f, 2.69f);      // escenas 5 y 7: donde se detiene CUAU para el azote
const float     E5_CUAU_YAW = 64.4f;
const glm::vec3 E6_SENT(16.63f, 0.0f, 2.01f);       // escena 6 (punetazo): el SENTINEL (mira hacia E5_SENT_YAW) ...
const glm::vec3 E6_CUAU(14.89f, 0.50f, 0.80f);      // ... y donde termina CUAU
const float     E6_CUAU_YAW = 73.2f;
const glm::vec3 E7_SENT(22.65f, 0.0f, -1.49f);      // escena 7 (azote final): el SENTINEL un poco mas atras
const float     E7_SENT_YAW = -151.2f;
const glm::vec3 E7_CUAU(18.82f, 0.60f, -3.42f);
const float     E7_CUAU_YAW = 45.4f;
const glm::vec3 E8_CUAU(15.20f, 0.0f, 4.20f);       // escena 8 (victoria): CUAU y los tres SENTINEL derribados
const float     E8_CUAU_YAW = -185.5f;
const glm::vec3 E8_SENT[3] = { glm::vec3(6.22f, 0.0f, 3.51f), glm::vec3(13.82f, 0.0f, 0.70f), glm::vec3(23.01f, 0.0f, -1.00f) };
const float     E8_SENT_YAW[3] = { 25.0f, -10.0f, -148.4f };
const glm::vec3 MAC_MANO(-3.9f, -0.1f, 0.85f);      // centro de la palma de CUAU en el marco del antebrazo (el eje del antebrazo queda 0.85 detras)
const float     MAC_AGARRE_Y = 4.6f;                // punto del mango (y = -4.6 en el OBJ) que queda dentro del puño

// Altura del piso en el recorrido de la escena 4 (de plaza.obj): plataforma a 1.85 hasta x = -7, escalones, y piso a x = -3.3
static float AlturaPiso4(float x)
{
	if (x <= -7.0f) return 1.85f;
	if (x >= -3.3f) return 0.04f;
	return glm::mix(1.85f, 0.04f, (x + 7.0f) / 3.7f);
}
const glm::vec3 PUERTA(12.0f, 0.0f, 11.0f);  // puerta del Templo de Santiago (el ábside está al sur, z ~ 30)

const int N_ESC = 10;
const Escena ESC[N_ESC] = {
	/* 0 introduccion  */ { { "t0a", "t0b", "t0c" }, true,  glm::vec3(-6, 24, -20), glm::vec3(6, 4, 18),     5.0f },
	/* 1 obreros       */ { { "t1a", "t1b", "t1c" },  false, glm::vec3(-16.9f, 7.6f, -1.1f), glm::vec3(-12.5f, 3.6f, 6.9f), 4.0f },
	/* 2 sentinels     */ { { "t2a", "t2b" },         false, glm::vec3(3.2f, 13.4f, 13.1f), glm::vec3(8.8f, 7.4f, 7.3f),   4.0f },
	/* 3 levanta MAC   */ { { "t3a", "t3b" },         false, glm::vec3(-9.7f, 11.4f, 6.2f), glm::vec3(-14.6f, 3.3f, 9.5f), 3.5f },
	/* 4 sprint        */ { { "t4a" },               false, glm::vec3(25.3f, 7.3f, -2.9f), glm::vec3(16.2f, 6.8f, 1.1f),  4.0f },
	/* 5 azote 1       */ { { "t5a" },               false, glm::vec3(13.5f, 6.6f, 1.1f), glm::vec3(17.5f, 1.9f, 9.0f),  3.5f },
	/* 6 punetazo      */ { { "t6a" },               false, glm::vec3(19.9f, 2.3f, -1.3f), glm::vec3(12.5f, 5.3f, 4.6f),   0.8f },
	/* 7 azote final   */ { { "t7a" },               false, glm::vec3(16.0f, 6.0f, -3.6f), glm::vec3(25.1f, 4.0f, 0.2f),   0.8f },
	/* 8 victoria      */ { { "t8a" },               false, glm::vec3(23.1f, 2.2f, -8.4f), glm::vec3(18.1f, 3.6f, 0.2f), 4.0f },
	/* 9 lapida        */ { { "t9a", "t9b", "t9c" }, true,  glm::vec3(0, 1.9f, 25.0f), glm::vec3(0, 1.5f, 31.3f), 5.0f },
};

int   escena = 0;
Fase  fase = F_CAMARA;
float tFase = 0.0f, tAnim = 0.0f, tReloj = 0.0f;
int   pagina = 0;
glm::vec3 camPos(-32, 32, -34), camMira(0, 4, 0), camDesde(0.0f), miraDesde(0.0f);
bool  camLibre = false;          // F1
bool  modoRelato = true;         // F2: relato / banco de pruebas (el modo anterior con G, R, H, F...)
bool  cerrarRelato = false;
int   golpesObr = 0, pasoEsq = 0;
float avisoT = -1.0f, tzig = 0.0f, avance = 0.0f;

// Actores de la escena actual (los llena ActualizarRelato y los dibuja DibujarRelato)
bool cuauVisible = false, macVisible = false, obrerosVisible = false;
glm::vec3 cuauPosR(0.0f); float cuauYawR = 0.0f, cuauPitchR = 0.0f;
bool sentVis[3] = { false, false, false }, sentRail[3] = { true, true, true };
glm::vec3 sentPos[3]; float sentYaw[3] = { 0, 0, 0 }, sentCaida[3] = { 0, 0, 0 };
float macMezcla = 1.0f;          // 0 = en el suelo, 1 = en la mano
Model *mdCuau = nullptr, *mdSentinel = nullptr, *mdRailgun = nullptr, *mdMac = nullptr, *mdAspa = nullptr;

// ---- MODO EDICION (F3): mueve los actores de la escena actual. Los ajustes se SUMAN a lo que calcula la escena ----
// actores: 0 CUAU, 1..3 SENTINEL, 4 OBREROS (el grupo). edOff[escena][actor] = (dx, dy, dz, giro en grados)
bool modoEdicion = false;
int  edSel = 0;
glm::vec4 edOff[10][5];     // global: arranca en ceros
static const char* NOM_ACTOR[5] = { "CUAU", "SENTINEL 1", "SENTINEL 2", "SENTINEL 3", "OBREROS" };
bool  edSucio = false;       // hay ajustes sin guardar
float tAutoGuardar = 0.0f;

// Los ajustes se guardan en ajustes_escenas.txt (carpeta desde donde se corre) y se cargan al abrir: un cierre no los pierde
static void GuardarAjustes()
{
	FILE* f = fopen("ajustes_escenas.txt", "w");
	if (!f) return;
	fprintf(f, "# escena actor dx dy dz giro   (actores: 0 CUAU, 1-3 SENTINEL, 4 OBREROS)\n");
	for (int e = 0; e < 10; e++) for (int a = 0; a < 5; a++)
	{
		const glm::vec4& o = edOff[e][a];
		if (o.x != 0.0f || o.y != 0.0f || o.z != 0.0f || o.w != 0.0f) fprintf(f, "%d %d %.3f %.3f %.3f %.2f\n", e, a, o.x, o.y, o.z, o.w);
	}
	fclose(f);
	edSucio = false;
}
static void CargarAjustes()
{
	FILE* f = fopen("ajustes_escenas.txt", "r");
	if (!f) return;
	char linea[256]; int n = 0;
	while (fgets(linea, sizeof(linea), f))
	{
		int e, a; float x, y, z, w;
		if (linea[0] != '#' && sscanf(linea, "%d %d %f %f %f %f", &e, &a, &x, &y, &z, &w) == 6 && e >= 0 && e < 10 && a >= 0 && a < 5)
		{ edOff[e][a] = glm::vec4(x, y, z, w); n++; }
	}
	fclose(f);
	std::printf("Ajustes cargados de ajustes_escenas.txt (%d)\n", n);
}

static float Clamp01(float x) { return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x); }
static float Rampa(float t, float a, float b) { return Suave(Clamp01((t - a) / (b - a))); }   // 0 antes de a, 1 despues de b

// Duracion (s) de la animacion de cada escena (en la 4 es la de CADA tramo del zigzag)
static float DurAnim(int e)
{
	switch (e) {
		case 1: return 4.2f;  case 2: return 7.0f;  case 3: return 3.8f;  case 4: return 2.2f;
		case 5: return 3.4f;  case 6: return 3.2f;  case 7: return 3.4f;  case 8: return 3.6f;
	}
	return 0.0f;
}

// ANIMACION 3 (sencilla): caida del SENTINEL con rebote. 0 = de pie, 1 = en el suelo; t = segundos desde el impacto
static float CaidaSentinel(float t)
{
	if (t <= 0.0f) return 0.0f;
	if (t < 0.55f) { float u = t / 0.55f; return u * u; }                       // cae acelerando
	if (t < 0.80f) { float u = (t - 0.55f) / 0.25f; return 1.0f - 0.12f * sin(3.14159f * u); }   // rebota
	return 1.0f;
}

static void PoseReposo(Rig& r)   // de pie, brazos colgando
{
	PoseT(r);
	r.p[BRAZO_I].rot = glm::vec3(0.0f, 0.0f, -78.0f);
	r.p[BRAZO_D].rot = glm::vec3(0.0f, 0.0f,  78.0f);
}

// Mezcla dos matrices rigidas (+ escala uniforme): posicion lineal, rotacion con slerp
static glm::mat4 MezclaMat(const glm::mat4& A, const glm::mat4& B, float s)
{
	auto rot = [](const glm::mat4& M) {
		return glm::quat_cast(glm::mat3(glm::normalize(glm::vec3(M[0])), glm::normalize(glm::vec3(M[1])), glm::normalize(glm::vec3(M[2]))));
	};
	glm::mat4 R = glm::mat4_cast(glm::slerp(rot(A), rot(B), s));
	R[3] = glm::vec4(glm::mix(glm::vec3(A[3]), glm::vec3(B[3]), s), 1.0f);
	return glm::scale(R, glm::vec3(glm::mix(glm::length(glm::vec3(A[0])), glm::length(glm::vec3(B[0])), s)));
}

static void IniciarEscena(int i)
{
	escena = i; pagina = 0; tFase = 0.0f; tAnim = 0.0f; golpesObr = 0; pasoEsq = 0; avisoT = -1.0f; tGolpe = 10.0f;
	tzig = 0.0f; avance = 0.0f; cuauYaw = 180.0f; cuauTilt = 0.0f; cuauOffset = glm::vec3(0.0f);
	camDesde = camPos; miraDesde = camMira;
	fase = ESC[i].textoDespues ? F_CAMARA : F_TEXTO;
	std::printf("Escena %d\n", i);
}

// Salta a una escena ya "lista" (modo desarrollador y pruebas)
static void IrAEscena(int i)
{
	if (i < 0) i = 0; if (i >= N_ESC) i = N_ESC - 1;
	IniciarEscena(i);
	camPos = ESC[i].cam; camMira = ESC[i].mira;
	fase = ESC[i].textoDespues ? F_TEXTO : (i == 2 ? F_ANIMA : F_ESPERA);
}

static void TrasTexto()
{
	if (ESC[escena].textoDespues) fase = F_FIN;
	else { fase = F_CAMARA; tFase = 0.0f; camDesde = camPos; miraDesde = camMira; }
}
static void TrasCamara()
{
	if (ESC[escena].textoDespues) { fase = F_TEXTO; pagina = 0; }
	else if (escena == 2) { fase = F_ANIMA; tAnim = 0.0f; }     // la salida de los SENTINEL es automatica
	else fase = F_ESPERA;
}

static void ProcesarTeclaRelato(int key)
{
	bool enter = (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER), espacio = (key == GLFW_KEY_SPACE);

	if (fase == F_TEXTO && enter) {
		pagina++;
		if (pagina >= 3 || !ESC[escena].paginas[pagina]) TrasTexto();
		return;
	}
	if (fase == F_FIN && enter) {
		if (escena >= N_ESC - 1) cerrarRelato = true; else IniciarEscena(escena + 1);
		return;
	}
	if (fase != F_ESPERA) return;     // en CAMARA y ANIMA se ignora el teclado

	switch (escena)
	{
	case 1:   // 3 martillazos y despues se puede despertar a CUAU
		if (!espacio) break;
		if (golpesObr < 3) { if (tGolpe > 0.95f) { tGolpe = 0.0f; golpesObr++; } }
		else if (tGolpe > 1.0f) { fase = F_ANIMA; tAnim = 0.0f; }
		break;
	case 3: case 5: case 6: case 7: case 8:
		if (espacio) { fase = F_ANIMA; tAnim = 0.0f; }
		break;
	case 4: {   // secuencia izquierda - derecha - izquierda
		int esperada = (pasoEsq == 1) ? GLFW_KEY_RIGHT : GLFW_KEY_LEFT;
		if (key == esperada) { fase = F_ANIMA; tAnim = 0.0f; }
		else if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) avisoT = 0.0f;
		break; }
	}
}

// Cuadro de la escena de "azote": CUAU camina hacia el SENTINEL k, levanta la MAC, golpea y el SENTINEL cae
static void EscenaAzote(float t, int k, const glm::vec3& posSent, float yawSent, const glm::vec3& cuauFin, float cuauYaw)
{
	cuauVisible = true; macVisible = true; sentVis[k] = true; sentPos[k] = posSent; sentYaw[k] = yawSent;
	cuauYawR = cuauYaw;
	glm::vec3 adelante(sin(glm::radians(cuauYaw)), 0.0f, cos(glm::radians(cuauYaw)));
	cuauPosR = cuauFin - adelante * (4.0f * (1.0f - Rampa(t, 0.0f, 1.1f)));    // camina 4 unidades hacia donde mira y se detiene en cuauFin
	if (t < 1.1f) PoseCaminar(rigCuau, tReloj, 22.0f, 7.0f);
	float a = Rampa(t, 0.2f, 1.1f), s = Rampa(t, 1.1f, 1.4f), r = Rampa(t, 2.0f, 3.2f);
	float brazo = glm::mix(glm::mix(-170.0f * a, -95.0f, s), -60.0f, r);   // arriba -> golpe horizontal -> recupera
	rigCuau.p[BRAZO_D].rot = glm::vec3(brazo, 0.0f, 78.0f);
	rigCuau.p[ANTEBRAZO_D].rot.y = 25.0f * a * (1.0f - s);
	rigCuau.p[TORSO].rot.x = 20.0f * s * (1.0f - r);
	sentCaida[k] = CaidaSentinel(t - 1.35f);
	animSierras = true;
}

static void ActualizarRelato(float dt)
{
	tReloj += dt; tFase += dt;
	if (edSucio) { tAutoGuardar += dt; if (tAutoGuardar > 2.0f) { GuardarAjustes(); tAutoGuardar = 0.0f; } }
	if (avisoT >= 0.0f) { avisoT += dt; if (avisoT > 1.3f) avisoT = -1.0f; }

	if (fase == F_CAMARA) {
		float u = Clamp01(tFase / std::max(ESC[escena].durCam, 0.01f)), s = Suave(u);
		camPos = glm::mix(camDesde, ESC[escena].cam, s);
		camMira = glm::mix(miraDesde, ESC[escena].mira, s);
		if (u >= 1.0f) TrasCamara();
	}
	if (fase == F_ANIMA || fase == F_FIN) tAnim += dt;
	if (fase == F_ANIMA && tAnim >= DurAnim(escena)) {
		if (escena == 4) { pasoEsq++; tAnim = 0.0f; fase = (pasoEsq >= 3) ? F_FIN : F_ESPERA; }
		else fase = F_FIN;
	}

	// ---- actores: todo oculto salvo lo que pide la escena (el escenario nunca se borra) ----
	cuauVisible = macVisible = obrerosVisible = false;
	for (int i = 0; i < 3; i++) { sentVis[i] = false; sentRail[i] = true; sentCaida[i] = 0.0f; }
	animSierras = false; animScarface = false; animEsquive = false; modoRig = false;
	rigCuau.luces = 0.0f; macMezcla = 1.0f;
	cuauPitchR = 0.0f;                    // de pie salvo que la escena 1 lo acueste (si no, al saltar de escena se quedaba acostado)
	PoseReposo(rigCuau);
	for (int i = 0; i < 3; i++) { PoseReposo(rigSent[i]); SujetarRailgun(rigSent[i]); }

	bool mostrar = (fase != F_TEXTO) || ESC[escena].textoDespues;     // mientras sale el texto del cambio de escena, solo la plaza
	if (!mostrar) return;
	bool anim = (fase == F_ANIMA || fase == F_FIN);
	float t = anim ? std::min(tAnim, DurAnim(escena)) : 0.0f;
	float pulso = 0.5f + 0.5f * sin(3.0f * tReloj);                    // ANIMACION 2: luces del casco que brillan senoidalmente

	switch (escena)
	{
	case 1: {   // obreros terminan a CUAU (acostado); al despertar se incorpora y se encienden las luces
		obrerosVisible = true; cuauVisible = true;
		cuauPosR = E1_CUAU; cuauYawR = E1_CUAU_YAW;
		float lev = anim ? Rampa(t, 0.2f, 3.0f) : 0.0f;
		cuauPitchR = -90.0f * (1.0f - lev);
		rigCuau.luces = anim ? Rampa(t, 2.0f, 2.8f) * pulso : 0.0f;
		break; }

	case 2: {   // los SENTINEL salen del Templo en fila, caminando con el railgun en la mano
		static const float zAlto[3] = { -9.0f, -4.0f, 1.0f };
		for (int i = 0; i < 3; i++) {
			sentVis[i] = true;
			float tt = std::max(0.0f, (anim ? t : 0.0f) - 1.4f * i);
			float z = std::max(12.5f - 5.0f * tt, zAlto[i]) + E2_Z_OFF[i];
			sentPos[i] = glm::vec3(PUERTA.x, 0.0f, z); sentYaw[i] = 180.0f;
			if (anim && z > zAlto[i] + 0.01f) { PoseCaminar(rigSent[i], tReloj + 0.5f * i, 24.0f, 6.5f); SujetarRailgun(rigSent[i]); }
		}
		break; }

	case 3: {   // CUAU ya empuña la MAC: la levanta, mira hacia arriba y las sierras arrancan
		cuauVisible = true; macVisible = true;
		cuauPosR = E3_CUAU; cuauYawR = E3_CUAU_YAW;
		float b = Rampa(t, 0.3f, 3.0f);
		rigCuau.p[BRAZO_D].rot.x = glm::mix(-35.0f, -80.0f, b);
		rigCuau.p[ANTEBRAZO_D].rot.y = glm::mix(75.0f, 80.0f, b);
		rigCuau.p[C_CABEZA].rot.x = -30.0f * b;                        // mira hacia arriba
		rigCuau.luces = pulso; macMezcla = 1.0f;
		animSierras = (t > 3.0f);
		break; }

	case 4: {   // CUAU corre hacia la camara; cada tecla (<- ->) lanza UN esquive hacia ese lado de la pantalla
		cuauVisible = true; for (int i = 0; i < 3; i++) sentVis[i] = true;
		posSentinel = E4_SENT;
		animScarface = true;                                           // Animation() avanza tScar: barrido, destello y retroceso
		glm::vec3 ruta = E4_FIN - E4_INICIO; ruta.y = 0.0f;
		float largo = glm::length(ruta); glm::vec3 dirAv = ruta / largo;
		glm::vec3 vista = ESC[4].mira - ESC[4].cam;
		glm::vec3 derCam = glm::normalize(glm::vec3(-vista.z, 0.0f, vista.x));   // derecha de la pantalla, sobre el piso
		static const float lado[3] = { -1.0f, 1.0f, -1.0f };                     // <- : izquierda de la pantalla, -> : derecha
		const float T = DurAnim(4);
		float u = (fase == F_ANIMA) ? Clamp01(tAnim / T) : 0.0f;                 // avance del esquive actual
		int k = std::min(pasoEsq, 2);
		float adelante = (std::min(pasoEsq, 3) + u) * (largo / 3.0f);            // cada esquive avanza un tercio del recorrido
		float lat = lado[k] * 4.0f * sin(3.14159f * u);                          // se desplaza 4 unidades y regresa a la linea
		glm::vec3 dirLat = derCam * lado[k];
		glm::vec3 base = E4_INICIO + dirAv * adelante;
		posCuau = glm::vec3(base.x, AlturaPiso4(base.x), base.z);
		cuauOffset = derCam * lat + glm::vec3(0.0f, 1.6f * sin(3.14159f * u), 0.0f);   // salto
		// ANIMACION 5 (compleja): giro suavizado hacia la velocidad (avance + lateral) e inclinacion hacia el lado del esquive
		glm::vec3 vel = dirAv * (largo / 3.0f / T) + dirLat * (4.0f * 3.14159f / T * cos(3.14159f * u)) * (fase == F_ANIMA ? 1.0f : 0.0f);
		float objetivo = glm::degrees(atan2(vel.x, vel.z));
		float dif = fmod(objetivo - cuauYaw + 540.0f, 360.0f) - 180.0f;
		cuauYaw += dif * std::min(1.0f, dt * 10.0f);
		float yawR = glm::radians(cuauYaw);
		float haciaDer = glm::dot(dirLat, glm::vec3(-cos(yawR), 0.0f, sin(yawR)));    // >0: el esquive va hacia la derecha del personaje
		cuauTilt = 18.0f * sin(3.14159f * u) * (haciaDer > 0.0f ? 1.0f : -1.0f);
		break; }

	case 5: EscenaAzote(t, 0, E5_SENT, E5_SENT_YAW, E5_CUAU, E5_CUAU_YAW); rigCuau.luces = pulso; break;
	case 7: EscenaAzote(t, 2, E7_SENT, E7_SENT_YAW, E7_CUAU, E7_CUAU_YAW); rigCuau.luces = pulso; break;

	case 6: {   // puñetazo con el brazo izquierdo
		cuauVisible = true; sentVis[1] = true; sentPos[1] = E6_SENT; sentYaw[1] = E5_SENT_YAW;
		cuauYawR = E6_CUAU_YAW;
		glm::vec3 adelante(sin(glm::radians(E6_CUAU_YAW)), 0.0f, cos(glm::radians(E6_CUAU_YAW)));
		cuauPosR = E6_CUAU - adelante * (6.2f * (1.0f - Rampa(t, 0.0f, 1.0f)));            // camina hacia donde mira y se detiene a golpear
		if (t < 1.0f) PoseCaminar(rigCuau, tReloj, 22.0f, 7.0f);
		float w = Rampa(t, 1.0f, 1.3f), h = Rampa(t, 1.3f, 1.5f), r = Rampa(t, 2.0f, 3.0f);
		float brazo = glm::mix(glm::mix(0.0f, 25.0f, w), -90.0f, h);   // hacia atras (se prepara) -> hacia delante (golpea)
		float codo = glm::mix(glm::mix(0.0f, -110.0f, w), -5.0f, h);   // codo doblado -> brazo extendido
		rigCuau.p[BRAZO_I].rot = glm::vec3(glm::mix(brazo, 0.0f, r), 0.0f, -78.0f);
		rigCuau.p[ANTEBRAZO_I].rot.y = glm::mix(codo, 0.0f, r);
		rigCuau.p[TORSO].rot.x = 10.0f * h * (1.0f - r);
		rigCuau.luces = pulso;
		sentCaida[1] = CaidaSentinel(t - 1.45f);                       // cae hacia atras
		break; }

	case 8: {   // victoria: 3 SENTINEL derribados, sus railguns en el piso, CUAU alza la MAC
		cuauVisible = true; macVisible = true;
		cuauPosR = E8_CUAU; cuauYawR = E8_CUAU_YAW;
		float a = Rampa(t, 0.3f, 2.0f);
		rigCuau.p[BRAZO_D].rot = glm::vec3(-150.0f * a, 0.0f, 78.0f);
		rigCuau.p[ANTEBRAZO_D].rot.y = 20.0f * a;
		rigCuau.p[C_CABEZA].rot.x = -28.0f * a;
		rigCuau.p[TORSO].rot.x = -8.0f * a;
		rigCuau.luces = pulso; animSierras = true;
		for (int i = 0; i < 3; i++) { sentVis[i] = true; sentPos[i] = E8_SENT[i]; sentYaw[i] = E8_SENT_YAW[i]; sentCaida[i] = 1.0f; sentRail[i] = false; }
		break; }
	}

	// ---- ajustes del modo edicion (F3): se suman a la posicion y al giro que calculo la escena ----
	if (escena == 4) {   // sprint: las mallas se colocan con posCuau / posSentinel (el SENTINEL 1 mueve al grupo completo)
		posCuau += glm::vec3(edOff[4][0]);
		posSentinel += glm::vec3(edOff[4][1]);
	} else {
		cuauPosR += glm::vec3(edOff[escena][0]); cuauYawR += edOff[escena][0].w;
		for (int i = 0; i < 3; i++) { sentPos[i] += glm::vec3(edOff[escena][1 + i]); sentYaw[i] += edOff[escena][1 + i].w; }
	}
}

static bool ActorVisible(int a)
{
	if (a == 0) return cuauVisible;
	if (a == 4) return obrerosVisible;
	return sentVis[a - 1];
}
static void SiguienteActor()
{
	for (int k = 1; k <= 5; k++) { int a = (edSel + k) % 5; if (ActorVisible(a)) { edSel = a; break; } }
	std::printf("Editando: %s\n", NOM_ACTOR[edSel]);
}
// Imprime posiciones absolutas y ajustes de la escena actual, y copiarlos a las constantes de arriba
static void ImprimirAjustes()
{
	GuardarAjustes();
	std::printf("---- ESCENA %d ---- (todos los ajustes quedaron guardados en ajustes_escenas.txt)\n", escena);
	for (int a = 0; a < 5; a++)
	{
		if (!ActorVisible(a)) continue;
		glm::vec3 pos(0.0f); float yaw = 0.0f;
		if (a == 0) { pos = (escena == 4) ? posCuau : cuauPosR; yaw = (escena == 4) ? cuauYaw : cuauYawR; }
		else if (a == 4) pos = ZP;
		else { pos = (escena == 4) ? posSentinel : sentPos[a - 1]; yaw = (escena == 4) ? 0.0f : sentYaw[a - 1]; }
		const glm::vec4& o = edOff[escena][a];
		std::printf("%-11s posicion (%.2f, %.2f, %.2f)  giro %.1f   | ajuste (%+.2f, %+.2f, %+.2f, giro %+.1f)%s\n", NOM_ACTOR[a],
			pos.x + (a == 4 ? o.x : 0), pos.y + (a == 4 ? o.y : 0), pos.z + (a == 4 ? o.z : 0), yaw, o.x, o.y, o.z, o.w,
			(escena == 4 && a >= 1 && a <= 3) ? "  (en esta escena el SENTINEL 1 mueve al grupo)" : "");
	}
}

// ---------------- HUD (texto en pantalla): PNG de UI/ dibujados como cuadros 2D ----------------
struct TexHUD { GLuint id; int w, h; };
static std::map<std::string, TexHUD> hudCache;
static GLuint hudVAO = 0, hudVBO = 0;

static void InitHUD()
{
	const float q[] = { 0, 0,  1, 0,  1, 1,  0, 0,  1, 1,  0, 1 };
	glGenVertexArrays(1, &hudVAO); glGenBuffers(1, &hudVBO);
	glBindVertexArray(hudVAO);
	glBindBuffer(GL_ARRAY_BUFFER, hudVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(q), q, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (GLvoid*)0);
	glBindVertexArray(0);
}

static TexHUD& HUDTex(const std::string& nombre)
{
	auto it = hudCache.find(nombre);
	if (it != hudCache.end()) return it->second;
	TexHUD t = { 0, 0, 0 };
	std::string ruta = "UI/" + nombre + ".png";
	unsigned char* img = SOIL_load_image(ruta.c_str(), &t.w, &t.h, 0, SOIL_LOAD_RGBA);
	if (img) {
		glGenTextures(1, &t.id);
		glBindTexture(GL_TEXTURE_2D, t.id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t.w, t.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);     // texto nitido: filtro lineal, sin mipmaps
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
		SOIL_free_image_data(img);
	} else std::printf("HUD: no se pudo cargar %s\n", ruta.c_str());
	return hudCache[nombre] = t;
}

// Dibuja un PNG centrado, a 'abajo' pixeles (de ventana de 1280 de ancho) del borde inferior. Las imagenes se generan al doble de tamaño.
static void DibujarHUD(Shader& hud, const std::string& nombre, float abajo, float alpha = 1.0f)
{
	TexHUD& t = HUDTex(nombre);
	if (!t.id) return;
	float f = (float)SCREEN_WIDTH / (float)WIDTH;
	float wl = t.w * 0.5f, hl = t.h * 0.5f;
	if (wl > 1180.0f) { float k = 1180.0f / wl; wl *= k; hl *= k; }
	float w = wl * f, h = hl * f;
	hud.Use();
	glUniform2f(glGetUniformLocation(hud.Program, "screen"), (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
	glUniform4f(glGetUniformLocation(hud.Program, "rect"), (SCREEN_WIDTH - w) * 0.5f, SCREEN_HEIGHT - h - abajo * f, w, h);
	glUniform1f(glGetUniformLocation(hud.Program, "alpha"), alpha);
	glUniform1i(glGetUniformLocation(hud.Program, "tex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, t.id);
	glBindVertexArray(hudVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

static const char* PromptActual()
{
	if (fase == F_FIN) return escena == N_ESC - 1 ? "p_fin" : "p_enter";
	if (fase != F_ESPERA) return nullptr;
	static const char* martillo[3] = { "p1_m0", "p1_m1", "p1_m2" };
	static const char* flechas[3] = { "p4_a", "p4_b", "p4_c" };
	switch (escena) {
		case 1: return (golpesObr < 3) ? martillo[golpesObr] : (tGolpe > 1.0f ? "p1_des" : nullptr);
		case 3: return "p3";
		case 4: return flechas[std::min(pasoEsq, 2)];
		case 5: return "p5";  case 6: return "p6";  case 7: return "p7";  case 8: return "p8";
	}
	return nullptr;
}

static void DibujarHUDRelato(Shader& hud)
{
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (fase == F_TEXTO) { if (ESC[escena].paginas[pagina]) DibujarHUD(hud, ESC[escena].paginas[pagina], 36.0f); }
	else if (const char* p = PromptActual()) DibujarHUD(hud, p, 36.0f);
	if (avisoT >= 0.0f) DibujarHUD(hud, "p4_mal", 120.0f, 1.0f - Clamp01((avisoT - 0.9f) / 0.4f));
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

// ---------------- dibujado de los actores de la escena ----------------
static void DibujarRelato(Shader& sh, const glm::mat4* mSent, const float* retro)
{
	GLuint P = sh.Program;
	const glm::vec3 Ejy(0, 1, 0), Ejx(1, 0, 0), Ejz(0, 0, 1);

	// ---- obreros (UNAM / IPN alternados) martillando a CUAU, a los dos lados de su cuerpo ----
	if (obrerosVisible)
	{
		static const glm::vec2 pts[6] = { {-2.6f,-2.6f}, {-2.6f,0.0f}, {-2.6f,2.6f}, {2.6f,-2.6f}, {2.6f,0.0f}, {2.6f,2.6f} };
		for (int i = 0; i < 6; i++)
		{
			glm::mat4 m = glm::translate(glm::mat4(1.0f), ZP + glm::vec3(edOff[escena][4]) + glm::vec3(pts[i].x, 0.0f, pts[i].y));
			m = glm::rotate(m, atan2(-pts[i].x, -pts[i].y), Ejy);                // mira hacia CUAU
			Rig& ob = rigObrero[i % 2];
			ob.p[OB_BRAZO].rot.x = AnguloMartillo(tGolpe - i * 0.08f);
			DibujarRig(ob, m, sh);
		}
	}

	// ---- CUAU ----
	if (cuauVisible)
	{
		if (escena == 4)   // malla de pose de sprint
		{
			glm::mat4 m = glm::translate(glm::mat4(1.0f), posCuau + cuauOffset);
			m = glm::rotate(m, glm::radians(cuauYaw + CUAU_YAW_OFFSET), Ejy);
			m = glm::rotate(m, glm::radians(cuauTilt), Ejz);
			m = glm::scale(m, glm::vec3(escGrupo[0]));
			SetModel(P, m);
			mdCuau->Draw(sh);
		}
		else
		{
			float sube = 2.9f * escGrupo[0] * sin(glm::radians(-cuauPitchR));      // acostado: el pivote (pies) sube por el grosor del cuerpo
			glm::mat4 b = glm::translate(glm::mat4(1.0f), cuauPosR + glm::vec3(0.0f, sube, 0.0f));
			b = glm::rotate(b, glm::radians(cuauYawR), Ejy);
			b = glm::rotate(b, glm::radians(cuauPitchR), Ejx);
			b = glm::scale(b, glm::vec3(escGrupo[0]));
			DibujarRig(rigCuau, b, sh);
		}
	}

	// ---- MAC: en la mano de CUAU (el eje de la maza continua el antebrazo) o, al inicio de la escena 3, en el suelo ----
	if (macVisible)
	{
		glm::mat4 m = rigCuau.M[ANTEBRAZO_D] * glm::translate(glm::mat4(1.0f), MAC_MANO)
			* glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), Ejz)
			* glm::translate(glm::mat4(1.0f), glm::vec3(-MAC_CENTRO.x, MAC_AGARRE_Y, 0.0f));   // el puño sujeta el mango a y = -MAC_AGARRE_Y
		if (macMezcla < 1.0f)
		{
			glm::mat4 Bc = glm::rotate(glm::translate(glm::mat4(1.0f), cuauPosR), glm::radians(cuauYawR), Ejy);
			glm::vec3 pg = glm::vec3(Bc * glm::vec4(-4.0f, 0.45f, 2.2f, 1.0f));              // en el suelo, a la derecha de CUAU
			glm::mat4 suelo = glm::translate(glm::mat4(1.0f), pg) * glm::rotate(glm::mat4(1.0f), glm::radians(cuauYawR), Ejy)
				* glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), Ejz) * glm::scale(glm::mat4(1.0f), glm::vec3(escGrupo[0]))
				* glm::translate(glm::mat4(1.0f), glm::vec3(-MAC_CENTRO.x, -0.9f, 0.0f));
			m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f * sin(3.14159f * macMezcla), 0.0f)) * MezclaMat(suelo, m, macMezcla);
		}
		SetModel(P, m);
		mdMac->Draw(sh);
		for (int i = 0; i < 3; i++)
		{
			glm::mat4 a = glm::translate(m, glm::vec3(MAC_CENTRO.x, ASPA_Y[i], MAC_CENTRO.z));
			a = glm::rotate(a, glm::radians(((i % 2 == 0) ? 1.0f : -1.0f) * rotAspa), Ejz);   // sentidos alternos
			SetModel(P, a);
			mdAspa->Draw(sh);
		}
	}

	// ---- SENTINEL ----
	if (escena == 4)   // mallas de pose "aim" con el barrido Scarface
	{
		for (int i = 0; i < 3; i++)
		{
			SetModel(P, mSent[i]);
			mdSentinel->Draw(sh);
			SetModel(P, glm::translate(mSent[i], glm::vec3(0.0f, 0.0f, -retro[i])));
			mdRailgun->Draw(sh);
		}
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			if (!sentVis[i]) continue;
			float ang = -90.0f * sentCaida[i];                                   // cae de espaldas
			float sube = 2.4f * escGrupo[1] * sin(glm::radians(-ang));
			glm::mat4 b = glm::translate(glm::mat4(1.0f), sentPos[i] + glm::vec3(0.0f, sube, 0.0f));
			b = glm::rotate(b, glm::radians(sentYaw[i]), Ejy);
			b = glm::rotate(b, glm::radians(ang), Ejx);
			b = glm::scale(b, glm::vec3(escGrupo[1]));
			rigSent[i].p[S_RAILGUN].visible = sentRail[i];
			DibujarRig(rigSent[i], b, sh);
			if (!sentRail[i])   // railgun tirada en el piso, junto al SENTINEL derribado
			{
				glm::vec3 lado = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(sentYaw[i]), Ejy) * glm::vec4(3.2f, 0.0f, -2.5f, 1.0f));
				glm::mat4 r = glm::translate(glm::mat4(1.0f), sentPos[i] + lado + glm::vec3(0.0f, 0.36f * escGrupo[1], 0.0f));
				r = glm::rotate(r, glm::radians(sentYaw[i] + 40.0f), Ejy);
				r = glm::rotate(r, glm::radians(90.0f), Ejz);                    // de lado: su alto (Y) queda horizontal
				r = glm::scale(r, glm::vec3(escGrupo[1]));
				SetModel(P, r);
				rigSent[0].p[S_RAILGUN].modelo->Draw(sh);
			}
		}
	}
}

int main(int argc, char** argv)
{
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "T68 Sci-Fi Tribute", nullptr, nullptr);
	if (nullptr == window) { std::cout << "Failed to create GLFW window" << std::endl; glfwTerminate(); return EXIT_FAILURE; }

	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialise GLAD" << std::endl;
		return EXIT_FAILURE;
	}
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");

	Model Plaza((char*)"Models/plaza/plaza.obj");
	Model Cuau((char*)"Models/CUAU-68/cuau_sprint.obj");
	Model Sentinel((char*)"Models/SENTINEL/sentinel_aim.obj");
	Model Railgun((char*)"Models/SENTINEL/railgun_aim.obj");
	Model MacCuerpo((char*)"Models/MAC/mac_cuerpo.obj");
	Model Aspa((char*)"Models/MAC/aspa.obj");
	mdCuau = &Cuau; mdSentinel = &Sentinel; mdRailgun = &Railgun; mdMac = &MacCuerpo; mdAspa = &Aspa;

	Shader hudShader("Shader/hud.vs", "Shader/hud.frag");
	InitHUD();

	// Modelos por partes (jerarquia)
	CargarRig(rigCuau, "Models/CUAU-68/partes", DEF_CUAU, N_CUAU, ALTURA_CUAU);
	CargarRig(rigSent[0], "Models/SENTINEL/partes", DEF_SENT, N_SENT, ALTURA_SENT);
	rigSent[1] = rigSent[0]; rigSent[2] = rigSent[0];
	CargarRig(rigObrero[0], "Models/OBRERO", DEF_OBRERO_UNAM, N_OBRERO, 0.0f);
	CargarRig(rigObrero[1], "Models/OBRERO", DEF_OBRERO_IPN,  N_OBRERO, 0.0f);

	lightingShader.Use();
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.difuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);

	glm::mat4 projection = glm::perspective(camera.GetZoom(),
		(GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 1000.0f);

	// ---- opciones de linea de comandos (pruebas): --escena N | --teclas t:TECLA,t:TECLA (SPACE ENTER LEFT RIGHT) | --foto archivo.png tiempo ----
	struct TeclaAuto { float t; int key; bool hecha; };
	std::vector<TeclaAuto> teclasAuto;
	std::string fotoRuta; float fotoT = -1.0f; bool camForzada = false, sinHud = false;
	IniciarEscena(0);                                  // el relato empieza en la introduccion (--escena N lo cambia)
	CargarAjustes();
	for (int i = 1; i < argc; i++)
	{
		std::string a = argv[i];
		if (a == "--escena" && i + 1 < argc) IrAEscena(atoi(argv[++i]));
		else if (a == "--sinhud") sinHud = true;   // capturas limpias: no dibuja el texto del relato
		else if (a == "--foto" && i + 2 < argc) { fotoRuta = argv[++i]; fotoT = (float)atof(argv[++i]); }
		else if (a == "--cam" && i + 6 < argc)   // camara fija para pruebas: --cam px py pz mx my mz
		{
			camPos = glm::vec3((float)atof(argv[i + 1]), (float)atof(argv[i + 2]), (float)atof(argv[i + 3]));
			camMira = glm::vec3((float)atof(argv[i + 4]), (float)atof(argv[i + 5]), (float)atof(argv[i + 6]));
			camForzada = true; i += 6;
		}
		else if (a == "--teclas" && i + 1 < argc)
		{
			std::string s = argv[++i]; size_t p = 0;
			while (p < s.size())
			{
				size_t e = s.find(',', p); if (e == std::string::npos) e = s.size();
				std::string par = s.substr(p, e - p); size_t c = par.find(':');
				if (c != std::string::npos)
				{
					std::string k = par.substr(c + 1);
					int key = (k == "SPACE") ? GLFW_KEY_SPACE : (k == "ENTER") ? GLFW_KEY_ENTER : (k == "LEFT") ? GLFW_KEY_LEFT :
					          (k == "RIGHT") ? GLFW_KEY_RIGHT : (k == "PGDN") ? GLFW_KEY_PAGE_DOWN : (k == "PGUP") ? GLFW_KEY_PAGE_UP :
					          (k == "TAB") ? GLFW_KEY_TAB : (k == "F3") ? GLFW_KEY_F3 : (k.size() == 1) ? (int)k[0] : GLFW_KEY_RIGHT;
					teclasAuto.push_back({ (float)atof(par.substr(0, c).c_str()), key, false });
				}
				p = e + 1;
			}
		}
	}
	if (camForzada) { camLibre = false; }
	lastFrame = glfwGetTime();    // el tiempo de carga de modelos no cuenta como primer cuadro

	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		DoMovement();
		Animation();
		if (modoRelato)
		{
			ActualizarRelato(deltaTime);
			for (TeclaAuto& k : teclasAuto) if (!k.hecha && tReloj >= k.t) { k.hecha = true; KeyCallback(window, k.key, 0, GLFW_PRESS, 0); KeyCallback(window, k.key, 0, GLFW_RELEASE, 0); }
			if (cerrarRelato) glfwSetWindowShouldClose(window, GL_TRUE);
		}
		const bool vistaRelato = modoRelato && !camLibre;
		const glm::vec3 posVista = vistaRelato ? camPos : camera.GetPosition();
		const glm::mat4 view = vistaRelato ? glm::lookAt(camPos, camMira, glm::vec3(0.0f, 1.0f, 0.0f)) : camera.GetViewMatrix();

		glClearColor(0.53f, 0.74f, 0.90f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		lightingShader.Use();
		GLuint P = lightingShader.Program;
		glUniform1i(glGetUniformLocation(P, "diffuse"), 0);
		glUniform3f(glGetUniformLocation(P, "viewPos"),
			posVista.x, posVista.y, posVista.z);

		glUniform3f(glGetUniformLocation(P, "dirLight.direction"), -0.3f, -1.0f, -0.4f);
		glUniform3f(glGetUniformLocation(P, "dirLight.ambient"), 0.5f, 0.5f, 0.5f);
		glUniform3f(glGetUniformLocation(P, "dirLight.diffuse"), 0.8f, 0.8f, 0.8f);
		glUniform3f(glGetUniformLocation(P, "dirLight.specular"), 0.1f, 0.1f, 0.1f);

		// ---- matrices de los 3 SENTINEL (barrido tipo Scarface) ----
		glm::mat4 mSent[3];
		float retro[3];
		bool destello[3];
		glm::vec3 luzPos(0.0f);
		bool hayDestello = false;
		for (int i = 0; i < 3; i++)
		{
			// la formacion de los 3 SENTINEL gira alrededor del centro del grupo
			float gy = glm::radians(yawGrupo[1]);
			glm::vec3 o = modoRelato ? offLinea[i] : offSentinel[i];
			glm::vec3 oGirado(o.x * cos(gy) + o.z * sin(gy), o.y, -o.x * sin(gy) + o.z * cos(gy));
			glm::vec3 pos = posSentinel + oGirado;
			glm::vec3 d = posCuau - pos;
			float yawBase = glm::degrees(atan2(d.x, d.z));  // el canon de sentinel_aim apunta a +Z local
			float barrido = animScarface ? (modoRelato ? 14.0f : 60.0f) * sin(1.5f * tScar + i * 0.9f) : 0.0f;   // en el relato barren poco: apuntan a CUAU
			float ciclo = fmod(tScar + i * 0.15f, 0.45f);
			destello[i] = animScarface && ciclo < 0.07f;
			retro[i] = (animScarface && ciclo < 0.12f) ? (1.0f - ciclo / 0.12f) * RETROCESO_MAX : 0.0f;
			float vibra = destello[i] ? 0.08f * sin(120.0f * tScar) : 0.0f;

			glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, vibra, 0.0f));
			m = glm::rotate(m, glm::radians(yawBase + barrido), glm::vec3(0.0f, 1.0f, 0.0f));
			m = glm::scale(m, glm::vec3(escGrupo[1]));
			mSent[i] = m;

			if (destello[i] && !hayDestello) {
				glm::mat4 mr = glm::translate(m, glm::vec3(0.0f, 0.0f, -retro[i]));
				luzPos = glm::vec3(mr * glm::vec4(bocaLocal, 1.0f));
				hayDestello = true;
			}
		}

		// Luz puntual = destello de la boca del railgun
		if (hayDestello) {
			glUniform3f(glGetUniformLocation(P, "pointLights[0].position"), luzPos.x, luzPos.y, luzPos.z);
			glUniform3f(glGetUniformLocation(P, "pointLights[0].ambient"), 0.4f, 0.25f, 0.1f);
			glUniform3f(glGetUniformLocation(P, "pointLights[0].diffuse"), 3.0f, 1.8f, 0.6f);
			glUniform3f(glGetUniformLocation(P, "pointLights[0].specular"), 1.0f, 0.8f, 0.4f);
		} else {
			glUniform3f(glGetUniformLocation(P, "pointLights[0].position"), 0, 0, 0);
			glUniform3f(glGetUniformLocation(P, "pointLights[0].ambient"), 0, 0, 0);
			glUniform3f(glGetUniformLocation(P, "pointLights[0].diffuse"), 0, 0, 0);
			glUniform3f(glGetUniformLocation(P, "pointLights[0].specular"), 0, 0, 0);
		}
		glUniform1f(glGetUniformLocation(P, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(P, "pointLights[0].linear"), 0.022f);
		glUniform1f(glGetUniformLocation(P, "pointLights[0].quadratic"), 0.0019f);

		// Spot apagado
		glUniform3f(glGetUniformLocation(P, "spotLight.position"), 0, 0, 0);
		glUniform3f(glGetUniformLocation(P, "spotLight.direction"), 0, -1, 0);
		glUniform3f(glGetUniformLocation(P, "spotLight.ambient"), 0, 0, 0);
		glUniform3f(glGetUniformLocation(P, "spotLight.diffuse"), 0, 0, 0);
		glUniform3f(glGetUniformLocation(P, "spotLight.specular"), 0, 0, 0);
		glUniform1f(glGetUniformLocation(P, "spotLight.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(P, "spotLight.linear"), 0.3f);
		glUniform1f(glGetUniformLocation(P, "spotLight.quadratic"), 0.7f);
		glUniform1f(glGetUniformLocation(P, "spotLight.cutOff"), glm::cos(glm::radians(12.0f)));
		glUniform1f(glGetUniformLocation(P, "spotLight.outerCutOff"), glm::cos(glm::radians(18.0f)));

		glUniform1f(glGetUniformLocation(P, "material.shininess"), 5.0f);
		glUniform1i(glGetUniformLocation(P, "transparency"), 0);
		glUniform1f(glGetUniformLocation(P, "emissive"), 0.0f);
		glUniform1f(glGetUniformLocation(P, "brillo"), 1.0f);

		// (la matriz 'view' ya se calculo arriba: camara del relato o camara libre)
		glUniformMatrix4fv(glGetUniformLocation(P, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(P, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// ---- Plaza ----
		SetModel(P, glm::scale(glm::mat4(1.0f), glm::vec3(PLAZA_ESCALA)));
		Plaza.Draw(lightingShader);

		if (modoRelato) DibujarRelato(lightingShader, mSent, retro);
		else {   // banco de pruebas (modo desarrollador, F2): mallas de pose, rigs con G / R / H y obreros con F

		// ---- CUAU-68: sprint esquivando en zigzag ----
		{
			glm::mat4 m = glm::translate(glm::mat4(1.0f), posCuau + cuauOffset);
			m = glm::rotate(m, glm::radians(cuauYaw + CUAU_YAW_OFFSET + yawGrupo[0]), glm::vec3(0.0f, 1.0f, 0.0f));
			m = glm::rotate(m, glm::radians(cuauTilt), glm::vec3(0.0f, 0.0f, 1.0f));
			m = glm::scale(m, glm::vec3(escGrupo[0]));
			if (modoRig)
				DibujarRig(rigCuau, glm::rotate(m, glm::radians(CUAU_RIG_YAW), glm::vec3(0.0f, 1.0f, 0.0f)), lightingShader);
			else
			{
				SetModel(P, m);
				Cuau.Draw(lightingShader);
			}
		}

		// ---- 3 SENTINEL con railgun ----
		for (int i = 0; i < 3; i++)
		{
			if (modoRig)
			{
				DibujarRig(rigSent[i], glm::rotate(mSent[i], glm::radians(SENT_RIG_YAW), glm::vec3(0.0f, 1.0f, 0.0f)), lightingShader);
				continue;
			}
			SetModel(P, mSent[i]);
			Sentinel.Draw(lightingShader);
			SetModel(P, glm::translate(mSent[i], glm::vec3(0.0f, 0.0f, -retro[i])));
			Railgun.Draw(lightingShader);
		}

		// ---- Obreros (prueba): circulo alrededor de CUAU, todos miran al centro y martillan con un pequeño desfase ----
		if (verObreros)
		{
			const int NOB = 6;
			for (int i = 0; i < NOB; i++)
			{
				float a = 6.2832f * i / NOB + 0.4f;
				glm::vec3 pos = posCuau + glm::vec3(6.0f * sin(a), 0.0f, 6.0f * cos(a));
				glm::vec3 d = posCuau - pos;
				glm::mat4 m = glm::translate(glm::mat4(1.0f), pos);
				m = glm::rotate(m, atan2(d.x, d.z), glm::vec3(0.0f, 1.0f, 0.0f));   // el obrero mira a +Z
				Rig& ob = rigObrero[i % 2];                       // se alternan UNAM e IPN
				ob.p[OB_BRAZO].rot.x = AnguloMartillo(tGolpe - i * 0.10f);
				DibujarRig(ob, m, lightingShader);
			}
		}

		// ---- MAC con 3 sierras girando ----
		{
			glm::mat4 m = glm::translate(glm::mat4(1.0f), posMac);
			m = glm::rotate(m, glm::radians(yawGrupo[2]), glm::vec3(0.0f, 1.0f, 0.0f));
			m = glm::scale(m, glm::vec3(escGrupo[2]));
			m = glm::translate(m, glm::vec3(-MAC_CENTRO.x, MAC_SUELO, -MAC_CENTRO.z));
			SetModel(P, m);
			MacCuerpo.Draw(lightingShader);

			for (int i = 0; i < 3; i++)
			{
				glm::mat4 a = glm::translate(m, glm::vec3(MAC_CENTRO.x, ASPA_Y[i], MAC_CENTRO.z));
				float sentido = (i % 2 == 0) ? 1.0f : -1.0f;
				a = glm::rotate(a, glm::radians(sentido * rotAspa), glm::vec3(0.0f, 0.0f, 1.0f));
				SetModel(P, a);
				Aspa.Draw(lightingShader);
			}
		}

		}   // fin del banco de pruebas

		if (modoRelato && !sinHud) DibujarHUDRelato(hudShader);

		// captura de pantalla para pruebas (--foto archivo.png tiempo): se toma antes de intercambiar buffers y se cierra
		if (fotoT >= 0.0f && tReloj >= fotoT)
		{
			SOIL_save_screenshot(fotoRuta.c_str(), SOIL_SAVE_TYPE_PNG, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			std::printf("captura guardada: %s\n", fotoRuta.c_str());
			break;
		}

		glfwSwapBuffers(window);
	}

	if (edSucio) GuardarAjustes();
	glfwTerminate();
	return 0;
}

void DoMovement()
{
	// En el relato la camara es del guion (las flechas son de la escena 4); solo se mueve con la camara libre (F1)
	if (camLibre || !modoRelato)
	{
		if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])    camera.ProcessKeyboard(FORWARD, deltaTime);
		if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])  camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])  camera.ProcessKeyboard(LEFT, deltaTime);
		if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT, deltaTime);
	}
	if (modoRelato)
	{
		if (modoEdicion)   // J/L = X, I/K = Z, U/O = Y (+ / -), Q/E = girar: mueven al actor seleccionado (TAB)
		{
			float v = 6.0f * deltaTime;
			glm::vec4& o = edOff[escena][edSel];
			if (keys[GLFW_KEY_J]) o.x -= v;   if (keys[GLFW_KEY_L]) o.x += v;
			if (keys[GLFW_KEY_I]) o.z -= v;   if (keys[GLFW_KEY_K]) o.z += v;
			if (keys[GLFW_KEY_U]) o.y += v;   if (keys[GLFW_KEY_O]) o.y -= v;
			if (keys[GLFW_KEY_Q]) o.w -= 60.0f * deltaTime;
			if (keys[GLFW_KEY_E]) o.w += 60.0f * deltaTime;
			if (keys[GLFW_KEY_J] || keys[GLFW_KEY_L] || keys[GLFW_KEY_I] || keys[GLFW_KEY_K] || keys[GLFW_KEY_U] || keys[GLFW_KEY_O] || keys[GLFW_KEY_Q] || keys[GLFW_KEY_E]) edSucio = true;
		}
		return;     // en el relato el resto es del banco de pruebas
	}

	// Mover el grupo seleccionado (TAB cambia): J/L = X, I/K = Z, U/O = Y
	float v = 10.0f * deltaTime;
	glm::vec3& p = *grupos[grupoSel];
	if (keys[GLFW_KEY_J]) p.x -= v;
	if (keys[GLFW_KEY_L]) p.x += v;
	if (keys[GLFW_KEY_I]) p.z -= v;
	if (keys[GLFW_KEY_K]) p.z += v;
	if (keys[GLFW_KEY_U]) p.y += v;
	if (keys[GLFW_KEY_O]) p.y -= v;

	// Q / E = girar el grupo seleccionado
	if (keys[GLFW_KEY_Q]) yawGrupo[grupoSel] -= 90.0f * deltaTime;
	if (keys[GLFW_KEY_E]) yawGrupo[grupoSel] += 90.0f * deltaTime;

	// Z = achicar, X = agrandar el grupo seleccionado
	if (keys[GLFW_KEY_Z]) escGrupo[grupoSel] *= (1.0f - 0.8f * deltaTime);
	if (keys[GLFW_KEY_X]) escGrupo[grupoSel] *= (1.0f + 0.8f * deltaTime);
}

void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS) keys[key] = true;
		else if (action == GLFW_RELEASE) keys[key] = false;
	}

	if (action == GLFW_PRESS)
	{
		// ---- modo desarrollador (siempre disponible) ----
		if (key == GLFW_KEY_F1) {   // camara libre: WASD / flechas + mouse; empieza donde estaba la camara del relato
			camLibre = !camLibre;
			if (camLibre) camera.SetLookAt(camPos, camMira);
			std::printf("Camara libre: %s\n", camLibre ? "SI" : "NO");
		}
		if (key == GLFW_KEY_F2) { modoRelato = !modoRelato; std::printf("Modo: %s\n", modoRelato ? "RELATO" : "BANCO DE PRUEBAS"); }
		// escena anterior / siguiente: AvPag y RePag (iguales en cualquier teclado) o las teclas a la derecha de la P ([ y ] en teclado ingles)
		if (key == GLFW_KEY_PAGE_UP   || key == GLFW_KEY_LEFT_BRACKET)  IrAEscena(escena - 1);
		if (key == GLFW_KEY_PAGE_DOWN || key == GLFW_KEY_RIGHT_BRACKET) IrAEscena(escena + 1);
		if (key == GLFW_KEY_C) {    // imprime el punto de camara con el formato de la tabla ESC[]
			glm::vec3 p = (camLibre || !modoRelato) ? camera.GetPosition() : camPos;
			glm::vec3 m = (camLibre || !modoRelato) ? camera.GetPosition() + camera.GetFront() * 10.0f : camMira;
			std::printf("glm::vec3(%.1f, %.1f, %.1f), glm::vec3(%.1f, %.1f, %.1f)\n", p.x, p.y, p.z, m.x, m.y, m.z);
		}

		if (key == GLFW_KEY_F3 && modoRelato) {
			modoEdicion = !modoEdicion;
			if (modoEdicion) {
				if (!ActorVisible(edSel)) SiguienteActor();
				std::printf("MODO EDICION: TAB elige actor | J L (x)  I K (z)  U O (altura)  Q E (giro) | P imprime | C imprime camara | F1 camara libre | [ ] cambian de escena\nEditando: %s\n", NOM_ACTOR[edSel]);
			} else { GuardarAjustes(); std::printf("Modo edicion: NO (ajustes guardados)\n"); }
		}
		if (modoRelato && modoEdicion) { if (key == GLFW_KEY_TAB) SiguienteActor(); if (key == GLFW_KEY_P) ImprimirAjustes(); }

		if (modoRelato) { if (!camLibre) ProcesarTeclaRelato(key); }
		else {
		if (key == GLFW_KEY_N) animSierras = !animSierras;
		if (key == GLFW_KEY_B) animScarface = !animScarface;
		if (key == GLFW_KEY_V) animEsquive = !animEsquive;
		if (key == GLFW_KEY_G) { modoRig = !modoRig; std::printf("Modelos por partes: %s\n", modoRig ? "SI" : "NO"); }
		if (key == GLFW_KEY_F) { verObreros = !verObreros; std::printf("Obreros: %s (ESPACIO = martillazo)\n", verObreros ? "SI" : "NO"); }
		if (key == GLFW_KEY_SPACE && verObreros) tGolpe = 0.0f;
		if (key == GLFW_KEY_R) { rigQuieto = !rigQuieto; std::printf("Modelos quietos (pose T): %s\n", rigQuieto ? "SI" : "NO"); }
		if (key == GLFW_KEY_H) {   // enciende / apaga visor, luces del casco y el 68 de CUAU
			rigCuau.luces = (rigCuau.luces > 0.5f) ? 0.0f : 1.0f;
			std::printf("Luces de CUAU: %s\n", rigCuau.luces > 0.5f ? "encendidas" : "apagadas");
		}
		if (key == GLFW_KEY_TAB) {
			grupoSel = (grupoSel + 1) % 3;
			std::printf("Moviendo: %s\n", nombreGrupo[grupoSel]);
		}
		if (key == GLFW_KEY_P) {
			std::printf("escalas CUAU %.3f  SENTINEL %.3f  MAC %.3f\n", escGrupo[0], escGrupo[1], escGrupo[2]);
			std::printf("giros  CUAU %.1f  SENTINEL %.1f  MAC %.1f\n", yawGrupo[0], yawGrupo[1], yawGrupo[2]);
			std::printf("posCuau(%.1f, %.1f, %.1f)  posSentinel(%.1f, %.1f, %.1f)  posMac(%.1f, %.1f, %.1f)\n",
				posCuau.x, posCuau.y, posCuau.z,
				posSentinel.x, posSentinel.y, posSentinel.z,
				posMac.x, posMac.y, posMac.z);
		}
		}   // fin del banco de pruebas
	}
}

// Anima: sierras, barrido Scarface (tiempo propio) y zigzag del CUAU
void Animation()
{
	if (animSierras) rotAspa += 540.0f * deltaTime;
	tGolpe += deltaTime;
	bool quieto = modoRig && rigQuieto;            // R: congela todo para revisar los modelos
	if (animScarface && !quieto) tScar += deltaTime;

	// Prueba de la jerarquia: ciclo de caminata en el lugar (CUAU y los 3 SENTINEL desfasados)
	if (modoRig)
	{
		if (quieto)
		{
			PoseT(rigCuau);
			for (int i = 0; i < 3; i++) { PoseT(rigSent[i]); SujetarRailgun(rigSent[i]); }
		}
		else
		{
			tRig += deltaTime;
			PoseCaminar(rigCuau, tRig, 28.0f, 5.0f);
			rigCuau.p[C_CABEZA].rot.y = 18.0f * sin(0.9f * tRig);
			for (int i = 0; i < 3; i++) { PoseCaminar(rigSent[i], tRig + i * 0.4f, 24.0f, 4.0f); SujetarRailgun(rigSent[i]); }
		}
	}

	if (quieto)
	{
		cuauOffset = glm::vec3(0.0f); cuauYaw = 0.0f; cuauTilt = 0.0f;   // CUAU de frente a la camara, sin zigzag
	}
	else if (animEsquive)
	{
		tEsq += deltaTime;
		float t = tEsq;

		float x = 6.0f * sin(2.0f * t);          // zigzag lateral
		float z = 4.0f * sin(0.7f * t);          // avance lento adelante/atras
		float salto = fabs(sin(4.0f * t)) * 1.2f; // brincos al esquivar
		cuauOffset = glm::vec3(x, salto, z);

		float vx = 12.0f * cos(2.0f * t);
		float vz = 2.8f * cos(0.7f * t);
		float objetivo = glm::degrees(atan2(vx, vz));

		// giro suave por el camino mas corto
		float dif = fmod(objetivo - cuauYaw + 540.0f, 360.0f) - 180.0f;
		float k = deltaTime * 10.0f;
		if (k > 1.0f) k = 1.0f;
		cuauYaw += dif * k;

		cuauTilt = 15.0f * cos(2.0f * t);        // se inclina al esquivar
	}
}

void MouseCallback(GLFWwindow *window, double xPos, double yPos)
{
	if (firstMouse) { lastX = xPos; lastY = yPos; firstMouse = false; }
	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;
	lastX = xPos; lastY = yPos;
	if (camLibre || !modoRelato) camera.ProcessMouseMovement(xOffset, yOffset);
}