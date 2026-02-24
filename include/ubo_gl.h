#ifndef UBO_GL_H
#define UBO_GL_H

#include "ubo_bindings.h"

#include <glad/glad.h>

class UBOGL
{
public:
	explicit UBOGL(UBOBinding binding);
	~UBOGL();

	template<uint32_t Size>
	void init()
	{
		static_assert(Size % 16 == 0, "UBO size must be 16-byte aligned!");
		glCreateBuffers(1, &ubo_);
		glNamedBufferStorage(ubo_, Size, nullptr, GL_DYNAMIC_STORAGE_BIT);

		// binding point
		glBindBufferBase(GL_UNIFORM_BUFFER, binding_, ubo_);
	} // end of init()

	void update(const void* data, const uint32_t size);
private:
	uint32_t binding_;
	uint32_t ubo_{};
};

#endif
