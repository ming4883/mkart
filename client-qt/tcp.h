#ifndef TCP_H
#define TCP_H

#include <QObject>
// TODO: https://stackoverflow.com/questions/25250171/how-to-use-the-qts-pimpl-idiom
class TcpPrivate;

class Tcp
{
    Q_DECLARE_PRIVATE(Tcp)
    TcpPrivate* d_ptr;

public:
    enum State
    {
        state_disconnected,
        state_connecting,
        state_connected,
    };

    enum Result
    {
        ret_succeeded,
        ret_err_not_connected,
        ret_err_invalid_argument,
        ret_err_unknown,
    };


    Tcp();
    ~Tcp();

    void init();
    void update();
    void shutdown();

    Tcp::State get_state();

    // todo std::error_code
    Tcp::Result disconnect();

    Tcp::Result connect(unsigned char in_ip[4], int in_port);

    // send a raw buffer specified by in_buffer_sz.
    Tcp::Result send_raw(const void* in_buffer_ptr, unsigned int in_buffer_sz);

    // read a message
    bool read_message(void*& out_message_ptr, unsigned int& out_message_sz);

};

#endif // TCP_H
