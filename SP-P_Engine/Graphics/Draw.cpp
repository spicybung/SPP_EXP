#include "SP-P_Engine/Graphics/Draw.h"
#include "SP-P_Engine/Graphics/Obj.h"
#include <pspgu.h>
#include <pspgum.h>
#include <math.h>

int g_draw_calls_this_frame = 0;
bool g_enable_culling = true;
unsigned int __attribute__((aligned(16))) list[262144];//262144 (1mb под команды GE)

// =======================================================
// ПРОГРАММНЫЙ FRUSTUM CULLING (RADAR OPTIMIZED FOR PSP)
// =======================================================
// Используем только 3 плоскости: 0-Right, 1-Left, 2-Near, 3-Far
// ВНИМАНИЕ: Far отключен! Дальность контролируется отдельной дельтой.
float g_frustum_planes[3][4];

void UpdateFrustum() {
	ScePspFMatrix4 proj, view;
	sceGumMatrixMode(GU_PROJECTION);
	sceGumStoreMatrix(&proj);
	sceGumMatrixMode(GU_VIEW);
	sceGumStoreMatrix(&view);

	float proj_f_mtx[16], view_f_mtx[16], clip_f_mtx[16];
	memcpy(proj_f_mtx, &proj, sizeof(ScePspFMatrix4));
	memcpy(view_f_mtx, &view, sizeof(ScePspFMatrix4));

	// Умножаем матрицы: clip_f_mtx = Projection * View
	for (int i = 0; i < 16; i += 4) {
		for (int j = 0; j < 4; ++j) {
			clip_f_mtx[i + j] = proj_f_mtx[j] * view_f_mtx[i] + proj_f_mtx[4 + j] * view_f_mtx[i + 1] + proj_f_mtx[8 + j] * view_f_mtx[i + 2] + proj_f_mtx[12 + j] * view_f_mtx[i + 3];
		}
	}

	// Извлекаем только 3 плоскости
	g_frustum_planes[0][0] = clip_f_mtx[3] - clip_f_mtx[0]; g_frustum_planes[0][1] = clip_f_mtx[7] - clip_f_mtx[4]; g_frustum_planes[0][2] = clip_f_mtx[11] - clip_f_mtx[8]; g_frustum_planes[0][3] = clip_f_mtx[15] - clip_f_mtx[12]; // Right
	g_frustum_planes[1][0] = clip_f_mtx[3] + clip_f_mtx[0]; g_frustum_planes[1][1] = clip_f_mtx[7] + clip_f_mtx[4]; g_frustum_planes[1][2] = clip_f_mtx[11] + clip_f_mtx[8]; g_frustum_planes[1][3] = clip_f_mtx[15] + clip_f_mtx[12]; // Left
	g_frustum_planes[2][0] = clip_f_mtx[3] + clip_f_mtx[2]; g_frustum_planes[2][1] = clip_f_mtx[7] + clip_f_mtx[6]; g_frustum_planes[2][2] = clip_f_mtx[11] + clip_f_mtx[10]; g_frustum_planes[2][3] = clip_f_mtx[15] + clip_f_mtx[14]; // Near

	// Нормализуем плоскости
	for (int i = 0; i < 3; ++i) {
		float length = sqrtf(g_frustum_planes[i][0] * g_frustum_planes[i][0] + g_frustum_planes[i][1] * g_frustum_planes[i][1] + g_frustum_planes[i][2] * g_frustum_planes[i][2]);
		if (length > 0.0001f) {
			float inv_len = 1.0f / length;
			g_frustum_planes[i][0] *= inv_len;
			g_frustum_planes[i][1] *= inv_len;
			g_frustum_planes[i][2] *= inv_len;
			g_frustum_planes[i][3] *= inv_len;
		}
	}
}

// Оптимизированная проверка (4 плоскости)
bool SphereInFrustum(float x, float y, float z, float radius) {
	for (int i = 0; i < 3; ++i) {
		float distance = g_frustum_planes[i][0] * x + g_frustum_planes[i][1] * y + g_frustum_planes[i][2] * z + g_frustum_planes[i][3];
		if (distance <= -radius) {
			return false; // Отсекаем!
		}
	}
	return true;
}

// =======================================================
// ПРЕДРАСЧЕТ МАТРИЦ (Вызвать ОДИН РАЗ после загрузки карты)
// =======================================================

