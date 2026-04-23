#pragma once
#include <opencv2/opencv.hpp>

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QRect>

#include "types.hpp"

namespace utils
{

inline uint32_t crc32(const uint8_t* data, size_t length)
{
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < length; ++i)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }

    return ~crc;
}

inline std::string getTimestamp()
{
    auto now     = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm     tm{};

    localtime_s(&tm, &t);

    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << tm.tm_hour << ":"
        << std::setw(2) << tm.tm_min  << ":"
        << std::setw(2) << tm.tm_sec;
    return oss.str();
}

inline std::vector<TrainingModel> loadTrainingModels(const QString& modelName)
{
    std::vector<TrainingModel> result;

    QString baseDir = QString("./assets/models/%1/").arg(modelName);
    QStringList types = {"train", "val"};

    for (const QString& type : types)
    {
        QString imgDir = baseDir + "images/" + type + "/";
        QString lblDir = baseDir + "labels/" + type + "/";

        QDir dir(imgDir);
        if (!dir.exists()) continue;

        QStringList files = dir.entryList(QStringList() << "img_*.jpg",
                                          QDir::Files,
                                          QDir::Name);

        for (const QString& imgFile : files)
        {
            QString imgPath = imgDir + imgFile;

            QString lblFile = imgFile;
            lblFile.chop(4);        // remove ".jpg"
            lblFile += ".txt";
            QString lblPath = lblDir + lblFile;

            TrainingModel m;
            m.imgPath = imgPath.toStdString();
            m.labelsPath = lblPath.toStdString();

            // ---------- name without extension ----------
            QString baseName = imgFile.left(imgFile.length() - 4).right(4); // 0001
            m.name = QString("%1_%2").arg(type).arg(baseName).toStdString(); // ex: train_0001

            // ---------- Load image ----------
            cv::Mat img = cv::imread(imgPath.toStdString());
            m.asset.img = img;

            // ---------- Load labels (YOLO format) ----------
            std::vector<QRect> rects;
            QFile file(lblPath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QTextStream in(&file);
                while (!in.atEnd())
                {
                    QString line = in.readLine();
                    if (line.trimmed().isEmpty()) continue;

                    QStringList parts = line.split(' ', Qt::SkipEmptyParts);
                    if (parts.size() != 5) continue; // YOLO: class x_center y_center w h

                    double x_center = parts[1].toDouble();
                    double y_center = parts[2].toDouble();
                    double width    = parts[3].toDouble();
                    double height   = parts[4].toDouble();

                    int x = int((x_center - width/2.0) * img.cols);
                    int y = int((y_center - height/2.0) * img.rows);
                    int w = int(width * img.cols);
                    int h = int(height * img.rows);

                    rects.push_back(QRect(x, y, w, h));
                }
                file.close();
            }
            m.asset.labels = rects;

            result.push_back(m);
        }
    }

    return result;
}

template <typename Predicate>
inline bool waitUntil(Predicate&& condition,
               std::chrono::milliseconds timeout,
               std::chrono::milliseconds poll = 5ms)
{
    auto start = std::chrono::steady_clock::now();

    while (true)
    {
        if (condition())
            return true;

        if (std::chrono::steady_clock::now() - start >= timeout)
            return false;

        std::this_thread::sleep_for(poll);
    }
}
}