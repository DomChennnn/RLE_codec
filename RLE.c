#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4267)



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

/* Stores parameters that specify how to the program should behave.
 *
 * Populated by the get_parms() function. */
struct parms {
    char* filename;
    char* filename2;
    unsigned char mode;
};

typedef struct parms Parms;


/** LOAD RGB DATA ****************************************/
typedef struct {
    unsigned char* red;
    unsigned char* green;
    unsigned char* blue;
    int width;
    int height;
} RGBData;


RGBData read_bmp_rgb(const char* filename) {
    RGBData rgb_data = { 0 };

    // 使用 stbi_load 函数加载 BMP 图像
    unsigned char* data;
    data = stbi_load(filename, &rgb_data.width, &rgb_data.height, NULL, STBI_rgb);

    if (data != NULL) {
        // 为 redd green 和 blue 分配内存
        rgb_data.red = (unsigned char*)malloc(rgb_data.width * rgb_data.height);
        rgb_data.green = (unsigned char*)malloc(rgb_data.width * rgb_data.height);
        rgb_data.blue = (unsigned char*)malloc(rgb_data.width * rgb_data.height);

        // 从数据中提取RGB数据
        for (int i = 0; i < rgb_data.width * rgb_data.height; ++i) {
            rgb_data.red[i] = data[3 * i];
            rgb_data.green[i] = data[3 * i + 1];
            rgb_data.blue[i] = data[3 * i + 2];
        }

        printf("Image loaded successfully.\n");
        printf("Width: %d\n", rgb_data.width);
        printf("Height: %d\n", rgb_data.height);

        // 释放红色通道数据内存
        stbi_image_free(data);
    }
    else {
        // 处理加载图像失败的情况
        printf("Error loading image.\n");
    }

    return rgb_data;
}

void free_rgb_data(RGBData* rgb_data) {
    // 释放分配的内存
    if (rgb_data->red != NULL) {
        free(rgb_data->red);
    }
    if (rgb_data->green != NULL) {
        free(rgb_data->green);
    }
    if (rgb_data->blue != NULL) {
        free(rgb_data->blue);
    }
}


// 重建图片
void reconstruct_image(RGBData rgb_data, const char* output_filename){
// 分配内存用于存储RGB图像数据
    unsigned char* image_data = (unsigned char*)malloc(rgb_data.width * rgb_data.height * 3);
    if (image_data == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }

    // 将RGB三个通道的数据合并为一张图像
    for (int i = 0; i < rgb_data.width * rgb_data.height; ++i) {
        image_data[i * 3] = rgb_data.red[i];
        image_data[i * 3 + 1] = rgb_data.green[i];
        image_data[i * 3 + 2] = rgb_data.blue[i];
    }

    // 保存图像为bmp文件
    int result = stbi_write_bmp(output_filename, rgb_data.width, rgb_data.height, 3, image_data);

    // 检查保存结果
    if (result) {
        printf("Image successfully reconstructed and saved to %s\n", output_filename);
    }
    else {
        fprintf(stderr, "Failed to save image to %s\n", output_filename);
    }

    // 释放内存
    free(image_data);
}


/** U S E R   I N T E R F A C E ****************************************/

/* This function simply displays helpful usage information if the
 * program is called improperly or with no command line arguments. */
void print_usage(const char* cmd)
{
    printf("Usage: %s MODE filename\n\n"
        "Available Modes:\n"
        "  -e     Compress: Performs RLE compression on \"filename\"\n"
        "                   and writes result to \"filename.rle\"\n\n"
        "  -d     Expand: Performs RLE expansion on \"filename\". The\n"
        "                 supplied \"filename\" must have the extension\n"
        "                 \".rle\" The result is written to \"filename\"\n"
        "                 with the extension \".rle\" removed.\n\n"
        "Examples:\n"
        "  %s -e test.bmp test.rle\n\tProduces RLE encoded file test.rle\n"
        "  %s -d test.rle rec.bmp\n\tExpands test.rle to disk as test.bmp\n",
        cmd, cmd);
}


