/*
 * 只用*字符，自动适配宽高比例+灰度阈值
 * 支持0/1或0~9像素矩阵
 * 编译：gcc -o logo_star logo_star.c
 * 运行：./logo_star matrix.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h> // 关键：用于ShellExecute

/* 可调参数 */
#define HORIZONTAL_SCALE 2   // 横向拉宽倍数（2或3，推荐2）
#define THRESHOLD '5'        // 阈值，像素值>=THRESHOLD才输出*

// 灰度字符集（从黑到白，顺序需与.txt一致）
const char* GRAYSCALE = "#@&B5J!~?7YPG ";
// 阈值索引（如5，表示比第5个字符“更黑”的都输出*）
#define GRAYSCALE_THRESHOLD 5

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
    int gs_len = (int)strlen(GRAYSCALE);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            char c = matrix[i][j];
            // 查找c在灰度字符集中的索引
            int idx = gs_len - 1; // 默认为最白
            for (int k = 0; k < gs_len; ++k) {
                if (c == GRAYSCALE[k]) {
                    idx = k;
                    break;
                }
            }
            if (idx > GRAYSCALE_THRESHOLD) { // 反色：索引大于阈值输出*
                for (int h = 0; h < HORIZONTAL_SCALE; ++h)
                    putchar('*');
            } else {
                for (int h = 0; h < HORIZONTAL_SCALE; ++h)
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
        MessageBoxA(NULL, "未找到matrix.txt或文件格式错误！\n请将matrix.txt放在本程序同目录下。", "错误", MB_OK | MB_ICONERROR);
        return 1;
    }
    // 输出到logo.txt
    FILE* fp = fopen("logo.txt", "w");
    if (!fp) {
        MessageBoxA(NULL, "无法创建logo.txt文件！", "错误", MB_OK | MB_ICONERROR);
        free_matrix(matrix, rows);
        return 1;
    }
    int gs_len = (int)strlen(GRAYSCALE);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            char c = matrix[i][j];
            int idx = gs_len - 1;
            for (int k = 0; k < gs_len; ++k) {
                if (c == GRAYSCALE[k]) {
                    idx = k;
                    break;
                }
            }
            if (idx > GRAYSCALE_THRESHOLD) {
                for (int h = 0; h < HORIZONTAL_SCALE; ++h)
                    fputc('*', fp);
            } else {
                for (int h = 0; h < HORIZONTAL_SCALE; ++h)
                    fputc(' ', fp);
            }
        }
        fputc('\n', fp);
    }
    fclose(fp);
    free_matrix(matrix, rows);

    // 自动用记事本打开logo.txt
    ShellExecuteA(NULL, "open", "notepad.exe", "logo.txt", NULL, SW_SHOWNORMAL);
    return 0;
}
