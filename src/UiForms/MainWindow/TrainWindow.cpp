#include "UiForms/MainWindow/TrainWindow.hpp"

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QStandardPaths>
#include <QRegularExpression>


#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

TrainWindow::TrainWindow(std::shared_ptr<WindowManager> wm, QWidget* parent)
    : QWidget(parent), wm(wm)
{
    setupUi();
    setupLayout();
    setupConnections();
}

void TrainWindow::setupUi()
{
    view = new RectImageView(this);
    view->setGeometry(200, 0, 1366, 768);

    captureButton = new QPushButton("Capture Screen", this);
    captureButton->setGeometry(0, 0, 200, 40);

    removeRecordButton = new QPushButton("Remove Last Record", this);
    removeRecordButton->setGeometry(0, 40, 200, 40);

    saveRecordButton = new QPushButton("Save Record", this);
    saveRecordButton->setEnabled(false);
    saveRecordButton->setGeometry(0, 80, 200, 40);

    saveRecordLabel = new QLabel("Record name:", this);
    saveRecordLabel->setGeometry(0, 120, 80, 40);

    saveRecordName = new QLineEdit(this);
    saveRecordName->setGeometry(80, 120, 120, 40);

    trainTypeGroup = new QButtonGroup(this);

    trainButton = new QRadioButton("train", this);
    valButton   = new QRadioButton("val", this);
    trainButton->setChecked(true);

    trainTypeGroup->addButton(trainButton);
    trainTypeGroup->addButton(valButton);
    trainTypeGroup->setExclusive(true);

    QWidget* radioWidget = new QWidget(this);
    radioWidget->setGeometry(0, 160, 200, 40);

    QHBoxLayout* radioLayout = new QHBoxLayout(radioWidget);
    radioLayout->addWidget(trainButton);
    radioLayout->addWidget(valButton);

    trainCountLabel = new QLabel("Train:0", this);
    trainCountLabel->setGeometry(0, 200, 200, 40);
    valCountLabel = new QLabel("Val:0", this);
    valCountLabel->setGeometry(0, 240, 200, 40);

    rectList = new QListWidget(this);
    rectList->setGeometry(0, 768-440, 200, 440);
}

void TrainWindow::setupLayout()
{
    setWindowTitle("Train Window");
    setFixedSize(1366 + 200, 768);
    setAttribute(Qt::WA_DeleteOnClose);
}

void TrainWindow::setupConnections()
{
    connect(saveRecordName, &QLineEdit::textChanged,
        this, &TrainWindow::onTextChanged);

    connect(captureButton, &QPushButton::clicked,
            this, &TrainWindow::onCapture);

    connect(removeRecordButton, &QPushButton::clicked,
            this, &TrainWindow::onRemove);

    connect(saveRecordButton, &QPushButton::clicked,
            this, &TrainWindow::onSave);

    connect(view, &RectImageView::rectListUpdated,
            this, &TrainWindow::onRectsUpdated);
}

void TrainWindow::onCapture()
{
    cv::Mat screen = wm->getRgbScreenshot();
    view->setImage(screen);
}

void TrainWindow::onRemove()
{
    view->removeLastRectangle();
}

void TrainWindow::onRectsUpdated(const QVector<QRect>& rects)
{
    rectList->clear();
    for (const auto& r : rects) {
        rectList->addItem(
            QString("x=%1 y=%2 w=%3 h=%4")
                .arg(r.x()).arg(r.y())
                .arg(r.width()).arg(r.height()));
    }

    if (!saveRecordName->text().isEmpty() && rectList->count() > 0) {
        saveRecordButton->setEnabled(true);
    } else {
        saveRecordButton->setEnabled(false);
    }
}

void TrainWindow::onTextChanged(const QString& text)
{
    saveRecordButton->setEnabled(!text.isEmpty() && rectList->count() > 0);
}

void TrainWindow::onSave()
{
    if (trainButton->isChecked())
        trainCount++;
    else
        valCount++;
    
    trainCountLabel->setText(QString("Train:%1").arg(trainCount));
    valCountLabel->setText(QString("Val:%1").arg(valCount));

    QString recordNameText = saveRecordName->text();
    if (recordNameText.isEmpty()) return;

    if (view->getRectangles().isEmpty()) return;

    QString baseDir = QString("./assets/models/%1/").arg(recordNameText);
    QString type = trainButton->isChecked() ? "train" : "val";
    QString imgDir = baseDir + "images/" + type + "/";
    QString lblDir = baseDir + "labels/" + type + "/";

    QDir dir;
    dir.mkpath(imgDir);
    dir.mkpath(lblDir);

    // ---------- calculate counter based on existing images ----------
    int counter = 0;
    QDir dirImg(imgDir);
    QStringList existingFiles = dirImg.entryList(QStringList() << "img_*.jpg", QDir::Files);

    QRegularExpression rx("img_(\\d{4})\\.jpg");

    for (const QString& f : existingFiles) {
        QRegularExpressionMatch match = rx.match(f);
        if (match.hasMatch()) {
            int num = match.captured(1).toInt();
            if (num >= counter) counter = num + 1;
        }
    }

    QString imgName = QString("img_%1.jpg").arg(counter, 4, 10, QChar('0'));
    QString imgPath = imgDir + imgName;
    QString lblPath = lblDir + imgName;
    lblPath.chop(4); // remove ".jpg"
    lblPath += ".txt";

    // ---------- save the image ----------
    cv::Mat imgOriginal = view->getCurrentImage();
    if (imgOriginal.empty()) return;

    cv::imwrite(imgPath.toStdString(), imgOriginal);

    QFile file(lblPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);

        QSize pixSize = view->pixmap().size();
        for (const QRect& r : view->getRectangles()) {
            double scaleX = static_cast<double>(imgOriginal.cols) / pixSize.width();
            double scaleY = static_cast<double>(imgOriginal.rows) / pixSize.height();

            double x_min = r.left() * scaleX;
            double y_min = r.top() * scaleY;
            double x_max = r.right() * scaleX;
            double y_max = r.bottom() * scaleY;

            double x_center = (x_min + x_max) / 2.0 / imgOriginal.cols;
            double y_center = (y_min + y_max) / 2.0 / imgOriginal.rows;
            double width    = (x_max - x_min) / imgOriginal.cols;
            double height   = (y_max - y_min) / imgOriginal.rows;

            out << "0 " << x_center << " " << y_center << " "
                << width << " " << height << "\n"; // class_id = 0
        }

        file.close();
    }

    saveYaml(recordNameText);

    view->removeAllRectangles();
    rectList->clear();
    saveRecordButton->setEnabled(false);
}

void TrainWindow::saveYaml(const QString& recordName) {
    QString baseDir = QDir::current().absoluteFilePath("assets/models/" + recordName);
    QString yamlPath = baseDir + "/dataset.yaml";

    QFile file(yamlPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Could not create YAML file!";
        return;
    }

    QTextStream out(&file);

    out << "train: " << baseDir + "/images/train\n";
    out << "val:   " << baseDir + "/images/val\n";
    out << "nc: 1\n";  // single label
    out << "names: ['mob']\n";  // class name

    file.close();
}