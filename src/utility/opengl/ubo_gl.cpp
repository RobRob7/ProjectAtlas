#include "ubo_gl.h"

#include <stdexcept>

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

void UBOGL::update(const void* data, const uint32_t size)
{
	if (!ubo_)
	{
		throw std::runtime_error("UBOGL::UPDATE MUST CALL INIT() first!");
	}
	glNamedBufferSubData(ubo_, 0, size, data);
} // end of update()