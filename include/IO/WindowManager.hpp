#pragma once

#include <windows.h>
#include <wingdi.h>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include "utils.hpp"
#include "types.hpp"

class WindowManager 
{
public:
    static bool checkWindowExists(const std::string& windowTitle)
    {
        std::wstring wtext(windowTitle.begin(), windowTitle.end());
        HWND tmpHwnd = FindWindowW(NULL, wtext.c_str());
        return tmpHwnd != NULL;
    }

    WindowManager(const std::string& windowTitle)
    {
        std::wstring wtext(windowTitle.begin(), windowTitle.end());
        hwnd = FindWindowW(NULL, wtext.c_str());
    }

    cv::Mat getScreenshot()
    {
        auto [width, height] = getWindowSize();

        HDC hdcWindow = GetDC(hwnd);
        HDC hdcMem = CreateCompatibleDC(hdcWindow);

        HBITMAP hBitmap = CreateCompatibleBitmap(hdcWindow, width, height);
        SelectObject(hdcMem, hBitmap);

        StretchBlt(hdcMem, 0, 0, width, height, hdcWindow, 0, 0, width, height, SRCCOPY);

        BITMAPINFOHEADER bi{};
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = width;
        bi.biHeight = -height;
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;

        cv::Mat mat(height, width, CV_8UC4);
        GetDIBits(hdcMem, hBitmap, 0, height, mat.data, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        ReleaseDC(hwnd, hdcWindow);

        return mat;
    }

    bool compareCursor(uint32_t cursorCrc32)
    {
        uint32_t crc = 0;
        CURSORINFO cursor_info = {0};
        cursor_info.cbSize = sizeof(CURSORINFO);

        if (GetCursorInfo(&cursor_info) && cursor_info.hCursor)
        {
            ICONINFO icon_info;
            if (GetIconInfo(cursor_info.hCursor, &icon_info))
            {
                HBITMAP hBitmap = icon_info.hbmColor ? icon_info.hbmColor : icon_info.hbmMask;

                BITMAP bmp;
                GetObject(hBitmap, sizeof(BITMAP), &bmp);

                BITMAPINFO bmi = {0};
                bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bmi.bmiHeader.biWidth = bmp.bmWidth;
                bmi.bmiHeader.biHeight = -bmp.bmHeight;
                bmi.bmiHeader.biPlanes = 1;
                bmi.bmiHeader.biBitCount = 32;
                bmi.bmiHeader.biCompression = BI_RGB;

                const int SAMPLE_BYTES = 128;
                uint8_t buffer[SAMPLE_BYTES] = {0};

                HDC hdc = GetDC(nullptr);

                GetDIBits(
                    hdc,
                    hBitmap,
                    0,
                    1,                
                    buffer,
                    &bmi,
                    DIB_RGB_COLORS
                );

                ReleaseDC(nullptr, hdc);

                crc = utils::crc32(buffer, SAMPLE_BYTES);

                DeleteObject(icon_info.hbmColor);
                DeleteObject(icon_info.hbmMask);
            }
        }
        return (crc == cursorCrc32);
    }

    cv::Mat getRgbScreenshot()
    {
        cv::Mat screenRGB;
        cv::Mat screen = getScreenshot();
        cv::cvtColor(screen, screenRGB, cv::COLOR_RGBA2RGB);

        return screenRGB;
    }

    std::pair<int,int> getWindowPos()
    {
        POINT pt = {0, 0};
        ClientToScreen(hwnd, &pt);
        return {pt.x, pt.y};
    }

    std::pair<int,int> getWindowSize()
    {
        RECT rc;
        GetClientRect(hwnd, &rc);
        int width  = rc.right - rc.left;
        int height = rc.bottom - rc.top;

        return {width, height};
    }

    WindowInfo getWindowInfo()
    {
        WindowInfo windowInfo;
        windowInfo.windowPosition = getWindowPos();
        windowInfo.windowSize = getWindowSize();
        windowInfo.windowCenter = {windowInfo.windowSize.first/2, windowInfo.windowSize.second/2};
        windowInfo.screenResolution = {GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};

        return windowInfo;
    }
private:
    HWND hwnd;
};