#pragma once

#ifdef PSP_SLIM
// Для Slim+ (разблокированные 4MB VRAM)
static unsigned int vram_end = 0x44400000;
#else
// Для Fat и Эмуляторов (стандартные 2MB VRAM)
static unsigned int vram_end = 0x44200000;
#endif

static unsigned int vram_alloc_ptr = 0x44154000;

inline void* alloc_vram(unsigned int size) {
	unsigned int aligned_size = size;
	if (aligned_size % 16 != 0) aligned_size = ((aligned_size / 16) + 1) * 16;

	if (vram_alloc_ptr + aligned_size > vram_end) {
		return nullptr;
	}

	void* ptr = (void*)vram_alloc_ptr;
	vram_alloc_ptr += aligned_size;
	return ptr;
}