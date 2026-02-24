#include "ubo_gl.h"

#include <glad/glad.h>

//--- PUBLIC ---//
UBOGL::UBOGL(UBOBinding binding)
	: binding_(static_cast<uint32_t>(binding))
{
} // end of constructor

UBOGL::~UBOGL()
{
	if (ubo_)
	{
		glDeleteBuffers(1, &ubo_);
		ubo_ = 0;
	}
} // end of destructor

void UBOGL::init(uint32_t size)
{
	glCreateBuffers(1, &ubo_);
	glNamedBufferStorage(ubo_, size, nullptr, GL_DYNAMIC_STORAGE_BIT);

	// binding point
	glBindBufferBase(GL_UNIFORM_BUFFER, binding_, ubo_);
} // end of init()

void UBOGL::update(const void* data, uint32_t size)
{
	glNamedBufferSubData(ubo_, 0, size, data);
} // end of update()