/* This function parses the command line arguments supplied in
 * argc/argv and populates the Params struct with the mode and filename
 * specified by the user on the command line. */
int get_parms(Parms* parms, const char* modes, int argc, char** argv)
{
    int i = 0;

    if (argc != 4 || *argv[1] != '-')
        return 0;

    while (modes[i] && modes[i] != *(argv[1] + 1))
        i++;

    if ((parms->mode = i) == strlen(modes)) {
        fprintf(stderr, "Invalid Mode %s\n", argv[1]);
        return 0;
    }

    parms->filename = argv[2];
    parms->filename2 = argv[3];

    return 1;
}


/** H E L P E R   F U N C T I O N S ************************************/

/* Returns a newly allocated string on the heap containing the supplied
 * filename with the specified extension added to its end.  This
 * function effectively just concatenates two strings. */
char* filename_add_ext(const char* filename, const char* ext)
{
    /* The 1 is for the . */
    int new_length = strlen(filename) + 4;
    char* new_name = malloc(new_length);

    strcpy(new_name, filename);
    strcat(new_name, ext);

    return new_name;
}

/* Returns a newly allocated string on the heap containing the supplied
 * filename with its extension removed.
 *
 * For example:
 *   if `filename` contains the string "test.txt.rle", then this
 *   function will return a string on the heap containing
 *   "test.txt" */
char* filename_rm_ext(const char* filename)
{
    int length = strlen(filename);
    char* new_name = malloc(length - 4);
    strncpy(new_name, filename, length - 4);
    //free(new_name);

    return new_name;
}

/* Returns a newly allocated string on the heap containing the supplied
 * filename with addstr at pos.
 *
 * For example:
 *   if `filename` contains the string "test.txt", addstr "rec", pos 0, then this
 *   function will return a string on the heap containing
 *   "rectest.txt" */
char* filename_add_at_pos(const char* filename, const char* addstr, int pos) {
    if (filename == NULL || addstr == NULL || pos < 0) {
        return NULL;
    }

    size_t filename_len = strlen(filename);
    size_t addstr_len = strlen(addstr);

    // 检查插入位置是否越界
    if (pos > filename_len) {
        return NULL;
    }

    // 计算新字符串的长度
    size_t new_len = filename_len + addstr_len + 1;
    char* new_filename = (char*)malloc(new_len);

    if (new_filename == NULL) {
        return NULL;  // 内存分配失败
    }

    // 将 filename 的前 pos 个字符复制到新字符串
    strncpy(new_filename, filename, pos);

    // 将 addstr 复制到新字符串的 pos 位置
    strncpy(new_filename + pos, addstr, addstr_len);

    // 将 filename 的剩余部分复制到新字符串
    strcpy(new_filename + pos + addstr_len, filename + pos);

    return new_filename;
}

/* This function returns zero if the supplied filename does not have the
 * extension ".rle"; otherwise it returns a non-zero value. */
