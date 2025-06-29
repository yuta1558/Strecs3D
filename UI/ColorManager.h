#ifndef COLORMANAGER_H
#define COLORMANAGER_H

#include <QColor>

class ColorManager {
public:
    // Stress gradient colors
    static const QColor HIGH_COLOR;    // #FF4D00 (red)
    static const QColor MIDDLE_COLOR;  // #FFFFFF (white)
    static const QColor LOW_COLOR;     // #004ca1 (blue)
    
    // Background color
    static const QColor BACKGROUND_COLOR; // #1a1a1a (dark gray)
};

#endif // COLORMANAGER_H 