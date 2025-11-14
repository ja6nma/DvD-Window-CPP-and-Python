//DvD Window (3)
#include <Windows.h>
#include <vector>
#include <set>
#include <thread>
#include <chrono>

class WindowManager {
private:
    std::vector<HWND> windows;
    std::vector<POINT> originalPositions;
    std::vector<SIZE> originalSizes;
    std::vector<POINT> currentPositions;
    std::vector<POINT> directions;
    std::set<HWND> processedWindows;
    bool running;
    int screenW, screenH;

    CRITICAL_SECTION cs;

public:
    WindowManager() : running(true) {
        InitializeCriticalSection(&cs);
        screenW = GetSystemMetrics(SM_CXSCREEN);
        screenH = GetSystemMetrics(SM_CYSCREEN);
    }

    ~WindowManager() {
        DeleteCriticalSection(&cs);
    }

    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
        WindowManager* manager = reinterpret_cast<WindowManager*>(lParam);
        return manager->AddWindow(hwnd);
    }

    BOOL AddWindow(HWND hwnd) {
        EnterCriticalSection(&cs);

        if (processedWindows.find(hwnd) != processedWindows.end()) {
            LeaveCriticalSection(&cs);
            return TRUE;
        }

        if (IsWindowVisible(hwnd) && GetParent(hwnd) == NULL) {
            if (!IsSystemWindow(hwnd)) {
                windows.push_back(hwnd);

                RECT rect;
                GetWindowRect(hwnd, &rect);
                originalPositions.push_back({ rect.left, rect.top });
                originalSizes.push_back({ rect.right - rect.left, rect.bottom - rect.top });
                currentPositions.push_back({ rect.left, rect.top });

                POINT dir;
                dir.x = 3 + (rand() % 5);
                dir.y = 3 + (rand() % 5);

                if (rand() % 2) dir.x = -dir.x;
                if (rand() % 2) dir.y = -dir.y;

                directions.push_back(dir);

                processedWindows.insert(hwnd);
            }
        }

        LeaveCriticalSection(&cs);
        return TRUE;
    }

    bool IsSystemWindow(HWND hwnd) {
        TCHAR className[256];
        GetClassName(hwnd, className, 256);

        if (lstrcmp(className, TEXT("Progman")) == 0 ||
            lstrcmp(className, TEXT("Shell_TrayWnd")) == 0 ||
            lstrcmp(className, TEXT("Button")) == 0) {
            return true;
        }

        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        if (!(style & WS_OVERLAPPEDWINDOW) && !(style & WS_POPUP)) {
            return true;
        }

        return false;
    }

    void DiscoverExistingWindows() {
        EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(this));
    }

    void DiscoverNewWindows() {
        std::thread([this]() {
            while (running) {
                EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(this));
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            }).detach();
    }

    void MoveWindows() {
        while (running) {
            EnterCriticalSection(&cs);

            for (int i = windows.size() - 1; i >= 0; i--) {
                if (!IsWindow(windows[i])) {
                    windows.erase(windows.begin() + i);
                    originalPositions.erase(originalPositions.begin() + i);
                    originalSizes.erase(originalSizes.begin() + i);
                    currentPositions.erase(currentPositions.begin() + i);
                    directions.erase(directions.begin() + i);
                }
            }

            for (size_t i = 0; i < windows.size(); i++) {
                if (!IsWindowVisible(windows[i]) || GetParent(windows[i]) != NULL) {
                    continue;
                }

                currentPositions[i].x += directions[i].x;
                currentPositions[i].y += directions[i].y;

                if (currentPositions[i].x <= 0) {
                    currentPositions[i].x = 0;
                    directions[i].x = -directions[i].x;
                }
                else if (currentPositions[i].x + originalSizes[i].cx >= screenW) {
                    currentPositions[i].x = screenW - originalSizes[i].cx;
                    directions[i].x = -directions[i].x;
                }

                if (currentPositions[i].y <= 0) {
                    currentPositions[i].y = 0;
                    directions[i].y = -directions[i].y;
                }
                else if (currentPositions[i].y + originalSizes[i].cy >= screenH) {
                    currentPositions[i].y = screenH - originalSizes[i].cy;
                    directions[i].y = -directions[i].y;
                }

                SetWindowPos(windows[i], 0,
                    currentPositions[i].x,
                    currentPositions[i].y,
                    originalSizes[i].cx,
                    originalSizes[i].cy,
                    SWP_NOZORDER | SWP_NOACTIVATE);
            }

            LeaveCriticalSection(&cs);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    void Stop() {
        running = false;
    }
};

WindowManager* g_manager = nullptr;

BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        if (g_manager) {
            g_manager->Stop();
        }
        return TRUE;
    }
    return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WindowManager manager;
    g_manager = &manager;

    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    srand(static_cast<unsigned int>(GetTickCount()));

    manager.DiscoverExistingWindows();
    manager.DiscoverNewWindows();
    manager.MoveWindows();

    return 0;
}
