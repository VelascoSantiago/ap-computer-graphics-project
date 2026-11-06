#include <iostream>
#include <cmath>
#include <cstdio>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
//  AJUSTES (lo que vas a tocar para acomodar la escena)
// ---------------------------------------------------------------------
const float PLAZA_ESCALA = 1.0f;
// Escalas iniciales (se cambian en vivo con Z / X sobre el grupo seleccionado)
float escGrupo[3] = { 0.35f, 0.35f, 0.35f };   // 0 CUAU, 1 SENTINEL, 2 MAC

// Angulo (grados) para que el CUAU mire hacia donde corre. Si corre de espaldas: 180.
const float CUAU_YAW_OFFSET = 0.0f;

// Boca del railgun en coordenadas locales del SENTINEL (OBJ). Ajustala viendo el destello.
glm::vec3 bocaLocal(9.0f, 9.0f, 0.0f);
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

const glm::vec3 offSentinel[3] = {
	glm::vec3(-16.0f, 0.0f, 0.0f),
	glm::vec3(  0.0f, 0.0f, -6.0f),
	glm::vec3( 16.0f, 0.0f, 0.0f)
};

bool animSierras = true;   // N
bool animScarface = true;  // B
bool animEsquive = true;   // V

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

int main()
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

	lightingShader.Use();
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.difuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);

	glm::mat4 projection = glm::perspective(camera.GetZoom(),
		(GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 1000.0f);

	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		DoMovement();
		Animation();

		glClearColor(0.53f, 0.74f, 0.90f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		lightingShader.Use();
		GLuint P = lightingShader.Program;
		glUniform1i(glGetUniformLocation(P, "diffuse"), 0);
		glUniform3f(glGetUniformLocation(P, "viewPos"),
			camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

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
			glm::vec3 o = offSentinel[i];
			glm::vec3 oGirado(o.x * cos(gy) + o.z * sin(gy), o.y, -o.x * sin(gy) + o.z * cos(gy));
			glm::vec3 pos = posSentinel + oGirado;
			glm::vec3 d = posCuau - pos;
			float yawBase = glm::degrees(atan2(-d.z, d.x)); // el canon apunta a +X local
			float barrido = animScarface ? 60.0f * sin(1.5f * tScar + i * 0.9f) : 0.0f;
			float ciclo = fmod(tScar + i * 0.15f, 0.45f);
			destello[i] = animScarface && ciclo < 0.07f;
			retro[i] = (animScarface && ciclo < 0.12f) ? (1.0f - ciclo / 0.12f) * RETROCESO_MAX : 0.0f;
			float vibra = destello[i] ? 0.08f * sin(120.0f * tScar) : 0.0f;

			glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, vibra, 0.0f));
			m = glm::rotate(m, glm::radians(yawBase + barrido), glm::vec3(0.0f, 1.0f, 0.0f));
			m = glm::scale(m, glm::vec3(escGrupo[1]));
			mSent[i] = m;

			if (destello[i] && !hayDestello) {
				glm::mat4 mr = glm::translate(m, glm::vec3(-retro[i], 0.0f, 0.0f));
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

		glm::mat4 view = camera.GetViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(P, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(P, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// ---- Plaza ----
		SetModel(P, glm::scale(glm::mat4(1.0f), glm::vec3(PLAZA_ESCALA)));
		Plaza.Draw(lightingShader);

		// ---- CUAU-68: sprint esquivando en zigzag ----
		{
			glm::mat4 m = glm::translate(glm::mat4(1.0f), posCuau + cuauOffset);
			m = glm::rotate(m, glm::radians(cuauYaw + CUAU_YAW_OFFSET + yawGrupo[0]), glm::vec3(0.0f, 1.0f, 0.0f));
			m = glm::rotate(m, glm::radians(cuauTilt), glm::vec3(0.0f, 0.0f, 1.0f));
			m = glm::scale(m, glm::vec3(escGrupo[0]));
			SetModel(P, m);
			Cuau.Draw(lightingShader);
		}

		// ---- 3 SENTINEL con railgun ----
		for (int i = 0; i < 3; i++)
		{
			SetModel(P, mSent[i]);
			Sentinel.Draw(lightingShader);
			SetModel(P, glm::translate(mSent[i], glm::vec3(-retro[i], 0.0f, 0.0f)));
			Railgun.Draw(lightingShader);
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

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

void DoMovement()
{
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])    camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])  camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])  camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT, deltaTime);

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
		if (key == GLFW_KEY_N) animSierras = !animSierras;
		if (key == GLFW_KEY_B) animScarface = !animScarface;
		if (key == GLFW_KEY_V) animEsquive = !animEsquive;
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
	}
}

// Anima: sierras, barrido Scarface (tiempo propio) y zigzag del CUAU
void Animation()
{
	if (animSierras) rotAspa += 540.0f * deltaTime;
	if (animScarface) tScar += deltaTime;

	if (animEsquive)
	{
		tEsq += deltaTime;
		float t = tEsq;

		float x = 6.0f * sin(2.0f * t);          // zigzag lateral
		float z = 4.0f * sin(0.7f * t);          // avance lento adelante/atras
		float salto = fabs(sin(4.0f * t)) * 1.2f; // brincos como esquivando balas
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
	camera.ProcessMouseMovement(xOffset, yOffset);
}