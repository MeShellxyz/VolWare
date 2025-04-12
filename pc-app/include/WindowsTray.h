#pragma once

#include <windows.h>

#include <functional>
#include <map>
#include <shellapi.h>
#include <string>

/**
 * WindowsTray - System tray icon manager for Windows
 *
 * Provides functionality to create and manage a system tray icon with
 * a context menu that appears when the user right-clicks the icon.
 */
class WindowsTray {
public:
    WindowsTray(HINSTANCE hInstance, const std::string &tooltip);
    ~WindowsTray();

    // Callback management
    void setOnExitCallback(std::function<void()> callback);
    UINT addMenuItem(const std::string &itemName,
                     std::function<void()> callback);

    // Window procedure
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                       LPARAM lParam);

private:
    // UI elements
    NOTIFYICONDATA m_nid;
    HWND m_hwnd;

    // Callbacks
    std::function<void()> m_onExitCallback;
    std::map<UINT, std::function<void()>> m_menuCallbacks;

    // Menu ID management
    UINT m_nextMenuId = 1001;
};