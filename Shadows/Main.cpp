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

void setCameraViewTransforms(Shader _shader);

void renderScene(Shader _shader, Room _room, Model _model, Cube _cube, Plane _plane, bool withTextures);
void renderSkybox(Skybox _skybox, Shader _shader);

void framebuffer_size_callback(GLFWwindow* window, int screenWidth, int screenHeight);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// settings
const unsigned int screenWidth = 1280;
const unsigned int screenHeight = 720;
bool shadows = true;
bool shadowsKeyPressed = false;
bool MoveLight = true;
bool MoveLightKeypressed = false;

unsigned int depthCubemap;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), -90.0f, 0.0f);
float lastX = screenWidth / 2.0f;
float lastY = screenHeight / 2.0f;
bool firstMouse = true;

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

	// configure depth map FBO
	// -----------------------
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	// create depth cubemap texture
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Create shaders and objects
	Shader skyboxShader("skybox.vert", "skybox.frag");
	Shader shader("pointLShadows.vert", "pointLShadows.frag");
	Shader simpleDepthShader("pointLShadowsDepth.vert", "pointLShadowsDepth.frag","pointLShadowsDepth.geo");

	// shader configuration
	// --------------------
	shader.use();
	shader.setInt("diffuseTexture", 0);
	shader.setInt("depthMap", 1);

	//Model samus("Samus/DolSzerosuitR1.obj");

	Room room;
	room.loadTexture("Textures/Wallpaper/1_Wallpaper design by Natasha Marsall_diffuse.jpg");
	room.loadTexture("Textures/Wallpaper/1_Wallpaper design by Natasha Marsall_specular.jpg");
	room.loadTexture("Textures/whiteness.jpg");
	//room.loadTexture("Textures/whiteness.jpg");
	room.setScale(glm::vec3(10.0f,5.0f,10.0f));
	room.setPos(glm::vec3(0.0f,5.0f,0.0f));

	Model samus("Models/obj_mesa/obj_mesa.obj");
	samus.setScale(glm::vec3(3.0f));
	samus.setPos(glm::vec3(3.0f, 0.0f, 3.0f));

	Cube container;
	container.loadTexture("container.png");
	//container.loadTexture("container_specular.png");

	Plane plane;
	plane.loadTexture("plane.jpg");
	//plane.loadTexture("plane_specular.jpg");
	
	// Skybox textures
	std::vector<std::string> faces
	{
		"skybox_right.jpg",
		"skybox_left.jpg",
		"skybox_top.jpg",
		"skybox_bottom.jpg",
		"skybox_front.jpg",
		"skybox_back.jpg"
	};
	Skybox skybox(faces);

	// lighting info
	// -------------
	glm::vec3 lightPos(0.0f, 2.0f, 3.0f);

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
			//lightPos.z = sin(glfwGetTime() * 0.5) * 3.0;
		}

		// rendering commands here
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 0. create depth cubemap transformation matrices
		// -----------------------------------------------
		float near_plane = 1.0f;
		float far_plane = 25.0f;
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
		std::vector<glm::mat4> shadowTransforms;
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

		// 1. render scene to depth cubemap
		// --------------------------------
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		simpleDepthShader.use();
		for (unsigned int i = 0; i < 6; ++i)
			simpleDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
		simpleDepthShader.setFloat("far_plane", far_plane);
		simpleDepthShader.setVec3("lightPos", lightPos);
		renderScene(simpleDepthShader,room,samus,container,plane,false);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		// 2. render scene as normal 
		// -------------------------
		glViewport(0, 0, screenWidth, screenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.use();
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		// set lighting uniforms
		shader.setVec3("lightPos", lightPos);
		shader.setVec3("viewPos", camera.position);
		shader.setInt("shadows", shadows); // enable/disable shadows by pressing 'SPACE'
		shader.setFloat("far_plane", far_plane);
		/*glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);*/
		renderScene(shader, room, samus,container,plane,true);
		renderSkybox(skybox,skyboxShader);

		// check and call events and swap the buffers
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;
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


void renderScene(Shader _shader, Room _room, Model _model, Cube _cube, Plane _plane, bool withTextures = false)
{
	glm::mat4 model = glm::mat4(1.0f);

	/*_shader.setInt("material.diffuse", 0);
	_shader.setInt("material.specular", 1);
	_shader.setFloat("material.shininess", 32.0f);

	_shader.setVec3("pointLight.position", camera.position);
	_shader.setVec3("pointLight.ambient", 0.05f, 0.05f, 0.05f);
	_shader.setVec3("pointLight.diffuse", 0.8f, 0.8f, 0.8f);
	_shader.setVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);
	_shader.setFloat("pointLight.constant", 1.0f);
	_shader.setFloat("pointLight.linear", 0.09);
	_shader.setFloat("pointLight.quadratic", 0.032);*/

	//Render Room
	model = _room.getModel();
	_shader.setMat4("model", model);
	if (withTextures){
		_room.bindTextures(depthCubemap);
	}
	else {
		_room.draw();
	}

	//// Floor plane
	/*if (withTextures) {
		_plane.bindTextures(depthCubemap);
	}
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(10.0f, 1.0f, 10.0f));
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0));
	_shader.setMat4("model", model);
	_plane.draw();*/


	//// cubes
	//if (withTextures){
	//	_cube.bindTextures(depthCubemap);
	//}
	//model = glm::mat4(1.0f);
	//model = glm::translate(model, glm::vec3(4.0f, -3.5f, 0.0));
	//model = glm::scale(model, glm::vec3(0.5f));
	//_shader.setMat4("model", model);
	//_cube.draw();
	//model = glm::mat4(1.0f);
	//model = glm::translate(model, glm::vec3(2.0f, 3.0f, 1.0));
	//model = glm::scale(model, glm::vec3(0.75f));
	//_shader.setMat4("model", model);
	//_cube.draw();
	//model = glm::mat4(1.0f);
	//model = glm::translate(model, glm::vec3(-3.0f, -1.0f, 0.0));
	//model = glm::scale(model, glm::vec3(0.5f));
	//_shader.setMat4("model", model);
	//_cube.draw();
	//model = glm::mat4(1.0f);
	//model = glm::translate(model, glm::vec3(-1.5f, 1.0f, 1.5));
	//model = glm::scale(model, glm::vec3(0.5f));
	//_shader.setMat4("model", model);
	//_cube.draw();
	//model = glm::mat4(1.0f);
	//model = glm::translate(model, glm::vec3(-1.5f, 2.0f, -3.0));
	//model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	//model = glm::scale(model, glm::vec3(0.75f));
	//_shader.setMat4("model", model);
	//_cube.draw();

	// model
	model = _model.getModel();
	_shader.setMat4("model", model);
	if (withTextures)
	{
		_model.DrawWithTextures(_shader, depthCubemap);
	}
	else
	{
		_model.Draw();
	}
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

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !shadowsKeyPressed)
	{
		shadows = !shadows;
		shadowsKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
	{
		shadowsKeyPressed = false;
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}