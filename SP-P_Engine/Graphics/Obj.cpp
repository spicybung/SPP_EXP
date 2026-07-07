#include "SP-P_Engine/Graphics/Obj.h"
#include <string>

OBJ* drawlist;
unsigned short indx = 0;
ClumpTEX* TEX_clump;

void MasteringOBJ() {
	ParserIPL();
	ParserIDE();

	drawlist = new OBJ[countobj];
	PreIMG();

	TEX_clump = new ClumpTEX[count_files - count_of_ids];

	// ========================================================
	// ПРОХОД 1: ЛИНКОВКА И КЭШИРОВАННАЯ ЗАГРУЗКА MDL
	// ========================================================
	unsigned int ide_idx = 0;
	unsigned int last_id = 0xFFFFFFFF;
	IMG* current_mdl_ptr = nullptr;
	IDE* current_ide_ptr = nullptr;

	for (unsigned int n = 0; n < countobj; ++n) {
		drawlist[n].IPL_struct = &IPL_struct[n];

		// Сброс словаря при переходе к альфа-объектам
		if (n == ALPHA_START_INDEX) {
			ide_idx = 0;
			last_id = 0xFFFFFFFF;
		}

		unsigned int current_id = drawlist[n].IPL_struct->id;

		if (current_id != last_id) {
			while (ide_idx < count_of_ids && IDE_struct[ide_idx].id < current_id) {
				++ide_idx;
			}

			if (ide_idx < count_of_ids && IDE_struct[ide_idx].id == current_id) {
				current_ide_ptr = &IDE_struct[ide_idx];
				current_mdl_ptr = ParserIMG((char*)(std::string(current_ide_ptr->m_names) + ".mdl").c_str());
			} else {
				current_ide_ptr = nullptr;
				current_mdl_ptr = nullptr;
			}
			last_id = current_id;
		}

		drawlist[n].IDE_struct = current_ide_ptr;
		drawlist[n].IMG_model_ptr = current_mdl_ptr;
		drawlist[n].MDL_clump = current_mdl_ptr ? current_mdl_ptr->cached_clump : nullptr;
	}

	// ========================================================
	// ПРОХОД 2: КЭШИРОВАННАЯ ЗАГРУЗКА ТЕКСТУР (GIM)
	// ========================================================
	last_id = 0xFFFFFFFF;

	for (unsigned int n = 0; n < countobj; ++n) {
		if (drawlist[n].MDL_clump == nullptr) continue;

		unsigned int current_id = drawlist[n].IPL_struct->id;

		if (current_id != last_id) {
			for (unsigned short n2 = 0; n2 < *drawlist[n].MDL_clump->count_materials_or_index_of_material[0]; ++n2) {
				drawlist[n].flg_loaded = true;

				for (unsigned int n3 = 0; n3 < indx; ++n3) {
					if (strncmp(drawlist[n].MDL_clump->material_name[n2][0], TEX_clump[n3].name[0], strlen(drawlist[n].MDL_clump->material_name[n2][0])) == 0) {
						drawlist[n].flg_loaded = false;
						*drawlist[n].MDL_clump->texture_index[n2] = (unsigned short)n3;
						break;
					}
				}

				if (drawlist[n].flg_loaded == true) {
					drawlist[n].IMG_texture_ptr[n2] = ParserIMG((char*)(std::string(drawlist[n].MDL_clump->material_name[n2][0]) + ".gim").c_str());

					if (drawlist[n].IMG_texture_ptr[n2] != nullptr && drawlist[n].IMG_texture_ptr[n2]->file != nullptr) {
						TEX_clump[indx] = *ParserTEX(drawlist[n].IMG_texture_ptr[n2]->file, &TEX_clump[indx]);
						delete[] drawlist[n].IMG_texture_ptr[n2]->file;
						drawlist[n].IMG_texture_ptr[n2]->file = nullptr;
						*drawlist[n].MDL_clump->texture_index[n2] = (unsigned short)indx++;
					} else {
						*drawlist[n].MDL_clump->texture_index[n2] = 0;
					}
				}
			}
			last_id = current_id;
		}
	}

	CloseIMG(); // Закрываем файл после полной инициализации карты
}