#include "UiForms/Settings/BotNavigationSettingsWindow.hpp"

// ═════════════════════════════════════════════════════════════════════════════
//  Constructor
// ═════════════════════════════════════════════════════════════════════════════

BotNavigationSettingsWindow::BotNavigationSettingsWindow(
    std::shared_ptr<WindowManager> wm,
    std::shared_ptr<cv::dnn::Net>  digitsNet,
    QWidget* parent
)
    : QWidget(parent), wm(wm), digitsNet(digitsNet)
{
    setAttribute(Qt::WA_DeleteOnClose);

    // Load data first — widgets may depend on it
    SettingsManager settingsManager;
    settings = settingsManager.loadSettings("assets/settings.json");

    setupUi();
    setupLayout();
    setupConnections();

    // Populate combo and find active zone
    for (const auto& nav : settings.navigationStructure)
        activeNavigationComboBox->addItem(QString::fromStdString(nav.name));

    for (int i = 0; i < (int)settings.navigationStructure.size(); ++i)
    {
        if (settings.navigationStructure[i].isActive)
        {
            currentZoneIndex = i;
            activeNavigationComboBox->setCurrentIndex(i);
            break;
        }
    }

    // Initial map + scroll list
    botNavigationMapView->syncCircles(currentZone());
    loadMapAtZoneOrigin();

    if (currentZone())
        populateScrollContent();
}

// ═════════════════════════════════════════════════════════════════════════════
//  Setup
// ═════════════════════════════════════════════════════════════════════════════

void BotNavigationSettingsWindow::setupUi()
{
    setWindowTitle("Navigation Settings");
    setFixedSize(300, 670);

    // ── Active navigation ─────────────────────────────────────────────────
    activeNavigationLabel    = new QLabel("Active Navigation");
    activeNavigationComboBox = new QComboBox();
    activeNavigationComboBox->setToolTip("Select the active navigation zone");

    // ── Map view ──────────────────────────────────────────────────────────
    botNavigationMapView = new BotNavigationMapView(this);
    botNavigationMapView->setMinimumSize(300, 300);
    botNavigationMapView->setCircleRadius(15);

    // ── Radio buttons ─────────────────────────────────────────────────────
    attackingArea  = new QRadioButton("Attacking Area");
    toGoWhenNoMobs = new QRadioButton("To go when no mobs");
    trajectory     = new QRadioButton("Trajectory");
    bypass         = new QRadioButton("Bypass");
    attackingArea->setChecked(true);

    attackingArea->setToolTip("Zone where the bot fights mobs (radius 15)");
    toGoWhenNoMobs->setToolTip("Position to walk to when no mobs are present (radius 5)");
    trajectory->setToolTip("Path waypoints for movement (radius 1)");
    bypass->setToolTip("Obstacle bypass points (radius 3)");

    selectionType = new QButtonGroup(this);
    selectionType->addButton(attackingArea);
    selectionType->addButton(toGoWhenNoMobs);
    selectionType->addButton(trajectory);
    selectionType->addButton(bypass);

    // ── Radius ────────────────────────────────────────────────────────────
    radiusLabel    = new QLabel("Radius:");
    radiusLineEdit = new QLineEdit("15.0");
    radiusLineEdit->setPlaceholderText("e.g. 15.0");
    radiusLineEdit->setToolTip("Circle radius for the next point (0 – 50)");

    QDoubleValidator* validator = new QDoubleValidator(0.0, 50.0, 1, radiusLineEdit);
    validator->setNotation(QDoubleValidator::StandardNotation);
    radiusLineEdit->setValidator(validator);

    // ── Buttons ───────────────────────────────────────────────────────────
    useCharPosButton = new QPushButton("Use character position");
    useCharPosButton->setToolTip("Add a point at the current in-game character position");

    saveButton = new QPushButton("Save");
    saveButton->setToolTip("Save all navigation settings to disk");

    newButton = new QPushButton("New");
    newButton->setToolTip("Create a new navigation zone");
}

