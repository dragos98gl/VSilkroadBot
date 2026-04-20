#include "UiForms/Bot/BotUi.hpp"

BotUi::BotUi(
        std::shared_ptr<WindowManager> windowManager,
        std::shared_ptr<cv::dnn::Net> mobsNet,
        std::shared_ptr<cv::dnn::Net> digitsNet,
        QWidget* parent)
    : bot(windowManager, mobsNet, digitsNet), QWidget(parent)
{
    setWindowTitle("Bot UI Window");
    setFixedWidth(360);
    setAttribute(Qt::WA_DeleteOnClose);

    setupUi();
    setupLayout();
    setupConnections();
    
    adjustSize();

    bot.start();
}

void BotUi::setupUi()
{
    doubleImageView = new DoubleImageView(this);
    onlyAttackingCheck = new QCheckBox();
    onlyAttackingCheck->setText("Only Attacking");
}

void BotUi::setupLayout()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(doubleImageView);
    mainLayout->addWidget(onlyAttackingCheck);
}

void BotUi::setupConnections()
{
    connect(&bot, &Bot::updateBotUI,this, &BotUi::onUpdateBotUI);
    connect(onlyAttackingCheck, &QCheckBox::toggled, this, &BotUi::onOnlyAttackingToggled);
    connect(this, &BotUi::settingsUpdated, &bot, &Bot::onSettingsUpdate);
}

void BotUi::onOnlyAttackingToggled()
{
    emit settingsUpdated(onlyAttackingCheck->isChecked());
}

void BotUi::onUpdateBotUI(
    const cv::Mat& mobsDetection, 
    const cv::Mat& map, 
    const std::pair<int, int>& charPosImg, 
    const std::pair<int, int>& attackingAreaPos,
    float attackingAreaRadius,
    bool isInsideAttackingArea 
    )
{
    cv::Mat topImg;
    cv::Mat attackingAreaDraw = frame_process::drawCombatArea(
        map,
        charPosImg,
        attackingAreaPos,
        attackingAreaRadius,
        100,100,isInsideAttackingArea);

    doubleImageView->setTopImage(matToQImage(attackingAreaDraw,320,320));
    doubleImageView->setBottomImage(matToQImage(mobsDetection,320,180));
}

QImage BotUi::matToQImage(const cv::Mat& img, int targetW, int targetH)
{
    if (img.empty())
        return QImage();

    cv::Mat resized;
    cv::resize(img, resized, cv::Size(targetW, targetH), 0, 0, cv::INTER_LINEAR);

    cv::Mat matRGB;
    cv::cvtColor(resized, matRGB, cv::COLOR_BGR2RGB);

    return QImage(matRGB.data,
                matRGB.cols,
                matRGB.rows,
                static_cast<int>(matRGB.step),
                QImage::Format_RGB888).copy();
}