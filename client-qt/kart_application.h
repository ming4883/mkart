#ifndef KART_APPLICATION_H
#define KART_APPLICATION_H

#include <QApplication>
#include "kart_glwindow.h"

class KartApplication : public QApplication
{
    Q_OBJECT

public:
    KartApplication(int &argc, char **argv);
    ~KartApplication();

    void init();

    static KartApplication* get();

    void on_imgui();

protected:
    void timerEvent(QTimerEvent *in_event) override;

private:
    QBasicTimer m_timer;
    KartGLWindow m_window;
};

#endif // KART_APPLICATION_H
