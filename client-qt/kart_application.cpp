#include "kart_application.h"
#include <QSurfaceFormat>

#include "fpv_view.h"

#include "remote_service.h"
#include "sensor_service.h"
#include "fpv_service.h"

FpvView g_fpv_view;

KartApplication::KartApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
}

KartApplication::~KartApplication()
{
    FpvService::shutdown();
    RemoteService::shutdown();
    SensorService::shutdown();
    m_timer.stop();
}

KartApplication* KartApplication::get()
{
    return qobject_cast<KartApplication*>(QCoreApplication::instance());
}

void KartApplication::init()
{    
    QSurfaceFormat format;
    format.setDepthBufferSize(24);

    QSurfaceFormat::setDefaultFormat(format);

    m_window.setMinimumSize(QSize(1280, 720));
    m_window.show();
    //m_window.showMaximized();

    SensorService::init();
    RemoteService::init();
    FpvService::init();

    m_timer.start(16, this);
}


void KartApplication::timerEvent(QTimerEvent *in_event)
{
    if (in_event->timerId() == m_timer.timerId())
    {
        // update services
        m_window.makeCurrent();

        SensorService::update();
        RemoteService::update();
        FpvService::update();

        m_window.doneCurrent();

        // trigger redraw
        m_window.update();
    }
    else
    {
        QApplication::timerEvent(in_event);
    }
}

void KartApplication::on_imgui()
{
    g_fpv_view.on_imgui();
}
