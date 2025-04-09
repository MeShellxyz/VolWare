#include "WindowsTray.h"
#include "resources.h"
#include <iostream>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001

WindowsTray::WindowsTray(HINSTANCE hInstance, const std::string &tooltip) {
    // Register window class
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "VolWareTrayClass";

    RegisterClassEx(&wc);

    // Create hidden window
    hwnd = CreateWindowEx(0, "VolWareTrayClass", "VolWare", 0, 0, 0, 0, 0,
                          HWND_MESSAGE, NULL, hInstance, NULL);

    // Set the window's user data to point to this object
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

    // Setup tray icon
    nid = {0};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;

    // Load icon - use a small app icon or a default Windows icon
    nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAYICON));

    // Set tooltip
    strncpy_s(nid.szTip, tooltip.c_str(), sizeof(nid.szTip) - 1);

    // Add the icon to the tray
    Shell_NotifyIcon(NIM_ADD, &nid);
}

WindowsTray::~WindowsTray() {
    if (nid.hIcon != NULL) {
        DestroyIcon(nid.hIcon);
        nid.hIcon = NULL;
    }
    // Remove icon from tray
    Shell_NotifyIcon(NIM_DELETE, &nid);

    // Destroy window
    DestroyWindow(hwnd);
}

void WindowsTray::setOnExitCallback(std::function<void()> callback) {
    onExitCallback = callback;
}

UINT WindowsTray::addMenuItem(const std::string &itemName,
                              std::function<void()> callback) {
    UINT menuId = ++nextMenuId;
    menuCallbacks[menuId] = callback;
    return menuId;
}

LRESULT CALLBACK WindowsTray::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                         LPARAM lParam) {
    WindowsTray *tray = (WindowsTray *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (uMsg) {
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) {
            // Show context menu
            POINT pt;
            GetCursorPos(&pt);

            HMENU hMenu = CreatePopupMenu();

            AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, "Quit");

            SetForegroundWindow(hwnd);
            TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_RIGHTALIGN, pt.x, pt.y,
                           0, hwnd, NULL);
            DestroyMenu(hMenu);
        }
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_TRAY_EXIT && tray) {
            if (tray->onExitCallback) {
                tray->onExitCallback();
            }
        } else {
            // Call the corresponding menu callback
            auto it = tray->menuCallbacks.find(LOWORD(wParam));
            if (it != tray->menuCallbacks.end()) {
                it->second();
            }
        }
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}