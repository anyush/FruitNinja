// WAPI1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WAPI1.h"
#include <math.h>
#include <time.h>
#include <iostream>
#include <fstream>

#define MAX_LOADSTRING 100
#define MAX_FRUIT_NUMBER 20
#define GRAVITY 0.5
#define COLOR_NUMBER 16
#define FPS 20
#define GAME_TIME 30
#define PROGRESS_BAR_HEIGHT 20
#define MOUSE_BUF_SIZE 3

typedef struct fruit
{
    bool active;
    double pos_x;
    double pos_y;
    int radius;
    double speed_v;
    double speed_h;
    int color_id;
} fruit_t;

typedef struct color
{
    int r;
    int g;
    int b;
} color_t;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
fruit_t fruits[MAX_FRUIT_NUMBER * 64];
color_t colors[COLOR_NUMBER];
int score = 0;
double time_since_start = 0;
POINT mouse_buf[MOUSE_BUF_SIZE];
int magic_val;
int mid_speed_v;
int board_num;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM                MyRegisterClassCOVER(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void act(HWND);
void new_game(HWND);
void update_mouse_buf(HWND);
int blend_colors(color_t, color_t, int);
void use_small_board(HWND, HMENU);
void use_medium_board(HWND, HMENU);
void use_big_board(HWND, HMENU);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.


    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WAPI1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    MyRegisterClassCOVER(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WAPI1));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+2);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WAPI1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

    return RegisterClassExW(&wcex);
}

