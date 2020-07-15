#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "Camera.h"
#include "Model.h"
#include "Shader.h"
#include "Room.h"
#include "Cube.h"
#include "Plane.h"
#include "Skybox.h"
#include "TransformComponent.h"
#include "shadowFBO.h"

void setCameraViewTransforms(Shader _shader);

void addObjects();
void shadowPass(Shader _shader, Room _room);
void renderPass(Shader _shader, Room _room);

void renderScene(Shader _shader, Room _room, Model _model, Cube _cube, bool withTextures);
void renderSkybox(Skybox _skybox, Shader _shader);

void framebuffer_size_callback(GLFWwindow* window, int screenWidth, int screenHeight);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// settings
const unsigned int screenWidth = 1280;
const unsigned int screenHeight = 720;
bool displayDepth = false;
bool displayDepthKeyPressed = false;
bool MoveLight = true;
bool MoveLightKeypressed = false;

// Shadow framebuffer object class
ShadowFBO shadowFBO;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), -90.0f, 0.0f);
float lastX = screenWidth / 2.0f;
float lastY = screenHeight / 2.0f;
bool firstMouse = true;

// Vector of objects
std::vector<Model> objects;

// time
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main() {
	
	// glfw: initialise and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create Window
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Shadows Demo - Yash V", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		std::cin.get();
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	//Capture mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		std::cin.get();
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//// configure depth map FBO
	shadowFBO.configureFBO();

	//Create shaders and objects
	Shader skyboxShader("Shaders/skybox.vert", "Shaders/skybox.frag");
	Shader shader("Shaders/pointLShadows.vert", "Shaders/PLTest.frag");
	Shader simpleDepthShader("Shaders/pointLShadowsDepth.vert", "Shaders/pointLShadowsDepth.frag","Shaders/pointLShadowsDepth.geo");

	// shader configuration
	// --------------------
	shader.use();
	shader.setInt("diffuseTexture", 0);
	shader.setInt("depthMap", 1);

	//Add all models
	addObjects();

	//add room
	Room room;
	room.loadTexture("Textures/Wallpaper/1_Wallpaper design by Natasha Marsall_diffuse.jpg");
	room.loadTexture("Textures/Wallpaper/1_Wallpaper design by Natasha Marsall_specular.jpg");
	room.loadTexture("Textures/whiteness.jpg");
	//room.loadTexture("Textures/whiteness.jpg");
	room.setScale(glm::vec3(10.0f,5.0f,10.0f));
	room.setPos(glm::vec3(0.0f,5.0f,0.0f));

	// Skybox textures
	std::vector<std::string> faces
	{
		"Textures/skybox_right.jpg",
		"Textures/skybox_left.jpg",
		"Textures/skybox_top.jpg",
		"Textures/skybox_bottom.jpg",
		"Textures/skybox_front.jpg",
		"Textures/skybox_back.jpg"
	};
	Skybox skybox(faces);

	// lighting info
	// -------------
	glm::vec3 lightPos(2.0f, 1.0f, 5.0f);

	// Render Loop
	while (!glfwWindowShouldClose(window))
	{
		// per frame time logic
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		processInput(window);

		// move light position over time
		if (MoveLight) {
			lightPos = camera.position;
			//lightPos.x = sin(glfwGetTime() * 0.5) * 3.0;
		}

		// rendering commands here
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 0. create depth cubemap transformation matrices
		// -----------------------------------------------
		float near_plane = 1.0f;
		float far_plane = 100.0f;

		shadowFBO.createCubemapTransformationMatrices(lightPos,near_plane,far_plane);

		// 1. render scene to depth cubemap
		// --------------------------------
		/*glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);*/
		simpleDepthShader.use();
		shadowFBO.bindFBO(simpleDepthShader);
		simpleDepthShader.setFloat("far_plane", far_plane);
		simpleDepthShader.setVec3("lightPos", lightPos);
		shadowPass(simpleDepthShader, room);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
				
		// 2. render scene as normal 
		// -------------------------
		glViewport(0, 0, screenWidth, screenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.use();
		setCameraViewTransforms(shader);

		//shader.setVec3("lightPos", lightPos);
		shader.setInt("displayDepth", displayDepth); // enable/disable shadows by pressing 'SPACE'
		shader.setFloat("far_plane", far_plane);

		//shader.setInt("material.diffuse", 0);
		//shader.setInt("material.specular", 1);
		//shader.setFloat("material.shininess", 32.0f);

		shader.setVec3("pointLight.position", lightPos);
		shader.setVec3("pointLight.ambient", 0.2f, 0.2f, 0.2f);
		shader.setVec3("pointLight.diffuse", 1.0f, 1.0f, 1.0f);
		shader.setVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);
		shader.setFloat("pointLight.constant", 1.0f);
		shader.setFloat("pointLight.linear", 0.045f);
		shader.setFloat("pointLight.quadratic", 0.0075f);

		renderPass(shader, room);

		renderSkybox(skybox,skyboxShader);

		// check and call events and swap the buffers
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;
}

void addObjects() {
	/* Frame
	objects.push_back(Model("Models/"));
	objects[0].setScale(glm::vec3(1.0f));
	objects[0].setPos(glm::vec3(0.0f, 0.0f, 0.0f));
	*/

	objects.push_back(Model("Models/obj_mesa/obj_mesa.obj"));
	objects[0].setScale(glm::vec3(4.0f));
	objects[0].setPos(glm::vec3(7.5f, 0.0f, 5.0f));

	objects.push_back(Model("Models/coffeeMug/coffeMug1_free_obj.obj"));
	objects[1].setScale(glm::vec3(0.02f));
	objects[1].setPos(glm::vec3(7.5f, 4.0f, 5.0f));

	objects.push_back(Model("Models/bed/krovat-2.obj"));
	objects[2].setScale(glm::vec3(5.0f));
	objects[2].setPos(glm::vec3(-5.0f, 0.0f, 5.0f));
	
	objects.push_back(Model("Models/wardrobe/Wardrobe  4 door.obj"));
	objects[3].setScale(glm::vec3(4.0f));
	objects[3].setPos(glm::vec3(-5.0f, 0.0f, -10.0f));

	objects.push_back(Model("Models/Samus/DolSzerosuitR1.obj"));
	objects[4].setScale(glm::vec3(0.4f));
	objects[4].setPos(glm::vec3(0.0f, 0.0f, 0.0f));

	objects.push_back(Model("Models/picture/frida.obj"));
	objects[5].setScale(glm::vec3(7.5f));
	objects[5].setPos(glm::vec3(-3.0f, -4.0f, -12.2f));

	objects.push_back(Model("Models/picture/frame.obj"));
	objects[6].setScale(glm::vec3(7.5f));
	objects[6].setPos(glm::vec3(-3.0f, -4.0f, -12.2f));

}

void setCameraViewTransforms(Shader _shader) 
{
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

	_shader.use();
	_shader.setMat4("view", view);
	_shader.setMat4("projection", projection);
	_shader.setVec3("viewPos", camera.position);
}

void shadowPass(Shader _shader, Room _room)
{
	glm::mat4 model = glm::mat4(1.0f);

	for (int i = 0; i < objects.size(); i++) {
		model = objects[i].getModel();
		_shader.setMat4("model", model);
		objects[i].Draw();
	}
	
	model = _room.getModel();
	_shader.setMat4("model", model);
	_room.draw();
}

void renderPass(Shader _shader, Room _room) {
	glm::mat4 model = glm::mat4(1.0f);

	for (int i = 0; i < objects.size(); i++) {
		model = objects[i].getModel();
		_shader.setMat4("model", model);
		objects[i].DrawWithTextures(_shader, shadowFBO.depthCubemap);
	}
	
	model = _room.getModel();
	_shader.setMat4("model", model);
	_room.bindTextures(shadowFBO.depthCubemap);
}

void renderSkybox(Skybox _skybox,Shader _shader) 
{
	_shader.use();
	glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
	_shader.setMat4("view", view);
	_shader.setMat4("projection", projection);
	_skybox.draw(_skybox.dayCubemapTexture);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	camera.Rotate(xoffset, yoffset);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.Move(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.Move(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.Move(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.Move(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !MoveLightKeypressed) {
		MoveLightKeypressed = true;
		MoveLight = !MoveLight;
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
	{
		MoveLightKeypressed = false;
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !displayDepthKeyPressed)
	{
		displayDepth = !displayDepth;
		displayDepthKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
	{
		displayDepthKeyPressed = false;
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
//	/*_shader.setInt("material.diffuse", 0);
//	_shader.setInt("material.specular", 1);
//	_shader.setFloat("material.shininess", 32.0f);
//
//	_shader.setVec3("pointLight.position", camera.position);
//	_shader.setVec3("pointLight.ambient", 0.05f, 0.05f, 0.05f);
//	_shader.setVec3("pointLight.diffuse", 0.8f, 0.8f, 0.8f);
//	_shader.setVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);
//	_shader.setFloat("pointLight.constant", 1.0f);
//	_shader.setFloat("pointLight.linear", 0.09);
//	_shader.setFloat("pointLight.quadratic", 0.032);*/
