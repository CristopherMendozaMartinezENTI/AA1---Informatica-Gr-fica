#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include <cassert>
#include <glm/gtc/matrix_transform.hpp> // lookAt
#include "GL_framework.h"
#include <vector>
#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <iostream>
#include <glm/gtx/transform.hpp>

std::vector< glm::vec3 > verticesChicken;
std::vector< glm::vec2 > uvsChicken;
std::vector< glm::vec3 > normalsChicken;

std::vector< glm::vec3 > verticesTrump;
std::vector< glm::vec2 > uvsTrump;
std::vector< glm::vec3 > normalsTrump;

std::vector< glm::vec3 > verticesCabine;
std::vector< glm::vec2 > uvsCabine;
std::vector< glm::vec3 > normalsCabine;

std::vector< glm::vec3 > verticesSupports;
std::vector< glm::vec2 > uvsSupports;
std::vector< glm::vec3 > normalsSupports;

std::vector< glm::vec3 > verticesWheel;
std::vector< glm::vec2 > uvsWheel;
std::vector< glm::vec3 > normalsWheel;

extern bool loadOBJ(const char * path,
	std::vector < glm::vec3 > & out_vertices,
	std::vector < glm::vec2 > & out_uvs,
	std::vector < glm::vec3 > & out_normals
);

//Para realizar el Dolly Zoom trabajeros con nuestro propio struct Camera 
struct Camera {
	float CameraPosition[3] = { 0.f, -21.f, -70.0f }; //Inicializamos la posicion inicial de la camara 
	float t; //Contador de tiempo
	float d; // Distancia inicial entre la camara y el objeto
	int width{ 1920 };
	int height{ 1080 };

	bool DollyZoom{ false };
	bool DollyLoop{ false };

	Camera() : t(0.33), d(255.0), width(0), height(0) {}

	//Llamamos a esta funcion cada vez que hacemos un resize de la ventana de esta manera actulizamos tambien el viewport de la camara.
	void resize(int w, int h) {
		glViewport(0, 0, w, h);
		width = w;
		height = h;
	}
};

Camera *cameraOptions;

bool show_test_window = false;

//Variables que utilizaremos en la interfaz de usuario
glm::vec3 LightColor(0.6f, 0.6f, 0.7f);
glm::vec3 ObjectColor(0.8f, 0.8f, 0.8f);
glm::vec3 lightPos(41.f, 23.f, 32.f);
glm::vec3 ViewPos(0.0f, 0.0f, 0.0f);
float ambientStrength = 1.8f;
float specularStrength = 35.3f;
float shininess = 32.f;
float FOV = glm::radians(50.f);
float pos_0[3] = { -8.0f, 0.0f, 0.0f };
float pos_1[3] = { -8.0f, 0.0f, 0.0f };
float pos_2[3] = { -8.0f, 0.0f, 0.0f };

int camera = 0;
float distanceCenter = 24;
float distanceCabin = (2 * 3.1415) / 20;
float speedMultiplayer = 0.5;
glm::vec3 centerScene = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 lightColor = glm::vec3(0.5f, 0.5f, 0.1f);
bool focusTrump = false;

//Exercise variables
int exercise = 0;
void SetExercise(int id)
{
	exercise = id;
}
int GetExercise()
{
	return exercise;
}int timer = 0;


void GUI() {
	bool show = true;
	ImGui::Begin("Simulation Parameters", &show, 0);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate

	ImGui::DragFloat3("Pos Trump", &pos_0[0]);
	ImGui::DragFloat3("Pos Pollo", &pos_1[0]);
	ImGui::DragFloat3("Pos Cabina", &pos_2[0]);

	ImGui::DragFloat("PosCenter", &distanceCenter, 0.1f);

	ImGui::DragFloat("Ambient Strength", &ambientStrength, 0.1f);
	ImGui::DragFloat("Specular Strength", &specularStrength, 0.1f);
	ImGui::DragFloat("Shininess", &shininess, 0.1f);
	ImGui::ColorEdit3("Light Color", &LightColor.x);
	ImGui::ColorEdit3("Model Color", &ObjectColor.x);
	ImGui::DragFloat3("Light Pos", &lightPos.x);
	ImGui::DragFloat3("View Pos", &ViewPos.x);
	ImGui::DragFloat3("Camera Position", { cameraOptions->CameraPosition }, 0.05f);

	if (ImGui::Button("Start Dolly Zoom")) {
		cameraOptions->DollyZoom = !cameraOptions->DollyZoom;
	}

	if (ImGui::Button("Reset Simulation"))
	{
		cameraOptions->CameraPosition[0] =  0.f;
		cameraOptions->CameraPosition[1] = -5.f;
		cameraOptions->CameraPosition[2] = -24.f;
		FOV = glm::radians(50.f);
	}

	ImGui::End();

	
	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 60), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

namespace RenderVars {
	float FOV = glm::radians(50.f);
	const float zNear = 1.f;
	const float zFar = 500.f;

	glm::mat4 _projection;
	glm::mat4 _modelView;
	glm::mat4 _MVP;
	glm::mat4 _inv_modelview;
	glm::vec4 _cameraPoint;

	struct prevMouse {
		float lastx, lasty;
		MouseEvent::Button button = MouseEvent::Button::None;
		bool waspressed = false;
	} prevMouse;

	float panv[3] = { 0.f, -20.f, -17.f };
	float rota[2] = { 0.f, -50.f };
}
namespace RV = RenderVars;

///////// fw decl
namespace ImGui {
	void Render();
}
namespace Box {
	void setupBox();
	void cleanupBox();
	void drawBox();
}

namespace Cube {
	void setupCube();
	void cleanupCube();
	void drawCube();
	void draw2Cubes();
	void draw2CubesMore();
}

namespace Axis {
	void setupAxis();
	void cleanupAxis();
	void drawAxis();
}

namespace MyLoadedModel {
	void setupModel();
	void cleanupModel();
	void updateModel(const glm::mat4& transform);
	void drawModel();
}

namespace MyLoadedModel2 {
	void setupModel();
	void cleanupModel();
	void updateModel(const glm::mat4& transform);
	void drawModel();
}

namespace MyLoadedModel3 {
	void setupModel();
	void cleanupModel();
	void updateModel(const glm::mat4& transform);
	void drawModel();
}

namespace MyLoadedModel4 {
	void setupModel();
	void cleanupModel();
	void updateModel(const glm::mat4& transform);
	void drawModel();
}

namespace MyLoadedModel5 {
	void setupModel();
	void cleanupModel();
	void updateModel(const glm::mat4& transform);
	void drawModel();
}


////////////////
void GLResize(int width, int height) {
	glViewport(0, 0, width, height);
	cameraOptions->resize(width, height);
	if (height != 0) RV::_projection = glm::perspective(FOV, (float)width / (float)height, RV::zNear, RV::zFar);
	else RV::_projection = glm::perspective(FOV, 0.f, RV::zNear, RV::zFar);
}

void GLmousecb(MouseEvent ev) {
	if (RV::prevMouse.waspressed && RV::prevMouse.button == ev.button) {
		float diffx = ev.posx - RV::prevMouse.lastx;
		float diffy = ev.posy - RV::prevMouse.lasty;
		switch (ev.button) {
		case MouseEvent::Button::Left: // ROTATE
			RV::rota[0] += diffx * 0.005f;
			RV::rota[1] += diffy * 0.005f;
			ViewPos.x += diffx;
			ViewPos.y += diffy;
			ViewPos.z += diffy;
			break;
		case MouseEvent::Button::Right: // MOVE XY
			RV::panv[0] += diffx * 0.03f;
			RV::panv[1] -= diffy * 0.03f;
			break;
		case MouseEvent::Button::Middle: // MOVE Z
			RV::panv[2] += diffy * 0.05f;
			break;
		default: break;
		}
	}
	else {
		RV::prevMouse.button = ev.button;
		RV::prevMouse.waspressed = true;
	}
	RV::prevMouse.lastx = ev.posx;
	RV::prevMouse.lasty = ev.posy;
}

