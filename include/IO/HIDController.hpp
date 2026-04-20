#pragma once

#include <windows.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <iostream>

#include <interception.h>

class HIDController {
public:
    HIDController()
    {
        start();
        pause();
    }

    ~HIDController()
    {
        stop();

        if (interceptionMouseThreadHandle.joinable())
            interceptionMouseThreadHandle.join();
        if (interceptionKeyboardThreadHandle.joinable())
            interceptionKeyboardThreadHandle.join();
    }

    std::pair<int,int> getCurrentMousePosition()
    {
        POINT p;
        GetCursorPos(&p);
        return {p.x, p.y};
    }

    std::pair<int,int> pixelToAbsolute(int x, int y)
    {
        int screenWidth  = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        int absX = static_cast<int>(
            std::round((double)x * 65535.0 / (screenWidth  - 1))
        );

        int absY = static_cast<int>(
            std::round((double)y * 65535.0 / (screenHeight - 1))
        );

        return {absX, absY};
    }

    void setMousePosition(std::pair<int,int> position)
    {
        auto oldPos = getCurrentMousePosition();
        auto absOld = pixelToAbsolute(oldPos.first, oldPos.second);

        int x = position.first;
        int y = position.second;
        
        std::lock_guard<std::mutex> lock(mouseMutex);

        InterceptionMouseStroke stroke{};
        stroke.flags = INTERCEPTION_MOUSE_MOVE_ABSOLUTE;
        stroke.x = x;
        stroke.y = y;

        if (mouse)
            interception_send(mcontext, mouse, reinterpret_cast<InterceptionStroke*>(&stroke), 1);

       /* stroke.flags = INTERCEPTION_MOUSE_MOVE_ABSOLUTE;
        stroke.state = 0;
        stroke.x = absOld.first;
        stroke.y = absOld.second;
        if (mouse)
            interception_send(mcontext, mouse, reinterpret_cast<InterceptionStroke*>(&stroke), 1);
    */}

    void leftClickMouse(std::pair<int,int> position)
    {
        auto oldPos = getCurrentMousePosition();
        auto absOld = pixelToAbsolute(oldPos.first, oldPos.second);

        int x = position.first;
        int y = position.second;

        std::lock_guard<std::mutex> lock(mouseMutex);

        InterceptionMouseStroke stroke{};

        stroke.flags = INTERCEPTION_MOUSE_MOVE_ABSOLUTE;
        stroke.state = INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN;
        stroke.x = x;
        stroke.y = y;

        if (mouse)
            interception_send(mcontext, mouse, reinterpret_cast<InterceptionStroke*>(&stroke), 1);

        stroke.flags = INTERCEPTION_MOUSE_MOVE_ABSOLUTE;
        stroke.state = INTERCEPTION_MOUSE_LEFT_BUTTON_UP;
        if (mouse)
            interception_send(mcontext, mouse, reinterpret_cast<InterceptionStroke*>(&stroke), 1);
/*
        stroke.flags = INTERCEPTION_MOUSE_MOVE_ABSOLUTE;
        stroke.state = 0;
        stroke.x = absOld.first;
        stroke.y = absOld.second;
        if (mouse)
            interception_send(mcontext, mouse, reinterpret_cast<InterceptionStroke*>(&stroke), 1);
  */  }

    void rightClickMouse(std::pair<int,int> position)
    {
        auto oldPos = getCurrentMousePosition();
        auto absOld = pixelToAbsolute(oldPos.first, oldPos.second);

        int x = position.first;
        int y = position.second;    

        std::lock_guard<std::mutex> lock(mouseMutex);

        InterceptionMouseStroke stroke{};

        stroke.flags = INTERCEPTION_MOUSE_MOVE_ABSOLUTE;
        stroke.state = INTERCEPTION_MOUSE_RIGHT_BUTTON_DOWN;
        stroke.x = x;
        stroke.y = y;

        if (mouse)
            interception_send(mcontext, mouse, reinterpret_cast<InterceptionStroke*>(&stroke), 1);

        stroke.flags = INTERCEPTION_MOUSE_MOVE_ABSOLUTE;
        stroke.state = INTERCEPTION_MOUSE_RIGHT_BUTTON_UP;
        if (mouse)
            interception_send(mcontext, mouse, reinterpret_cast<InterceptionStroke*>(&stroke), 1);
/*
        stroke.flags = INTERCEPTION_MOUSE_MOVE_ABSOLUTE;
        stroke.state = 0;
        stroke.x = absOld.first;
        stroke.y = absOld.second;
        if (mouse)
            interception_send(mcontext, mouse, reinterpret_cast<InterceptionStroke*>(&stroke), 1);
  */  }
    
    void stop()
    {
        running = false;

        interception_destroy_context(mcontext);
        interception_destroy_context(kcontext);
    }

    void pause()
    {
        isPause = true;
        interception_destroy_context(mcontext);
    }

    void resume()
    {
        mcontext = interception_create_context();
        interception_set_filter(mcontext, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_MOVE);
        isPause = false;
    }

    void start()
    {
        mcontext = interception_create_context();
        kcontext = interception_create_context();

        interception_set_filter(mcontext, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_MOVE);
        interception_set_filter(kcontext, interception_is_keyboard, INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP);

        interceptionMouseThreadHandle = std::thread(&HIDController::mouseInterceptionThread, this);
        interceptionKeyboardThreadHandle = std::thread(&HIDController::keyboardInterceptionThread, this);

        running = true;
    }
private:

    void mouseInterceptionThread()
    {
        while (running)
        {
            if (isPause)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                continue;
            }

            InterceptionDevice dev = interception_wait_with_timeout(mcontext, 1);
            if (!running || !dev) continue;

            if (interception_receive(mcontext, dev, &mstroke, 1) > 0 && interception_is_mouse(dev))
            {
                auto stroke = *reinterpret_cast<InterceptionMouseStroke*>(&mstroke);

                {
                    std::lock_guard<std::mutex> lock(mouseMutex);
                    interception_send(mcontext, dev, reinterpret_cast<InterceptionStroke*>(&stroke), 1);
                    mouse = dev; 
                }
            }
        }
    }

    void keyboardInterceptionThread()
    {
        while (running)
        {
            InterceptionDevice dev = interception_wait_with_timeout(kcontext, 1);
            if (!running || !dev) continue;

            if (interception_receive(kcontext, dev, &kstroke, 1) > 0 && interception_is_keyboard(dev))
            {
                auto stroke = *reinterpret_cast<InterceptionKeyStroke*>(&kstroke);

                interception_send(kcontext, dev, reinterpret_cast<InterceptionStroke*>(&stroke), 1);

                if (stroke.state == 0 && stroke.code == SCANCODE_GRAVE)
                {
                    if (isPause)
                    {
                        resume();
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    }
                    else
                    {
                        pause();
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    }
                }
            }
        }
    }

private:
    static const int SCANCODE_ESC = 0x01;
    static const int SCANCODE_Q = 0x10;
    static const int SCANCODE_R = 0x13;
    static const int SCANCODE_GRAVE = 0x29;

    InterceptionContext mcontext{nullptr};
    InterceptionDevice mouse{0};
    InterceptionStroke mstroke;

    InterceptionContext kcontext{nullptr};
    InterceptionDevice keyboard{0};
    InterceptionStroke kstroke;

    std::thread interceptionMouseThreadHandle;
    std::thread interceptionKeyboardThreadHandle;

    std::atomic<bool> running{true};
    std::atomic<bool> isPause{false};
    std::mutex mouseMutex;
};