ATOM MyRegisterClassCOVER(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WAPI1);
    wcex.lpszClassName = L"COVERWND";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   RECT rt = { 0, 0, 400, 300 + PROGRESS_BAR_HEIGHT };
   AdjustWindowRect(&rt, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, TRUE);

   int centerX = GetSystemMetrics(SM_CXSCREEN) / 2;
   int centerY = GetSystemMetrics(SM_CYSCREEN) / 2;

   HWND hWnd = CreateWindowEx(WS_EX_TOPMOST, szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
       centerX - (rt.right - rt.left) / 2, centerY - (rt.bottom - rt.top) / 2, rt.right - rt.left, rt.bottom - rt.top, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HCURSOR cursor = NULL;
    RECT rc;
    int centerX = GetSystemMetrics(SM_CXSCREEN) / 2;
    int centerY = GetSystemMetrics(SM_CYSCREEN) / 2;
    HMENU hMenu = GetMenu(hWnd);
    std::ifstream m_fl_i;
    std::ofstream m_fl_o;
    char buf[13];

    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case ID_NEWGAME:
                new_game(hWnd);
                break;
            case ID_BOARD_S:
                use_small_board(hWnd, hMenu);
                break;
            case ID_BOARD_M:
                use_medium_board(hWnd, hMenu);
                break;
            case ID_BOARD_B:
                use_big_board(hWnd, hMenu);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {      
            PAINTSTRUCT ps;
            RECT rc;
            GetClientRect(hWnd, &rc);
            HDC hdc = BeginPaint(hWnd, &ps);
            int transparency;

            if (time_since_start < GAME_TIME)
                transparency = 255;
            else
                transparency = 128;

            color_t white_color = { 255, 255, 255 };
            color_t black_color = { 0, 0, 0 };
            color_t green_color = { 0, 255, 0 };
            color_t trace_color = { 255, 255, 0 };
            color_t progress_bar_bkg_color = { 200, 200, 200 };
            color_t progress_bar_color = green_color;

            HPEN white_pen = CreatePen(PS_SOLID, 0, blend_colors(white_color, green_color, transparency));
            HPEN black_pen = CreatePen(PS_SOLID, 0, blend_colors(black_color, green_color, transparency));
            HPEN null_pen = CreatePen(PS_NULL, 0, 0);
            HPEN trace_pen = CreatePen(PS_SOLID, 2, blend_colors(trace_color, green_color, transparency));
            HPEN progress_bar_bkg_pen = CreatePen(BS_SOLID, 0, blend_colors(progress_bar_bkg_color, green_color, transparency));
            HPEN progress_bar_pen = CreatePen(BS_SOLID, 0, blend_colors(progress_bar_color, green_color, transparency));
            HPEN old_pen = (HPEN)SelectObject(hdc, white_pen);

            HBRUSH white_brush = CreateSolidBrush(blend_colors(white_color, green_color, transparency));
            HBRUSH black_brush = CreateSolidBrush(blend_colors(black_color, green_color, transparency));
            HBRUSH progress_bar_bkg_brush = CreateSolidBrush(blend_colors(progress_bar_bkg_color, green_color, transparency));
            HBRUSH progress_bar_brush = CreateSolidBrush(blend_colors(progress_bar_color, green_color, transparency));
            HBRUSH old_brush = (HBRUSH)SelectObject(hdc, white_brush);
            

            HBRUSH fruit_brushes[COLOR_NUMBER];
            for (int i = 0; i < COLOR_NUMBER; i++)
                fruit_brushes[i] = CreateSolidBrush(blend_colors(colors[i], green_color, transparency));

            // Board
            for (int i = 0; i < 12; i++)
            {
                for (int j = 0; j < 16; j++)
                {
                    SelectObject(hdc, black_pen);
                    SelectObject(hdc, black_brush);
                    Rectangle(hdc, (i % 2 + 2 * j) * 50, i * 50, (i % 2 + 2 * j + 1) * 50, (i + 1) * 50);
                    SelectObject(hdc, white_pen);
                    SelectObject(hdc, white_brush);
                    Rectangle(hdc, ((i + 1) % 2 + 2 * j) * 50, i * 50, ((i + 1) % 2 + 2 * j + 1) * 50, (i + 1) * 50);
                }
            }

            // Fruits
            SelectObject(hdc, null_pen);
            for (int i = 0; i < MAX_FRUIT_NUMBER * 64; i++)
            {
                if (fruits[i].active)
                {
                    SelectObject(hdc, fruit_brushes[fruits[i].color_id]);
                    Ellipse(hdc, fruits[i].pos_x - fruits[i].radius,
                        fruits[i].pos_y - fruits[i].radius,
                        fruits[i].pos_x + fruits[i].radius,
                        fruits[i].pos_y + fruits[i].radius);
                }

            }

            SelectObject(hdc, old_pen);
            SelectObject(hdc, old_brush);

            // Mouse trace
            SelectObject(hdc, trace_pen);
            MoveToEx(hdc, mouse_buf[0].x, mouse_buf[0].y, NULL);
            for (int i = 1; i < MOUSE_BUF_SIZE; i++)
                LineTo(hdc, mouse_buf[i].x, mouse_buf[i].y);

            SelectObject(hdc, old_pen);
            
            // Score
            WCHAR txt[5];
            _itow_s(score, txt, 10);
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 255, 0));
            HFONT f = CreateFontA(40, 20, 0, 0, 400, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, "FNT");
            HFONT old_f = (HFONT)SelectObject(hdc, f);
            DrawText(hdc, txt, -1, &rc, DT_TOP | DT_RIGHT);
            
            SelectObject(hdc, old_f);
            DeleteObject(f);

            // Progress bar
            SelectObject(hdc, progress_bar_bkg_pen);
            SelectObject(hdc, progress_bar_bkg_brush);
            Rectangle(hdc, rc.left, rc.bottom - PROGRESS_BAR_HEIGHT, rc.right, rc.bottom);
            SelectObject(hdc, progress_bar_pen);
            SelectObject(hdc, progress_bar_brush);
            Rectangle(hdc, rc.left, rc.bottom - PROGRESS_BAR_HEIGHT, (rc.right - rc.left) * time_since_start / GAME_TIME, rc.bottom);

            SelectObject(hdc, old_pen);
            SelectObject(hdc, old_brush);
            

            // Final screen
            if (time_since_start > GAME_TIME)
            {
                RECT rc_fin = rc;
                rc_fin.top = (rc.bottom - rc.top) / 2 - 20;
                rc_fin.bottom = (rc.bottom - rc.top) / 2 + 40;
                _itow_s(score, txt, 10);
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, RGB(255, 255, 255));
                f = CreateFontA(60, 30, 0, 0, 700, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, "FNT");
                old_f = (HFONT)SelectObject(hdc, f);
                DrawText(hdc, L"Score:", -1, &rc_fin, DT_CENTER);
                rc_fin.top += 50;
                rc_fin.bottom += 50;
                DrawText(hdc, txt, -1, &rc_fin, DT_CENTER);
                SelectObject(hdc, old_f);
                DeleteObject(f);
            }

            DeleteObject(white_pen);
            DeleteObject(white_brush);
            DeleteObject(black_pen);
            DeleteObject(black_brush);
            DeleteObject(trace_pen);
            DeleteObject(null_pen);
            DeleteObject(progress_bar_bkg_pen);
            DeleteObject(progress_bar_pen);
            DeleteObject(progress_bar_bkg_brush);
            DeleteObject(progress_bar_brush);
            for (int i = 0; i < COLOR_NUMBER; i++)
                DeleteObject(fruit_brushes[i]);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_CREATE:
        
        m_fl_i.open("FruitNinja.ini");
        if (m_fl_i)
        {
            m_fl_i >> buf;
            m_fl_i >> buf;

            switch ((int)buf[5] - '0')
            {
            case 1:
                use_medium_board(hWnd, hMenu);
                break;
            case 2:
                use_big_board(hWnd, hMenu);
                break;
            default:
                use_small_board(hWnd, hMenu);
                break;
            }
            m_fl_i.close();
        }
        else
            use_small_board(hWnd, hMenu);
        SetTimer(hWnd, 1, 100, NULL);
        srand(time(NULL));
        break;
    case WM_DESTROY:
        m_fl_o.open("FruitNinja.ini", std::ofstream::out | std::ofstream::trunc);
        m_fl_o << "[GAME]\nSIZE=" << board_num;
        m_fl_o.close();
        PostQuitMessage(0);
        break;
    case WM_MOUSEMOVE:
        cursor = LoadCursor(hInst, (LPWSTR)IDC_CURSOR1);
        SetTimer(hWnd, 8, 3 * 1000, NULL);
        SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);
        UpdateWindow(hWnd);
        return TRUE;
    case WM_NCMOUSEMOVE:
        cursor = LoadCursor(NULL, IDC_ARROW);
        SetTimer(hWnd, 8, 3 * 1000, NULL);
        SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);
        UpdateWindow(hWnd);
        return TRUE;
    case WM_SETCURSOR:
        SetCursor(cursor);
        return true;
    case WM_WINDOWPOSCHANGING:
    {
        ReleaseCapture();
        return 0;
    }
    case WM_TIMER:
        if (wParam == 8) {
            SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TOPMOST);
            SetLayeredWindowAttributes(hWnd, 0, (255 * 50) / 100, LWA_ALPHA);
            UpdateWindow(hWnd);
            break;
        }
        if (wParam == 3)
        {
            if (time_since_start <= GAME_TIME)
            {
                update_mouse_buf(hWnd);
                act(hWnd);
            }
            else
                KillTimer(hWnd, 3);
            InvalidateRect(hWnd, NULL, FALSE);    
            break;
        }
        if (wParam == 1 && time_since_start <= GAME_TIME)
        {
            time_since_start += 0.1;
            break;
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}



