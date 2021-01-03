/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2008 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcapacitybar.h"
#include "kstyleextensions.h"

#include <math.h>

#include <QStyle>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QLinearGradient>
#include <QStyleOptionProgressBar>

#define ROUND_MARGIN     6
#define VERTICAL_SPACING 1

static const int LightShade = 100;
static const int MidShade = 200;
static const int DarkShade = 300;

class Q_DECL_HIDDEN KCapacityBar::Private
{
public:
    Private(KCapacityBar::DrawTextMode drawTextMode)
        : drawTextMode(drawTextMode) {}

    ~Private() {}

    QString text;
    int value = 0;
    bool fillFullBlocks = true;
    bool continuous = true;
    int barHeight = 12;
    Qt::Alignment horizontalTextAlignment = Qt::AlignCenter;
    QStyle::ControlElement ce_capacityBar = QStyle::ControlElement(0);

    KCapacityBar::DrawTextMode drawTextMode;
};

KCapacityBar::KCapacityBar(QWidget *parent)
    : KCapacityBar(DrawTextOutline, parent)
{
}

KCapacityBar::KCapacityBar(KCapacityBar::DrawTextMode drawTextMode, QWidget *parent)
    : QWidget(parent)
    , d(new Private(drawTextMode))
{
    d->ce_capacityBar = KStyleExtensions::customControlElement(QStringLiteral("CE_CapacityBar"), this);
}

KCapacityBar::~KCapacityBar() = default;

void KCapacityBar::setValue(int value)
{
    d->value = value;
    update();
}

int KCapacityBar::value() const
{
    return d->value;
}

void KCapacityBar::setText(const QString &text)
{
    bool updateGeom = d->text.isEmpty() || text.isEmpty();
    d->text = text;
    if (updateGeom) {
        updateGeometry();
    }

#ifndef QT_NO_ACCESSIBILITY
    setAccessibleName(text);
#endif

    update();
}

QString KCapacityBar::text() const
{
    return d->text;
}

void KCapacityBar::setFillFullBlocks(bool fillFullBlocks)
{
    d->fillFullBlocks = fillFullBlocks;
    update();
}

bool KCapacityBar::fillFullBlocks() const
{
    return d->fillFullBlocks;
}

void KCapacityBar::setContinuous(bool continuous)
{
    d->continuous = continuous;
    update();
}

bool KCapacityBar::continuous() const
{
    return d->continuous;
}

void KCapacityBar::setBarHeight(int barHeight)
{
    // automatically convert odd values to even. This will make the bar look
    // better.
    d->barHeight = (barHeight % 2) ? barHeight + 1 : barHeight;
    updateGeometry();
}

int KCapacityBar::barHeight() const
{
    return d->barHeight;
}

void KCapacityBar::setHorizontalTextAlignment(Qt::Alignment horizontalTextAlignment)
{
    Qt::Alignment alignment = horizontalTextAlignment;

    // if the value came with any vertical alignment flag, remove it.
    alignment &= ~Qt::AlignTop;
    alignment &= ~Qt::AlignBottom;
    alignment &= ~Qt::AlignVCenter;

    d->horizontalTextAlignment = alignment;
    update();
}

Qt::Alignment KCapacityBar::horizontalTextAlignment() const
{
    return d->horizontalTextAlignment;
}

void KCapacityBar::setDrawTextMode(DrawTextMode mode)
{
    d->drawTextMode = mode;
    update();
}

KCapacityBar::DrawTextMode KCapacityBar::drawTextMode() const
{
    return d->drawTextMode;
}

