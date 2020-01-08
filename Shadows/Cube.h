#ifndef _CUBE_H_
#define _CUBE_H_

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "stb_image.h"

#include <iostream>
#include <vector>


class Cube {
public:
	
	unsigned int VAO;
	
	Cube();
	void loadTexture(char const * path);
	void bindTextures(unsigned int _shadowCubemap);
	void draw();
private:
	unsigned int VBO;
	std::vector<unsigned int> textures;
};

#endif