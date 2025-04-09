#pragma once

#include <windows.h>

#include <functional>
#include <map>
#include <shellapi.h>
#include <string>

class WindowsTray {
private:
    NOTIFYICONDATA nid;
    HWND hwnd;
    std::function<void()> onExitCallback;
    std::map<UINT, std::function<void()>> menuCallbacks;
    UINT nextMenuId = 1001;

public:
    WindowsTray(HINSTANCE hInstance, const std::string &tooltip);
    ~WindowsTray();

    void setOnExitCallback(std::function<void()> callback);
    UINT addMenuItem(const std::string &itemName,
                     std::function<void()> callback);
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                       LPARAM lParam);
};