void act(HWND hWnd)
{
    RECT rc;
    GetClientRect(hWnd, &rc);

    for (int i = 0; i < MAX_FRUIT_NUMBER; i++)
    {
        bool spawn_fruit = true;
        for (int j = 0; j < 64; j++)
        {
            if (!fruits[i * 64 + j].active) continue;
            
            if (fruits[i * 64 + j].pos_x + fruits[i * 64 + j].radius < 0 ||
                fruits[i * 64 + j].pos_x - fruits[i * 64 + j].radius > rc.right ||
                fruits[i * 64 + j].pos_y + fruits[i * 64 + j].radius < 0 ||
                fruits[i * 64 + j].pos_y - fruits[i * 64 + j].radius > rc.bottom)
            {
                fruits[i * 64 + j].active = false;
                continue;
            }
            spawn_fruit = false;
            fruits[i * 64 + j].speed_v += GRAVITY;
            fruits[i * 64 + j].pos_x += fruits[i * 64 + j].speed_h;
            fruits[i * 64 + j].pos_y += fruits[i * 64 + j].speed_v;


            POINT p;
            GetCursorPos(&p);
            ScreenToClient(hWnd, &p);
            
            if (sqrt(pow(fruits[i * 64 + j].pos_x - p.x, 2) + pow(fruits[i * 64 + j].pos_y - p.y, 2)) < fruits[i * 64 + j].radius)
            {
                score += 1;
                int step = fruits[i * 64 + j].radius/4;

                for (int k = 3; k > -1; k--)
                {
                    fruits[i * 64 + j + k*step].active = true;
                    fruits[i * 64 + j + k*step].color_id = fruits[i * 64 + j].color_id;
                    fruits[i * 64 + j + k*step].pos_x = fruits[i * 64 + j].pos_x;
                    fruits[i * 64 + j + k*step].pos_y = fruits[i * 64 + j].pos_y;
                    fruits[i * 64 + j + k*step].radius = fruits[i * 64 + j].radius / 2;
                    fruits[i * 64 + j + k*step].speed_h = fruits[i * 64 + j].speed_h + (rand() % 100 - 50.0) / 10;
                    fruits[i * 64 + j + k*step].speed_v = fruits[i * 64 + j].speed_v + (rand() % 100 - 50.0) / 10;
                }
            }

        }
        
        if (spawn_fruit && rand()%magic_val == 0)
        {
            fruits[i * 64].active = true;
            fruits[i * 64].radius = 32;
            fruits[i * 64].pos_x = rand() % rc.right;
            fruits[i * 64].pos_y = rc.bottom;
            fruits[i * 64].color_id = rand()%COLOR_NUMBER;
            fruits[i * 64].speed_h = rand() % 10 - 5.0;
            fruits[i * 64].speed_v = -(rand() % 10 + mid_speed_v);
        }

    }
}

