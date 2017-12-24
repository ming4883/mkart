#include "sensor_service.h"


#include <QGyroscope>
#include <QRotationSensor>

class SensorContext
{
public:

    QGyroscope m_gyroscope;
    QRotationSensor m_rot_sensor;

    SensorContext()
    {
    }

    void init()
    {
        m_gyroscope.start();
        m_rot_sensor.start();

    }

    void update()
    {

    }

    void shutdown()
    {
        m_gyroscope.stop();
        m_rot_sensor.stop();
    }
};

SensorContext g_sensor_context;

void SensorService::init()
{
    g_sensor_context.init();
}

void SensorService::update()
{
    g_sensor_context.update();
}

void SensorService::shutdown()
{
    g_sensor_context.shutdown();
}

void SensorService::get_readings(Readings& out_readings)
{
    auto gyro = g_sensor_context.m_gyroscope.reading();

    if (gyro)
    {
        out_readings.gyro_x_deg = (float)gyro->x();
        out_readings.gyro_y_deg = (float)gyro->y();
        out_readings.gyro_z_deg = (float)gyro->z();
    }
    else
    {
        out_readings.gyro_x_deg = 0;
        out_readings.gyro_y_deg = 0;
        out_readings.gyro_z_deg = 0;
    }

    auto rot = g_sensor_context.m_rot_sensor.reading();

    if (rot)
    {
        out_readings.rot_x_deg = (float)rot->x();
        out_readings.rot_y_deg = (float)rot->y();
        out_readings.rot_z_deg = (float)rot->z();
    }
    else
    {
        out_readings.rot_x_deg = 0;
        out_readings.rot_y_deg = 0;
        out_readings.rot_z_deg = 0;
    }
}

