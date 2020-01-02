#ifndef _SKYBOX_H_
#define _SKYBOX_H_

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "stb_image.h"

#include <iostream>
#include <vector>
#include <string>


class Skybox {
public:
	Skybox(std::vector<std::string> faces);
	unsigned int VAO;
	unsigned int dayCubemapTexture;
	unsigned int loadCubemap(std::vector<std::string> faces);
	void draw(unsigned int cubemapTexture);
private:
	unsigned int VBO;
	void configureSkybox();
	
};
#endif