int check_ext(const char* filename)
{
    if (strstr(filename, ".rle"))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/* This function returns zero if the supplied file does not have the
 * !RLE "magic" byte sequence as its first four bytes; otherwise it
 * returns a non-zero value. */
int check_magic(FILE* fp)
{
    int i = 0, container;
    char rle[] = "!RLE";

    for (; i < 4; i++)
    {
        container = fgetc(fp);
        if (container != rle[i] || container == EOF)
        {
            return 0;
        }
    }
    return 1;
}

size_t getFileSize(const char* filename)
{
    struct stat file_info;

    // 使用 stat 函数获取文件信息，返回值为 0 表示成功
    if (stat(filename, &file_info) == 0) {
        // 文件大小以字节为单位
        printf("File Size: %ld bytes\n", file_info.st_size);
    }
    else {
        // 处理获取文件信息失败的情况
        perror("Error getting file information");
    }

    return file_info.st_size;

}

/** R L E ************************************************************/
unsigned char* rle_compress(const unsigned char* data, int size, int* compressed_size) {
    if (data == NULL || size <= 0 || compressed_size == NULL) {
        return NULL;
    }

    // 估计压缩后的大小，最坏情况下为原大小的2倍
    unsigned char* compressed_data = (unsigned char*)malloc(size * 2);
    if (compressed_data == NULL) {
        return NULL;  // 内存分配失败
    }

    int count = 1;
    int index = 0;

    for (int i = 1; i < size; ++i) {
        if (data[i] == data[i - 1]) {
            if (count == 255) {
                compressed_data[index++] = data[i - 1];
                compressed_data[index++] = count;
                count = 1;  // 重置计数器
            }
            else {
                count++;
            }
        }
        else {
            compressed_data[index++] = data[i - 1];
            compressed_data[index++] = count;
            count = 1;
        }
    }

    // 处理最后一个字符
    compressed_data[index++] = data[size - 1];
    compressed_data[index++] = count;

    // 更新压缩后的大小
    *compressed_size = index;
    // 重新分配刚好大小的内存
    compressed_data = (unsigned char*)realloc(compressed_data, index);
    return compressed_data;
}

unsigned char* rle_decompress(const unsigned char* compressed_data, int compressed_size, int decompressed_size) {
    if (compressed_data == NULL || compressed_size <= 0 || decompressed_size <= 0) {
        return NULL;
    }

    // 预估解压后的大小，最坏情况下为压缩大小的2倍
    unsigned char* decompressed_data = (unsigned char*)malloc(decompressed_size);
    if (decompressed_data == NULL) {
        return NULL;  // 内存分配失败
    }

    int index = 0;

    for (int i = 0; i < compressed_size; i += 2) {
        unsigned char value = compressed_data[i];
        int count = compressed_data[i + 1];

        for (int j = 0; j < count; ++j) {
            decompressed_data[index++] = value;
        }
    }

    // 重新分配刚好大小的内存
    decompressed_data = (unsigned char*)realloc(decompressed_data, index);
    return decompressed_data;
}

void rle_encode(const char* filename, const char* binname) {
    RGBData rgb_data = read_bmp_rgb(filename);
    int compressed_size_red = 0;
    int compressed_size_blue = 0;
    int compressed_size_green = 0;

    unsigned char* compressed_data_red;
    unsigned char* compressed_data_blue;
    unsigned char* compressed_data_green;
    compressed_data_red = rle_compress(rgb_data.red, rgb_data.width * rgb_data.height, &compressed_size_red);
    compressed_data_green = rle_compress(rgb_data.green, rgb_data.width * rgb_data.height, &compressed_size_green);
    compressed_data_blue = rle_compress(rgb_data.blue, rgb_data.width * rgb_data.height, &compressed_size_blue);

    //head
    FILE* binfp = fopen(binname, "wb");

    if (!binfp)
    {
        fprintf(stderr, "Failed to open %s\n", binname);
        return;
    }

    fprintf(binfp, "!RLE"); //magic number
    fwrite(&rgb_data.width, sizeof(int), 1, binfp);//image size
    fwrite(&rgb_data.height, sizeof(int), 1, binfp);
    fwrite(&compressed_size_red, sizeof(int), 1, binfp);//compressed_size of each channel
    fwrite(&compressed_size_green, sizeof(int), 1, binfp);
    fwrite(&compressed_size_blue, sizeof(int), 1, binfp);
    fwrite(compressed_data_red, sizeof(unsigned char), compressed_size_red, binfp);//write data
    fwrite(compressed_data_green, sizeof(unsigned char), compressed_size_green, binfp);
    fwrite(compressed_data_blue, sizeof(unsigned char), compressed_size_blue, binfp);

    if (rgb_data.red != NULL) {
        free_rgb_data(&rgb_data);
    }
    free(compressed_data_red);
    free(compressed_data_green);
    free(compressed_data_blue);

    fclose(binfp);
}

void rle_decode(const char* filename, const char* recname) {
    RGBData rgb_data = { 0 };
    int compressed_size_red = 0;
    int compressed_size_blue = 0;
    int compressed_size_green = 0;

    unsigned char* compressed_data_red = NULL;
    unsigned char* compressed_data_blue = NULL;
    unsigned char* compressed_data_green = NULL;

    FILE* fp = fopen(filename, "rb");

    if (!fp)
    {
        fprintf(stderr, "Failed to open %s\n", filename);
        return;
    }

    /* Check if file user wants to expand is RLE */
    if (!check_ext(filename) || !check_magic(fp))
    {
        fprintf(stderr, "error -- %s is not RLE compressed\n", filename);
        fclose(fp);
        return;
    }

    fseek(fp, 4, SEEK_SET);

    fread(&rgb_data.width, sizeof(int), 1, fp);//image size
    fread(&rgb_data.height, sizeof(int), 1, fp);
    fread(&compressed_size_red, sizeof(int), 1, fp);//compressed_size of each channel
    fread(&compressed_size_green, sizeof(int), 1, fp);
    fread(&compressed_size_blue, sizeof(int), 1, fp);

    // 分配内存
    compressed_data_red = (unsigned char*)malloc(compressed_size_red);
    compressed_data_green = (unsigned char*)malloc(compressed_size_green);
    compressed_data_blue = (unsigned char*)malloc(compressed_size_blue);

    fread(compressed_data_red, sizeof(unsigned char), compressed_size_red, fp);
    fread(compressed_data_green, sizeof(unsigned char), compressed_size_green, fp);
    fread(compressed_data_blue, sizeof(unsigned char), compressed_size_blue, fp);


    rgb_data.red = rle_decompress(compressed_data_red, compressed_size_red, rgb_data.width * rgb_data.height);
    rgb_data.green = rle_decompress(compressed_data_green, compressed_size_green, rgb_data.width * rgb_data.height);
    rgb_data.blue = rle_decompress(compressed_data_blue, compressed_size_blue, rgb_data.width * rgb_data.height);

    reconstruct_image(rgb_data, recname);

    free(compressed_data_red);
    free(compressed_data_green);
    free(compressed_data_blue);
    
    if (rgb_data.red != NULL) {
        free_rgb_data(&rgb_data);
    }

    fclose(fp);
}


/** M A I N ************************************************************/
void test() {
    char filedir[100] = ".\\data\\";
    char rledir[100] = ".\\bin\\";
    char recdir[100] = ".\\rec\\";
    char bmpext[10] = ".bmp";
    char rleext[10] = ".rle";
    time_t start, end;

    for (int idx = 1; idx < 4; idx++)
    {
        start = 1000 * time(NULL);
        char filename[100] = "\0";
        strcpy(filename, filedir);
        char rlename[100] = "\0";
        strcpy(rlename, rledir);
        char recname[100] = "\0";
        strcpy(recname, recdir);
        char s[10] = "\0";
        _itoa(idx, s, 10);
        strcat(filename, s);
        strcat(filename, bmpext);
        strcat(rlename, s);
        strcat(rlename, rleext);
        strcat(recname, s);
        strcat(recname, bmpext);

        rle_encode(filename, rlename);
        rle_decode(rlename, recname);

    }
}


int main(int argc, char** argv)
{
    //codec one image
    Parms parms;

    /* simply an array of function pointers */
    void (*action[])(const char* filename, const char* filename2) = {
        rle_encode,
        rle_decode
    };

    /* get mode and filename from command line arguments;
     * prints the usage and exits if improper arguments are supplied */
    if (!get_parms(&parms, "ed", argc, argv)) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    /* call the function for the specified mode and pass
     * it the filename supplied at the command line */
    action[parms.mode](parms.filename, parms.filename2);

    return 0;

}