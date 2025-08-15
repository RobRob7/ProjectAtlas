#ifndef TEXTURE_H
#define TEXTURE_H

#include "stb/stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

class Texture
{
public:
	// default constructor loads texture from file path
	Texture(const char* texturePath);

	// get texture ID
	unsigned int getID() const;

	// bind/activate texture
	void bindActivateTexture() const;

private:
	// texture ID of internal texture object
	unsigned int internalTextureID;

	// order number of texture
	unsigned int orderID;

	// class member variable (keeps track of current textureID)
	inline static int textureCount = 0;
};
#endif

