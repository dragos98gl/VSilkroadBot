#include "UiForms/MainWindow/ModifyExistingModelWindow.hpp"

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QStandardPaths>
#include <QRegularExpression>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "types.hpp"
#include "utils.hpp"

ModifyExistingModelWindow::ModifyExistingModelWindow(std::shared_ptr<WindowManager> wm_, QString modelName_, QWidget* parent)
    : QWidget(parent), wm(wm_), modelName(modelName_)
{
    QDir currentDir = QDir::currentPath();
    QString modelPath = currentDir.absoluteFilePath(QString("models/%1/runs/detect/out/weights/best.onnx").arg(modelName));

    if (QFileInfo::exists(modelPath)) {
        mobsNet = std::make_unique<cv::dnn::Net>(cv::dnn::readNetFromONNX(modelPath.toStdString().c_str()));
        mobsNet->setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        mobsNet->setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);
    }


    setupLayout();
    setupUi();
    setupConnections();

    trainingModels = utils::loadTrainingModels(modelName);
    filesList->clear();
    for (const auto& m : trainingModels)
        filesList->addItem(QString::fromStdString(m.name));
}

void ModifyExistingModelWindow::setupUi()
{    
    view = new RectImageView(this);
    view->setGeometry(200, 0, 1366, 768);

    captureButton = new QPushButton("Capture Screen", this);
    captureButton->setGeometry(0, 0, 200, 40);

    removeRecordButton = new QPushButton("Remove Record", this);
    removeRecordButton->setGeometry(0, 40, 200, 40);

    saveRecordButton = new QPushButton("Save Record", this);
    saveRecordButton->setEnabled(false);
    saveRecordButton->setGeometry(0, 80, 200, 40);

    saveRecordLabel = new QLabel("Record name:" + modelName, this);
    saveRecordLabel->setGeometry(10, 120, 200, 20);

    assistedModeCheck = new QCheckBox("Use assited mode", this);
    assistedModeCheck->setGeometry(10, 142, 200, 20);
    assistedModeCheck->setChecked(true);

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

    filesList = new QListWidget(this);
    filesList->setGeometry(0, 200, 200, 100);

    rectList = new QListWidget(this);
    rectList->setGeometry(0, 768-468, 200, 768-(768-468));
}
void ModifyExistingModelWindow::setupLayout()
{
    setWindowTitle("Modify existing model");
    setFixedSize(1366 + 200, 768);
    setAttribute(Qt::WA_DeleteOnClose);

}

void ModifyExistingModelWindow::setupConnections()
{
    connect(captureButton, &QPushButton::clicked,
            this, &ModifyExistingModelWindow::onCapture);

    connect(removeRecordButton, &QPushButton::clicked,
            this, &ModifyExistingModelWindow::onRemove);

    connect(saveRecordButton, &QPushButton::clicked,
            this, &ModifyExistingModelWindow::onSave);

    connect(view, &RectImageView::rectListUpdated,
            this, &ModifyExistingModelWindow::onRectsUpdated);

    connect(filesList, &QListWidget::currentRowChanged,
            this, &ModifyExistingModelWindow::onFileSelected);

    connect(rectList, &QListWidget::currentRowChanged,
            this, &ModifyExistingModelWindow::onRectListItemSelected);
}

void ModifyExistingModelWindow::onFileSelected(int row)
{
    if (row < 0 || row >= trainingModels.size())
        return;

    const TrainingModel& model = trainingModels[row];

    QVector<QRect> qRects;
    for (const QRect& r : model.asset.labels)
        qRects.append(r);

    view->setImageAndRects(model.asset.img, qRects);
}

void ModifyExistingModelWindow::onRectListItemSelected(int row)
{
    view->setSelectedRect(row);
}

