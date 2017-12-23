#include "rpi_server.h"

#include <loguru.hpp> // todo: remove this
#include <unistd.h> // usleep
#include <dukglue/dukglue.h>

RpiServer* RpiServer::ms_inst = nullptr;

RpiServer::RpiServer(int in_port)
    : m_acceptor(m_io_service, asio_tcp::endpoint(asio_tcp::v4(), in_port))
    , m_sock_peer(nullptr)
    , m_duk_ctx(nullptr)
{
}

void RpiServer::startup()
{
    ms_inst = this;

    m_duk_ctx = duk_create_heap(nullptr, nullptr, nullptr, this, handle_duktap_fatal);

    m_gpio.startup(*this);
    m_v4l2.startup(*this);

    // script init
    init_commands();

    wait_for_connection();
}

bool RpiServer::update()
{
    m_io_service.poll();
    
    if (m_state.load() == state_connected)
    {
        receive_commands();

        send_keepalive();
    }

    m_v4l2.update();

    usleep(200);
    
    return m_state.load() != state_exit;
}

void RpiServer::shutdown()
{
    if (m_sock_peer)
        delete m_sock_peer;

    m_v4l2.shutdown();
    m_gpio.shutdown();

    duk_destroy_heap(m_duk_ctx);
    m_duk_ctx = nullptr;

    ms_inst = nullptr;
}

void RpiServer::wait_for_connection()
{
    if (m_sock_peer)
        delete m_sock_peer;
    
    m_state.store(state_disconnected);

    m_io_service.reset();

    usleep(10000);
    
    m_sock_peer = new asio_tcp::socket(m_io_service);
    m_acceptor.async_accept(*m_sock_peer, [this](asio_error_code_cref err) { 
        handle_accept(err);
    });

    m_last_time = Clock::now();

    LOG_F(INFO, "listening\n");
}

void RpiServer::receive_commands()
{
    asio::error_code err;
    size_t buf_size = sizeof(m_read_buf);
    if(size_t avail = m_sock_peer->available(err))
    {
        LOG_F(INFO, "received %d bytes\n", avail);

        m_out_arch.reset();

        while(avail > buf_size)
        {
            size_t recv = m_sock_peer->receive(asio::buffer(m_read_buf, buf_size), 0, err);

            if (!err)
            {
                for(size_t i = 0; i < recv; ++i)
                    m_out_arch.write(m_read_buf[i]);

                avail -= buf_size;
            }
            else
            {
                handle_error(err);
                return;
            }
        }

        if (avail > 0)
        {
            size_t recv = m_sock_peer->receive(asio::buffer(m_read_buf, avail), 0, err);
            if (!err)
            {
                for(size_t i = 0; i < recv; ++i)
                    m_out_arch.write(m_read_buf[i]);
            }
            else
            {
                handle_error(err);
                return;
            }
        }

        m_out_arch.write((uint8_t)0);

        const uint8_t* data_ptr;
        size_t data_size;
        m_out_arch.get_buffer(data_ptr, data_size);
        //LOG_F(INFO, "Recieved: %s\n", (const char*)data_ptr);
        //duk_eval_string_noresult(m_duk_ctx, (const char*)data_ptr);
        if (duk_peval_string_noresult(m_duk_ctx, (const char*)data_ptr) != 0)
        {
            LOG_F(ERROR, "command error: %s\n", duk_safe_to_string(m_duk_ctx, -1));
        }
    }
}

void RpiServer::send_keepalive()
{
    asio::error_code err;
    TimePt now = Clock::now();
    auto delta_t = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_time).count();

    if (delta_t > 5000)
    {
        unsigned char buf[4] = {0, 0, 0, 0};
        m_sock_peer->send(asio::buffer(buf), 0, err);

        if(err)
        {
            handle_error(err);
        }
        //LOG_F(INFO, "idle\n");
        m_last_time = now;
    }
}

bool RpiServer::send(const void* in_buf_ptr, size_t in_buf_size)
{
    if (m_sock_peer)
    {
        asio::error_code err;
        m_sock_peer->send(asio::buffer(in_buf_ptr, in_buf_size), 0, err);

        if(err)
        {
            handle_error(err);
            return false;
        }

        /*
        m_sock_peer->async_send(asio::buffer(in_buf_ptr, in_buf_size), [this](
            const asio::error_code& error, // Result of operation.
            std::size_t bytes_transferred  ){

            if (error)
                handle_error(error);
        });
        */

        return true;
    }

    return false;
}

void RpiServer::handle_accept(const asio::error_code& in_err)
{
    if(!in_err)
    {
        LOG_F(INFO, "connected\n");
        m_state.store(state_connected);
    }
    else
    {
        handle_error(in_err);
    }
}

void RpiServer::handle_error(const asio::error_code& in_err)
{
    LOG_F(ERROR, "error %s\n", in_err.message().c_str());

    wait_for_connection();
}

void RpiServer::init_commands()
{
    struct Cmds
    {
        static void exit()
        {
            if (!RpiServer::get())
                return;

            LOG_F(INFO, "exit requested\n");
            RpiServer::get()->exit();
        }

        static void print(const char* in_msg)
        {
            LOG_F(INFO, "%s\n", in_msg);
        }
    };
    
    dukglue_register_function(m_duk_ctx, Cmds::exit, "exit");
    dukglue_register_function(m_duk_ctx, Cmds::print, "print");
}

void RpiServer::handle_duktap_fatal(void* udata, const char* msg)
{
    RpiServer* thiz = (RpiServer*)udata;
    LOG_F(ERROR, "DUKTAP FATAL ERROR:\n%s\n", (msg ? msg : "(no message)"));
    thiz->exit();
}