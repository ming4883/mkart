#ifndef KART_GLWINDOW_H
#define KART_GLWINDOW_H

#include <QOpenGLWindow>
#include <QBasicTimer>

class KartGLWindow : public QOpenGLWindow
{
    Q_OBJECT

public:
    KartGLWindow();
    ~KartGLWindow();

    void initializeGL() override;

    void paintGL() override;
};

#endif // KART_GLWINDOW_H