void ModifyExistingModelWindow::onCapture()
{
    cv::Mat screen = wm->getRgbScreenshot();

    view->removeAllRectangles();
    view->setImage(screen);

    if (assistedModeCheck->isChecked())
    {
        std::vector<NetOutput> netOut =
            frame_process::mobsNN::searchForMob(screen, mobsNet.get());

        QVector<QRect> rects;
        for (const auto& o : netOut)
        {
            rects.append(QRect(
                o.box.x,
                o.box.y,
                o.box.width,
                o.box.height
            ));
        }

        view->setRectangles(rects);
    }
}

void ModifyExistingModelWindow::onRemove()
{    
    int row = rectList->currentRow();
    if (row < 0)
        return;

    view->removeRectangle(row);
}

void ModifyExistingModelWindow::onRectsUpdated(const QVector<QRect>& rects)
{
    rectList->clear();
    for (const auto& r : rects) {
        rectList->addItem(
            QString("x=%1 y=%2 w=%3 h=%4")
                .arg(r.x()).arg(r.y())
                .arg(r.width()).arg(r.height()));
    }

    saveRecordButton->setEnabled(rectList->count() > 0);
}

std::vector<QRect> ModifyExistingModelWindow::qVectorToStdVector(const QVector<QRect>& v)
{
    std::vector<QRect> out;
    out.reserve(v.size());
    for (const QRect& r : v) out.push_back(r);
    return out;
}

void ModifyExistingModelWindow::onSave()
{
    if (modelName.isEmpty()) return;
    if (view->getRectangles().isEmpty()) return;

    int selectedRow = filesList->currentRow(); // selected element in the file list
    bool editingExisting = (selectedRow >= 0 && selectedRow < trainingModels.size());

    QString baseDir = QString("./models/%1/").arg(modelName);
    QString type = trainButton->isChecked() ? "train" : "val";
    QString imgDir = baseDir + "images/" + type + "/";
    QString lblDir = baseDir + "labels/" + type + "/";

    QDir dir;
    dir.mkpath(imgDir);
    dir.mkpath(lblDir);

    cv::Mat imgOriginal = view->getCurrentImage();
    if (imgOriginal.empty()) return;

    QString imgPath;
    QString lblPath;
    QString imgName;

    if (editingExisting)
    {
        // --- overwrite existing file ---
        imgPath = QString::fromStdString(trainingModels[selectedRow].imgPath);
        lblPath = QString::fromStdString(trainingModels[selectedRow].labelsPath);

        imgName = QFileInfo(imgPath).fileName();
    }
    else
    {
        // --- create a new record ---
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

        imgName = QString("img_%1.jpg").arg(counter, 4, 10, QChar('0'));
        imgPath = imgDir + imgName;
        lblPath = lblDir + imgName;
        lblPath.chop(4);
        lblPath += ".txt";
    }

    // ---------- save the image ----------
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

    // ---------- update trainingModels vector ----------
    TrainingModel updatedModel;
    updatedModel.imgPath = imgPath.toStdString();
    updatedModel.labelsPath = lblPath.toStdString();
    QString baseName = imgName.left(imgName.length()-4).right(4);
    updatedModel.name = QString("%1_%2").arg(type).arg(baseName).toStdString(); // ex: train_0001
    updatedModel.asset.img = imgOriginal.clone();
    updatedModel.asset.labels = qVectorToStdVector(view->getRectangles());

    if (editingExisting)
    {
        trainingModels[selectedRow] = updatedModel;
        filesList->item(selectedRow)->setText(QString::fromStdString(updatedModel.name));
    }
    else
    {
        trainingModels.push_back(updatedModel);
        filesList->addItem(QString::fromStdString(updatedModel.name));
    }

    // ---------- update YAML ----------
    saveYaml(modelName);

    view->removeAllRectangles();
    rectList->clear();
    saveRecordButton->setEnabled(false);
}

void ModifyExistingModelWindow::saveYaml(const QString& recordName) {
    QString baseDir = QDir::current().absoluteFilePath("models/" + recordName);
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