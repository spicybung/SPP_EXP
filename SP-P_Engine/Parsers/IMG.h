#pragma once

#include "SP-P_Engine/Parsers/Model.h"
#include "SP-P_Engine/Parsers/IPL.h"

struct IMG
{
	unsigned int offsets;
	unsigned int sizes;
	char names[24];
	unsigned char* file;
	bool gim;

	void* raw_data;
	ClumpMDL* cached_clump;

	void* vaddr; // Единый указатель на вершины (в VRAM или RAM)
};

extern unsigned int count_files; // количество файлов в IMG

void PreIMG(); // Читаем метаинформацию с IMG
IMG* ParserIMG(char* filename); // Читаем IMG
void CloseIMG(); // Закрываем архив после загрузки