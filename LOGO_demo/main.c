/*
 * 只用*字符，自动适配宽高比例+灰度阈值
 * 支持0/1或0~9像素矩阵
 * 编译：gcc -o logo_star logo_star.c
 * 运行：./logo_star matrix.txt
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 可调参数 */
#define HORIZONTAL_SCALE 2   // 横向拉宽倍数（2或3，推荐2）
#define THRESHOLD '5'        // 阈值，像素值>=THRESHOLD才输出*

// 灰度字符集（从黑到白，顺序需与.txt一致）
const char* GRAYSCALE = "#@&B5J!~?7YPG ";
// 阈值索引（如5，表示比第5个字符“更黑”的都输出*）
#define GRAYSCALE_THRESHOLD 5

// 全局变量
static char* logo = NULL;
static int total_lines = 0, line_len = 0;
#define WIN_MARGIN 40 // 窗口边距
static int font_height = 12;
static int font_width = 6;
#define FONT_HEIGHT_MIN 6
#define FONT_HEIGHT_MAX 48

// 读取像素矩阵文件，返回动态分配的logo字符串
char* load_logo(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) return NULL;
    char** lines = (char**)malloc(2048 * sizeof(char*));
    int row = 0, col = -1;
    char line[2048];
    while (fgets(line, sizeof(line), fp)) {
        int len = (int)strcspn(line, "\r\n");
        line[len] = '\0';
        if (col == -1) col = len;
        lines[row] = (char*)malloc((col+1) * sizeof(char));
        strcpy(lines[row], line);
        row++;
    }
    fclose(fp);
    total_lines = row;
    line_len = col;

    // 生成logo字符串
    int gs_len = (int)strlen(GRAYSCALE);
    int bufsize = (col * HORIZONTAL_SCALE + 1) * row; // 每行末尾加\0
    char* logo_data = (char*)malloc(bufsize);
    int pos = 0;
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            char c = lines[i][j];
            int idx = gs_len - 1;
            for (int k = 0; k < gs_len; ++k) {
                if (c == GRAYSCALE[k]) { idx = k; break; }
            }
            char outch = (idx > GRAYSCALE_THRESHOLD) ? '*' : ' ';
            for (int h = 0; h < HORIZONTAL_SCALE; ++h)
                logo_data[pos++] = outch;
        }
        logo_data[pos++] = '\0'; // 每行结尾用\0分隔
        free(lines[i]);
    }
    free(lines);
    return logo_data;
}

// Win32窗口过程
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        // WinMain中已加载logo
        break;
    case WM_MOUSEWHEEL: {
        if (GetKeyState(VK_CONTROL) & 0x8000) { // Ctrl按下
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            double scale = 1.1; // 每次缩放10%
            int old_height = font_height;
            
            if (delta > 0) {
                font_height = (int)(font_height * scale + 0.5);
            } else if (delta < 0) {
                font_height = (int)(font_height / scale + 0.5);
            }

            // 限制范围
            if (font_height < FONT_HEIGHT_MIN) font_height = FONT_HEIGHT_MIN;
            if (font_height > FONT_HEIGHT_MAX) font_height = FONT_HEIGHT_MAX;
            font_width = font_height / 2; // 保持比例
            if (font_width < 1) font_width = 1;
            
            if (font_height != old_height) {
                // 重新计算并设置窗口大小
                int client_width = line_len * HORIZONTAL_SCALE * font_width;
                int client_height = total_lines * font_height;
                RECT rect = {0, 0, client_width, client_height};
                DWORD style = GetWindowLong(hwnd, GWL_STYLE);
                AdjustWindowRectEx(&rect, style, FALSE, GetWindowLong(hwnd, GWL_EXSTYLE));
                SetWindowPos(hwnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
                InvalidateRect(hwnd, NULL, TRUE); // 强制重绘
            }
        }
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        
        // 设置字体
        HFONT hFont = CreateFont(
            font_height, font_width, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH, "Consolas"
        );
        HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
        
        // 设置背景为白色
        SetBkMode(hdc, OPAQUE);
        SetBkColor(hdc, RGB(255, 255, 255));

        RECT clientRect;
        GetClientRect(hwnd, &clientRect);

        // 计算整个logo作为一个块的尺寸
        int logo_pixel_width = line_len * HORIZONTAL_SCALE * font_width;
        int logo_pixel_height = total_lines * font_height;

        // 计算左上角起始坐标，使其在窗口中居中
        int start_x = (clientRect.right - logo_pixel_width) / 2;
        int start_y = (clientRect.bottom - logo_pixel_height) / 2;
        
        // 确保坐标不为负（当窗口比logo还小时）
        if (start_x < 0) start_x = 0;
        if (start_y < 0) start_y = 0;

        // 逐行使用TextOut绘制，精确定位
        for (int i = 0; i < total_lines; ++i) {
            char* lineText = logo + i * (line_len * HORIZONTAL_SCALE + 1);
            int current_y = start_y + i * font_height;
            TextOut(hdc, start_x, current_y, lineText, (int)strlen(lineText));
        }

        SelectObject(hdc, oldFont);
        DeleteObject(hFont);
        EndPaint(hwnd, &ps);
        break;
    }
    case WM_DESTROY:
        if (logo) free(logo);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// WinMain入口
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    logo = load_logo("matrix.txt");
    if (!logo) {
        MessageBox(NULL, "未找到matrix.txt或文件格式错误！", "错误", MB_OK | MB_ICONERROR);
        return 1;
    }

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "LogoWin";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    RegisterClass(&wc);

    // 初始窗口大小适配logo
    int client_width = line_len * HORIZONTAL_SCALE * font_width;
    int client_height = total_lines * font_height;
    RECT rect = {0, 0, client_width, client_height};
    DWORD style = WS_OVERLAPPEDWINDOW;
    AdjustWindowRectEx(&rect, style, FALSE, 0);
    
    HWND hwnd = CreateWindow("LogoWin", "Logo ASCII Art", style,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

