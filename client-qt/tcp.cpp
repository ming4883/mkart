#include "tcp.h"

#include <atomic>
#include <stdlib.h>
#include <QtNetwork>
#include <vector>

class TcpPrivate : public QObject
{
    Q_DISABLE_COPY(TcpPrivate)
    Q_DECLARE_PUBLIC(Tcp)
    Tcp* q_ptr;

    QTcpSocket* m_sock;
    std::atomic<Tcp::State> m_state;
    std::vector<quint8> m_message_buffer;
    std::atomic<qint64> m_message_remain;
    std::atomic<bool> m_message_ready;

    TcpPrivate(Tcp* i_thiz) :
        q_ptr(i_thiz),
        m_sock(nullptr),
        m_state(Tcp::state_disconnected),
        m_message_remain(0),
        m_message_ready(false)
    {
    }

    void init()
    {
        m_state.store(Tcp::state_disconnected);
        m_message_ready.store(false);
        m_message_remain.store(0);

        m_sock = new QTcpSocket();
        connect(m_sock, &QTcpSocket::connected, this, &TcpPrivate::handle_connected);
        connect(m_sock, &QTcpSocket::disconnected, this, &TcpPrivate::handle_disconnected);
        connect(m_sock, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &TcpPrivate::handle_error);
    }

    void shutdown()
    {
        delete m_sock;
    }

    void handle_connected()
    {
        qDebug() << "Tcp connected";
        //m_state.store(Tcp::state_connected);
    }

    void handle_disconnected()
    {
        qDebug() << "Tcp disconnected";
        //m_state.store(Tcp::state_disconnected);
    }

    void handle_error(QAbstractSocket::SocketError sock_err)
    {
        qDebug() << "Tcp error: " << sock_err;
        //m_state.store(Tcp::state_disconnected);
    }

    void update()
    {
        if(!m_sock)
            return;

        auto state = m_sock->state();

        switch(state)
        {
        case QTcpSocket::ConnectingState:
            m_state.store(Tcp::state_connecting);
            break;
        case QTcpSocket::ConnectedState:
            m_state.store(Tcp::state_connected);
            break;
        default:
            m_state.store(Tcp::state_disconnected);
            break;
        }

        read_message();
    }

    void read_message()
    {
        if(m_state.load() == Tcp::state_connected)
        {
            auto bytesAvailable = m_sock->bytesAvailable();

            if (bytesAvailable == 0)
                return;

            if (m_message_remain.load() > 0)
            {
                qint64 remain = m_message_remain.load();

                qint64 read = m_sock->read(reinterpret_cast<char*>(&m_message_buffer[(qint64)m_message_buffer.size() - remain]), remain);

                if (read < 0)
                {
                    m_message_remain.store(0);
                    qDebug() << "error reading message";
                    return;
                }

                remain -= read;

                if (remain == 0)
                {
                    //qDebug() << "message read with " << m_message_buffer.size() << " bytes";
                    m_message_ready.store(true);
                }

                m_message_remain.store(remain);

            }
            else
            {
                quint8 msg_len_buf[4];
                if(m_sock->read(reinterpret_cast<char*>(msg_len_buf), 4) != 4) // read message length
                    return;

                quint32 msg_len = *reinterpret_cast<quint32*>(msg_len_buf); // assume msg_len was sent with the same endien
                m_message_remain.store(msg_len);

                m_message_ready.store(false);
                m_message_buffer.clear();
                m_message_buffer.resize(msg_len);
            }
        }
    }
};

Tcp::Tcp()
    : d_ptr(new TcpPrivate(this))
{

}

Tcp::~Tcp()
{
    delete d_ptr;
}

void Tcp::init()
{
    Q_D(Tcp);
    d->init();
}

void Tcp::update()
{
    Q_D(Tcp);
    d->update();
}

void Tcp::shutdown()
{
    Q_D(Tcp);
    d->shutdown();
}


Tcp::State Tcp::get_state()
{
    Q_D(Tcp);
    Tcp::State state = d->m_state.load();
    return state;
}

Tcp::Result Tcp::disconnect()
{
    Q_D(Tcp);
    d->m_sock->abort();
    return ret_succeeded;
}

Tcp::Result Tcp::connect(unsigned char in_ip[4], int in_port)
{
    Q_D(Tcp);
    QString ip = QString("%1.%2.%3.%4").arg(in_ip[0]).arg(in_ip[1]).arg(in_ip[2]).arg(in_ip[3]);
    qDebug() << "Connecting to " << ip << ":" << in_port;

    d->m_sock->connectToHost(ip, (quint16)in_port);
    return ret_succeeded;
}

Tcp::Result Tcp::send_raw(const void* in_buffer_ptr, unsigned int in_buffer_sz)
{
    Q_D(Tcp);

    if (state_connected != d->m_state.load())
        return ret_err_not_connected;

    d->m_sock->write((const char*)in_buffer_ptr, (quint64)in_buffer_sz);

    return ret_succeeded;
}

bool Tcp::read_message(void*& out_message_ptr, unsigned int& out_message_sz)
{
    Q_D(Tcp);

    if (false == d->m_message_ready.load())
        return false;

    out_message_ptr = &d->m_message_buffer[0];
    out_message_sz = (unsigned int)d->m_message_buffer.size();

    // message consumed, mark as un-ready
    d->m_message_ready.store(false);

    return true;
}