void GLinit(int width, int height) {
	//Inicializamos una nueva Camara
	//Igualmaos los valores del with y el height de la ventano con los de las variables width y height de la camara y hacemos un resize
	cameraOptions = new Camera();
	cameraOptions->width = width;
	cameraOptions->height = height;
	//cameraOptions->resize(width, height);

	glViewport(0, 0, width, height);
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	RV::_projection = glm::perspective(FOV, (float)width / (float)height, RV::zNear, RV::zFar);

	//Hacemos un setup de todos los elementos dentro de la escena 
	bool res = loadOBJ("chicken.obj", verticesChicken, uvsChicken, normalsChicken);
	bool res2 = loadOBJ("trump.obj", verticesTrump, uvsTrump, normalsTrump);
	bool res3 = loadOBJ("Cabine.obj", verticesCabine, uvsCabine, normalsCabine);
	bool res4 = loadOBJ("Supports.obj", verticesSupports, uvsSupports, normalsSupports);
	bool res5 = loadOBJ("Wheel.obj", verticesWheel, uvsWheel, normalsWheel);

	MyLoadedModel::setupModel();
	MyLoadedModel2::setupModel();
	MyLoadedModel3::setupModel();
	MyLoadedModel4::setupModel();
	MyLoadedModel5::setupModel();
	Box::setupBox();
	Cube::setupCube();

}

void GLcleanup() {
	Box::cleanupBox();
	Axis::cleanupAxis();
	MyLoadedModel::cleanupModel();
	MyLoadedModel2::cleanupModel();
	MyLoadedModel3::cleanupModel();
	MyLoadedModel4::cleanupModel();
	MyLoadedModel5::cleanupModel();
}

void GLrender(float dt) {

	//Igualamos el tiempo (que se cuenta durante la ejecucion del programa) al contador de tiempo de la camara
	cameraOptions->t = dt;

	//Ponemos en practica la formula matematica del Dolly Zoom para conseguir la distancia entre el objeto y la camara
	cameraOptions->d = cameraOptions->width / (2 * glm::tan(FOV / 2));

	if (cameraOptions->DollyZoom) {
		//Multiplicamos el tiempo que ha pasado por la distancia entre el objeto y la camara 
		//Seguidamente lo igualamos a la posicion de la camara en Z para realizar el zoom de forma dinamica
		//Al final dividimos todo esto entro 2 para que la camara se mantenga cerca del modelo en la relacion al tama�o de la ventana 
		cameraOptions->CameraPosition[2] = (-cameraOptions->t*cameraOptions->d) / 2;

		//"Abrimos" el FOV o lo "cerramos" mientras realizamos el zoom 
		if (cameraOptions->DollyLoop) {
			FOV -= 0.03f;
			if (FOV < 0.5f)
				cameraOptions->DollyLoop = false;
		}
		else
		{
			FOV += 0.03f;
			if (FOV > 2.5f)
				cameraOptions->DollyLoop = true;
		}
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RV::_modelView = glm::mat4(1.f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glm::vec3 focusCabin;
	glm::vec3 camaraTrumpChicken;
	//Realiazmos las transformaciones necesarias para colocar la camara delante de nuestro modelo
	RV::_modelView = glm::mat4(1.f);
	switch (camera)
	{
	case 0:
		RV::_modelView = glm::translate(RV::_modelView, glm::vec3(cameraOptions->CameraPosition[0], cameraOptions->CameraPosition[1], cameraOptions->CameraPosition[2]));
		RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1], glm::vec3(1.f, 0.f, 0.f));
		RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0], glm::vec3(0.f, 1.f, 0.f));

		//RV::_modelView = glm::translate(RV::_modelView, glm::vec3(0.0f, -20.0f, -70.0f));
		//RV::_modelView = glm::rotate(RV::_modelView, glm::radians(60.0f), glm::vec3(0.f, 1.f, 0.f));
		break;
	case 1:
		timer += dt;
		if (timer / 100 > 4)
		{
			timer = 0;
			focusTrump = !focusTrump;
		}


		if (focusTrump)
		{
			camaraTrumpChicken = glm::vec3(distanceCenter*cos((float)dt*speedMultiplayer + distanceCabin) - 1, distanceCenter *sin((float)dt*speedMultiplayer + distanceCabin) + 20, 0);
			RV::_modelView = glm::lookAt(camaraTrumpChicken, glm::vec3(camaraTrumpChicken.x + 1, camaraTrumpChicken.y, camaraTrumpChicken.z), glm::vec3(0.0, 1.0, 0.0));
		}
		else
		{
			camaraTrumpChicken = glm::vec3(distanceCenter*cos((float)dt*speedMultiplayer + distanceCabin) + 1.5, distanceCenter *sin((float)dt*speedMultiplayer + distanceCabin) + 19, 0);
			RV::_modelView = glm::lookAt(camaraTrumpChicken, glm::vec3(camaraTrumpChicken.x - 1, camaraTrumpChicken.y, camaraTrumpChicken.z), glm::vec3(0.0, 1.0, 0.0));
		}
		break;
	case 2:
		focusCabin = glm::vec3(distanceCenter*cos((float)dt*speedMultiplayer + distanceCabin), distanceCenter *sin((float)dt*speedMultiplayer + distanceCabin) + 20, 0);
		RV::_modelView = glm::lookAt(glm::vec3(focusCabin.x, focusCabin.y, focusCabin.z - 10), focusCabin, glm::vec3(0.0, 1.0, 0.0));
		break;
	case 3:
		glm::mat4 matrix;
		focusCabin = glm::vec3(distanceCenter*cos((float)dt*speedMultiplayer + distanceCabin), distanceCenter *sin((float)dt*speedMultiplayer + distanceCabin) + 20, 0);
		glm::vec3 focusCamara = glm::vec3(focusCabin.x, focusCabin.y - 10, focusCabin.z);
		focusCabin = glm::vec3(focusCabin.x, focusCabin.y + 0.5, focusCabin.z);
		matrix = glm::translate(matrix, focusCabin);
		matrix = glm::rotate(matrix, (float)dt, glm::vec3(0.0, 1.0, 0.0));
		glm::vec4 aux = glm::vec4(1.0);
		aux = matrix * aux;
		RV::_modelView = glm::lookAt(glm::vec3(aux.x, aux.y, aux.z), focusCamara, glm::vec3(0.0, 1.0, 0.0));
		break;
	}

	//RV::_modelView = glm::translate(RV::_modelView, glm::vec3(cameraOptions->CameraPosition[0], cameraOptions->CameraPosition[1], cameraOptions->CameraPosition[2]));
	//RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1], glm::vec3(1.f, 0.f, 0.f));
	//RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0], glm::vec3(0.f, 1.f, 0.f));

	RV::_MVP = RV::_projection * RV::_modelView;

	//Realizamos la proyeccion respecto al FOV. Este varia durante el dolly zoom, de esta manera creamos ese efecto optico.
	RV::_projection = glm::perspective(FOV, (float)cameraOptions->width / (float)cameraOptions->height, RV::zNear, RV::zFar);

	//Renderizamos los modelos 

	//MyLoadedModel3::drawModel();
	glm::mat4 model;
	std::cout << exercise << std::endl;
	switch (exercise) {
	case 0:
		//Trump
		model = glm::mat4(1.0);
		model = glm::translate(model, { pos_0[0], pos_0[1], pos_0[2] });
		model = glm::translate(model, glm::vec3(glm::vec3(distanceCenter*cos((float)dt*speedMultiplayer + distanceCabin) + 1, distanceCenter *sin((float)dt*speedMultiplayer + distanceCabin) - 2.5, 0)));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.005f));
		MyLoadedModel2::updateModel(model);
		MyLoadedModel2::drawModel();

		//Chicken
		model = glm::mat4(1.0);
		model = glm::translate(model, { pos_1[0], pos_1[1], pos_1[2] });
		model = glm::translate(model, glm::vec3(glm::vec3(distanceCenter*cos((float)dt*speedMultiplayer + distanceCabin) - 0.5, distanceCenter *sin((float)dt*speedMultiplayer + distanceCabin) - 2, 0)));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.005f));
		MyLoadedModel::updateModel(model);
		MyLoadedModel::drawModel();

		//Wheel
		model = glm::mat4(1.0);
		model = glm::translate(model, centerScene);
		model = glm::rotate(model, (float)dt*speedMultiplayer, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(0.7f));
		MyLoadedModel5::updateModel(model);
		MyLoadedModel5::drawModel();

		//Feet Wheel
		model = glm::mat4(1.0);
		model = glm::translate(model, centerScene);
		model = glm::scale(model, glm::vec3(0.7f));
		MyLoadedModel4::updateModel(model);
		MyLoadedModel4::drawModel();

		MyLoadedModel::drawModel();
		MyLoadedModel2::drawModel();


		for (int i = 0; i < 20; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, { -0.5f, 25.0f, pos_2[2] });
			model = glm::translate(model, glm::vec3(glm::vec3(distanceCenter * cos((float)(dt * speedMultiplayer + distanceCabin * i)), distanceCenter * sin((float)(dt * speedMultiplayer + distanceCabin * i)), 0)));
			model = glm::scale(model, glm::vec3(0.5f));
			MyLoadedModel3::updateModel(model);
			MyLoadedModel3::drawModel();
		}
		break;
	case 1:
		break;
	case 2:
		break;
	}

	Cube::drawCube();
	ImGui::Render();
}