void BotNavigationSettingsWindow::setupLayout()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(6);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    // ── Row: active navigation ────────────────────────────────────────────
    QHBoxLayout* activeNavLayout = new QHBoxLayout();
    activeNavLayout->addWidget(activeNavigationLabel);
    activeNavLayout->addWidget(activeNavigationComboBox);
    mainLayout->addLayout(activeNavLayout);

    // ── Map view ──────────────────────────────────────────────────────────
    mainLayout->addWidget(botNavigationMapView);

    // ── Radio buttons (2×2 grid) ──────────────────────────────────────────
    QGridLayout* radioLayout = new QGridLayout();
    radioLayout->addWidget(attackingArea,  0, 0);
    radioLayout->addWidget(toGoWhenNoMobs, 0, 1);
    radioLayout->addWidget(trajectory,     1, 0);
    radioLayout->addWidget(bypass,         1, 1);
    mainLayout->addLayout(radioLayout);

    // ── Row: radius ───────────────────────────────────────────────────────
    QHBoxLayout* radiusLayout = new QHBoxLayout();
    radiusLayout->addWidget(radiusLabel);
    radiusLayout->addWidget(radiusLineEdit);
    mainLayout->addLayout(radiusLayout);

    // ── Use character position ────────────────────────────────────────────
    mainLayout->addWidget(useCharPosButton);

    // ── Scrollable point list ─────────────────────────────────────────────
    QScrollArea* scrollArea    = new QScrollArea(this);
    QWidget*     scrollContent = new QWidget();
    pointsLayout               = new QVBoxLayout(scrollContent);

    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumHeight(160);
    pointsLayout->setAlignment(Qt::AlignTop);
    scrollArea->setWidget(scrollContent);

    mainLayout->addWidget(scrollArea);

    // ── Row: save / new ───────────────────────────────────────────────────
    QHBoxLayout* saveLayout = new QHBoxLayout();
    saveLayout->addWidget(saveButton);
    saveLayout->addWidget(newButton);
    mainLayout->addLayout(saveLayout);
}

void BotNavigationSettingsWindow::setupConnections()
{
    connect(botNavigationMapView, &BotNavigationMapView::circleListChanged,
            this, &BotNavigationSettingsWindow::onCircleListChanged);

    connect(activeNavigationComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BotNavigationSettingsWindow::onNavigationChanged);

    connect(saveButton,       &QPushButton::clicked,
            this, &BotNavigationSettingsWindow::onSave);

    connect(newButton,        &QPushButton::clicked,
            this, &BotNavigationSettingsWindow::onNewNavigation);

    connect(useCharPosButton, &QPushButton::clicked,
            this, &BotNavigationSettingsWindow::onUseCharPos);

    connect(radiusLineEdit,   &QLineEdit::textChanged,
            this, &BotNavigationSettingsWindow::onRadiusChanged);

    // Sync radius when point type changes
    connect(selectionType, &QButtonGroup::buttonClicked,
            this, [this](QAbstractButton*) {
                float r = defaultRadiusFor(selectedNavigationType());
                radiusLineEdit->setText(QString::number(r));
            });
}

// ═════════════════════════════════════════════════════════════════════════════
//  Helpers
// ═════════════════════════════════════════════════════════════════════════════

NavigationStructure* BotNavigationSettingsWindow::currentZone() const
{
    if (currentZoneIndex < 0 || currentZoneIndex >= (int)settings.navigationStructure.size())
        return nullptr;

    return const_cast<NavigationStructure*>(&settings.navigationStructure[currentZoneIndex]);
}

NavigationType BotNavigationSettingsWindow::selectedNavigationType() const
{
    QAbstractButton* btn = selectionType->checkedButton();
    if (btn == toGoWhenNoMobs) return NavigationType::TO_GO_WHEN_NO_MOBS;
    if (btn == trajectory)     return NavigationType::TRAJECTORY;
    if (btn == bypass)         return NavigationType::BYPASS;
    return NavigationType::ATTACKING_AREA;
}

