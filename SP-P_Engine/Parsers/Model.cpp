#include "SP-P_Engine/Parsers/Model.h"
#include <cstring> //memcpy

ClumpMDL* ParserMDL(unsigned char* model) {
	char flag = *(model + 3);
	unsigned int extra_header = (flag == 0) ? 16 : 0;
	unsigned int submesh_hdr_size = (flag == 0) ? 48 : 32; // <-- ДОБАВЛЕНО: Новый размер сабмеша

	unsigned int submesh_iter = 0;
	unsigned int sub_hdr_ofst = 16 + extra_header;

	unsigned short* count_mats = (unsigned short*)(model + 16 + extra_header + 26);

	// Умножаем на новый динамический размер сабмеша
	unsigned int header_size = *count_mats * submesh_hdr_size + 16 + extra_header;

	unsigned char* tiny_meta = new unsigned char[header_size];
	memcpy(tiny_meta, model, header_size);

	ClumpMDL* MDL_clump = new ClumpMDL;

	MDL_clump->sign[0] = (char*)tiny_meta;
	MDL_clump->flags[0] = (char*)(tiny_meta + 3);
	MDL_clump->mdl_size = (unsigned int*)(tiny_meta + 4);
	MDL_clump->vertex_count = (unsigned int*)(tiny_meta + 8);
	MDL_clump->face_count = (unsigned int*)(tiny_meta + 12);

	if (flag == 0) {
		MDL_clump->wrld_radius = (float*)(tiny_meta + 16);
		MDL_clump->model_radius = (float*)(tiny_meta + 20);
	} else {
		MDL_clump->wrld_radius = nullptr;
		MDL_clump->model_radius = nullptr;
	}

	do {
		MDL_clump->material_name[submesh_iter][0] = (char*)(tiny_meta + sub_hdr_ofst);
		MDL_clump->count_vertices_in_submesh[submesh_iter] = (unsigned short*)(tiny_meta + sub_hdr_ofst + 24);
		MDL_clump->count_materials_or_index_of_material[submesh_iter] = (unsigned short*)(tiny_meta + sub_hdr_ofst + 26);
		MDL_clump->texture_index[submesh_iter] = (unsigned short*)(tiny_meta + sub_hdr_ofst + 28);
		MDL_clump->dyn_ligting_on[submesh_iter] = (bool*)(tiny_meta + sub_hdr_ofst + 30);
		MDL_clump->dis_cull[submesh_iter] = (bool*)(tiny_meta + sub_hdr_ofst + 31);

		// --- ЧИТАЕМ ПАРАМЕТРЫ WRLD (Смещение +32, 36, 40, 44) ---
		if (flag == 0) {
			MDL_clump->uv_range[submesh_iter] = (float*)(tiny_meta + sub_hdr_ofst + 32);
			MDL_clump->min_u[submesh_iter] = (float*)(tiny_meta + sub_hdr_ofst + 36);
			MDL_clump->min_v[submesh_iter] = (float*)(tiny_meta + sub_hdr_ofst + 40);
			MDL_clump->submesh_radius[submesh_iter] = (float*)(tiny_meta + sub_hdr_ofst + 44); // <-- ДОБАВЛЕНО
		} else {
			MDL_clump->uv_range[submesh_iter] = nullptr;
			MDL_clump->min_u[submesh_iter] = nullptr;
			MDL_clump->min_v[submesh_iter] = nullptr;
			MDL_clump->submesh_radius[submesh_iter] = nullptr;
		}

		sub_hdr_ofst += submesh_hdr_size; // Шагаем на 48 (или 32) байта
		++submesh_iter;
	} while (*MDL_clump->count_materials_or_index_of_material[0] > submesh_iter);

	return MDL_clump;
}