//////////////////////////////////// COMPILE AND LINK
GLuint compileShader(const char* shaderStr, GLenum shaderType, const char* name = "") {
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderStr, NULL);
	glCompileShader(shader);
	GLint res;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
	if (res == GL_FALSE) {
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &res);
		char *buff = new char[res];
		glGetShaderInfoLog(shader, res, &res, buff);
		fprintf(stderr, "Error Shader %s: %s", name, buff);
		delete[] buff;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}
void linkProgram(GLuint program) {
	glLinkProgram(program);
	GLint res;
	glGetProgramiv(program, GL_LINK_STATUS, &res);
	if (res == GL_FALSE) {
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &res);
		char *buff = new char[res];
		glGetProgramInfoLog(program, res, &res, buff);
		fprintf(stderr, "Error Link: %s", buff);
		delete[] buff;
	}
}


////////////////////////////////////////////////// BOX
namespace Box {
	GLuint BoxVao;
	GLuint BoxVbo[2];
	GLuint BoxShaders[2];
	GLuint BoxProgram;

	float BoxVerts[] = {
		// -5,0,-5 -- 5, 10, 5
		-5.f,  0.f, -5.f,
		 5.f,  0.f, -5.f,
		 5.f,  0.f,  5.f,
		-5.f,  0.f,  5.f,
		-5.f, 10.f, -5.f,
		 5.f, 10.f, -5.f,
		 5.f, 10.f,  5.f,
		-5.f, 10.f,  5.f,
	};
	GLubyte BoxIdx[] = {
		1, 0, 2, 3, // Floor - TriangleStrip
		0, 1, 5, 4, // Wall - Lines
		1, 2, 6, 5, // Wall - Lines
		2, 3, 7, 6, // Wall - Lines
		3, 0, 4, 7  // Wall - Lines
	};

	const char* vertShader_xform =
		"#version 330\n\
	in vec3 in_Position;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		gl_Position = mvpMat * vec4(in_Position, 1.0);\n\
	}";
	const char* fragShader_flatColor =
		"#version 330\n\
	out vec4 out_Color;\n\
	uniform vec4 color;\n\
	uniform mat4 vert_Normal; \n\
	void main() {\n\
		out_Color = color; \n\
	}";

	void setupBox() {
		glGenVertexArrays(1, &BoxVao);
		glBindVertexArray(BoxVao);
		glGenBuffers(2, BoxVbo);

		glBindBuffer(GL_ARRAY_BUFFER, BoxVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, BoxVerts, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BoxVbo[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 20, BoxIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		BoxShaders[0] = compileShader(vertShader_xform, GL_VERTEX_SHADER, "cubeVert");
		BoxShaders[1] = compileShader(fragShader_flatColor, GL_FRAGMENT_SHADER, "cubeFrag");

		BoxProgram = glCreateProgram();
		glAttachShader(BoxProgram, BoxShaders[0]);
		glAttachShader(BoxProgram, BoxShaders[1]);
		glBindAttribLocation(BoxProgram, 0, "in_Position");
		linkProgram(BoxProgram);
	}
	void cleanupBox() {
		glDeleteBuffers(2, BoxVbo);
		glDeleteVertexArrays(1, &BoxVao);

		glDeleteProgram(BoxProgram);
		glDeleteShader(BoxShaders[0]);
		glDeleteShader(BoxShaders[1]);
	}
	void drawBox() {
		glBindVertexArray(BoxVao);
		glUseProgram(BoxProgram);
		glUniformMatrix4fv(glGetUniformLocation(BoxProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));
		glUniformMatrix4fv(glGetUniformLocation(BoxProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		// FLOOR
		glUniform4f(glGetUniformLocation(BoxProgram, "color"), 0.0f, 0.0f, 0.0f, 1.f);
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);
		// WALLS
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, (void*)(sizeof(GLubyte) * 4));
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, (void*)(sizeof(GLubyte) * 8));
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, (void*)(sizeof(GLubyte) * 12));
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, (void*)(sizeof(GLubyte) * 16));

		glUseProgram(0);
		glBindVertexArray(0);
	}
}

namespace Cube
{
	GLuint cubeVao;
	GLuint cubeVbo[3];
	GLuint cubeShaders[2];
	GLuint cubeProgram;
	glm::mat4 objMat = glm::mat4(1.f);

	extern const float halfW = 0.5f;
	int numVerts = 24 + 6; // 4 vertex/face * 6 faces + 6 PRIMITIVE RESTART

						   //   4---------7
						   //  /|        /|
						   // / |       / |
						   //5---------6  |
						   //|  0------|--3
						   //| /       | /
						   //|/        |/
						   //1---------2
	glm::vec3 verts[] = {
		glm::vec3(-halfW, -halfW, -halfW),
		glm::vec3(-halfW, -halfW,  halfW),
		glm::vec3(halfW, -halfW,  halfW),
		glm::vec3(halfW, -halfW, -halfW),
		glm::vec3(-halfW,  halfW, -halfW),
		glm::vec3(-halfW,  halfW,  halfW),
		glm::vec3(halfW,  halfW,  halfW),
		glm::vec3(halfW,  halfW, -halfW)
	};
	glm::vec3 norms[] = {
		glm::vec3(0.f, -1.f,  0.f),
		glm::vec3(0.f,  1.f,  0.f),
		glm::vec3(-1.f,  0.f,  0.f),
		glm::vec3(1.f,  0.f,  0.f),
		glm::vec3(0.f,  0.f, -1.f),
		glm::vec3(0.f,  0.f,  1.f)
	};

