#include "kart_glwindow.h"

#include <QTimerEvent>
#include <QOpenGLExtraFunctions>
#include <QtImGui.h>
#include <imgui.h>

#include "kart_application.h"

KartGLWindow::KartGLWindow()
{
}

KartGLWindow::~KartGLWindow()
{
}

void KartGLWindow::initializeGL()
{
    QtImGui::initialize(this);
}

void KartGLWindow::paintGL()
{
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

    f->glDisable(GL_SCISSOR_TEST);
    f->glClearColor(0.25f, 0.25f, 0.375f, 1.0f);
    f->glClearDepthf(1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QtImGui::newFrame();
    KartApplication::get()->on_imgui();
    ImGui::Render();
}
