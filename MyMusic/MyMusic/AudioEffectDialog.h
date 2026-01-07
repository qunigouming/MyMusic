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
    ParamControlWidget(const QString& title, QWidget* parent = nullptr);
private:
    QLabel* _titleLabel;
    QLabel* _valLabel;
    QSlider* _slider;
};

// --- Main Dialog ---

class AudioEffectDialog : public QDialog {
    Q_OBJECT
public:
    explicit AudioEffectDialog(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void showEvent(QShowEvent* event) override;

signals:
    void EQValueChanged(int index, int value);
    void envComboChanged(int index);
private:
    void setupUI();
    void setupStyles();

    // Helper to create the top Tab buttons
    QPushButton* createTabButton(const QString& text, int id);

    // Helper to build the Equalizer Page content
    QWidget* createEqualizerPage();

    QPoint _dragPosition;
    QStackedWidget* _stack;
    QButtonGroup* _tabGroup;
};

