#include "remote_service.h"

#include <kart_commands.h>

class RemoteContext
{
public:
    Tcp m_tcp;

    MemOutputArchive m_out_archive;
};

RemoteContext g_remote_context;

void RemoteService::init()
{
    g_remote_context.m_tcp.init();
}

void RemoteService::update()
{
    g_remote_context.m_tcp.update();
}

void RemoteService::shutdown()
{
    g_remote_context.m_tcp.shutdown();
}

Tcp::State RemoteService::get_state()
{
    return g_remote_context.m_tcp.get_state();
}

Tcp::Result RemoteService::disconnect()
{
    return g_remote_context.m_tcp.disconnect();
}

Tcp::Result RemoteService::connect(unsigned char in_ip[4], int in_port)
{
    return g_remote_context.m_tcp.connect(in_ip, in_port);
}

Tcp::Result RemoteService::send_cmd(const Cmd& in_cmd)
{
    auto& archive = g_remote_context.m_out_archive;

    archive.reset();

    archive.write((uint8_t)0); // size
    archive.write((uint8_t)in_cmd.get_id()); // id
    in_cmd.pack(archive);

    const uint8_t* buf;
    size_t sz;
    archive.get_buffer(buf, sz);

    ((uint8_t*)buf)[0] = (uint8_t)(sz & 0xff);

    //LOG_INFO("sending {:02x}, {:02x}, {:02x} ... {:02x}, {:02x}, {:02x}", buf[0], buf[1], buf[2], buf[sz-3], buf[sz-2], buf[sz-1]);

    return g_remote_context.m_tcp.send_raw(buf, sz);
}