void KCapacityBar::drawCapacityBar(QPainter *p, const QRect &rect) const
{
    if (d->ce_capacityBar) {
        QStyleOptionProgressBar opt;
        opt.initFrom(this);
        opt.rect = rect;
        opt.minimum = 0;
        opt.maximum = 100;
        opt.progress = d->value;
        opt.state |= QStyle::State_Horizontal;
        opt.text = d->text;
        opt.textAlignment = Qt::AlignCenter;
        opt.textVisible = true;
        style()->drawControl(d->ce_capacityBar, &opt, p, this);

        return;
    }

    p->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    p->save();

    QRect drawRect(rect);

    if (d->drawTextMode == DrawTextOutline) {
        drawRect.setHeight(d->barHeight);
    }

    QPainterPath outline;
    outline.moveTo(rect.left() + ROUND_MARGIN / 4 + 1, rect.top());
    outline.lineTo(rect.left() + drawRect.width() - ROUND_MARGIN / 4 - 1, rect.top());
    outline.quadTo(rect.left() + drawRect.width() + ROUND_MARGIN / 2, drawRect.height() / 2 + rect.top(), rect.left() + drawRect.width() - ROUND_MARGIN / 4 - 1, drawRect.height() + rect.top());
    outline.lineTo(rect.left() + ROUND_MARGIN / 4 + 1, drawRect.height() + rect.top());
    outline.quadTo(-ROUND_MARGIN / 2 + rect.left(), drawRect.height() / 2 + rect.top(), rect.left() + ROUND_MARGIN / 4 + 1, rect.top());
    const QColor fillColor = palette().window().color().darker(DarkShade);
    p->fillPath(outline, QColor(fillColor.red(), fillColor.green(), fillColor.blue(), 50));

    QRadialGradient bottomGradient(QPointF(rect.width() / 2, drawRect.bottom() + 1), rect.width() / 2);
    bottomGradient.setColorAt(0, palette().window().color().darker(LightShade));
    bottomGradient.setColorAt(1, Qt::transparent);
    p->fillRect(QRect(rect.left(), drawRect.bottom() + rect.top(), rect.width(), 1), bottomGradient);

    p->translate(rect.left() + 2, rect.top() + 1);

    drawRect.setWidth(drawRect.width() - 4);
    drawRect.setHeight(drawRect.height() - 2);

    QPainterPath path;
    path.moveTo(ROUND_MARGIN / 4, 0);
    path.lineTo(drawRect.width() - ROUND_MARGIN / 4, 0);
    path.quadTo(drawRect.width() + ROUND_MARGIN / 2, drawRect.height() / 2, drawRect.width() - ROUND_MARGIN / 4, drawRect.height());
    path.lineTo(ROUND_MARGIN / 4, drawRect.height());
    path.quadTo(-ROUND_MARGIN / 2, drawRect.height() / 2, ROUND_MARGIN / 4, 0);

    QLinearGradient linearGradient(0, 0, 0, drawRect.height());
    linearGradient.setColorAt(0.5, palette().window().color().darker(MidShade));
    linearGradient.setColorAt(1, palette().window().color().darker(LightShade));
    p->fillPath(path, linearGradient);

    p->setBrush(Qt::NoBrush);
    p->setPen(Qt::NoPen);

    if (d->continuous || !d->fillFullBlocks) {
        int start = (layoutDirection() == Qt::LeftToRight) ? -1
                    : (drawRect.width() + 2) - (drawRect.width() + 2) * (d->value / 100.0);

        p->setClipRect(QRect(start, 0, (drawRect.width() + 2) * (d->value / 100.0), drawRect.height()), Qt::IntersectClip);
    }

    int left = (layoutDirection() == Qt::LeftToRight) ? 0
               : drawRect.width();

    int right = (layoutDirection() == Qt::LeftToRight) ? drawRect.width()
                : 0;

    int roundMargin = (layoutDirection() == Qt::LeftToRight) ? ROUND_MARGIN
                      : -ROUND_MARGIN;

    int spacing = 2;
    int verticalSpacing = VERTICAL_SPACING;
    int slotWidth = 6;
    int start = roundMargin / 4;

    QPainterPath internalBar;
    internalBar.moveTo(left + roundMargin / 4, 0);
    internalBar.lineTo(right - roundMargin / 4, 0);
    internalBar.quadTo(right + roundMargin / 2, drawRect.height() / 2, right - roundMargin / 4, drawRect.height());
    internalBar.lineTo(left + roundMargin / 4, drawRect.height());
    internalBar.quadTo(left - roundMargin / 2, drawRect.height() / 2, left + roundMargin / 4, 0);

    QLinearGradient fillInternalBar(left, 0, right, 0);
    fillInternalBar.setColorAt(0, palette().window().color().darker(MidShade));
    fillInternalBar.setColorAt(0.5, palette().window().color().darker(LightShade));
    fillInternalBar.setColorAt(1, palette().window().color().darker(MidShade));

    if (d->drawTextMode == KCapacityBar::DrawTextInline) {
        p->save();
        p->setOpacity(p->opacity() * 0.7);
    }

    if (!d->continuous) {
        int numSlots = (drawRect.width() - ROUND_MARGIN - ((slotWidth + spacing) * 2)) / (slotWidth + spacing);
        int stopSlot = floor((numSlots + 2) * (d->value / 100.0));

        int plusOffset = d->fillFullBlocks ? ((drawRect.width() - ROUND_MARGIN - ((slotWidth + spacing) * 2)) - (numSlots * (slotWidth + spacing))) / 2.0
                         : 0;

        if (!d->fillFullBlocks || stopSlot) {
            QPainterPath firstSlot;
            firstSlot.moveTo(left + roundMargin / 4, verticalSpacing);
            firstSlot.lineTo(left + slotWidth + roundMargin / 4 + plusOffset, verticalSpacing);
            firstSlot.lineTo(left + slotWidth + roundMargin / 4 + plusOffset, drawRect.height() - verticalSpacing);
            firstSlot.lineTo(left + roundMargin / 4, drawRect.height() - verticalSpacing);
            firstSlot.quadTo(left, drawRect.height() / 2, left + roundMargin / 4, verticalSpacing);
            p->fillPath(firstSlot, fillInternalBar);
            start += slotWidth + spacing + plusOffset;

            bool stopped = false;
            for (int i = 0; i < numSlots + 1; i++) {
                if (d->fillFullBlocks && (i == (stopSlot + 1))) {
                    stopped = true;
                    break;
                }
                p->fillRect(QRect(rect.left() + start, rect.top() + verticalSpacing, slotWidth, drawRect.height() - verticalSpacing * 2), fillInternalBar);
                start += slotWidth + spacing;
            }

            if (!d->fillFullBlocks || (!stopped && (stopSlot != (numSlots + 1)) && (stopSlot != numSlots))) {
                QPainterPath lastSlot;
                lastSlot.moveTo(start, verticalSpacing);
                lastSlot.lineTo(start, drawRect.height() - verticalSpacing);
                lastSlot.lineTo(start + slotWidth + plusOffset, drawRect.height() - verticalSpacing);
                lastSlot.quadTo(start + roundMargin, drawRect.height() / 2, start + slotWidth + plusOffset, verticalSpacing);
                lastSlot.lineTo(start, verticalSpacing);
                p->fillPath(lastSlot, fillInternalBar);
            }
        }
    } else {
        p->fillPath(internalBar, fillInternalBar);
    }

    if (d->drawTextMode == KCapacityBar::DrawTextInline) {
        p->restore();
    }

    p->save();
    p->setClipping(false);
    QRadialGradient topGradient(QPointF(rect.width() / 2, drawRect.top()), rect.width() / 2);
    const QColor fillTopColor = palette().window().color().darker(LightShade);
    topGradient.setColorAt(0, QColor(fillTopColor.red(), fillTopColor.green(), fillTopColor.blue(), 127));
    topGradient.setColorAt(1, Qt::transparent);
    p->fillRect(QRect(rect.left(), rect.top() + drawRect.top(), rect.width(), 2), topGradient);
    p->restore();

    p->save();
    p->setClipRect(QRect(-1, 0, rect.width(), drawRect.height() / 2), Qt::ReplaceClip);
    QLinearGradient glassGradient(0, -5, 0, drawRect.height());
    const QColor fillGlassColor = palette().base().color();
    glassGradient.setColorAt(0, QColor(fillGlassColor.red(), fillGlassColor.green(), fillGlassColor.blue(), 255));
    glassGradient.setColorAt(1, Qt::transparent);
    p->fillPath(internalBar, glassGradient);
    p->restore();

    p->restore();

    if (d->drawTextMode == KCapacityBar::DrawTextInline) {
        QRect rect(drawRect);
        rect.setHeight(rect.height() + 4);
        p->drawText(rect, Qt::AlignCenter, fontMetrics().elidedText(d->text, Qt::ElideRight, drawRect.width() - 2 * ROUND_MARGIN));
    } else {
        p->drawText(rect, Qt::AlignBottom | d->horizontalTextAlignment, fontMetrics().elidedText(d->text, Qt::ElideRight, drawRect.width()));
    }
}

QSize KCapacityBar::minimumSizeHint() const
{
    int width = fontMetrics().boundingRect(d->text).width() + ((d->drawTextMode == KCapacityBar::DrawTextInline) ? ROUND_MARGIN * 2 : 0);

    int height = (d->drawTextMode == KCapacityBar::DrawTextInline) ?
                 qMax(fontMetrics().height(), d->barHeight) :
                 (d->text.isEmpty() ? 0 : fontMetrics().height() + VERTICAL_SPACING * 2) + d->barHeight;

    if (height % 2) {
        height++;
    }

    return QSize(width, height);
}

void KCapacityBar::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setClipRect(event->rect());
    drawCapacityBar(&p, contentsRect());
    p.end();
}

void KCapacityBar::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::StyleChange) {
        d->ce_capacityBar = KStyleExtensions::customControlElement(QStringLiteral("CE_CapacityBar"), this);
    }
}