	glm::vec3 cubeVerts[] = {
		verts[1], verts[0], verts[2], verts[3],
		verts[5], verts[6], verts[4], verts[7],
		verts[1], verts[5], verts[0], verts[4],
		verts[2], verts[3], verts[6], verts[7],
		verts[0], verts[4], verts[3], verts[7],
		verts[1], verts[2], verts[5], verts[6]
	};
	glm::vec3 cubeNorms[] = {
		norms[0], norms[0], norms[0], norms[0],
		norms[1], norms[1], norms[1], norms[1],
		norms[2], norms[2], norms[2], norms[2],
		norms[3], norms[3], norms[3], norms[3],
		norms[4], norms[4], norms[4], norms[4],
		norms[5], norms[5], norms[5], norms[5]
	};
	GLubyte cubeIdx[] = {
		0, 1, 2, 3, UCHAR_MAX,
		4, 5, 6, 7, UCHAR_MAX,
		8, 9, 10, 11, UCHAR_MAX,
		12, 13, 14, 15, UCHAR_MAX,
		16, 17, 18, 19, UCHAR_MAX,
		20, 21, 22, 23, UCHAR_MAX
	};

	const char* cube_vertShader =
		"#version 330\n\
	in vec3 in_Position;\n\
	in vec3 in_Normal;\n\
	out vec4 vert_Normal;\n\
	uniform mat4 objMat;\n\
	uniform mat4 mv_Mat;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);\n\
		vert_Normal = mv_Mat * objMat * vec4(in_Normal, 0.0);\n\
	}";
	const char* cube_fragShader =
		"#version 330\n\
	in vec4 vert_Normal;\n\
	out vec4 out_Color;\n\
	uniform mat4 mv_Mat;\n\
	uniform vec4 color;\n\
	void main() {\n\
		out_Color = vec4(color.xyz * dot(vert_Normal, mv_Mat*vec4(0.0, 1.0, 0.0, 0.0)) + color.xyz * 0.3, 1.0 );\n\
	}";

	void setupCube() {
		glGenVertexArrays(1, &cubeVao);
		glBindVertexArray(cubeVao);
		glGenBuffers(3, cubeVbo);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNorms), cubeNorms, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glPrimitiveRestartIndex(UCHAR_MAX);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeVbo[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIdx), cubeIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		cubeShaders[0] = compileShader(cube_vertShader, GL_VERTEX_SHADER, "cubeVert");
		cubeShaders[1] = compileShader(cube_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		cubeProgram = glCreateProgram();
		glAttachShader(cubeProgram, cubeShaders[0]);
		glAttachShader(cubeProgram, cubeShaders[1]);
		glBindAttribLocation(cubeProgram, 0, "in_Position");
		glBindAttribLocation(cubeProgram, 1, "in_Normal");
		linkProgram(cubeProgram);
	}

	void cleanupCube() {
		glDeleteBuffers(3, cubeVbo);
		glDeleteVertexArrays(1, &cubeVao);

		glDeleteProgram(cubeProgram);
		glDeleteShader(cubeShaders[0]);
		glDeleteShader(cubeShaders[1]);
	}

	void updateCube(const glm::mat4& transform) {
		objMat = transform;
	}

	void drawCube() {
		glEnable(GL_PRIMITIVE_RESTART);
		glBindVertexArray(cubeVao);
		glUseProgram(cubeProgram);
		glm::mat4 t1 = glm::translate(glm::mat4(), glm::vec3(-10.0f, 15.0f, -10.0f));
		glm::mat4 s1 = glm::scale(glm::mat4(), glm::vec3(1.0f, 1.0f, 1.0f));
		objMat = t1 * s1;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), LightColor.x, LightColor.y, LightColor.z, 1);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		glUseProgram(0);
		glBindVertexArray(0);
		glDisable(GL_PRIMITIVE_RESTART);
	}

	void draw2Cubes() {
		glEnable(GL_PRIMITIVE_RESTART);
		glBindVertexArray(cubeVao);
		glUseProgram(cubeProgram);
		glm::mat4 t1 = glm::translate(glm::mat4(), glm::vec3(-4.0f, 2.0f, -8.0f));
		glm::mat4 s1 = glm::scale(glm::mat4(), glm::vec3(5.0f, 5.0f, 5.0f));
		objMat = t1 * s1;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.1f, 1.f, 1.f, 0.f);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		t1 = glm::translate(glm::mat4(), glm::vec3(4.0f, 2.0f, -8.0f));
		objMat = t1 * s1;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		glUseProgram(0);
		glBindVertexArray(0);
		glDisable(GL_PRIMITIVE_RESTART);
	}

	void draw2CubesMore() {
		glEnable(GL_PRIMITIVE_RESTART);
		glBindVertexArray(cubeVao);
		glUseProgram(cubeProgram);
		glm::mat4 t1 = glm::translate(glm::mat4(), glm::vec3(-8.0f, 2.0f, 0.0f));
		glm::mat4 s1 = glm::scale(glm::mat4(), glm::vec3(5.0f, 5.0f, 5.0f));
		objMat = t1 * s1;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.1f, 1.f, 1.f, 0.f);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		t1 = glm::translate(glm::mat4(), glm::vec3(8.0f, 2.0f, .0f));
		objMat = t1 * s1;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		glUseProgram(0);
		glBindVertexArray(0);
		glDisable(GL_PRIMITIVE_RESTART);
	}
}


////////////////////////////////////////////////// AXIS
namespace Axis {
	GLuint AxisVao;
	GLuint AxisVbo[3];
	GLuint AxisShader[2];
	GLuint AxisProgram;

	float AxisVerts[] = {
		0.0, 0.0, 0.0,
		1.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 1.0
	};
	float AxisColors[] = {
		1.0, 0.0, 0.0, 1.0,
		1.0, 0.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 0.0, 1.0, 1.0,
		0.0, 0.0, 1.0, 1.0
	};
	GLubyte AxisIdx[] = {
		0, 1,
		2, 3,
		4, 5
	};
	const char* Axis_vertShader =
		"#version 330\n\
		in vec3 in_Position;\n\
		in vec4 in_Color;\n\
		out vec4 vert_color;\n\
		uniform mat4 mvpMat;\n\
		void main() {\n\
			vert_color = in_Color;\n\
			gl_Position = mvpMat * vec4(in_Position, 1.0);\n\
		}";
	const char* Axis_fragShader =
		"#version 330\n\
		in vec4 vert_color;\n\
		out vec4 out_Color;\n\
		void main() {\n\
			out_Color = vert_color;\n\
		}";

