#ifndef _ROOM_H_
#define _ROOM_H_

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "stb_image.h"

#include "TransformComponent.h"

#include <iostream>
#include <vector>

class Transform;

class Room : public Transform{
public:

	unsigned int VAO;

	Room();
	void loadTexture(char const * path);
	void draw();

	void bindTextures(unsigned int _shadowCubemap);
	void drawFloorCeiling();
	void drawWalls();
	
private:
	unsigned int VBO;
	std::vector<unsigned int> textures;
};

#endif