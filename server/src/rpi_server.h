#ifndef OV7670_SERVER_H
#define OV7670_SERVER_H

#include <asio.hpp>
#include <atomic>

#include "archive.h"
#include "timing.h"
#include "duktape.h"
#include "v4l2.h"
#include "gpio.h"

typedef duk_context* ScriptContext;

class RpiServer
{    
public:
    RpiServer(int in_port);

    static RpiServer* get() { return ms_inst; }

    void startup();
    bool update();
    void shutdown();

    void exit() {m_state.store(state_exit);}
    void* script_context() {return m_duk_ctx;}
    V4L2& v4l2() {return m_v4l2;}
    GPIO& gpio() {return m_gpio;}

    bool send(const void* in_buf_ptr, size_t in_buf_size);
    bool is_connected() const {return m_state.load() == state_connected;}

private:
    typedef asio::ip::tcp asio_tcp;
    typedef const asio::error_code& asio_error_code_cref;
    
    enum State
    {
        state_disconnected,
        state_connected,
        state_exit,
    };

    enum IOState
    {
        io_idle,
        io_sending,
        io_reading,
    };

    static RpiServer* ms_inst;
    
    uint8_t m_read_buf[8];
    MemOutputArchive m_out_arch;

    asio::io_service m_io_service;
    asio_tcp::acceptor m_acceptor;
    asio_tcp::socket* m_sock_peer;
    std::atomic<State> m_state;
    std::atomic<IOState> m_io_state;
    TimePt m_last_time;
    duk_context* m_duk_ctx;
    V4L2 m_v4l2;
    GPIO m_gpio;

    void wait_for_connection();
    void receive_commands();
    void send_keepalive();
    
    void handle_error(const asio::error_code& in_err);
    void handle_accept(const asio::error_code& in_err);

    void init_commands();
    static void handle_duktap_fatal(void* udata, const char* msg);
};

#endif  // OV7670_SERVER_H
