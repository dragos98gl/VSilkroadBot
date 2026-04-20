#pragma once

#include <limits>
#include <vector>
#include <opencv2/opencv.hpp>

#include "Consts.hpp"
#include "IO/WindowManager.hpp"

namespace frame_process
{

namespace digitsNN
{
inline std::vector<cv::Mat> segmentDigitsWithCenterPadding(
    const cv::Mat& binaryImg,
    int padSize = 32,      
    int minCharWidth = 1
) {
    CV_Assert(binaryImg.type() == CV_8UC1);

    std::vector<cv::Mat> characters;
    int w = binaryImg.cols;
    int h = binaryImg.rows;

    cv::Mat work = binaryImg;

    int startX = -1;

    for (int x = 0; x < w; x++) {
        bool columnHasPixel = false;

        for (int y = 0; y < h; y++) {
            if (work.at<uchar>(y, x) > 0) {
                columnHasPixel = true;
                break;
            }
        }

        if (columnHasPixel && startX == -1)
            startX = x;

        if (!columnHasPixel && startX != -1) {
            int endX = x - 1;
            int charWidth = endX - startX + 1;

            if (charWidth >= minCharWidth) {
                cv::Mat charROI = work(cv::Rect(startX, 0, charWidth, h));

                int top = h, bottom = 0;
                int whitePixels = 0;

                for (int yy = 0; yy < h; yy++) {
                    for (int xx = 0; xx < charROI.cols; xx++) {
                        if (charROI.at<uchar>(yy, xx) > 200) {
                            whitePixels++;
                            top = std::min(top, yy);
                            bottom = std::max(bottom, yy);
                        }
                    }
                }

                if (whitePixels > 5 && (float)whitePixels / (charWidth * h) > 0.01f) {
                    cv::Mat charCropped =
                        charROI(cv::Rect(0, top, charWidth, bottom - top + 1)).clone();

                    int cx = charCropped.cols / 2;
                    int cy = charCropped.rows / 2;

                    int halfPad = padSize / 2;
                    int x1 = std::max(0, cx - halfPad);
                    int y1 = std::max(0, cy - halfPad);
                    int x2 = std::min(charCropped.cols, cx + halfPad);
                    int y2 = std::min(charCropped.rows, cy + halfPad);

                    cv::Mat padded = cv::Mat::zeros(padSize, padSize, charCropped.type());

                    cv::Rect srcROI(x1, y1, x2 - x1, y2 - y1);
                    cv::Rect dstROI(halfPad - (cx - x1), halfPad - (cy - y1), srcROI.width, srcROI.height);
                    charCropped(srcROI).copyTo(padded(dstROI));

                    characters.push_back(padded);
                }
            }
            startX = -1;
        }
    }

    return characters;
}

inline std::string ocr_read_char(const cv::Mat& charImg, std::shared_ptr<cv::dnn::Net> digitsNet) {
    cv::Mat img;
    
    if (charImg.channels() == 1) {
        cv::cvtColor(charImg, img, cv::COLOR_GRAY2BGR);
    } else {
        charImg.copyTo(img);
    }
    
    cv::resize(img, img, cv::Size(32, 32));
    img.convertTo(img, CV_32F, 1.0 / 255.0);
    
    cv::Mat blob = cv::dnn::blobFromImage(img, 1.0, cv::Size(32, 32), 
                                          cv::Scalar(), false, false);
    
    digitsNet->setInput(blob);
    cv::Mat output = digitsNet->forward();
    
    cv::Point classIdPoint;
    double confidence;
    cv::minMaxLoc(output, nullptr, &confidence, nullptr, &classIdPoint);
    
    int classId = classIdPoint.x;
    std::string classes = "0123456789XY:-";
    
    if (classId < 0 || classId >= (int)classes.size())
        return "0";
    
    return std::string(1, classes[classId]);
}
}

namespace mobsNN
{
inline cv::Mat letterbox(const cv::Mat& src, int new_size,
                  float& scale, int& top, int& left)
{
    int w = src.cols;
    int h = src.rows;

    scale = std::min((float)new_size / w, (float)new_size / h);
    int nw = int(w * scale);
    int nh = int(h * scale);

    cv::Mat resized;
    cv::resize(src, resized, cv::Size(nw, nh));

    top  = (new_size - nh) / 2;
    left = (new_size - nw) / 2;

    cv::Mat out(new_size, new_size, src.type(), cv::Scalar(114,114,114));
    resized.copyTo(out(cv::Rect(left, top, nw, nh)));

    return out;
}

inline std::vector<NetOutput> searchForMob(cv::Mat& screenRGB,cv::dnn::Net* mobsNet, float confThreshold=0.60f,float nmsThreshold=0.45f)
{
    float scale;
    int top, left;

    cv::Mat imgLB = frame_process::mobsNN::letterbox(screenRGB, 640, scale, top, left);
    cv::Mat blob;
    cv::dnn::blobFromImage(
        imgLB, blob,
        1.0 / 255.0,
        cv::Size(),
        cv::Scalar(),
        true,
        false
    );

    mobsNet->setInput(blob);
    cv::Mat output = mobsNet->forward();

    // 1. Reshape and Transpose: go from [1, 5, 8400] to [8400, 5]
    cv::Mat rawData(output.size[1], output.size[2], CV_32F, output.data);
    cv::Mat detections = rawData.t(); // Now we have 8400 rows, each with 5 values

    std::vector<cv::Rect> boxes;
    std::vector<float> confidences;
    std::vector<cv::Point> centers;

    for (int i = 0; i < detections.rows; ++i) {
        // Get pointer to current row
        float* row = detections.ptr<float>(i);
        float confidence = row[4]; // In YOLOv8 (1 class), index 4 is the score

        if (confidence > confThreshold) {
            float cx = row[0];
            float cy = row[1];
            float w  = row[2];
            float h  = row[3];

            // Calculate coordinates for cv::Rect (left, top, width, height)
            // Apply inverse transformation for letterbox (scale, left, top)
            int x = int((cx - w / 2.0f - left) / scale);
            int y = int((cy - h / 2.0f - top) / scale);
            int width = int(w / scale);
            int height = int(h / scale);

            boxes.push_back(cv::Rect(x, y, width, height));
            confidences.push_back(confidence);
            centers.push_back(cv::Point(x+width/2, y+height/1.5));
        }
    }

    // 2. Apply Non-Maximum Suppression to clean up duplicates
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

    std::vector<NetOutput> netOut;
    for (int idx : indices) {
        netOut.push_back({boxes[idx], confidences[idx], centers[idx]});
    }

    return netOut;
}
}

inline cv::Scalar colorFromType(NavigationType type)
{
    switch (type)
    {
    case NavigationType::ATTACKING_AREA:
        return cv::Scalar(255, 0, 0);   
    case NavigationType::TO_GO_WHEN_NO_MOBS:
        return cv::Scalar(0, 255, 0);   
    case NavigationType::TRAJECTORY:
        return cv::Scalar(0, 0, 255);   
    case NavigationType::BYPASS:
        return cv::Scalar(255, 255, 255); 
    default:
        return cv::Scalar(255, 255, 255);
    }
}

inline double alphaFromType(NavigationType type)
{
    switch (type)
    {
    case NavigationType::TRAJECTORY:
        return 1.0;  
    default:
        return 0.3; 
    }
}

inline void drawNavigationElements(cv::Mat& map, const NavigationStructure& nav)
{
    CV_Assert(!map.empty());

    auto drawGroup = [&](const std::vector<NavigationElement>& elements)
    {
        for (const auto& e : elements)
        {
            cv::Mat overlay = map.clone();

            cv::circle(
                overlay,
                { e.pos.first, e.pos.second },
                static_cast<int>(e.radius),
                frame_process::colorFromType(e.type),
                -1,
                cv::LINE_AA
            );

            double alpha = frame_process::alphaFromType(e.type);

            cv::addWeighted(
                overlay,
                alpha,
                map,
                1.0 - alpha,
                0,
                map
            );
        }
    };

    drawGroup(nav.toGoWhenNoMobs);
    drawGroup(nav.trajectory);
    drawGroup(nav.bypass);
}

inline cv::Mat drawDetectedMobsRect(cv::Mat& img, std::vector<NetOutput>& netOut)
{
    cv::Mat out = img.clone();
    
    // DRAW DETECTIONS
    for (int idx = 0; idx < netOut.size(); ++idx) 
    {
        cv::Rect box = netOut[idx].box;
        float conf = netOut[idx].confidence;

        cv::rectangle(out, box, cv::Scalar(0, 255, 0), 2);
        
        // DRAW DETECTIONS CONFIDENCE
        std::string label = "Obj: " + std::to_string(conf).substr(0, 4);
        cv::putText(out, label, cv::Point(box.x, box.y - 5), 
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
    }

    return out;
}

inline cv::Mat drawCombatArea(const cv::Mat& img,
                           std::pair<int,int> charPos,
                           std::pair<int,int> areaPos,
                           int areaRadius,
                           int width, int height,
                           bool isInsideAttackingArea)
{
    width  = std::min(width,  img.cols);
    height = std::min(height, img.rows);

    int x0 = areaPos.first - width  / 2;
    int y0 = areaPos.second - height / 2;

    x0 = std::max(0, std::min(x0, img.cols - width));
    y0 = std::max(0, std::min(y0, img.rows - height));

    cv::Rect roi(x0, y0, width, height);
    cv::Mat cropped = img(roi).clone();

    int cx = charPos.first - x0;
    int cy = charPos.second - y0;
    cv::circle(cropped,
               cv::Point(cx, cy),
               2,
               cv::Scalar(255, 255, 255),
               -1);

    cv::Mat overlay = cropped.clone();

    int charX = areaPos.first - x0;
    int charY = areaPos.second - y0;

    charX = std::clamp(charX, 0, width - 1);
    charY = std::clamp(charY, 0, height - 1);

    if (isInsideAttackingArea)
        cv::circle(overlay,
                cv::Point(charX, charY),
                areaRadius,
                cv::Scalar(255, 0, 0),
                -1);
    else
        cv::circle(overlay,
                cv::Point(charX, charY),
                areaRadius,
                cv::Scalar(0, 0, 255),
                -1);

    double alpha = 0.2;
    cv::addWeighted(overlay, alpha,
                    cropped, 1.0 - alpha,
                    0,
                    cropped);

    return cropped;
}

inline cv::Mat getMinimap(const cv::Mat& screenRGB)
{
    int x = 1249;
    int y = 56;
    int size = 105;
    float radius = size / 2;

    cv::Rect roiRect(x, y, size, size);
    cv::Mat roi = screenRGB(roiRect);

    static cv::Mat circleMask;
    if (circleMask.empty()) {
        circleMask = cv::Mat::zeros(size, size, CV_8UC1);
        cv::circle(circleMask, cv::Point(radius, radius), radius, cv::Scalar(255), -1);
    }
    
    cv::Mat roiCircle;
    roi.copyTo(roiCircle, circleMask);
    
    //cv::namedWindow("ROI Circle", cv::WINDOW_NORMAL);
    //cv::resizeWindow("ROI Circle", 400, 400);
    //cv::imshow("ROI Circle", roiCircle);

    return roiCircle;
}

inline bool arePlayersNearby(const cv::Mat& minimap)
{
    cv::Mat hsv;
    cv::cvtColor(minimap, hsv, cv::COLOR_BGR2HSV);

    cv::Scalar lower(36, 150, 150);
    cv::Scalar upper(37, 255, 255);

    cv::Mat hsvMask;
    cv::inRange(hsv, lower, upper, hsvMask);

    return cv::countNonZero(hsvMask) > 0;

    //cv::Mat overlay;
    //cv::cvtColor(hsvMask, overlay, cv::COLOR_GRAY2BGR);
    //cv::addWeighted(minimap, 0.1, overlay, 0.9, 0, overlay);

    //cv::namedWindow("Overlay", cv::WINDOW_NORMAL);
    //cv::resizeWindow("Overlay", 400, 400);
    //cv::imshow("Overlay", overlay);
    //cv::waitKey(1);
}

inline bool comparePixels(const cv::Vec3b& pixel, const QColor& color)
{
    return (pixel[0] == color.blue() &&
            pixel[1] == color.green() &&
            pixel[2] == color.red());
}

inline bool checkIfMob(std::shared_ptr<WindowManager> wm)
{
    return wm->compareCursor(mobCursorCrc32); 
}

inline std::pair<int,int> positionToWindow(std::pair<int,int> windowPos,std::pair<int,int> position,std::pair<int,int> screenSize)
{
    return {static_cast<int>(65535.0 * (windowPos.first + position.first) / screenSize.first),
                 static_cast<int>(65535.0 * (windowPos.second + position.second) / screenSize.second)};
}

inline std::pair<int,int> positionToWindow(std::pair<int,int> windowPos,QPoint position,std::pair<int,int> screenSize)
{
    return {static_cast<int>(65535.0 * (windowPos.first + position.x()) / screenSize.first),
                 static_cast<int>(65535.0 * (windowPos.second + position.y()) / screenSize.second)};
}

inline std::pair<int,int> positionToWindow(std::pair<int,int> windowPos,cv::Point position,std::pair<int,int> screenSize)
{
    return {static_cast<int>(65535.0 * (windowPos.first + position.x) / screenSize.first),
                 static_cast<int>(65535.0 * (windowPos.second + position.y) / screenSize.second)};
}
}