void PrecalculateMatrices() {
	for (unsigned int i = 0; i < countobj; ++i) {
		if (drawlist[i].MDL_clump == nullptr) continue;
		int flag = (int)(*drawlist[i].MDL_clump->flags[0]);

		if (flag == 0) {
			// Достаем WRLD масштаб, который мы сохранили в конвертере
			// wrld_radius лежит сразу после 16-байтного заголовка в .mdl файле.
			float &scale = *drawlist[i].MDL_clump->wrld_radius;
			float* mat = drawlist[i].IPL_struct->rotation_matrix;
				mat[0] *= scale; mat[1] *= scale; mat[2] *= scale;
				mat[4] *= scale; mat[5] *= scale; mat[6] *= scale;
				mat[8] *= scale; mat[9] *= scale; mat[10] *= scale;
			}
	}
}

// =======================================================
// ЕДИНОРАЗОВАЯ НАСТРОЙКА ОСВЕЩЕНИЯ И СТЕЙТОВ
// =======================================================
void InitRenderStates() {
	sceGuDisable(GU_LIGHTING);
	sceGuEnable(GU_LIGHT0);
	sceGuEnable(GU_NORMALIZED_NORMAL);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xFFCCCCCC);

	ScePspFVector3 sunDir = { -0.9701f, 0.0f, 0.2425f };
	sceGuLight(0, GU_DIRECTIONAL, GU_AMBIENT | GU_DIFFUSE, &sunDir);
	sceGuLightColor(0, GU_DIFFUSE, 0xFFFFFFFF);
	sceGuLightColor(0, GU_AMBIENT, 0xFFCCCCCC);

	sceGuColorMaterial(GU_AMBIENT | GU_DIFFUSE);
	sceGuMaterial(GU_AMBIENT | GU_DIFFUSE, 0xFFFFFFFF);

	sceGuEnable(GU_CLIP_PLANES);
	//sceGuDisable(GU_CLIP_PLANES);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuDepthFunc(GU_LEQUAL);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_CULL_FACE);
	sceGuFrontFace(GU_CCW);
	sceGuClearColor(0xFFFFB466);
	sceGuClearDepth(65535);

	sceGuDisable(GU_BLEND);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuAlphaFunc(GU_GREATER, 0x10, 0xFF);
	sceGuDepthMask(GU_FALSE);
}