	void setupAxis() {
		glGenVertexArrays(1, &AxisVao);
		glBindVertexArray(AxisVao);
		glGenBuffers(3, AxisVbo);

		glBindBuffer(GL_ARRAY_BUFFER, AxisVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, AxisVerts, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, AxisVbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, AxisColors, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 4, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(1);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, AxisVbo[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 6, AxisIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		AxisShader[0] = compileShader(Axis_vertShader, GL_VERTEX_SHADER, "AxisVert");
		AxisShader[1] = compileShader(Axis_fragShader, GL_FRAGMENT_SHADER, "AxisFrag");

		AxisProgram = glCreateProgram();
		glAttachShader(AxisProgram, AxisShader[0]);
		glAttachShader(AxisProgram, AxisShader[1]);
		glBindAttribLocation(AxisProgram, 0, "in_Position");
		glBindAttribLocation(AxisProgram, 1, "in_Color");
		linkProgram(AxisProgram);
	}
	void cleanupAxis() {
		glDeleteBuffers(3, AxisVbo);
		glDeleteVertexArrays(1, &AxisVao);

		glDeleteProgram(AxisProgram);
		glDeleteShader(AxisShader[0]);
		glDeleteShader(AxisShader[1]);
	}
	void drawAxis() {
		glBindVertexArray(AxisVao);
		glUseProgram(AxisProgram);
		glUniformMatrix4fv(glGetUniformLocation(AxisProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));
		glDrawElements(GL_LINES, 6, GL_UNSIGNED_BYTE, 0);

		glUseProgram(0);
		glBindVertexArray(0);
	}
}

////////////////////////////////////////////////// MyModel
namespace MyLoadedModel {
	GLuint modelVao;
	GLuint modelVbo[3];
	GLuint modelShaders[2];
	GLuint modelProgram;
	glm::mat4 objMat = glm::mat4(1.f);

	const char* model_vertShader =
		"#version 330\n\
	in vec3 in_Position;\n\
	in vec3 in_Normal;\n\
	uniform vec3 lightPos;\n\
	out vec4 vert_Normal;\n\
	out vec3 Normal; \n\
	out vec3 FragPos; \n\
	uniform mat4 objMat;\n\
	uniform mat4 mv_Mat;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);\n\
		vert_Normal = mv_Mat * objMat * vec4(in_Normal, 0.0);\n\
		FragPos = vec3(objMat * vec4(in_Position, 1.0)); \n\
		Normal = in_Normal; \n\
	}";


	const char* model_fragShader =
		"#version 330\n\
	in vec4 vert_Normal;\n\
	in vec3 FragPos; \n\
	out vec4 out_Color;\n\
	in vec3 Normal; \n\
	uniform mat4 mv_Mat;\n\
	uniform vec3 LightColor; \n\
	uniform vec3 ObjectColor; \n\
	uniform vec3 lightPos; \n\
	uniform vec3 viewPos; \n\
	uniform float ambientStrength;\n\
	uniform float specularStrength; \n\
	uniform float shininess; \n\
	void main() {\n\
		//Realizamos los calculos necesarios para conseguir luz ambiente \n\
		vec3 ambient = ambientStrength * LightColor;\n\
		//Realizamos los calculos necesarios para conseguir ilumacion difusa\n\
		vec3 norm = normalize(Normal); \n\
		vec3 lightDir = normalize(lightPos - FragPos); \n\
		float diff = max(dot(norm, lightDir), 0.0); \n\
		if(diff < 0.2) diff = 0;\n\
		if(diff >= 0.2 && diff < 0.4) diff = 0.2; \n\
		if(diff >= 0.4 && diff < 0.5) diff = 0; \n\
		if(diff >= 0.5) diff = 1;\n\
		vec3 diffuse = diff * LightColor; \n\
		//Realizamos los calculos necesarios para conseguir luz especular \n\
		vec3 viewDir = normalize(viewPos - FragPos); \n\
 		vec3 reflectDir = reflect(-lightDir, norm);	\n\
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess); \n\
		if(spec < 0.2) spec = 0;\n\
		if(spec >= 0.2 && spec < 0.4) spec = 0; \n\
		if(spec >= 0.4 && spec < 0.5) spec = 0; \n\
		if(spec >= 0.5) spec = 1;\n\
		vec3 specular = specularStrength * spec * LightColor; \n\
		//Renderizamos el modelo junto a los tres tipos de ilumacion \n\
		vec3 result = (ambient + diffuse) * ObjectColor; \n\
		out_Color = vec4(result, 1.0); \n\
	}";

	void setupModel() {
		glGenVertexArrays(1, &modelVao);
		glBindVertexArray(modelVao);
		glGenBuffers(3, modelVbo);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[0]);

		glBufferData(GL_ARRAY_BUFFER, verticesChicken.size() * sizeof(glm::vec3), &verticesChicken[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[1]);

		glBufferData(GL_ARRAY_BUFFER, normalsChicken.size() * sizeof(glm::vec3), &normalsChicken[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		modelShaders[0] = compileShader(model_vertShader, GL_VERTEX_SHADER, "cubeVert");
		modelShaders[1] = compileShader(model_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		modelProgram = glCreateProgram();
		glAttachShader(modelProgram, modelShaders[0]);
		glAttachShader(modelProgram, modelShaders[1]);
		glBindAttribLocation(modelProgram, 0, "in_Position");
		glBindAttribLocation(modelProgram, 1, "in_Normal");
		linkProgram(modelProgram);
	}
	void cleanupModel() {

		glDeleteBuffers(2, modelVbo);
		glDeleteVertexArrays(1, &modelVao);

		glDeleteProgram(modelProgram);
		glDeleteShader(modelShaders[0]);
		glDeleteShader(modelShaders[1]);
	}
	void updateModel(const glm::mat4& transform) {
		objMat = transform;
	}
	void drawModel() {

		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);

		//glm::mat4 t1 = glm::translate(glm::mat4(), glm::vec3(-4.0f, 10.0f, 0.0f));
		//glm::mat4 s1 = glm::scale(glm::mat4(), glm::vec3(0.1, 0.1f, 0.1f));
		//objMat = t1 * s1;
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		//Le pasamos al shader las variables que utlizaremos en la interfaz para que de esta forma se pueda modificar la ilumacion de forma dinamica
		glUniform3f(glGetUniformLocation(modelProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(modelProgram, "LightColor"), LightColor.x, LightColor.y, LightColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "ObjectColor"), ObjectColor.x, ObjectColor.y, ObjectColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "viewPos"), ViewPos.x, ViewPos.y, ViewPos.z);
		glUniform1f(glGetUniformLocation(modelProgram, "ambientStrength"), ambientStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "specularStrength"), specularStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "shininess"), shininess);
		glDrawArrays(GL_TRIANGLES, 0, 50000);
		glUseProgram(0);
		glBindVertexArray(0);
	}

	void drawModel2() {

		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		//Le pasamos al shader las variables que utlizaremos en la interfaz para que de esta forma se pueda modificar la ilumacion de forma dinamica
		glUniform3f(glGetUniformLocation(modelProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(modelProgram, "LightColor"), LightColor.x, LightColor.y, LightColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "ObjectColor"), ObjectColor.x, ObjectColor.y, ObjectColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "viewPos"), ViewPos.x, ViewPos.y, ViewPos.z);
		glUniform1f(glGetUniformLocation(modelProgram, "ambientStrength"), ambientStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "specularStrength"), specularStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "shininess"), shininess);
		glDrawArrays(GL_TRIANGLES, 0, 50000);
		glUseProgram(0);
		glBindVertexArray(0);
	}

}

////////////////////////////////////////////////// MyModel2
namespace MyLoadedModel2 {
	GLuint modelVao;
	GLuint modelVbo[3];
	GLuint modelShaders[2];
	GLuint modelProgram;
	glm::mat4 objMat = glm::mat4(1.f);

	const char* model_vertShader =
		"#version 330\n\
	in vec3 in_Position;\n\
	in vec3 in_Normal;\n\
	uniform vec3 lightPos;\n\
	out vec4 vert_Normal;\n\
	out vec3 Normal; \n\
	out vec3 FragPos; \n\
	uniform mat4 objMat;\n\
	uniform mat4 mv_Mat;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);\n\
		vert_Normal = mv_Mat * objMat * vec4(in_Normal, 0.0);\n\
		FragPos = vec3(objMat * vec4(in_Position, 1.0)); \n\
		Normal = in_Normal; \n\
	}";


