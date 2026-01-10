#include "AudioEffectDialog.h"
#include <QApplication>

// --- SwitchButton ---
SwitchButton::SwitchButton(QWidget* parent) : QWidget(parent) {
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}
void SwitchButton::setChecked(bool check) {
    if (_checked != check) {
        _checked = check;
        update();
    }
}
void SwitchButton::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) setChecked(!_checked);
}
void SwitchButton::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRect rect(0, 0, width(), height());
    p.setPen(Qt::NoPen);
    p.setBrush(_checked ? QColor("#FF3333") : QColor("#555555"));
    p.drawRoundedRect(rect, height() / 2, height() / 2);
    int margin = 3;
    int circleSize = height() - (margin * 2);
    int x = _checked ? (width() - circleSize - margin) : margin;
    p.setBrush(Qt::white);
    p.drawEllipse(x, margin, circleSize, circleSize);
}

// --- EQBandWidget (Vertical) ---
EQBandWidget::EQBandWidget(const QString& freqText, QWidget* parent) : QWidget(parent) {
    setMinimumHeight(200);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    _valLabel = new QLabel("0dB");
    _valLabel->setAlignment(Qt::AlignCenter);
    _valLabel->setStyleSheet("color: #FF3333; font-size: 10px;");

    _slider = new QSlider(Qt::Vertical);
    _slider->setRange(-12, 12);
    _slider->setValue(0);
    
    _freqLabel = new QLabel(freqText);
    _freqLabel->setAlignment(Qt::AlignCenter);
    _freqLabel->setStyleSheet("color: #888888; font-size: 11px;");

    layout->addWidget(_valLabel);
    layout->addWidget(_slider);
    layout->addWidget(_freqLabel);
    layout->setAlignment(Qt::AlignCenter);

    connect(_slider, &QSlider::valueChanged, [this](int val) {
        _valLabel->setText(QString("%1dB").arg(val));
        emit valueChanged(val);
    });
}

// --- ParamControlWidget (Horizontal) ---
ParamControlWidget::ParamControlWidget(const QString& title, ParamType type, QWidget* parent) : QWidget(parent) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 10, 20, 10);
    mainLayout->setSpacing(5);

    // Slider Row
    QHBoxLayout* sliderRow = new QHBoxLayout();
    QLabel* minLab = new QLabel("-");
    QLabel* maxLab = new QLabel("+");
    _slider = new QSlider(Qt::Horizontal);
    if (type == ParamType::Base) {
        _slider->setRange(-10, 10);
        _slider->setValue(0);
    }
    else {
        _slider->setRange(0, 10);
        _slider->setValue(0);
    }
    connect(_slider, &QSlider::valueChanged, this, &ParamControlWidget::valueChanged);
    sliderRow->addWidget(minLab);
    sliderRow->addWidget(_slider);
    sliderRow->addWidget(maxLab);

    // Title & Value Row
    QHBoxLayout* textRow = new QHBoxLayout();
    _titleLabel = new QLabel(title);
    _valLabel = new QLabel("0");
    _valLabel->setStyleSheet("color: #FF3333; font-weight: bold;");

    textRow->addStretch();
    textRow->addWidget(_titleLabel);
    textRow->addSpacing(10);
    textRow->addWidget(_valLabel); // Value sits next to title or centered
    textRow->addStretch();

    mainLayout->addLayout(sliderRow);
    mainLayout->addLayout(textRow);

    connect(_slider, &QSlider::valueChanged, [this](int val) {
        _valLabel->setText(QString("%1").arg(val));
    });
}

// --- AudioEffectDialog ---

AudioEffectDialog::AudioEffectDialog(QWidget* parent) : QDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(800, 550);
    setupUI();
    setupStyles();
}

void AudioEffectDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);

    // --- 1. Header (Tabs + Switch + Close) ---
    QHBoxLayout* headerLayout = new QHBoxLayout();

    _tabGroup = new QButtonGroup(this);
    _tabGroup->setExclusive(true);

    headerLayout->addWidget(createTabButton("音效", 0)); // Sound Effect
    headerLayout->addSpacing(10);
    headerLayout->addWidget(createTabButton("设备适配", 1)); // Device Match
    headerLayout->addSpacing(10);
    headerLayout->addWidget(createTabButton("均衡器", 2)); // Equalizer
    headerLayout->addStretch();

    // Global toggle switch
    QLabel* switchLabel = new QLabel("音效: 均衡器");
    SwitchButton* switchBtn = new SwitchButton(this);
    switchBtn->setChecked(true);

    // Close Button
    QPushButton* closeBtn = new QPushButton("✕");
    closeBtn->setObjectName("closeBtn");
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    headerLayout->addWidget(switchLabel);
    headerLayout->addSpacing(10);
    headerLayout->addWidget(switchBtn);
    headerLayout->addSpacing(15);
    headerLayout->addWidget(closeBtn);

    mainLayout->addLayout(headerLayout);

    // --- 2. Stacked Content (Switchable Views) ---
    _stack = new QStackedWidget(this);

    // Page 0: Placeholder
    QLabel* p0 = new QLabel("Sound Effect Module (Not Implemented)");
    p0->setAlignment(Qt::AlignCenter);
    _stack->addWidget(p0);

    // Page 1: Placeholder
    QLabel* p1 = new QLabel("Device Match Module (Not Implemented)");
    p1->setAlignment(Qt::AlignCenter);
    _stack->addWidget(p1);

    // Page 2: Equalizer (Wrapped in Scroll Area)
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidget(createEqualizerPage()); // <--- The Scrollable Content

    _stack->addWidget(scrollArea);

    mainLayout->addWidget(_stack);

    // Set Default Tab
    _tabGroup->button(2)->setChecked(true);
    _stack->setCurrentIndex(2);
}

QPushButton* AudioEffectDialog::createTabButton(const QString& text, int id) {
    QPushButton* btn = new QPushButton(text);
    btn->setCheckable(true);
    btn->setObjectName("tabButton");
    btn->setCursor(Qt::PointingHandCursor);
    _tabGroup->addButton(btn, id);

    // Switch stack page when clicked
    connect(btn, &QPushButton::toggled, [this, id](bool checked) {
        if (checked) _stack->setCurrentIndex(id);
    });
    return btn;
}

QWidget* AudioEffectDialog::createEqualizerPage() {
    QWidget* container = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 10, 0); // Right margin for scrollbar space
    layout->setSpacing(20);

    // --- Presets Row ---
    QHBoxLayout* presetLayout = new QHBoxLayout();
    QLabel* presetLabel = new QLabel("均衡器");
    QComboBox* presetCombo = new QComboBox();
    presetCombo->addItems({ "无", "Custom", "Rock", "Pop", "Jazz" });
    QPushButton* saveBtn = new QPushButton("保存");
    saveBtn->setFixedSize(60, 28);

    presetLayout->addWidget(presetLabel);
    presetLayout->addWidget(presetCombo);
    presetLayout->addWidget(saveBtn);
    presetLayout->addStretch();
    layout->addLayout(presetLayout);

    // --- EQ Sliders ---
    QHBoxLayout* eqContainer = new QHBoxLayout();

    // dB Grid Labels
    QVBoxLayout* dbLabels = new QVBoxLayout();
    dbLabels->setContentsMargins(0, 0, 0, 0); // Align with sliders area
    QLabel* db12 = new QLabel("+12dB");
    db12->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    dbLabels->addWidget(db12);
    dbLabels->addStretch();
    auto db0 = new QLabel("0dB");
    db0->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    dbLabels->addWidget(db0);
    dbLabels->addStretch();
    auto dbnegative12 = new QLabel("-12dB");
    dbnegative12->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    dbLabels->addWidget(dbnegative12);
    //dbLabels->addSpacing(25); // Space for freq label alignment
    eqContainer->addLayout(dbLabels);

    // Bands
    QStringList freqs = { "31", "63", "125", "250", "500", "1k", "2k", "4k", "8k", "16k" };
    int index = 0;
    for (const QString& freq : freqs) {
        EQBandWidget* widget = new EQBandWidget(freq);
        connect(widget, &EQBandWidget::valueChanged, [this, index](int val) {
            emit EQValueChanged(index, val);
        });
        eqContainer->addWidget(widget);
        ++index;
    }
    layout->addLayout(eqContainer);

    // --- Bottom Controls ---
    QHBoxLayout* controlsHeader = new QHBoxLayout();
    controlsHeader->addWidget(new QLabel("增强音效"));
    QPushButton* resetBtn = new QPushButton("重置");
    resetBtn->setFixedSize(60, 28);
    controlsHeader->addWidget(resetBtn);
    controlsHeader->addStretch();
    layout->addLayout(controlsHeader);

    QGridLayout* slidersGrid = new QGridLayout();
    ParamControlWidget* bassWidget = new ParamControlWidget("低音", ParamControlWidget::Base);
    slidersGrid->addWidget(bassWidget, 0, 0);
    connect(bassWidget, &ParamControlWidget::valueChanged, this, &AudioEffectDialog::bassChanged);

    ParamControlWidget* trableWidget = new ParamControlWidget("高音", ParamControlWidget::Base);
    slidersGrid->addWidget(trableWidget, 0, 1);
    connect(bassWidget, &ParamControlWidget::valueChanged, this, &AudioEffectDialog::TrableChanged);

    ParamControlWidget* envDepth = new ParamControlWidget("环绕深度", ParamControlWidget::Base);
    slidersGrid->addWidget(envDepth, 1, 0);
    connect(envDepth, &ParamControlWidget::valueChanged, this, &AudioEffectDialog::envDepthChanged);

    ParamControlWidget* envIntensity = new ParamControlWidget("环绕强度", ParamControlWidget::Strength);
    slidersGrid->addWidget(envIntensity, 1, 1);
    connect(envIntensity, &ParamControlWidget::valueChanged, this, &AudioEffectDialog::envIntensityChanged);
    layout->addLayout(slidersGrid);

    // --- Env Dropdown ---
    QHBoxLayout* envLayout = new QHBoxLayout();
    QComboBox* envCombo = new QComboBox();
    envCombo->addItems({ "无", "浴室", "教堂", "演唱会", "大厅", "房间", "音乐厅", "声乐板", "弹簧", "精神错乱", "地下通道"});
    envLayout->addWidget(new QLabel("环境音效"));
    envLayout->addWidget(envCombo);
    envLayout->addStretch();
    layout->addLayout(envLayout);

    connect(envCombo, &QComboBox::currentIndexChanged, this, &AudioEffectDialog::envComboChanged);

    return container;
}

