#ifndef UBO_GL_H
#define UBO_GL_H

#include "ubo_bindings.h"

class UBOGL
{
public:
	explicit UBOGL(UBOBinding binding);
	~UBOGL();

	void init(uint32_t size);
	void update(const void* data, uint32_t size);
private:
	uint32_t binding_;
	uint32_t ubo_{};
};

#endif
