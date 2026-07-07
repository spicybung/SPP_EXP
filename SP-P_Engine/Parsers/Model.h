#pragma once

struct ClumpMDL
{
	char* sign[3];
	char* flags[1];
	unsigned int* mdl_size;
	unsigned int* vertex_count;
	unsigned int* face_count;
	float* wrld_radius; // Общий радиус WRLD одного района (для восстановления из 2х байт)
	float* model_radius; // Индивидуальный радиус модели для просчёта frustrum culling
	// тут 8 пустых резервных байт
	// Заголовки сабмешей:
	char* material_name[32][24];
	unsigned short* count_vertices_in_submesh[32];
	unsigned short* count_materials_or_index_of_material[32];
	unsigned short* texture_index[32];
	bool* dyn_ligting_on[32];
	bool* dis_cull[32];

	// --- ДОБАВЛЕНО ДЛЯ WRLD UV ---
	float* uv_range[32]; // Индивидуальный множитель UV
	float* min_u[32];
	float* min_v[32];
	float* submesh_radius[32]; // Индивидуальный радиус сабмеша (для будущих коллизий)
};

ClumpMDL* ParserMDL(unsigned char* model);