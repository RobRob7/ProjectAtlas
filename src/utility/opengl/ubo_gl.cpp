#include "ubo_gl.h"

#include <stdexcept>

//--- PUBLIC ---//
UBOGL::UBOGL(uint32_t binding)
	: binding_(binding)
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

void UBOGL::bind()
{
	// binding point
	glBindBufferBase(GL_UNIFORM_BUFFER, binding_, ubo_);
} // end of bind()

void UBOGL::update(const void* data, const uint32_t size)
{
	if (!ubo_)
	{
		throw std::runtime_error("UBOGL::UPDATE MUST CALL INIT() first!");
	}
	glNamedBufferSubData(ubo_, 0, size, data);
} // end of update()