#include "SP-P_Engine/Parsers/IMG.h"
#include "SP-P_Engine/Graphics/Vram_manager.h"
#include <pspkernel.h> // sceKernelDcache**
#include <malloc.h>
#include <string>

using namespace std;

IMG* new_header;
unsigned int count_files = 0; // количество файлов в IMG
FILE* global_img_file = nullptr;
//deprecated:
//unsigned int IMG_SIZE = 0;

void PreIMG() {
	global_img_file = fopen("models/SPP.IMG", "rb");

	fseek(global_img_file, 0, SEEK_END);
	[[maybe_unused]]const unsigned int IMG_SIZE = ftell(global_img_file);// размер IMG (пусть будет)
	fseek(global_img_file, 4, SEEK_SET);
	fread(&count_files, sizeof(unsigned int), 1, global_img_file);

	new_header = new IMG[count_files];

	for (unsigned int i = 0; i < count_files; ++i) {
		memset(new_header[i].names, '\0', 24);
		new_header[i].file = nullptr;
		new_header[i].cached_clump = nullptr;
		new_header[i].raw_data = nullptr;
		new_header[i].vaddr = nullptr;

		fread(&new_header[i].offsets, sizeof(unsigned int), 1, global_img_file);
		fread(&new_header[i].sizes, sizeof(unsigned int), 1, global_img_file);
		fread(&new_header[i].names, sizeof(char), 24, global_img_file);

		const char* null_pos = (const char*)memchr(new_header[i].names, '\0', 24);
		unsigned short len = null_pos ? null_pos - new_header[i].names : 24;
		const char* dot = nullptr;
		for (const char* p = new_header[i].names + len - 1; p >= new_header[i].names; --p) {
			if (*p == '.') { dot = p; break; }
		}

		std::string extension(dot + 1, new_header[i].names + len - (dot + 1));
		new_header[i].gim = (extension == "gim");
	}
}

void CloseIMG() {
	if (global_img_file != nullptr) {
		fclose(global_img_file);
		global_img_file = nullptr;
	}
}

IMG* ParserIMG(char* filename) {
	if (global_img_file == nullptr) return nullptr;

	for (unsigned int i = 0; i < count_files; ++i) {
		if (std::string(filename) == std::string(new_header[i].names)) {
			if (new_header[i].cached_clump != nullptr || new_header[i].file != nullptr) return &new_header[i];

			fseek(global_img_file, new_header[i].offsets * 2048, SEEK_SET);

			if (new_header[i].gim) {
				unsigned int gim_size;
				fseek(global_img_file, 20, SEEK_CUR);
				fread(&gim_size, sizeof(unsigned int), 1, global_img_file);
				new_header[i].file = new unsigned char[gim_size + 16];
				fseek(global_img_file, new_header[i].offsets * 2048, SEEK_SET);
				fread(new_header[i].file, sizeof(char), gim_size + 16, global_img_file);
			} else {//TODO: функционал model парсера задублирован, вернуть делегацию на mdl_parser
				unsigned char flag;
				unsigned int mdl_size;
				unsigned short count_of_materials;

				fseek(global_img_file, 3, SEEK_CUR);
				fread(&flag, sizeof(unsigned char), 1, global_img_file);
				fread(&mdl_size, sizeof(unsigned int), 1, global_img_file);

				unsigned int extra_header = (flag == 0) ? 16 : 0;
				unsigned int submesh_hdr_size = (flag == 0) ? 48 : 32; // Динамический размер

				fseek(global_img_file, 34 + extra_header, SEEK_CUR);
				fread(&count_of_materials, sizeof(unsigned short), 1, global_img_file);

				// Считаем точный размер файла вместе с толстыми хедерами
				mdl_size = 16 + mdl_size + (count_of_materials * submesh_hdr_size);
				if (mdl_size % 16 != 0) mdl_size = ((mdl_size / 16) + 1) * 16;

				void* mapped_ptr = memalign(16, mdl_size);

				if (mapped_ptr) {
					new_header[i].raw_data = mapped_ptr;
					fseek(global_img_file, new_header[i].offsets * 2048, SEEK_SET);
					fread(mapped_ptr, sizeof(char), mdl_size, global_img_file);
					sceKernelDcacheWritebackInvalidateRange(mapped_ptr, mdl_size);

					// <-- ИСПРАВЛЕНИЕ: Точный отступ до вершин с новыми размерами
					unsigned int header_size = 16 + extra_header + (count_of_materials * submesh_hdr_size);
					unsigned int geom_size = mdl_size - header_size;

					new_header[i].vaddr = (char*)mapped_ptr + header_size;

					if (geom_size > 0) {
						void* fast_vram = alloc_vram(geom_size);
						if (fast_vram != nullptr) {
							memcpy(fast_vram, new_header[i].vaddr, geom_size);
							sceKernelDcacheWritebackInvalidateRange(fast_vram, geom_size);
							new_header[i].vaddr = fast_vram;
						}
					}
					new_header[i].cached_clump = ParserMDL((unsigned char*)mapped_ptr);
				}
			}
			return &new_header[i];
		}
	}
	return nullptr;
}