	const char* model_fragShader =
		"#version 330\n\
	in vec4 vert_Normal;\n\
	in vec3 FragPos; \n\
	out vec4 out_Color;\n\
	in vec3 Normal; \n\
	uniform mat4 mv_Mat;\n\
	uniform vec3 LightColor; \n\
	uniform vec3 ObjectColor; \n\
	uniform vec3 lightPos; \n\
	uniform vec3 viewPos; \n\
	uniform float ambientStrength;\n\
	uniform float specularStrength; \n\
	uniform float shininess; \n\
	void main() {\n\
		//Realizamos los calculos necesarios para conseguir luz ambiente \n\
		vec3 ambient = ambientStrength * LightColor;\n\
		//Realizamos los calculos necesarios para conseguir ilumacion difusa\n\
		vec3 norm = normalize(Normal); \n\
		vec3 lightDir = normalize(lightPos - FragPos); \n\
		float diff = max(dot(norm, lightDir), 0.0); \n\
		if(diff < 0.2) diff = 0;\n\
		if(diff >= 0.2 && diff < 0.4) diff = 0.2; \n\
		if(diff >= 0.4 && diff < 0.5) diff = 0; \n\
		if(diff >= 0.5) diff = 1;\n\
		vec3 diffuse = diff * LightColor; \n\
		//Realizamos los calculos necesarios para conseguir luz especular \n\
		vec3 viewDir = normalize(viewPos - FragPos); \n\
 		vec3 reflectDir = reflect(-lightDir, norm);	\n\
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess); \n\
		if(spec < 0.2) spec = 0;\n\
		if(spec >= 0.2 && spec < 0.4) spec = 0; \n\
		if(spec >= 0.4 && spec < 0.5) spec = 0; \n\
		if(spec >= 0.5) spec = 1;\n\
		vec3 specular = specularStrength * spec * LightColor; \n\
		//Renderizamos el modelo junto a los tres tipos de ilumacion \n\
		vec3 result = (ambient + diffuse) * ObjectColor; \n\
		out_Color = vec4(result, 1.0); \n\
	}";

	void setupModel() {
		glGenVertexArrays(1, &modelVao);
		glBindVertexArray(modelVao);
		glGenBuffers(3, modelVbo);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[0]);

		glBufferData(GL_ARRAY_BUFFER, verticesTrump.size() * sizeof(glm::vec3), &verticesTrump[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[1]);

		glBufferData(GL_ARRAY_BUFFER, normalsTrump.size() * sizeof(glm::vec3), &normalsTrump[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		modelShaders[0] = compileShader(model_vertShader, GL_VERTEX_SHADER, "cubeVert");
		modelShaders[1] = compileShader(model_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		modelProgram = glCreateProgram();
		glAttachShader(modelProgram, modelShaders[0]);
		glAttachShader(modelProgram, modelShaders[1]);
		glBindAttribLocation(modelProgram, 0, "in_Position");
		glBindAttribLocation(modelProgram, 1, "in_Normal");
		linkProgram(modelProgram);
	}
	void cleanupModel() {

		glDeleteBuffers(2, modelVbo);
		glDeleteVertexArrays(1, &modelVao);

		glDeleteProgram(modelProgram);
		glDeleteShader(modelShaders[0]);
		glDeleteShader(modelShaders[1]);
	}
	void updateModel(const glm::mat4& transform) {
		objMat = transform;
	}
	void drawModel() {

		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);
		//glm::mat4 t1 = glm::translate(glm::mat4(), glm::vec3(-14.0f, 8.0f, 0.0f));
		//glm::mat4 s1 = glm::scale(glm::mat4(), glm::vec3(0.05, 0.05f, 0.05f));
		//objMat = t1 * s1;
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		//Le pasamos al shader las variables que utlizaremos en la interfaz para que de esta forma se pueda modificar la ilumacion de forma dinamica
		glUniform3f(glGetUniformLocation(modelProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(modelProgram, "LightColor"), LightColor.x, LightColor.y, LightColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "ObjectColor"), ObjectColor.x, ObjectColor.y, ObjectColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "viewPos"), ViewPos.x, ViewPos.y, ViewPos.z);
		glUniform1f(glGetUniformLocation(modelProgram, "ambientStrength"), ambientStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "specularStrength"), specularStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "shininess"), shininess);
		glDrawArrays(GL_TRIANGLES, 0, 50000);
		glUseProgram(0);
		glBindVertexArray(0);
	}

	void drawModel2() {

		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		//Le pasamos al shader las variables que utlizaremos en la interfaz para que de esta forma se pueda modificar la ilumacion de forma dinamica
		glUniform3f(glGetUniformLocation(modelProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(modelProgram, "LightColor"), LightColor.x, LightColor.y, LightColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "ObjectColor"), ObjectColor.x, ObjectColor.y, ObjectColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "viewPos"), ViewPos.x, ViewPos.y, ViewPos.z);
		glUniform1f(glGetUniformLocation(modelProgram, "ambientStrength"), ambientStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "specularStrength"), specularStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "shininess"), shininess);
		glDrawArrays(GL_TRIANGLES, 0, 50000);
		glUseProgram(0);
		glBindVertexArray(0);

	}

}

////////////////////////////////////////////////// MyModel3
namespace MyLoadedModel3 {
	GLuint modelVao;
	GLuint modelVbo[3];
	GLuint modelShaders[2];
	GLuint modelProgram;
	glm::mat4 objMat = glm::mat4(1.f);

	const char* model_vertShader =
		"#version 330\n\
	in vec3 in_Position;\n\
	in vec3 in_Normal;\n\
	uniform vec3 lightPos;\n\
	out vec4 vert_Normal;\n\
	out vec3 Normal; \n\
	out vec3 FragPos; \n\
	uniform mat4 objMat;\n\
	uniform mat4 mv_Mat;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);\n\
		vert_Normal = mv_Mat * objMat * vec4(in_Normal, 0.0);\n\
		FragPos = vec3(objMat * vec4(in_Position, 1.0)); \n\
		Normal = in_Normal; \n\
	}";


	const char* model_fragShader =
		"#version 330\n\
	in vec4 vert_Normal;\n\
	in vec3 FragPos; \n\
	out vec4 out_Color;\n\
	in vec3 Normal; \n\
	uniform mat4 mv_Mat;\n\
	uniform vec3 LightColor; \n\
	uniform vec3 ObjectColor; \n\
	uniform vec3 lightPos; \n\
	uniform vec3 viewPos; \n\
	uniform float ambientStrength;\n\
	uniform float specularStrength; \n\
	uniform float shininess; \n\
	void main() {\n\
		//Realizamos los calculos necesarios para conseguir luz ambiente \n\
		vec3 ambient = ambientStrength * LightColor;\n\
		//Realizamos los calculos necesarios para conseguir ilumacion difusa\n\
		vec3 norm = normalize(Normal); \n\
		vec3 lightDir = normalize(lightPos - FragPos); \n\
		float diff = max(dot(norm, lightDir), 0.0); \n\
		if(diff < 0.2) diff = 0;\n\
		if(diff >= 0.2 && diff < 0.4) diff = 0.2; \n\
		if(diff >= 0.4 && diff < 0.5) diff = 0; \n\
		if(diff >= 0.5) diff = 1;\n\
		vec3 diffuse = diff * LightColor; \n\
		//Realizamos los calculos necesarios para conseguir luz especular \n\
		vec3 viewDir = normalize(viewPos - FragPos); \n\
 		vec3 reflectDir = reflect(-lightDir, norm);	\n\
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess); \n\
		if(spec < 0.2) spec = 0;\n\
		if(spec >= 0.2 && spec < 0.4) spec = 0; \n\
		if(spec >= 0.4 && spec < 0.5) spec = 0; \n\
		if(spec >= 0.5) spec = 1;\n\
		vec3 specular = specularStrength * spec * LightColor; \n\
		//Renderizamos el modelo junto a los tres tipos de ilumacion \n\
		vec3 result = (ambient + diffuse) * ObjectColor; \n\
		out_Color = vec4(result, 1.0); \n\
	}";