// =======================================================
// ГЛАВНЫЙ ЦИКЛ ОТРИСОВКИ (ДИНАМИЧЕСКИЙ CULLING)
// =======================================================
void OnDraw()
{
	g_draw_calls_this_frame = 0;

	sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);

    // ============================================================
    // ЭТАП 0: ДИНАМИЧЕСКОЕ ОБНОВЛЕНИЕ КАМЕРЫ И МАТРИЦ
    // ============================================================
    sceGumMatrixMode(GU_PROJECTION);
    sceGumLoadIdentity();
    // СЮДА ПЕРЕНЕСЕНО: Теперь fov и дальность обновляются каждый кадр!
    sceGumPerspective(fov, 480.0f / 272.0f, front_plane, draw_dist);
	Camera();
	sceGumUpdateMatrix();

    // Пересчитываем плоскости камеры для отсечения геометрии
    UpdateFrustum();
	// Базовые стейты
	sceGuDisable(GU_LIGHTING);
	sceGuDisable(GU_BLEND);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuDepthMask(GU_FALSE);
	sceGuEnable(GU_CULL_FACE);
	sceGuFrontFace(GU_CCW);

	ClumpTEX* last_bound_tex_ptr = nullptr;
	unsigned int i = 0;
	bool is_lighting_enabled = false;
	bool is_culling_disabled = false;

	int vertex_format;
	int prim_type;
	unsigned int vertex_stride;

	// Кэшируем камеру для сверхбыстрого доступа
	float cx = m_camera_position_x;
	float cy = m_camera_position_y;

	while (i < countobj) {
		unsigned int current_instances = drawlist[i].IPL_struct->count_instances;

		// ============================================================
		// ЭТАП 1: СОБИРАЕМ СПИСОК ВИДИМЫХ КЛОНОВ (CULLING)
		// ============================================================
		unsigned int visible_instances[2048]; // на стеке
		unsigned int num_visible = 0;

		float obj_radius = 50.0f; // Запас
		if (drawlist[i].MDL_clump->model_radius != nullptr) {
			obj_radius = *drawlist[i].MDL_clump->model_radius;
		}

		// --- НАСТРОЙКИ ДЕЛЬТ ОТСЕЧЕНИЯ ---
		// 1. Насколько метров дальше аппаратного draw_dist мы отсекаем геометрию программно
		float far_cull_delta = 85.0f;
		// 2. Искусственное "раздувание" модели, чтобы она не пропадала по краям экрана
		//float edge_cull_delta = 15.0f;

		// Вычисляем квадрат дистанции ровно 1 раз для всей группы
		float cull_dist = (float)draw_dist + obj_radius + far_cull_delta;//155+, иначе frustrum culling съедает объекты по краям экрана
		float cull_dist_sq = cull_dist * cull_dist;

		for (unsigned int inst = i; inst < i + current_instances; ++inst) {
			// 2. Если куллинг выключен, принудительно добавляем объект
			if (!g_enable_culling) {
				if (num_visible < 2048) visible_instances[num_visible++] = inst;
				continue;
			}

            float ox = drawlist[inst].IPL_struct->x_ipl;
            float oy = drawlist[inst].IPL_struct->y_ipl;
            float oz = drawlist[inst].IPL_struct->z_ipl;

            float dx = ox - cx;
            float dy = oy - cy;
            // dz убрано! Теперь дальность считается как Цилиндр (высота игнорируется)

            if ((dx * dx + dy * dy) <= cull_dist_sq) {
				//float cull_radius_delta = 10.0f;
                // ПРОВЕРКА FRUSTUM CULLING (Попадает ли объект в fov камеры?)
                if (SphereInFrustum(ox, oy, oz, obj_radius )) {
                    if (num_visible < 2048) {
                        visible_instances[num_visible++] = inst;
                    }
                }
            }
        }

		// Если ни один клон столба не попал в кадр - пропускаем весь процесс биндинга текстур!
		if (num_visible == 0) {
			i += current_instances;
			continue;
		}

		// ============================================================
		// ЭТАП 2: ОТРИСОВКА МАТЕРИАЛОВ (ТОЛЬКО ДЛЯ ВИДИМЫХ КЛОНОВ)
		// ============================================================
		int flag = (int)(*drawlist[i].MDL_clump->flags[0]);

		prim_type = (flag == 0) ? GU_TRIANGLE_STRIP : ((flag == 1) ? GU_TRIANGLE_STRIP : GU_TRIANGLES);
		vertex_format = (flag == 0) ? (GU_TEXTURE_8BIT | GU_COLOR_5551 | GU_VERTEX_16BIT | GU_TRANSFORM_3D) :
			(GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_NORMAL_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_3D);
		vertex_stride = (flag == 0) ? 10 : 36;

		if (i == ALPHA_START_INDEX) {
			if (is_lighting_enabled) {
				sceGuDisable(GU_LIGHTING);
				is_lighting_enabled = false;
			}
			sceGuEnable(GU_BLEND);
			sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
			last_bound_tex_ptr = nullptr;
		}

		IMG* img = drawlist[i].IMG_model_ptr;
		if (img == nullptr || img->vaddr == nullptr) {
			i += current_instances;
			continue;
		}

		unsigned short num_materials = *drawlist[i].MDL_clump->count_materials_or_index_of_material[0];
		unsigned int geom_offset = *drawlist[i].MDL_clump->mdl_size;

		// Компенсация за глобальный 16-байтный заголовок в формате WRLD
		if (flag == 0) {
			geom_offset -= 16;
		}

		// --- Применение динамического освещения (только для глухих объектов) ---
		if (i < ALPHA_START_INDEX) {
			if (*drawlist[i].MDL_clump->dyn_ligting_on[0] != is_lighting_enabled) {
				if (*drawlist[i].MDL_clump->dyn_ligting_on[0]) {
					sceGuEnable(GU_LIGHTING);
				} else {
					sceGuDisable(GU_LIGHTING);
				}
				is_lighting_enabled = *drawlist[i].MDL_clump->dyn_ligting_on[0];
			}
		}

		// Проверка флага куллинга 
		if (*drawlist[i].MDL_clump->dis_cull[0] != is_culling_disabled) {
			if (*drawlist[i].MDL_clump->dis_cull[0]) {
				sceGuDisable(GU_CULL_FACE);
			} else {
				sceGuEnable(GU_CULL_FACE);
			}
			is_culling_disabled = *drawlist[i].MDL_clump->dis_cull[0];
		}

		// Отрисовка сабмешей модели
		for (int m0 = num_materials - 1; m0 >= 0; --m0) {
			unsigned int countInSub = *drawlist[i].MDL_clump->count_vertices_in_submesh[m0];
			if (countInSub <= 0) continue;

			geom_offset -= countInSub * vertex_stride;
			void* current_vertex_ptr = (void*)((char*)img->vaddr + geom_offset);
			// --- Аппаратная распаковка WRLD (UV и Координаты) ---
			if (flag == 0) {
				// Восстановление UV (Аппаратная матрица текстуры)
				float uv_range = *drawlist[i].MDL_clump->uv_range[m0];
				float min_u = *drawlist[i].MDL_clump->min_u[m0];
				float min_v = *drawlist[i].MDL_clump->min_v[m0];
				float tex_scale = (128.0f / 255.0f) * uv_range;//view_f_mtx
				sceGuTexScale(tex_scale, tex_scale);
				sceGuTexOffset(min_u, min_v);
			} else {
				sceGuTexScale(1.0f, 1.0f);
				sceGuTexOffset(0.0f, 0.0f);
			}

			unsigned int tex_idx = *drawlist[i].MDL_clump->texture_index[m0];
			if (tex_idx != 0xFFFF) {
				ClumpTEX* tex = &TEX_clump[tex_idx];
				if (tex != last_bound_tex_ptr && tex->data != nullptr) {
					int gu_fmt = GU_PSM_8888;
					if (tex->pix_form == 0x00) gu_fmt = GU_PSM_5650;
					else if (tex->pix_form == 0x01) gu_fmt = GU_PSM_5551;
					else if (tex->pix_form == 0x02) gu_fmt = GU_PSM_4444;
					else if (tex->pix_form == 0x04) gu_fmt = GU_PSM_T4;
					else if (tex->pix_form == 0x05) gu_fmt = GU_PSM_T8;

					sceGuTexMode(gu_fmt, 0, 0, tex->pix_swizzle);

					if ((gu_fmt == GU_PSM_T4 || gu_fmt == GU_PSM_T8) && tex->pal_data != nullptr) {
						int clut_fmt = (tex->pal_form == 0x03) ? GU_PSM_8888 : GU_PSM_4444;
						sceGuClutMode(clut_fmt, 0, 0xFF, 0);
						unsigned int bytes_per_color = (tex->pal_form == 0x03) ? 4 : 2;
						unsigned int blocks = (tex->pal_count * bytes_per_color) / 32;
						sceGuClutLoad(blocks, tex->pal_data);
					}

					sceGuTexImage(0, tex->width, tex->height, tex->width, tex->data);
					sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
					sceGuTexFilter(GU_NEAREST, GU_NEAREST);//можно и включить (был GU_LINEAR)
					last_bound_tex_ptr = tex;
				}
			}

			// Проходим ТОЛЬКО по массиву видимых клонов, который мы собрали выше!
			for (unsigned int v = 0; v < num_visible; ++v) {
				unsigned int inst = visible_instances[v];

				ScePspFMatrix4 __attribute__((aligned(16))) aligned_matrix;
				memcpy(&aligned_matrix, drawlist[inst].IPL_struct->rotation_matrix, sizeof(ScePspFMatrix4));

				sceGuSetMatrix(GU_MODEL, &aligned_matrix);
				sceGuDrawArray(prim_type, vertex_format, countInSub, nullptr, current_vertex_ptr);

				++g_draw_calls_this_frame;
			}
			//for (unsigned int inst = i; inst < i + current_instances; ++inst) {


			//	// 2. ОТПРАВКА ГОТОВОЙ МАТРИЦЫ И ОТРИСОВКА
			//	// Кастуем float[16] к ScePspFMatrix4* для видеочипа
			//	////sceGuSetMatrix(GU_MODEL, (ScePspFMatrix4*)drawlist[inst].IPL_struct->rotation_matrix);

			//	//чуть больше загрузка CPU, но меньше GE (чем каст в ScePspFMatrix4*)
			//	ScePspFMatrix4 __attribute__((aligned(16))) aligned_matrix;
			//	memcpy(&aligned_matrix, drawlist[inst].IPL_struct->rotation_matrix, sizeof(ScePspFMatrix4));

			//	sceGuSetMatrix(GU_MODEL, &aligned_matrix);

			//	sceGuDrawArray(prim_type, vertex_format, countInSub, nullptr, current_vertex_ptr);

			//	++g_draw_calls_this_frame;
			//}
		}
		i += current_instances;
	}
}