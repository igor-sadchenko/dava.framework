/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "ScrollAreaController.h"
#include "UI/UIScreenManager.h"

using namespace DAVA;

ScrollAreaController::ScrollAreaController(QObject* parent)
    : QObject(parent)
    , backgroundControl(new UIControl)
{
    backgroundControl->SetName("Background control of scroll area controller");
    ScopedPtr<UIScreen> davaUIScreen(new UIScreen());
    davaUIScreen->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    davaUIScreen->GetBackground()->SetColor(Color(0.3f, 0.3f, 0.3f, 1.0f));
    UIScreenManager::Instance()->RegisterScreen(0, davaUIScreen);
    UIScreenManager::Instance()->SetFirst(0);

    UIScreenManager::Instance()->GetScreen()->AddControl(backgroundControl);
}

void ScrollAreaController::SetNestedControl(DAVA::UIControl* arg)
{
    if (nullptr != nestedControl)
    {
        backgroundControl->RemoveControl(nestedControl);
    }
    nestedControl = arg;
    if (nullptr != nestedControl)
    {
        backgroundControl->AddControl(nestedControl);
        UpdateCanvasContentSize();
    }
}

void ScrollAreaController::SetMovableControl(DAVA::UIControl* arg)
{
    if (arg != movableControl)
    {
        movableControl = arg;
        UpdatePosition();
    }
}

void ScrollAreaController::AdjustScale(qreal newScale, QPointF mousePos)
{
    newScale = fmax(minScale, newScale);
    newScale = fmin(maxScale, newScale); //crop scale to 800
    if (scale == newScale)
    {
        return;
    }
    QPoint oldPos = position;
    float oldScale = scale;
    scale = newScale;
    UpdateCanvasContentSize();
    emit ScaleChanged(scale);

    if (oldScale == 0 || viewSize.width() <= 0 || viewSize.height() <= 0)
    {
        SetPosition(QPoint(0, 0));
        return;
    }

    QPoint absPosition = oldPos / oldScale;
    QPointF deltaMousePos = mousePos * (1 - newScale / oldScale);
    QPoint newPosition(absPosition.x() * scale - deltaMousePos.x(), absPosition.y() * scale - deltaMousePos.y());

    newPosition.setX(qBound(0, newPosition.x(), (canvasSize - viewSize).width()));
    newPosition.setY(qBound(0, newPosition.y(), (canvasSize - viewSize).height()));
    SetPosition(newPosition);
}

QSize ScrollAreaController::GetCanvasSize() const
{
    return canvasSize;
}

QSize ScrollAreaController::GetViewSize() const
{
    return viewSize;
}

QPoint ScrollAreaController::GetPosition() const
{
    return position;
}

qreal ScrollAreaController::GetScale() const
{
    return scale;
}

qreal ScrollAreaController::GetMinScale() const
{
    return minScale;
}

qreal ScrollAreaController::GetMaxScale() const
{
    return maxScale;
}

void ScrollAreaController::UpdateCanvasContentSize()
{
    Vector2 contentSize;
    if (nullptr != nestedControl)
    {
        const auto& gd = nestedControl->GetGeometricData();

        contentSize = gd.GetAABBox().GetSize() * scale;
    }
    Vector2 marginsSize(Margin * 2, Margin * 2);
    Vector2 tmpSize = contentSize + marginsSize;
    backgroundControl->SetSize(tmpSize);
    canvasSize = QSize(tmpSize.dx, tmpSize.dy);
    UpdatePosition();
    emit CanvasSizeChanged(canvasSize);
}

void ScrollAreaController::SetScale(qreal arg)
{
    if (scale != arg)
    {
        AdjustScale(arg, QPoint(viewSize.width() / 2, viewSize.height() / 2)); //like cursor at center of view
    }
}

void ScrollAreaController::SetViewSize(QSize viewSize_)
{
    if (viewSize_ != viewSize)
    {
        viewSize = viewSize_;
        auto newSize = Vector2(viewSize_.width(), viewSize_.height());
        UIScreenManager::Instance()->GetScreen()->SetSize(newSize);
        UpdatePosition();
        emit ViewSizeChanged(viewSize);
    }
}

void ScrollAreaController::SetPosition(QPoint position_)
{
    if (position_ != position)
    {
        position = position_;
        UpdatePosition();
        emit PositionChanged(position);
    }
}

void ScrollAreaController::UpdatePosition()
{
    if (nullptr != movableControl)
    {
        QSize offset = (canvasSize - viewSize) / 2;

        if (offset.width() > 0)
        {
            offset.setWidth(position.x());
        }
        if (offset.height() > 0)
        {
            offset.setHeight(position.y());
        }
        offset -= QSize(Margin, Margin);
        Vector2 position(-offset.width(), -offset.height());
        movableControl->SetPosition(position);

        QPoint positionPoint(static_cast<int>(position.x), static_cast<int>(position.y));
        NestedControlPositionChanged(positionPoint);
    }
}