	void setupModel() {
		glGenVertexArrays(1, &modelVao);
		glBindVertexArray(modelVao);
		glGenBuffers(3, modelVbo);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[0]);

		glBufferData(GL_ARRAY_BUFFER, verticesCabine.size() * sizeof(glm::vec3), &verticesCabine[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[1]);

		glBufferData(GL_ARRAY_BUFFER, normalsCabine.size() * sizeof(glm::vec3), &normalsCabine[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		modelShaders[0] = compileShader(model_vertShader, GL_VERTEX_SHADER, "cubeVert");
		modelShaders[1] = compileShader(model_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		modelProgram = glCreateProgram();
		glAttachShader(modelProgram, modelShaders[0]);
		glAttachShader(modelProgram, modelShaders[1]);
		glBindAttribLocation(modelProgram, 0, "in_Position");
		glBindAttribLocation(modelProgram, 1, "in_Normal");
		linkProgram(modelProgram);
	}
	void cleanupModel() {

		glDeleteBuffers(2, modelVbo);
		glDeleteVertexArrays(1, &modelVao);

		glDeleteProgram(modelProgram);
		glDeleteShader(modelShaders[0]);
		glDeleteShader(modelShaders[1]);
	}
	void updateModel(const glm::mat4& transform) {
		objMat = transform;
	}
	void drawModel() {

		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);
		//glm::mat4 t1 = glm::translate(glm::mat4(), glm::vec3(-8.0f, 4.8f, 0.0f));
		//glm::mat4 s1 = glm::scale(glm::mat4(), glm::vec3(5.0f, 5.0f, 5.0f));
		//objMat = t1 * s1;
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		//Le pasamos al shader las variables que utlizaremos en la interfaz para que de esta forma se pueda modificar la ilumacion de forma dinamica
		glUniform3f(glGetUniformLocation(modelProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(modelProgram, "LightColor"), LightColor.x, LightColor.y, LightColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "ObjectColor"), ObjectColor.x, ObjectColor.y, ObjectColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "viewPos"), ViewPos.x, ViewPos.y, ViewPos.z);
		glUniform1f(glGetUniformLocation(modelProgram, "ambientStrength"), ambientStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "specularStrength"), specularStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "shininess"), shininess);
		glDrawArrays(GL_TRIANGLES, 0, 50000);
		glUseProgram(0);
		glBindVertexArray(0);
	}

	void drawModel2() {

		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		//Le pasamos al shader las variables que utlizaremos en la interfaz para que de esta forma se pueda modificar la ilumacion de forma dinamica
		glUniform3f(glGetUniformLocation(modelProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(modelProgram, "LightColor"), LightColor.x, LightColor.y, LightColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "ObjectColor"), ObjectColor.x, ObjectColor.y, ObjectColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "viewPos"), ViewPos.x, ViewPos.y, ViewPos.z);
		glUniform1f(glGetUniformLocation(modelProgram, "ambientStrength"), ambientStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "specularStrength"), specularStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "shininess"), shininess);
		glDrawArrays(GL_TRIANGLES, 0, 50000);
		glUseProgram(0);
		glBindVertexArray(0);

	}
}

////////////////////////////////////////////////// MyModel4
namespace MyLoadedModel4 {
	GLuint modelVao;
	GLuint modelVbo[3];
	GLuint modelShaders[2];
	GLuint modelProgram;
	glm::mat4 objMat = glm::mat4(1.f);

	const char* model_vertShader =
		"#version 330\n\
	in vec3 in_Position;\n\
	in vec3 in_Normal;\n\
	uniform vec3 lightPos;\n\
	out vec4 vert_Normal;\n\
	out vec3 Normal; \n\
	out vec3 FragPos; \n\
	uniform mat4 objMat;\n\
	uniform mat4 mv_Mat;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);\n\
		vert_Normal = mv_Mat * objMat * vec4(in_Normal, 0.0);\n\
		FragPos = vec3(objMat * vec4(in_Position, 1.0)); \n\
		Normal = in_Normal; \n\
	}";


	const char* model_fragShader =
		"#version 330\n\
	in vec4 vert_Normal;\n\
	in vec3 FragPos; \n\
	out vec4 out_Color;\n\
	in vec3 Normal; \n\
	uniform mat4 mv_Mat;\n\
	uniform vec3 LightColor; \n\
	uniform vec3 ObjectColor; \n\
	uniform vec3 lightPos; \n\
	uniform vec3 viewPos; \n\
	uniform float ambientStrength;\n\
	uniform float specularStrength; \n\
	uniform float shininess; \n\
	void main() {\n\
		//Realizamos los calculos necesarios para conseguir luz ambiente \n\
		vec3 ambient = ambientStrength * LightColor;\n\
		//Realizamos los calculos necesarios para conseguir ilumacion difusa\n\
		vec3 norm = normalize(Normal); \n\
		vec3 lightDir = normalize(lightPos - FragPos); \n\
		float diff = max(dot(norm, lightDir), 0.0); \n\
		if(diff < 0.2) diff = 0;\n\
		if(diff >= 0.2 && diff < 0.4) diff = 0.2; \n\
		if(diff >= 0.4 && diff < 0.5) diff = 0; \n\
		if(diff >= 0.5) diff = 1;\n\
		vec3 diffuse = diff * LightColor; \n\
		//Realizamos los calculos necesarios para conseguir luz especular \n\
		vec3 viewDir = normalize(viewPos - FragPos); \n\
 		vec3 reflectDir = reflect(-lightDir, norm);	\n\
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess); \n\
		if(spec < 0.2) spec = 0;\n\
		if(spec >= 0.2 && spec < 0.4) spec = 0; \n\
		if(spec >= 0.4 && spec < 0.5) spec = 0; \n\
		if(spec >= 0.5) spec = 1;\n\
		vec3 specular = specularStrength * spec * LightColor; \n\
		//Renderizamos el modelo junto a los tres tipos de ilumacion \n\
		vec3 result = (ambient + diffuse) * ObjectColor; \n\
		out_Color = vec4(result, 1.0); \n\
	}";

	void setupModel() {
		glGenVertexArrays(1, &modelVao);
		glBindVertexArray(modelVao);
		glGenBuffers(3, modelVbo);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[0]);

