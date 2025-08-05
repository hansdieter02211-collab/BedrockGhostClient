#include <windows.h>
#include <tlhelp32.h>
#include <commdlg.h>
#include <string>
#include <iostream>

HWND hProcessNameEdit, hInjectButton, hSelectDllButton;
std::wstring selectedDllPath = L"";

// Hilfsfunktion: Prozess-ID holen
DWORD GetProcId(const std::wstring& procName) {
    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (!snapshot) return 0;

    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (procName == entry.szExeFile) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return 0;
}

// DLL Inject-Funktion
bool InjectDLL(DWORD procId, const std::wstring& dllPath) {
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);
    if (!hProc) return false;

    void* allocMem = VirtualAllocEx(hProc, nullptr, dllPath.size() * sizeof(wchar_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!allocMem) return false;

    WriteProcessMemory(hProc, allocMem, dllPath.c_str(), dllPath.size() * sizeof(wchar_t), nullptr);
    HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    FARPROC loadLibAddr = GetProcAddress(hKernel32, "LoadLibraryW");

    HANDLE hThread = CreateRemoteThread(hProc, nullptr, 0, (LPTHREAD_START_ROUTINE)loadLibAddr, allocMem, 0, nullptr);
    if (!hThread) return false;

    WaitForSingleObject(hThread, INFINITE);
    VirtualFreeEx(hProc, allocMem, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProc);
    return true;
}

// Datei-Auswahl-Dialog
std::wstring OpenDllDialog(HWND hwnd) {
    wchar_t filePath[MAX_PATH] = L"";

    OPENFILENAMEW ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"DLL Files\0*.dll\0All Files\0*.*\0";
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn)) {
        return std::wstring(filePath);
    }
    return L"";
}

// Fenster-Prozedur
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND:
        if ((HWND)lParam == hSelectDllButton) {
            selectedDllPath = OpenDllDialog(hwnd);
            MessageBoxW(hwnd, selectedDllPath.c_str(), L"Ausgewählte DLL", MB_OK);
        }
        else if ((HWND)lParam == hInjectButton) {
            wchar_t procName[256];
            GetWindowTextW(hProcessNameEdit, procName, 256);
            DWORD pid = GetProcId(procName);

            if (!pid) {
                MessageBoxW(hwnd, L"Prozess nicht gefunden!", L"Fehler", MB_ICONERROR);
                return 0;
            }

            if (selectedDllPath.empty()) {
                MessageBoxW(hwnd, L"Bitte zuerst eine DLL auswählen!", L"Fehler", MB_ICONERROR);
                return 0;
            }

            if (InjectDLL(pid, selectedDllPath)) {
                MessageBoxW(hwnd, L"Injection erfolgreich!", L"Erfolg", MB_OK);
            }
            else {
                MessageBoxW(hwnd, L"Injection fehlgeschlagen!", L"Fehler", MB_ICONERROR);
            }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// WinMain: Einstiegspunkt
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"MyInjectorWindow";

    WNDCLASSW wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"Simple DLL Injector",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 200,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) return 0;

    // UI Elemente erstellen
    CreateWindowW(L"STATIC", L"Prozessname (z.B. Minecraft.Windows.exe):", WS_VISIBLE | WS_CHILD,
        20, 20, 360, 20, hwnd, nullptr, hInstance, nullptr);

    hProcessNameEdit = CreateWindowW(L"EDIT", L"Minecraft.Windows.exe", WS_VISIBLE | WS_CHILD | WS_BORDER,
        20, 45, 340, 23, hwnd, nullptr, hInstance, nullptr);

    hSelectDllButton = CreateWindowW(L"BUTTON", L"📁 DLL auswählen", WS_VISIBLE | WS_CHILD,
        20, 80, 150, 30, hwnd, nullptr, hInstance, nullptr);

    hInjectButton = CreateWindowW(L"BUTTON", L"🚀 Inject", WS_VISIBLE | WS_CHILD,
        210, 80, 150, 30, hwnd, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, nCmdShow);

    // Message Loop
    MSG msg = { };
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}