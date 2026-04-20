#pragma once

#include <math.h>
#include "FrameProcess.hpp"

namespace movement
{
inline float getAngleError(
    const std::pair<float,float>& currentPos,
    const std::pair<float,float>& prevPos,
    const std::pair<float,float>& target)
{
    float dx = target.first - currentPos.first;
    float dy = target.second - currentPos.second;

    float thetaTarget = std::atan2(dy, dx);

    float vx = currentPos.first - prevPos.first;
    float vy = currentPos.second - prevPos.second;

    float thetaCurrent = std::atan2(vy, vx);

    float errorAngle = thetaTarget - thetaCurrent;
    while(errorAngle > M_PI)  errorAngle -= 2*M_PI;
    while(errorAngle < -M_PI) errorAngle += 2*M_PI;

    return errorAngle * 180.0f / M_PI;
}

inline float getDistance(
    const std::pair<float,float>& currentPos,
    const std::pair<float,float>& target)
{
    float dx = target.first - currentPos.first;
    float dy = target.second - currentPos.second;
    return std::sqrt(dx*dx + dy*dy); 
}

inline std::pair<float,float> setMouseWrtChar(std::pair<float,float> windowCenter,float dist, float angle)
{           
    float theta = angle * M_PI / 180;

    float dx = dist * cos(theta);
    float dy = dist * sin(theta);

    dy = -dy;

    return std::pair{
        windowCenter.first  + static_cast<int>(dx),
        windowCenter.second + static_cast<int>(dy)
    };
}

inline std::pair<float, float> getCharPos(cv::Mat& frame,std::shared_ptr<cv::dnn::Net> digitsNet)
{
    cv::Rect roiXRect(1243, 32, 56, 14);
    cv::Rect roiYRect(1304, 32, 56, 14);

    cv::Mat croppedX = frame(roiXRect).clone();
    cv::Mat croppedY = frame(roiYRect).clone();
    
    cv::Mat croppedXGray, croppedYGray;
    cv::cvtColor(croppedX, croppedXGray, cv::COLOR_RGB2GRAY);
    cv::cvtColor(croppedY, croppedYGray, cv::COLOR_RGB2GRAY);

    cv::threshold(croppedXGray, croppedXGray, 250, 255, cv::THRESH_BINARY);
    cv::threshold(croppedYGray, croppedYGray, 250, 255, cv::THRESH_BINARY);

    cv::Mat resizedX;
    cv::Mat resizedY;

    cv::resize(croppedXGray, resizedX, cv::Size(), 3.0, 3.0, cv::INTER_CUBIC);
    cv::resize(croppedYGray, resizedY, cv::Size(), 3.0, 3.0, cv::INTER_CUBIC);

    auto charsX = frame_process::digitsNN::segmentDigitsWithCenterPadding(resizedX, 32);
    auto charsY = frame_process::digitsNN::segmentDigitsWithCenterPadding(resizedY, 32);

    int x_joc=0;
    int y_joc=0;
    std::string x_joc_str="";
    std::string y_joc_str="";

    int idx = 0;
    for (auto& ch : charsX) {
        if (idx>=2)
            x_joc_str += frame_process::digitsNN::ocr_read_char(ch,digitsNet);
        
        idx++;
    }

    idx = 0;
    for (const cv::Mat& ch : charsY) {
        if (idx>=2)
            y_joc_str += frame_process::digitsNN::ocr_read_char(ch,digitsNet);

        idx++;
    }

    try
    {
        x_joc = stoi(x_joc_str);
        y_joc = stoi(y_joc_str);
    } catch (const std::exception& e)
    {
        x_joc=0;
        y_joc=0;
    }

    return {x_joc,y_joc};
}

inline void sortNavigationWrtDistanceClosestFirst(
    std::vector<NavigationElement>& nav,
    const std::pair<float,float>& target)
{
    std::sort(nav.begin(), nav.end(),
        [&](const NavigationElement& a, const NavigationElement& b)
        {
            float da = getDistance(a.pos, target);
            float db = getDistance(b.pos, target);
            return da < db;
        });
}

inline bool isInsideCircle(int px, int py, int cx, int cy, int radius)
{
    int dx = px - cx;
    int dy = py - cy;

    return (dx*dx + dy*dy) <= radius*radius;
}

inline void sortNavigationWrtDistanceFarestFirst(
    std::vector<NavigationElement>& nav,
    const std::pair<float,float>& target)
{
    std::sort(nav.begin(), nav.end(),
        [&](const NavigationElement& a, const NavigationElement& b)
        {
            float da = getDistance(a.pos, target);
            float db = getDistance(b.pos, target);
            return da > db;
        });
}

inline std::pair<float, float> convertToGameCoordinates(std::pair<float, float> img)
{
    return {
        (img.first  - 2851.6f) / 0.16697f,
        (img.second - 705.5f)  / -0.16647f
    };
}

inline float convertRadiusToGame(float imgRadius)
{
    float sx = 0.16697f;
    float sy = 0.16647f;

    return imgRadius / ((std::abs(sx) + std::abs(sy)) * 0.5f);
}

inline std::pair<float, float> convertToImgCoordinates(std::pair<float, float> pos)
{
    return {
        0.16697f * pos.first  + 2851.6f,
       -0.16647f * pos.second + 705.5f
    };
}

inline float convertRadiusToImg(float radius)
{
    return radius * (std::abs(0.16697f) + std::abs(0.16647f)) * 0.5f;
}
}