#include "Button.h"
#include <QPainter>
#include <QEvent>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

Button::Button(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
    , m_backgroundColor(QColor(42, 42, 42, 150)) // #2a2a2a
    , m_textColor(Qt::white)
    , m_hoverColor(QColor(42, 42, 42, 150).lighter(120))
    , m_pressedColor(QColor(42, 42, 42, 150).darker(120))
    , m_currentColor(m_backgroundColor)
    , m_borderRadius(5)
    , m_paddingHorizontal(20)
    , m_paddingVertical(12)
    , m_animationEnabled(true)
    , m_hoverAnimation(true)
    , m_clickAnimation(true)
    , m_isHovered(false)
    , m_isPressed(false)
{
    setupDefaultStyle();
}

Button::Button(const QString& text, const QColor& backgroundColor, 
                         const QColor& textColor, QWidget* parent)
    : QPushButton(text, parent)
    , m_backgroundColor(backgroundColor)
    , m_textColor(textColor)
    , m_hoverColor(backgroundColor.lighter(120))
    , m_pressedColor(backgroundColor.darker(120))
    , m_currentColor(m_backgroundColor)
    , m_borderRadius(5)
    , m_paddingHorizontal(20)
    , m_paddingVertical(12)
    , m_animationEnabled(true)
    , m_hoverAnimation(true)
    , m_clickAnimation(true)
    , m_isHovered(false)
    , m_isPressed(false)
{
    setupDefaultStyle();
}

void Button::setupDefaultStyle()
{
    setMinimumHeight(40);
    setCursor(Qt::PointingHandCursor);
    updateStyle();
}

void Button::setCustomStyle(const QColor& backgroundColor, const QColor& textColor)
{
    m_backgroundColor = backgroundColor;
    m_textColor = textColor;
    m_hoverColor = backgroundColor.lighter(120);
    m_pressedColor = backgroundColor.darker(120);
    m_currentColor = m_backgroundColor;
    updateStyle();
}

void Button::setHoverColor(const QColor& hoverColor)
{
    m_hoverColor = hoverColor;
}

void Button::setPressedColor(const QColor& pressedColor)
{
    m_pressedColor = pressedColor;
}

void Button::setBorderRadius(int radius)
{
    m_borderRadius = radius;
    updateStyle();
}

void Button::setPadding(int horizontal, int vertical)
{
    m_paddingHorizontal = horizontal;
    m_paddingVertical = vertical;
    updateStyle();
}

void Button::setAnimationEnabled(bool enabled)
{
    m_animationEnabled = enabled;
}

void Button::setHoverAnimation(bool enabled)
{
    m_hoverAnimation = enabled;
}

void Button::setClickAnimation(bool enabled)
{
    m_clickAnimation = enabled;
}

void Button::enterEvent(QEnterEvent* event)
{
    QPushButton::enterEvent(event);
    m_isHovered = true;
    
    if (m_animationEnabled && m_hoverAnimation) {
        QPropertyAnimation* animation = new QPropertyAnimation(this, "currentColor");
        animation->setDuration(150);
        animation->setStartValue(m_currentColor);
        animation->setEndValue(m_hoverColor);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        m_currentColor = m_hoverColor;
        update();
    }
}

void Button::leaveEvent(QEvent* event)
{
    QPushButton::leaveEvent(event);
    m_isHovered = false;
    
    if (m_animationEnabled && m_hoverAnimation) {
        QPropertyAnimation* animation = new QPropertyAnimation(this, "currentColor");
        animation->setDuration(150);
        animation->setStartValue(m_currentColor);
        animation->setEndValue(m_backgroundColor);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        m_currentColor = m_backgroundColor;
        update();
    }
}

void Button::mousePressEvent(QMouseEvent* event)
{
    QPushButton::mousePressEvent(event);
    m_isPressed = true;
    
    if (m_animationEnabled && m_clickAnimation) {
        QPropertyAnimation* animation = new QPropertyAnimation(this, "currentColor");
        animation->setDuration(100);
        animation->setStartValue(m_currentColor);
        animation->setEndValue(m_pressedColor);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        m_currentColor = m_pressedColor;
        update();
    }
}

void Button::mouseReleaseEvent(QMouseEvent* event)
{
    QPushButton::mouseReleaseEvent(event);
    m_isPressed = false;
    
    if (m_animationEnabled && m_clickAnimation) {
        QPropertyAnimation* animation = new QPropertyAnimation(this, "currentColor");
        animation->setDuration(100);
        animation->setStartValue(m_currentColor);
        animation->setEndValue(m_isHovered ? m_hoverColor : m_backgroundColor);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        m_currentColor = m_isHovered ? m_hoverColor : m_backgroundColor;
        update();
    }
}

void Button::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QRect rect = this->rect();
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_currentColor);
    painter.drawRoundedRect(rect, m_borderRadius, m_borderRadius);
    painter.setPen(m_textColor);
    painter.setFont(font());
    QRect textRect = rect.adjusted(m_paddingHorizontal, m_paddingVertical, 
                                  -m_paddingHorizontal, -m_paddingVertical);
    painter.drawText(textRect, Qt::AlignCenter, text());
}

void Button::updateStyle()
{
    update();
}

QColor Button::currentColor() const
{
    return m_currentColor;
}

void Button::setCurrentColor(const QColor& color)
{
    m_currentColor = color;
    update();
} 