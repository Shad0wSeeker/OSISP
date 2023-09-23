#include <windows.h>
#include <tchar.h>
#include <vector>
#include <algorithm>

struct Shape {
    int startX, startY, endX, endY;
    bool isEllipse;

    Shape(int x1, int y1, int x2, int y2, bool ellipse)
        : startX(x1), startY(y1), endX(x2), endY(y2), isEllipse(ellipse) {}
};

std::vector<Shape> shapes; // Вектор для хранения фигур
int selectedShapeIndex = -1; // Индекс выбранной фигуры

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

HBRUSH yellowBrush = NULL;
HPEN yellowPen = NULL;
HBRUSH greenBrush = NULL;
HPEN greenPen = NULL;
bool isEllipse = false;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    const wchar_t* className = L"MyWindowClass";
    WNDCLASSEXW wc = { 0 };

    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = className;

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HWND hwnd = CreateWindowExW(0, className, L"Графический редактор", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBoxW(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG Msg;
    while (GetMessageW(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessageW(&Msg);
    }
    return Msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    HDC hdc;
    PAINTSTRUCT ps;
    static int startX, startY, endX, endY;
    static bool isDrawing = false;

    switch (msg) {
    case WM_LBUTTONDOWN:
        startX = LOWORD(lParam);
        startY = HIWORD(lParam);
        isDrawing = true;
        break;

    case WM_LBUTTONUP:
        endX = LOWORD(lParam);
        endY = HIWORD(lParam);
        isDrawing = false;

        // Определяем, какую фигуру рисовать
        isEllipse = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;

        // Сохраняем фигуру в векторе
        shapes.push_back(Shape(startX, startY, endX, endY, isEllipse));

        // Вызываем функцию для перерисовки окна
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case WM_MOUSEMOVE:
        if (isDrawing) {
            // Обновляем координаты конечной точки при движении мыши
            endX = LOWORD(lParam);
            endY = HIWORD(lParam);

            // Перерисовываем окно в режиме реального времени
            hdc = GetDC(hwnd);

            // Определяем, какую фигуру рисовать
            isEllipse = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;

            if (isEllipse) {
                // Рассчитываем радиус как половину минимальной стороны
                int radius = min(abs(endX - startX), abs(endY - startY)) / 2;
                int centerX, centerY;

                if (endX > startX) {
                    centerX = startX + radius;
                }
                else {
                    centerX = startX - radius;
                }

                if (endY > startY) {
                    centerY = startY + radius;
                }
                else {
                    centerY = startY - radius;
                }

                Ellipse(hdc, centerX - radius, centerY - radius, centerX + radius, centerY + radius);
            }
            else {
                // Рассчитываем размеры квадрата на основе координат startX, startY, endX, endY
                
                          
                int size = min(abs(endX - startX), abs(endY - startY));
                int left, top, right, bottom;

                if (endX > startX) {
                    left = startX;
                    right = left + size;
                }
                else {
                    right = startX;
                    left = right - size;
                }

                if (endY > startY) {
                    top = startY;
                    bottom = top + size;
                }
                else {
                    bottom = startY;
                    top = bottom - size;
                }

                Rectangle(hdc, left, top, right, bottom);
            }

            ReleaseDC(hwnd, hdc);
        }

        selectedShapeIndex = -1;
        for (int i = static_cast<int>(shapes.size()) - 1; i >= 0; --i) {
            const Shape& shape = shapes[i];
            if (endX >= shape.startX && endX <= shape.endX && endY >= shape.startY && endY <= shape.endY) {
                // Мышь находится над фигурой
                selectedShapeIndex = i;
                break;
            }
        }
        break;


    case WM_RBUTTONDOWN:
        // Удаляем фигуру, над которой находится мышь
        if (selectedShapeIndex != -1 && selectedShapeIndex < static_cast<int>(shapes.size())) {
            shapes.erase(shapes.begin() + selectedShapeIndex);
            selectedShapeIndex = -1; // Очищаем выбранную фигуру
            // Вызываем функцию для перерисовки окна
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;

    case WM_PAINT:
        // Перерисовываем окно
        hdc = BeginPaint(hwnd, &ps);

        // Отрисовываем все сохраненные фигуры
        for (const Shape& shape : shapes) {
            if (shape.isEllipse) {
                // Если фигура - круг, то устанавливаем желтую кисть и контур
                yellowBrush = CreateSolidBrush(RGB(255, 255, 0)); // Желтая кисть
                yellowPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 0)); // Желтый контур
                SelectObject(hdc, yellowBrush);
                SelectObject(hdc, yellowPen);

                // Рассчитываем радиус как половину минимальной стороны
                int radius = min(abs(shape.endX - shape.startX), abs(shape.endY - shape.startY)) / 2;
                int centerX = (shape.startX + shape.endX) / 2;
                int centerY = (shape.startY + shape.endY) / 2;

                // Рисуем круг
                Ellipse(hdc, centerX - radius, centerY - radius, centerX + radius, centerY + radius);

                DeleteObject(yellowBrush);
                DeleteObject(yellowPen);
            }
            else {
                // Если фигура - квадрат, то устанавливаем зеленую кисть и контур
                greenBrush = CreateSolidBrush(RGB(0, 255, 0)); // Зеленая кисть
                greenPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0)); // Зеленый контур
                SelectObject(hdc, greenBrush);
                SelectObject(hdc, greenPen);

                // Рисуем квадрат вместо прямоугольника
                int size = min(abs(shape.endX - shape.startX), abs(shape.endY - shape.startY));
                int left, top, right, bottom;

                if (shape.endX > shape.startX) {
                    left = shape.startX;
                    right = left + size;
                }
                else {
                    right = shape.startX;
                    left = right - size;
                }

                if (shape.endY > shape.startY) {
                    top = shape.startY;
                    bottom = top + size;
                }
                else {
                    bottom = shape.startY;
                    top = bottom - size;
                }

                Rectangle(hdc, left, top, right, bottom);
                DeleteObject(greenBrush);
                DeleteObject(greenPen);
            }
        }

        EndPaint(hwnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}
