/*
 * 只用*字符，自动适配宽高比例+灰度阈值
 * 支持0/1或0~9像素矩阵
 * 编译：gcc -o logo_star logo_star.c
 * 运行：./logo_star matrix.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 可调参数 */
#define HORIZONTAL_SCALE 2   // 横向拉宽倍数（2或3，推荐2）
#define THRESHOLD '6'        // 阈值，像素值>=THRESHOLD才输出*

// 读取像素矩阵文件
char** read_pixel_matrix(const char* filename, int* rows, int* cols) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("错误：无法打开文件 %s\n", filename);
        return NULL;
    }
    int max_lines = 1024;
    char** matrix = (char**)malloc(max_lines * sizeof(char*));
    char line[2048];
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

// 只用*输出logo，自动拉宽，支持阈值
void print_star_logo(char** matrix, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            char c = matrix[i][j];
            if (c >= THRESHOLD && c <= '9') {
                for (int k = 0; k < HORIZONTAL_SCALE; ++k)
                    putchar('*');
            } else {
                for (int k = 0; k < HORIZONTAL_SCALE; ++k)
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
    print_star_logo(matrix, rows, cols);
    free_matrix(matrix, rows);
    return 0;
}
