#include "texture.h"

Texture::Texture(const char* texturePath) {
	// set order ID, increment texture count
	orderID = textureCount++;

	// texture object
	glGenTextures(1, &internalTextureID);

	// bind texture
	glBindTexture(GL_TEXTURE_2D, internalTextureID);

	// set texture wrapping/filtering options (on currently bound texture)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load texture
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(texturePath, &width, &height, &nrChannels, 0);

	// load successful
	if (data) {
		// generate texture
		GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	// bad load
	else {
		std::cout << "Failed to load texture!" << std::endl;
	}

	// free image memory
	stbi_image_free(data);
} // end of Texture constructor

// get texture ID
unsigned int Texture::getID() const {
	return internalTextureID;
}

// bind/activate texture
void Texture::bindActivateTexture() const {
	glActiveTexture(0x84C0 + orderID);
	glBindTexture(GL_TEXTURE_2D, internalTextureID);
}


