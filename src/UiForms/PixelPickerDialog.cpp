#include "UiForms/PixelPickerDialog.hpp"


PixelPickerDialog::PixelPickerDialog(std::shared_ptr<WindowManager> wm, QWidget* parent)
    : QDialog(parent), wm(wm)
{
    setupUi();
    setupLayout();
    setupConnections();
}

PixelInfo PixelPickerDialog::getResult() const 
{
    return currentResult; 
}

void PixelPickerDialog::setupUi() 
{
    QFont font;
    font.setPointSize(16);

    imagePixelPicker = new ImagePixelPicker(this);
    imagePixelPicker->setGeometry(200, 0, 1366, 768);

    captureButton = new QPushButton("Capture Screen", this);
    captureButton->setGeometry(0, 0, 200, 40);

    RGBLabel = new QLabel("RGB:", this);
    RGBLabel->setGeometry(0, 40, 200, 40);
    RGBLabel->setFont(font);

    PositionLabel = new QLabel("Position:", this);
    PositionLabel->setGeometry(0, 80, 200, 40);
    PositionLabel->setFont(font);

    colorDisplay = new QWidget(this);
    colorDisplay->setGeometry(0, 120, 200, 200);
    colorDisplay->setAutoFillBackground(true);
    QPalette pal = colorDisplay->palette();
    pal.setColor(QPalette::Window, QColor(255,255,255));
    colorDisplay->setPalette(pal);

    saveButton = new QPushButton("Save", this);
    saveButton->setGeometry(0, 768-40, 200, 40);
}

void PixelPickerDialog::setupLayout()
{
    setWindowTitle("Pixel Picker Window");
    setFixedSize(1366 + 200, 768);
    setAttribute(Qt::WA_DeleteOnClose);
}

void PixelPickerDialog::setupConnections() 
{
    connect(captureButton, &QPushButton::clicked, this, &PixelPickerDialog::onCapture);
    connect(saveButton, &QPushButton::clicked, this, &PixelPickerDialog::onSave);
    connect(imagePixelPicker, &ImagePixelPicker::pixelClicked, this, &PixelPickerDialog::onPixelClicked);
}

void PixelPickerDialog::onPixelClicked(const PixelInfo& info)
{
    currentResult.color = info.color;
    currentResult.position = info.position;

    RGBLabel->setText(QString("RGB: (%1, %2, %3)")
                        .arg(info.color.red())
                        .arg(info.color.green())
                        .arg(info.color.blue()));
    PositionLabel->setText(QString("Position: (%1, %2)")
                            .arg(info.position.x())
                            .arg(info.position.y()));
    QPalette pal = colorDisplay->palette();
    pal.setColor(QPalette::Window, info.color);
    colorDisplay->setPalette(pal);
}

void PixelPickerDialog::onCapture()
{
    cv::Mat screen = wm->getScreenshot();
    imagePixelPicker->setImage(screen);
}

void PixelPickerDialog::onSave()
{
    accept();
}