float BotNavigationSettingsWindow::defaultRadiusFor(NavigationType type) const
{
    switch (type)
    {
        case NavigationType::ATTACKING_AREA:     return 15.f;
        case NavigationType::TO_GO_WHEN_NO_MOBS: return  5.f;
        case NavigationType::BYPASS:             return  3.f;
        case NavigationType::TRAJECTORY:         return  1.f;
    }
    return 15.f;
}

void BotNavigationSettingsWindow::addElementToCurrentZone(NavigationElement e)
{
    NavigationStructure* zone = currentZone();
    if (!zone) return;

    e.radius = radiusLineEdit->text().toFloat();

    switch (e.type)
    {
        case NavigationType::ATTACKING_AREA:     zone->attackingArea.push_back(e);   break;
        case NavigationType::TO_GO_WHEN_NO_MOBS: zone->toGoWhenNoMobs.push_back(e); break;
        case NavigationType::TRAJECTORY:         zone->trajectory.push_back(e);      break;
        case NavigationType::BYPASS:             zone->bypass.push_back(e);          break;
    }

    populateScrollContent();
    botNavigationMapView->syncCircles(zone);
}

void BotNavigationSettingsWindow::loadMapAtCharPos()
{
    cv::Mat screenRGB  = wm->getRgbScreenshot();
    auto    charPosImg = movement::convertToImgCoordinates(
                             movement::getCharPos(screenRGB, digitsNet));

    botNavigationMapView->loadImage(
        MAP_PATH,
        QPoint(charPosImg.first  - MAP_VIEW_HALF,
               charPosImg.second - MAP_VIEW_HALF));
}