		glBufferData(GL_ARRAY_BUFFER, verticesSupports.size() * sizeof(glm::vec3), &verticesSupports[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[1]);

		glBufferData(GL_ARRAY_BUFFER, normalsSupports.size() * sizeof(glm::vec3), &normalsSupports[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		modelShaders[0] = compileShader(model_vertShader, GL_VERTEX_SHADER, "cubeVert");
		modelShaders[1] = compileShader(model_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		modelProgram = glCreateProgram();
		glAttachShader(modelProgram, modelShaders[0]);
		glAttachShader(modelProgram, modelShaders[1]);
		glBindAttribLocation(modelProgram, 0, "in_Position");
		glBindAttribLocation(modelProgram, 1, "in_Normal");
		linkProgram(modelProgram);
	}
	void cleanupModel() {

		glDeleteBuffers(2, modelVbo);
		glDeleteVertexArrays(1, &modelVao);

		glDeleteProgram(modelProgram);
		glDeleteShader(modelShaders[0]);
		glDeleteShader(modelShaders[1]);
	}
	void updateModel(const glm::mat4& transform) {
		objMat = transform;
	}
	void drawModel() {

		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);
		//glm::mat4 t1 = glm::translate(glm::mat4(), glm::vec3(-8.0f, 4.8f, 0.0f));
		//glm::mat4 s1 = glm::scale(glm::mat4(), glm::vec3(5.0f, 5.0f, 5.0f));
		//objMat = t1 * s1;
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		//Le pasamos al shader las variables que utlizaremos en la interfaz para que de esta forma se pueda modificar la ilumacion de forma dinamica
		glUniform3f(glGetUniformLocation(modelProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(modelProgram, "LightColor"), LightColor.x, LightColor.y, LightColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "ObjectColor"), ObjectColor.x, ObjectColor.y, ObjectColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "viewPos"), ViewPos.x, ViewPos.y, ViewPos.z);
		glUniform1f(glGetUniformLocation(modelProgram, "ambientStrength"), ambientStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "specularStrength"), specularStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "shininess"), shininess);
		glDrawArrays(GL_TRIANGLES, 0, 50000);
		glUseProgram(0);
		glBindVertexArray(0);
	}

	void drawModel2() {

		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		//Le pasamos al shader las variables que utlizaremos en la interfaz para que de esta forma se pueda modificar la ilumacion de forma dinamica
		glUniform3f(glGetUniformLocation(modelProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(modelProgram, "LightColor"), LightColor.x, LightColor.y, LightColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "ObjectColor"), ObjectColor.x, ObjectColor.y, ObjectColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "viewPos"), ViewPos.x, ViewPos.y, ViewPos.z);
		glUniform1f(glGetUniformLocation(modelProgram, "ambientStrength"), ambientStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "specularStrength"), specularStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "shininess"), shininess);
		glDrawArrays(GL_TRIANGLES, 0, 50000);
		glUseProgram(0);
		glBindVertexArray(0);

	}
}

////////////////////////////////////////////////// MyModel5
namespace MyLoadedModel5 {
	GLuint modelVao;
	GLuint modelVbo[3];
	GLuint modelShaders[2];
	GLuint modelProgram;
	glm::mat4 objMat = glm::mat4(1.f);

	const char* model_vertShader =
		"#version 330\n\
	in vec3 in_Position;\n\
	in vec3 in_Normal;\n\
	uniform vec3 lightPos;\n\
	out vec4 vert_Normal;\n\
	out vec3 Normal; \n\
	out vec3 FragPos; \n\
	uniform mat4 objMat;\n\
	uniform mat4 mv_Mat;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);\n\
		vert_Normal = mv_Mat * objMat * vec4(in_Normal, 0.0);\n\
		FragPos = vec3(objMat * vec4(in_Position, 1.0)); \n\
		Normal = in_Normal; \n\
	}";


	const char* model_fragShader =
		"#version 330\n\
	in vec4 vert_Normal;\n\
	in vec3 FragPos; \n\
	out vec4 out_Color;\n\
	in vec3 Normal; \n\
	uniform mat4 mv_Mat;\n\
	uniform vec3 LightColor; \n\
	uniform vec3 ObjectColor; \n\
	uniform vec3 lightPos; \n\
	uniform vec3 viewPos; \n\
	uniform float ambientStrength;\n\
	uniform float specularStrength; \n\
	uniform float shininess; \n\
	void main() {\n\
		//Realizamos los calculos necesarios para conseguir luz ambiente \n\
		vec3 ambient = ambientStrength * LightColor;\n\
		//Realizamos los calculos necesarios para conseguir ilumacion difusa\n\
		vec3 norm = normalize(Normal); \n\
		vec3 lightDir = normalize(lightPos - FragPos); \n\
		float diff = max(dot(norm, lightDir), 0.0); \n\
		if(diff < 0.2) diff = 0;\n\
		if(diff >= 0.2 && diff < 0.4) diff = 0.2; \n\
		if(diff >= 0.4 && diff < 0.5) diff = 0; \n\
		if(diff >= 0.5) diff = 1;\n\
		vec3 diffuse = diff * LightColor; \n\
		//Realizamos los calculos necesarios para conseguir luz especular \n\
		vec3 viewDir = normalize(viewPos - FragPos); \n\
 		vec3 reflectDir = reflect(-lightDir, norm);	\n\
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess); \n\
		if(spec < 0.2) spec = 0;\n\
		if(spec >= 0.2 && spec < 0.4) spec = 0; \n\
		if(spec >= 0.4 && spec < 0.5) spec = 0; \n\
		if(spec >= 0.5) spec = 1;\n\
		vec3 specular = specularStrength * spec * LightColor; \n\
		//Renderizamos el modelo junto a los tres tipos de ilumacion \n\
		vec3 result = (ambient + diffuse) * ObjectColor; \n\
		out_Color = vec4(result, 1.0); \n\
	}";

	void setupModel() {
		glGenVertexArrays(1, &modelVao);
		glBindVertexArray(modelVao);
		glGenBuffers(3, modelVbo);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[0]);

		glBufferData(GL_ARRAY_BUFFER, verticesWheel.size() * sizeof(glm::vec3), &verticesWheel[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[1]);

		glBufferData(GL_ARRAY_BUFFER, normalsWheel.size() * sizeof(glm::vec3), &normalsWheel[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		modelShaders[0] = compileShader(model_vertShader, GL_VERTEX_SHADER, "cubeVert");
		modelShaders[1] = compileShader(model_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		modelProgram = glCreateProgram();
		glAttachShader(modelProgram, modelShaders[0]);
		glAttachShader(modelProgram, modelShaders[1]);
		glBindAttribLocation(modelProgram, 0, "in_Position");
		glBindAttribLocation(modelProgram, 1, "in_Normal");
		linkProgram(modelProgram);
	}
	void cleanupModel() {

		glDeleteBuffers(2, modelVbo);
		glDeleteVertexArrays(1, &modelVao);

		glDeleteProgram(modelProgram);
		glDeleteShader(modelShaders[0]);
		glDeleteShader(modelShaders[1]);
	}
	void updateModel(const glm::mat4& transform) {
		objMat = transform;
	}
	void drawModel() {

		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);
		//glm::mat4 t1 = glm::translate(glm::mat4(), glm::vec3(-8.0f, 4.8f, 0.0f));
		//glm::mat4 s1 = glm::scale(glm::mat4(), glm::vec3(5.0f, 5.0f, 5.0f));
		//objMat = t1 * s1;
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		//Le pasamos al shader las variables que utlizaremos en la interfaz para que de esta forma se pueda modificar la ilumacion de forma dinamica
		glUniform3f(glGetUniformLocation(modelProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(modelProgram, "LightColor"), LightColor.x, LightColor.y, LightColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "ObjectColor"), ObjectColor.x, ObjectColor.y, ObjectColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "viewPos"), ViewPos.x, ViewPos.y, ViewPos.z);
		glUniform1f(glGetUniformLocation(modelProgram, "ambientStrength"), ambientStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "specularStrength"), specularStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "shininess"), shininess);
		glDrawArrays(GL_TRIANGLES, 0, 50000);
		glUseProgram(0);
		glBindVertexArray(0);
	}

	void drawModel2() {

		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		//Le pasamos al shader las variables que utlizaremos en la interfaz para que de esta forma se pueda modificar la ilumacion de forma dinamica
		glUniform3f(glGetUniformLocation(modelProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(modelProgram, "LightColor"), LightColor.x, LightColor.y, LightColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "ObjectColor"), ObjectColor.x, ObjectColor.y, ObjectColor.z);
		glUniform3f(glGetUniformLocation(modelProgram, "viewPos"), ViewPos.x, ViewPos.y, ViewPos.z);
		glUniform1f(glGetUniformLocation(modelProgram, "ambientStrength"), ambientStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "specularStrength"), specularStrength);
		glUniform1f(glGetUniformLocation(modelProgram, "shininess"), shininess);
		glDrawArrays(GL_TRIANGLES, 0, 50000);
		glUseProgram(0);
		glBindVertexArray(0);

	}
}