void AudioEffectDialog::setupStyles() {
    this->setStyleSheet(R"(
        QWidget {
            background-color: #141416;
            color: white;
            font-family: 'Segoe UI', sans-serif;
            font-size: 14px;
        }
        
        /* Tab Buttons */
        QPushButton#tabButton {
            background: transparent;
            border: none;
            color: #888888;
            font-size: 15px;
            padding: 5px;
        }
        QPushButton#tabButton:checked {
            color: white;
            font-weight: bold;
            border-bottom: 2px solid #FF3333;
        }
        QPushButton#tabButton:hover { color: #DDDDDD; }

        /* Close Button */
        QPushButton#closeBtn {
            background: transparent; border: none; font-size: 16px; color: #888;
        }
        QPushButton#closeBtn:hover { color: white; }

        /* Scroll Area */
        QScrollArea { background: transparent; border: none; }
        QScrollBar:vertical {
            border: none; background: #222; width: 6px; border-radius: 3px;
        }
        QScrollBar::handle:vertical {
            background: #444; border-radius: 3px;
        }

        /* ComboBox & Standard Buttons */
        QComboBox {
            background-color: #2D2D30; border: none; border-radius: 14px;
            padding: 5px 15px; min-width: 80px;
        }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView {
            background-color: #2D2D30; selection-background-color: #FF3333;
        }
        
        QPushButton {
            background-color: #2D2D30; border: 1px solid #3E3E42;
            border-radius: 14px; color: #BBBBBB;
        }
        QPushButton:hover { background-color: #3E3E42; }

        /* Horizontal Slider */
        QSlider::groove:horizontal {
            border: 1px solid #333; height: 4px; background: #333;
            margin: 2px 0; border-radius: 2px;
        }
        QSlider::handle:horizontal {
            background: white; border: 1px solid #5c5c5c;
            width: 14px; height: 14px; margin: -6px 0; border-radius: 7px;
        }
        QSlider::sub-page:horizontal { background: #FF3333; border-radius: 2px; }

        /* Vertical Slider */
        QSlider::groove:vertical {
            border: 1px solid #333; width: 5px; background: #FF3333;
            border-radius: 2px;
        }
        QSlider::handle:vertical {
            background: white; border: 1px solid #5c5c5c;
            height: 14px; width: 14px; margin: 0 -6px; border-radius: 7px;
        }
        QSlider::sub-page:vertical {
            background: #333; border-radius: 2px;
            width: 6px;
        }
    )");
}

// --- Window Logic ---
void AudioEffectDialog::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QColor("#141416"));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect(), 10, 10);
}
void AudioEffectDialog::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        _dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}
void AudioEffectDialog::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - _dragPosition);
        event->accept();
    }
}
void AudioEffectDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    if (parentWidget()) move(parentWidget()->geometry().center() - rect().center());
}