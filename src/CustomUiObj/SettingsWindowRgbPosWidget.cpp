#include "CustomUiObj/SettingsWindowRgbPosWidget.hpp"

SettingsWindowRgbPosWidget::SettingsWindowRgbPosWidget(const QString& title,
                            std::shared_ptr<WindowManager> wm,
                            PixelInfo currnentValue,
                            QWidget* parent)
    : QWidget(parent), wm(wm), currentValue(currnentValue), title(title)
{
    setupUi();
    setupLayout();
    setupConnections();
}

void SettingsWindowRgbPosWidget::setupUi()
{
    titleLabel = new QLabel(title, this);

    QString rgbString = QString("R: %1 G: %2 B: %3").arg(currentValue.color.red()).arg(currentValue.color.green()).arg(currentValue.color.blue());
    rgbLabel = new QLabel(rgbString, this);

    QString posString = QString("X:%1 Y:%2").arg(currentValue.position.x()).arg(currentValue.position.y());
    posLabel = new QLabel(posString, this);

    colorDisplay = new QWidget(this);
    colorDisplay->setFixedSize(45, 45);
    colorDisplay->setAutoFillBackground(true);
    updateColor(currentValue.color);

    editButton = new QPushButton("Edit", this);
    editButton->setFixedSize(45, 45);
}

void SettingsWindowRgbPosWidget::setupLayout()
{
    textLayout = new QVBoxLayout();
    textLayout->addWidget(titleLabel);
    textLayout->addWidget(rgbLabel);
    textLayout->addWidget(posLabel);
    textLayout->addStretch();

    rightLayout = new QHBoxLayout();
    rightLayout->addWidget(colorDisplay);
    rightLayout->addWidget(editButton);

    mainLayout = new QHBoxLayout(this);
    mainLayout->addLayout(textLayout);
    mainLayout->addLayout(rightLayout);
    mainLayout->setContentsMargins(5,5,5,5);

}
void SettingsWindowRgbPosWidget::setupConnections()
{
    connect(editButton, &QPushButton::clicked, this, &SettingsWindowRgbPosWidget::onEditClicked);
}

PixelInfo SettingsWindowRgbPosWidget::getResult() const {
    return currentValue;
}

void SettingsWindowRgbPosWidget::updateColor(const QColor& color)
{
    QPalette pal = colorDisplay->palette();
    pal.setColor(QPalette::Window, color);
    colorDisplay->setPalette(pal);
}

void SettingsWindowRgbPosWidget::onEditClicked()
{
    PixelPickerDialog* dlg = new PixelPickerDialog(wm, this);
    if (dlg->exec() == QDialog::Accepted) {
        currentValue = dlg->getResult();

        rgbLabel->setText(QString("R: %1 G: %2 B: %3")
                            .arg(currentValue.color.red())
                            .arg(currentValue.color.green())
                            .arg(currentValue.color.blue()));

        posLabel->setText(QString("X: %1 Y: %2")
                            .arg(currentValue.position.x())
                            .arg(currentValue.position.y()));
        updateColor(currentValue.color);
    }
}