#ifndef UBO_GL_H
#define UBO_GL_H

#include <glad/glad.h>

#include <cstdint>

class UBOGL
{
public:
	explicit UBOGL(uint32_t binding);
	~UBOGL();

	template<uint32_t Size>
	void init()
	{
		static_assert(Size % 16 == 0, "UBO size must be 16-byte aligned!");
		glCreateBuffers(1, &ubo_);
		glNamedBufferStorage(ubo_, Size, nullptr, GL_DYNAMIC_STORAGE_BIT);
	} // end of init()

	void bind();

	void update(const void* data, const uint32_t size);
private:
	uint32_t binding_;
	uint32_t ubo_{};
};

#endif
