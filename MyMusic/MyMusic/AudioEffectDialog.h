#pragma once

#include <QDialog>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QPainter>
#include <QMouseEvent>
#include <QVector>

// --- Custom Components ---

// 1. Toggle Switch (Same as before)
class SwitchButton : public QWidget {
    Q_OBJECT
public:
    explicit SwitchButton(QWidget* parent = nullptr);
    void setChecked(bool check);
protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    QSize sizeHint() const override { return QSize(50, 26); }
private:
    bool _checked = false;
};

// 2. Vertical EQ Band Widget (Label + Slider + Freq)
class EQBandWidget : public QWidget {
    Q_OBJECT
public:
    EQBandWidget(const QString& freqText, QWidget* parent = nullptr);
    void setValue(int value);
signals:
    void valueChanged(int val);
private:
    QLabel* _valLabel;
    QSlider* _slider;
    QLabel* _freqLabel;
};

// 3. Horizontal Parameter Widget (Title + Slider + Value)
class ParamControlWidget : public QWidget {
    Q_OBJECT
public:
    enum ParamType {
        Base,               // Base form
        Strength            // environment strenght form
    };
    ParamControlWidget(const QString& title, ParamType type, QWidget* parent = nullptr);
    void setValue(int value);
signals:
    void valueChanged(int val);
private:
    QLabel* _titleLabel;
    QLabel* _valLabel;
    QSlider* _slider;
};

// --- Main Dialog ---

class AudioEffectDialog : public QDialog {
    Q_OBJECT
public:
    enum PresetEQ {
        Custom = 0,
        Null,
        Pop,
        Dance,
        Blues,
        Classical,
        Jazz,
        SlowSong,
        ElectronicMusic,
        Rock,
        Country,
        ACG,
        Bass,
        MegaBass,
        BassTrable,
        Speaker,
        Live,
        Mid,
        Soft,
        SoftBass,
        SoftTrable,
        HeavyMetal,
        NationalCustoms,
        Ballad,
        Rap
    };
    explicit AudioEffectDialog(QWidget* parent = nullptr);
    ~AudioEffectDialog() = default;

    void resetParamControls();

    void setPresetEQ(int preset);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void showEvent(QShowEvent* event) override;

signals:
    void EQValueChanged(int index, int value);
    void envComboChanged(int index);
    void envDepthChanged(int value);
    void envIntensityChanged(int value);
    void bassChanged(int value);
    void TrableChanged(int value);
private:
    void setupUI();
    void setupStyles();
    void setEQValues(QVector<int> values);

    // Helper to create the top Tab buttons
    QPushButton* createTabButton(const QString& text, int id);

    // Helper to build the Equalizer Page content
    QWidget* createEqualizerPage();

    QPoint _dragPosition;
    QStackedWidget* _stack;
    QButtonGroup* _tabGroup;

    QVector<EQBandWidget*> _eqWidgets;
    QVector<int> _eqValues;
    ParamControlWidget* _bassWidget;
    ParamControlWidget* _trableWidget;
    ParamControlWidget* _envDepth;
    ParamControlWidget* _envIntensity;

    bool _isProgrammaticChange = true;
};

