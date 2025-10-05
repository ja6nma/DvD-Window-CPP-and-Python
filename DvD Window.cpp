#include <Windows.h>
#include <iostream>
#include <vector>

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    std::vector<HWND>* windows = reinterpret_cast<std::vector<HWND>*>(lParam);

    if (IsWindowVisible(hwnd) && GetParent(hwnd) == NULL) {
        windows->push_back(hwnd);
    }

    return TRUE;
}

int main() {
    std::vector<HWND> windows;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));

    std::vector<POINT> originalPositions;
    std::vector<SIZE> originalSizes;

    for (HWND hwnd : windows) {
        RECT rect;
        GetWindowRect(hwnd, &rect);
        originalPositions.push_back({ rect.left, rect.top });
        originalSizes.push_back({ rect.right - rect.left, rect.bottom - rect.top });
    }

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    std::vector<POINT> currentPositions;
    std::vector<POINT> directions;


    for (size_t i = 0; i < windows.size(); i++) {
        currentPositions.push_back(originalPositions[i]);


        POINT dir;
        dir.x = (rand() % 5) + 2;  
        dir.y = (rand() % 5) + 2;
        if (rand() % 2) dir.x = -dir.x;
        if (rand() % 2) dir.y = -dir.y;

        directions.push_back(dir);
    }


    while (true) {
        for (size_t i = 0; i < windows.size(); i++) {

            currentPositions[i].x += directions[i].x;
            currentPositions[i].y += directions[i].y;


            if (currentPositions[i].x <= 0 || currentPositions[i].x + originalSizes[i].cx >= screenW)
                directions[i].x = -directions[i].x;
            if (currentPositions[i].y <= 0 || currentPositions[i].y + originalSizes[i].cy >= screenH)
                directions[i].y = -directions[i].y;


            SetWindowPos(windows[i], 0,
                currentPositions[i].x,
                currentPositions[i].y,
                originalSizes[i].cx,
                originalSizes[i].cy,
                SWP_NOZORDER);
        }
        Sleep(10); 
    }

    return 0;
}