void new_game(HWND hWnd)
{
    POINT p;
    UpdateWindow(hWnd);
    score = 0;
    time_since_start = 0;
    for (int i = 0; i < 64 * MAX_FRUIT_NUMBER; i++)
        fruits[i].active = false;
    for (int i = 0; i < MOUSE_BUF_SIZE; i++)
    {
        GetCursorPos(&p);
        ScreenToClient(hWnd, &p);
        mouse_buf[i] = p;
    }
    for (int i = 0; i < COLOR_NUMBER; i++) {
        colors[i].r = rand() % 256;
        colors[i].g = rand() % 256;
        colors[i].b = rand() % 256;
    }
    SetTimer(hWnd, 3, 1000 / FPS, NULL);
}


void update_mouse_buf(HWND hWnd)
{
    POINT p;
    for (int i = 0; i < MOUSE_BUF_SIZE - 1; i++)
        mouse_buf[i] = mouse_buf[i + 1];
    GetCursorPos(&p);
    ScreenToClient(hWnd, &p);
    mouse_buf[MOUSE_BUF_SIZE - 1] = p;
}

int blend_colors(color_t c1, color_t c2, int tr)
{
    return RGB(c1.r*tr/255 + c2.r*(255-tr)/255, c1.g*tr/255 + c2.g*(255-tr)/255, c1.b*tr/255 + c2.b*(255-tr)/255);
}

void use_small_board(HWND hWnd, HMENU hMenu)
{
    int centerX = GetSystemMetrics(SM_CXSCREEN) / 2;
    int centerY = GetSystemMetrics(SM_CYSCREEN) / 2;
    CheckMenuItem(hMenu, ID_BOARD_S, MF_CHECKED);
    CheckMenuItem(hMenu, ID_BOARD_M, MF_UNCHECKED);
    CheckMenuItem(hMenu, ID_BOARD_B, MF_UNCHECKED);
    magic_val = 300;
    mid_speed_v = 10;
    RECT rc = { 0, 0, 400, 300 + PROGRESS_BAR_HEIGHT };
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, TRUE);
    SetWindowPos(hWnd, HWND_TOPMOST, centerX - (rc.right - rc.left) / 2, centerY - (rc.bottom - rc.top) / 2, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION);
    UpdateWindow(hWnd);
    new_game(hWnd);
    board_num = 0;
}

void use_medium_board(HWND hWnd, HMENU hMenu)
{
    int centerX = GetSystemMetrics(SM_CXSCREEN) / 2;
    int centerY = GetSystemMetrics(SM_CYSCREEN) / 2;
    CheckMenuItem(hMenu, ID_BOARD_S, MF_UNCHECKED);
    CheckMenuItem(hMenu, ID_BOARD_M, MF_CHECKED);
    CheckMenuItem(hMenu, ID_BOARD_B, MF_UNCHECKED);
    magic_val = 250;
    mid_speed_v = 15;
    RECT rc = { 0, 0, 600, 500 + PROGRESS_BAR_HEIGHT };
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, TRUE);
    SetWindowPos(hWnd, HWND_TOPMOST, centerX - (rc.right - rc.left) / 2, centerY - (rc.bottom - rc.top) / 2, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION);
    UpdateWindow(hWnd);
    new_game(hWnd);
    board_num = 1;
}

void use_big_board(HWND hWnd, HMENU hMenu)
{
    int centerX = GetSystemMetrics(SM_CXSCREEN) / 2;
    int centerY = GetSystemMetrics(SM_CYSCREEN) / 2;
    CheckMenuItem(hMenu, ID_BOARD_S, MF_UNCHECKED);
    CheckMenuItem(hMenu, ID_BOARD_M, MF_UNCHECKED);
    CheckMenuItem(hMenu, ID_BOARD_B, MF_CHECKED);
    magic_val = 170;
    mid_speed_v = 17;
    RECT rc = { 0, 0, 800, 600 + PROGRESS_BAR_HEIGHT };
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, TRUE);
    SetWindowPos(hWnd, HWND_TOPMOST, centerX - (rc.right - rc.left) / 2, centerY - (rc.bottom - rc.top) / 2, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION);
    UpdateWindow(hWnd);
    new_game(hWnd);
    board_num = 2;
}