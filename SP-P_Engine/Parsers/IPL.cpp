#include "SP-P_Engine/Parsers/IPL.h"
#include <ctype.h> // для isdigit
#include <cstdlib> // atoi

IPL* IPL_struct;
unsigned int countobj = 0;//количество файлов в IPL
//организация константы с единоразовым заполнением (через константную ссылку): 
unsigned int g_internal_alpha_start = 0;
const unsigned int& ALPHA_START_INDEX = g_internal_alpha_start;//откуда начинаются объекты с альфой


// Быстрый конвертер кватерниона в 4x4 Матрицу для OpenGL
void MathQuatToMatrix(float qx, float qy, float qz, float qw, float* matrix, float x_ipl, float y_ipl, float z_ipl)

{
    float xx = qx * qx; float xy = qx * qy; float xz = qx * qz; float xw = qx * qw;
    float yy = qy * qy; float yz = qy * qz; float yw = qy * qw;
    float zz = qz * qz; float zw = qz * qw;

    matrix[0] = 1.0f - 2.0f * (yy + zz);
    matrix[1] = 2.0f * (xy + zw);
    matrix[2] = 2.0f * (xz - yw);
    matrix[3] = 0.0f;


    matrix[4] = 2.0f * (xy - zw);
    matrix[5] = 1.0f - 2.0f * (xx + zz);
    matrix[6] = 2.0f * (yz + xw);
    matrix[7] = 0.0f;


    matrix[8] = 2.0f * (xz + yw);
    matrix[9] = 2.0f * (yz - xw);
    matrix[10] = 1.0f - 2.0f * (xx + yy);
    matrix[11] = 0.0f;

    // Когда вы конвертируете кватернион в матрицу, добавьте позиции в конец:
    matrix[12] = x_ipl;
    matrix[13] = y_ipl;
    matrix[14] = z_ipl;
    matrix[15] = 1.0f;
}



void ParserIPL() {
    FILE* iplfile = fopen("models/SPP.IPL", "rb");

    // --- ШАГ 1: ЧИТАЕМ ALPHA_START_INDEX ИЗ ПЕРВОЙ СТРОКИ ---
    char header_buf[32];
    if (fgets(header_buf, sizeof(header_buf), iplfile)) {
        if (header_buf[0] == '#') {
            // Читаем число после решетки и вычитаем 3
            g_internal_alpha_start = (unsigned int)atoi(&header_buf[1]) - 3;
        }
    }
    rewind(iplfile); // возвращаемся в начало для подсчета объектов



    // Считаем объекты (строки, начинающиеся с цифр)
    char count_buf[256];
    countobj = 0;
    while (fgets(count_buf, sizeof(count_buf), iplfile)) {
        if (isdigit(count_buf[0])) ++countobj;
    }

    IPL_struct = new IPL[countobj];


    for (unsigned int i = 0; i < countobj; ++i) {
        IPL_struct[i].count_instances = 1; // инициализация по умолчанию
    }

    rewind(iplfile);

    unsigned int last_model_id = 0;
    int inst_id_counter = 0;
    int n_last = 0;
    unsigned int i0 = 0;


    // --- ШАГ 2: ПАРСИНГ ДАННЫХ ---
    char line_buf[256];
    while (fgets(line_buf, sizeof(line_buf), iplfile)) {
        // Пропускаем все, что не данные (комментарии, inst, end)
        if (!isdigit(line_buf[0])) continue;

        sscanf(line_buf, "%i, %[^,], %*i, %f, %f, %f, %f, %f, %f, %f",
            &IPL_struct[i0].id,
            IPL_struct[i0].names,
            &IPL_struct[i0].x_ipl, &IPL_struct[i0].y_ipl, &IPL_struct[i0].z_ipl,
            &IPL_struct[i0].qx, &IPL_struct[i0].qy, &IPL_struct[i0].qz, &IPL_struct[i0].qw);

        // Логика батчинга по ID
        if (i0 != 0 && (last_model_id != IPL_struct[i0].id)) {
            int batch_size = i0 - n_last;
            for (int n = i0 - 1; n >= n_last; --n) {
                IPL_struct[n].count_instances = batch_size;
            }

            n_last = i0;
            ++inst_id_counter;
        }

        IPL_struct[i0].inst_id = inst_id_counter;

        MathQuatToMatrix(
            IPL_struct[i0].qx, IPL_struct[i0].qy,
            IPL_struct[i0].qz, -IPL_struct[i0].qw,
            IPL_struct[i0].rotation_matrix,
            IPL_struct[i0].x_ipl, IPL_struct[i0].y_ipl, IPL_struct[i0].z_ipl
        );

        last_model_id = IPL_struct[i0].id;
        ++i0;
    }



    // Хвост последнего батча
    int final_batch = i0 - n_last;
    for (int n = i0 - 1; n >= n_last; --n) {
        IPL_struct[n].count_instances = final_batch;
    }

    fclose(iplfile);
}