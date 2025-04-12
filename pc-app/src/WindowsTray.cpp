#include "WindowsTray.h"
#include "resources.h"
#include <iostream>

// Custom window message for tray icon interaction
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
    m_hwnd = CreateWindowEx(0,                  // Extended style
                            "VolWareTrayClass", // Class name
                            "VolWare",          // Window title
                            0,                  // Style
                            0, 0, 0, 0,   // Position and size (not visible)
                            HWND_MESSAGE, // Message-only window
                            NULL,         // Menu
                            hInstance,    // Instance
                            NULL          // Additional data
    );

    // Store this object pointer with the window for callbacks
    SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);

    // Initialize notification icon data
    m_nid = {0};
    m_nid.cbSize = sizeof(NOTIFYICONDATA);
    m_nid.hWnd = m_hwnd;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYICON;

    // Load tray icon
    m_nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAYICON));

    // Set tooltip text
    strncpy_s(m_nid.szTip, tooltip.c_str(), sizeof(m_nid.szTip) - 1);

    // Add the icon to the system tray
    Shell_NotifyIcon(NIM_ADD, &m_nid);
}

WindowsTray::~WindowsTray() {
    // Clean up icon resource
    if (m_nid.hIcon != NULL) {
        DestroyIcon(m_nid.hIcon);
        m_nid.hIcon = NULL;
    }

    // Remove icon from system tray
    Shell_NotifyIcon(NIM_DELETE, &m_nid);

    // Destroy the window
    DestroyWindow(m_hwnd);
}

void WindowsTray::setOnExitCallback(std::function<void()> callback) {
    m_onExitCallback = std::move(callback);
}

UINT WindowsTray::addMenuItem(const std::string &itemName,
                              std::function<void()> callback) {
    UINT menuId = ++m_nextMenuId;
    m_menuCallbacks[menuId] = std::move(callback);
    return menuId;
}

LRESULT CALLBACK WindowsTray::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                         LPARAM lParam) {
    // Retrieve the WindowsTray instance associated with this window
    WindowsTray *tray = (WindowsTray *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (uMsg) {
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) {
            // Show context menu on right-click
            POINT pt;
            GetCursorPos(&pt);

            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, "Quit");

            // Set foreground window to ensure menu works correctly
            SetForegroundWindow(hwnd);

            // Display context menu
            TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_RIGHTALIGN, pt.x, pt.y,
                           0, hwnd, NULL);

            DestroyMenu(hMenu);
        }
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_TRAY_EXIT && tray) {
            // Handle exit menu item
            if (tray->m_onExitCallback) {
                tray->m_onExitCallback();
            }
        } else {
            // Handle custom menu items
            auto it = tray->m_menuCallbacks.find(LOWORD(wParam));
            if (it != tray->m_menuCallbacks.end()) {
                it->second();
            }
        }
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}