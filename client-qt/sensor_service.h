#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H


class SensorService
{
public:

    struct Readings
    {
        float rot_x_deg;
        float rot_y_deg;
        float rot_z_deg;

        float gyro_x_deg;
        float gyro_y_deg;
        float gyro_z_deg;
    };


    static void init();

    static void update();

    static void shutdown();

    static void get_readings(Readings& out_readings);

};

#endif // SENSOR_SERVICE_H
