#ifndef _CUBE_H_
#define _CUBE_H_

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "stb_image.h"

#include <iostream>


class Cube {
public:
	Cube(char const * diffuseTexturePath, char const * specularTexturePath);
	unsigned int VAO;
	unsigned int diffuseMap, specularMap;
	void draw();
private:
	unsigned int VBO;
	void configureCube();
	unsigned int loadTexture(char const * path);
};

#endif