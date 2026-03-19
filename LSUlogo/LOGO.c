/*
 * 升级版：多级灰度字符画Logo生成器
 * 支持0~9灰度像素，自动映射字符
 * 编译：gcc -o logo_ascii logo_ascii.c
 * 运行：./logo_ascii matrix.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 可自定义的灰度字符集（从黑到白） */
#define CHARSET "@#*+=-:. "

// 读取像素矩阵文件，返回二维动态数组，行列数通过指针返回
char** read_pixel_matrix(const char* filename, int* rows, int* cols) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("错误：无法打开文件 %s\n", filename);
        return NULL;
    }
    int max_lines = 1024;
    char** matrix = (char**)malloc(max_lines * sizeof(char*));
    char line[1024];
    int row = 0, col = -1;
    while (fgets(line, sizeof(line), fp)) {
        int len = (int)strcspn(line, "\r\n");
        line[len] = '\0';
        if (col == -1) col = len;
        else if (len != col) {
            printf("错误：第%d行长度与首行不一致（%d != %d）\n", row+1, len, col);
            for (int i = 0; i < row; ++i) free(matrix[i]);
            free(matrix);
            fclose(fp);
            return NULL;
        }
        matrix[row] = (char*)malloc((col+1) * sizeof(char));
        strcpy(matrix[row], line);
        row++;
        if (row >= max_lines) {
            printf("错误：像素矩阵行数超出最大支持（%d）\n", max_lines);
            for (int i = 0; i < row; ++i) free(matrix[i]);
            free(matrix);
            fclose(fp);
            return NULL;
        }
    }
    fclose(fp);
    *rows = row;
    *cols = col;
    return matrix;
}

// 推荐灰度字符集（从黑到白，字符越多越细腻）
#define CHARSET "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. "

// 输出字符画（灰度映射，横向拉宽）
void print_ascii_logo(char** matrix, int rows, int cols) {
    int charset_len = (int)strlen(CHARSET);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            char c = matrix[i][j];
            if (c >= '0' && c <= '9') {
                int idx = (c - '0') * (charset_len - 1) / 9;
                putchar(CHARSET[idx]);
                putchar(CHARSET[idx]); // 横向拉宽
            } else {
                putchar(' ');
                putchar(' ');
            }
        }
        putchar('\n');
    }
}

// 释放二维数组
void free_matrix(char** matrix, int rows) {
    for (int i = 0; i < rows; ++i)
        free(matrix[i]);
    free(matrix);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("用法：%s <像素矩阵文本文件>\n", argv[0]);
        return 1;
    }
    int rows, cols;
    char** matrix = read_pixel_matrix(argv[1], &rows, &cols);
    if (!matrix) {
        printf("读取像素矩阵失败，请检查文件格式！\n");
        return 1;
    }
    print_ascii_logo(matrix, rows, cols);
    free_matrix(matrix, rows);
    return 0;
}
