#include "SP-P_Engine/Parsers/IDE.h"
#include <cstdio>
#include <cstring>

IDE* IDE_struct;
unsigned int count_of_ids = 0;

void ParserIDE() {
	FILE* idefile = fopen("models/SPP.IDE", "rb");
	while (!feof(idefile)) {
		if ((fgetc(idefile)) == '\n') {
			++count_of_ids;
		}
	}
	--count_of_ids;

	IDE_struct = new IDE[count_of_ids];
	fseek(idefile, 6, SEEK_SET);
	for (unsigned int i = 0; i < count_of_ids; ++i) {
		memset(IDE_struct[i].m_names, '\0', 24);
		memset(IDE_struct[i].t_names, '\0', 24);

		fscanf(idefile, "%i%*c%*c", &IDE_struct[i].id);
		fscanf(idefile, "%[^,]", &IDE_struct[i].m_names[0]);
		fscanf(idefile, "%*c%*c");
		fscanf(idefile, "%[^,]", &IDE_struct[i].t_names[0]);
		strcat(IDE_struct[i].t_names, ".gim");
		fscanf(idefile, "%*c%*c%*i%*c%*c%*i%*c%*c");
	}
	fclose(idefile);
}