void BotNavigationSettingsWindow::loadMapAtZoneOrigin()
{
    NavigationStructure* zone = currentZone();

    if (zone && !zone->attackingArea.empty())
    {
        const auto& p = zone->attackingArea[0].pos;
        botNavigationMapView->loadImage(
            MAP_PATH,
            QPoint(p.first  - MAP_VIEW_HALF,
                   p.second - MAP_VIEW_HALF));
    }
    else
    {
        loadMapAtCharPos();
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  Scroll content
// ═════════════════════════════════════════════════════════════════════════════

void BotNavigationSettingsWindow::populateScrollContent()
{
    while (QLayoutItem* item = pointsLayout->takeAt(0))
    {
        if (QWidget* w = item->widget())
            w->deleteLater();
        delete item;
    }

    NavigationStructure* zone = currentZone();
    if (!zone) return;

    auto addGroup = [&](const QString& label,
                        std::vector<NavigationElement>& elems,
                        NavigationType type)
    {
        for (int i = 0; i < (int)elems.size(); ++i)
            addPointItem(label, QPoint(elems[i].pos.first, elems[i].pos.second), type, i);
    };

    addGroup("Attacking",  zone->attackingArea,  NavigationType::ATTACKING_AREA);
    addGroup("No mobs",    zone->toGoWhenNoMobs, NavigationType::TO_GO_WHEN_NO_MOBS);
    addGroup("Trajectory", zone->trajectory,     NavigationType::TRAJECTORY);
    addGroup("Bypass",     zone->bypass,         NavigationType::BYPASS);
}

void BotNavigationSettingsWindow::addPointItem(
    const QString& name, const QPoint& pos, NavigationType type, int index)
{
    QWidget* itemWidget = new QWidget();
    itemWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    itemWidget->setStyleSheet("background-color:#2b2b2b; border-radius:6px;");

    QHBoxLayout* layout = new QHBoxLayout(itemWidget);
    layout->setContentsMargins(6, 2, 6, 2);
    layout->addWidget(new QLabel(name));
    layout->addStretch();
    layout->addWidget(new QLabel(QString("(%1, %2)").arg(pos.x()).arg(pos.y())));

    pointsLayout->addWidget(itemWidget);

    connect(itemWidget, &QWidget::customContextMenuRequested,
            this, [=](const QPoint& localPos)
    {
        QMenu menu;
        QAction* removeAction = menu.addAction("Remove");

        if (menu.exec(itemWidget->mapToGlobal(localPos)) != removeAction)
            return;

        NavigationStructure* zone = currentZone();
        if (!zone) return;

        auto eraseAt = [](std::vector<NavigationElement>& v, int i)
        {
            if (i >= 0 && i < (int)v.size())
                v.erase(v.begin() + i);
        };

        switch (type)
        {
            case NavigationType::ATTACKING_AREA:     eraseAt(zone->attackingArea,  index); break;
            case NavigationType::TO_GO_WHEN_NO_MOBS: eraseAt(zone->toGoWhenNoMobs, index); break;
            case NavigationType::TRAJECTORY:         eraseAt(zone->trajectory,     index); break;
            case NavigationType::BYPASS:             eraseAt(zone->bypass,         index); break;
        }

        populateScrollContent();
        botNavigationMapView->syncCircles(zone);
    });
}

// ═════════════════════════════════════════════════════════════════════════════
//  Slots
// ═════════════════════════════════════════════════════════════════════════════

void BotNavigationSettingsWindow::onCircleListChanged(const MapCircle c)
{
    NavigationElement e = c.nElement;
    e.type = selectedNavigationType();
    addElementToCurrentZone(e);
}

void BotNavigationSettingsWindow::onUseCharPos()
{
    cv::Mat screenRGB  = wm->getRgbScreenshot();
    auto    charPosImg = movement::convertToImgCoordinates(
                             movement::getCharPos(screenRGB, digitsNet));

    NavigationElement e;
    e.pos  = charPosImg;
    e.type = selectedNavigationType();
    addElementToCurrentZone(e);
}

void BotNavigationSettingsWindow::onNavigationChanged(int index)
{
    if (index < 0 || index >= (int)settings.navigationStructure.size()) return;

    currentZoneIndex = index;

    for (auto& nav : settings.navigationStructure)
        nav.isActive = false;
    settings.navigationStructure[index].isActive = true;

    populateScrollContent();
    botNavigationMapView->syncCircles(currentZone());
    loadMapAtZoneOrigin();
}

void BotNavigationSettingsWindow::onNewNavigation()
{
    QDialog    dialog(this);
    QLineEdit* nameEdit  = new QLineEdit();
    QPushButton* okBtn   = new QPushButton("Ok");
    QPushButton* cancelBtn = new QPushButton("Cancel");

    dialog.setWindowTitle("New Navigation");

    QVBoxLayout* layout    = new QVBoxLayout(&dialog);
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);

    nameEdit->setPlaceholderText("Zone name...");
    layout->addWidget(new QLabel("Name:"));
    layout->addWidget(nameEdit);
    layout->addLayout(btnLayout);

    connect(okBtn,     &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    const QString name = nameEdit->text().trimmed();
    if (name.isEmpty()) return;

    // Deactivate all, add new active zone
    for (auto& nav : settings.navigationStructure)
        nav.isActive = false;

    NavigationStructure newNav;
    newNav.name     = name.toStdString();
    newNav.isActive = true;
    settings.navigationStructure.push_back(newNav);

    // Update combo without triggering onNavigationChanged prematurely
    QSignalBlocker blocker(activeNavigationComboBox);
    activeNavigationComboBox->addItem(name);
    activeNavigationComboBox->setCurrentIndex(activeNavigationComboBox->count() - 1);

    currentZoneIndex = (int)settings.navigationStructure.size() - 1;

    populateScrollContent();
    loadMapAtCharPos();
}

void BotNavigationSettingsWindow::onSave()
{
    SettingsManager settingsManager;
    settingsManager.saveSettings(settings, "assets/settings.json");
}

void BotNavigationSettingsWindow::onRadiusChanged(const QString& text)
{
    botNavigationMapView->setCircleRadius(text.toFloat());
}