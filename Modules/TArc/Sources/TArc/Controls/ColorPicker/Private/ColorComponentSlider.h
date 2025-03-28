#pragma once

#include <QWidget>
#include <QPointer>

namespace DAVA
{
namespace TArc
{
class ValueSlider;
class GradientSlider;
class ColorComponentSlider : public QWidget
{
    Q_OBJECT

signals:
    void started(double);
    void changing(double);
    void changed(double);
    void canceled();

public:
    explicit ColorComponentSlider(QWidget* parent = NULL);

    void SetColorRange(const QColor& c1, const QColor& c2);
    void SetValue(double val);
    double GetValue() const;

    void SetValueRange(double min, double max);
    double GetMaxValue() const;

private:
    QPointer<ValueSlider> value;
    QPointer<GradientSlider> gradient;
};
}
}
