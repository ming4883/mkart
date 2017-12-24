#include "fpv_service.h"

#include "timing.h"
#include <QtNetwork>
#include <QOpenGLTexture>
#include <QOpenGLPixelTransferOptions>

class FpvContext : public QObject
{
    //Q_OBJECT
public:
    Tcp m_tcp;
    QOpenGLTexture* m_tex;
    bool m_capture;
    float m_last_recv_time;
    float m_avg_recv_time;

    FpvContext() : m_tex(nullptr), m_capture(false), m_last_recv_time(0), m_avg_recv_time(0)
    {
    }

    void init()
    {
        m_tex = nullptr;
        m_tcp.init();
        m_capture = false;
        m_last_recv_time = current_time_in_ms();
    }

    void update()
    {
        m_tcp.update();

        void* msg_ptr;
        unsigned int msg_len;
        if (m_tcp.read_message(msg_ptr, msg_len))
        {
            if (is_4cc(msg_ptr, 'J', 'P', 'E', 'G'))
            {
                on_recieve_jpeg(((char*)msg_ptr) + 4, msg_len - 4);
            }
        }
    }

    bool is_4cc(void* in_ptr, char in_a, char in_b, char in_c, char in_d)
    {
        char* c = (char*)in_ptr;
        return c[0] == in_a && c[1] == in_b && c[2] == in_c && c[3] == in_d;
    }

    void on_recieve_jpeg(void* in_ptr, unsigned int in_len)
    {
        //qDebug() << "Image recieved";
        //m_img = new QImage(256, 128, QImage::Format_RGBA8888);
        //fill_test_pattern();

        // update the statistics
        float recv_time = current_time_in_ms();
        float delta_recv_time = recv_time - m_last_recv_time;
        if (0 == m_avg_recv_time)
        {
            m_avg_recv_time = delta_recv_time;
        }
        else
        {
            m_avg_recv_time = (m_avg_recv_time + delta_recv_time) * 0.5f;
        }

        m_last_recv_time = recv_time;

        // decode
        QImage loaded = QImage::fromData(reinterpret_cast<uchar*>(in_ptr), (int)in_len);
        if(loaded.isNull())
            return;

        loaded = loaded.convertToFormat(QImage::Format_RGBA8888);

        if (nullptr == m_tex)
        {
            m_tex = new QOpenGLTexture(loaded, QOpenGLTexture::DontGenerateMipMaps);
            m_tex->setMinificationFilter(QOpenGLTexture::Nearest);
            m_tex->setMagnificationFilter(QOpenGLTexture::Linear);
            m_tex->setWrapMode(QOpenGLTexture::ClampToEdge);
        }
        else
        {
            QOpenGLPixelTransferOptions uploadOptions;
            uploadOptions.setAlignment(1);
            m_tex->setData(0, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, (const void*)loaded.constBits(), &uploadOptions);
        }
    }

    Tcp::Result set_capturing(bool in_enabled)
    {
        m_capture = in_enabled;
        const char* msg_true = "v4l2_set_capturing(true);";
        const char* msg_false = "v4l2_set_capturing(false);";

        const char* msg = in_enabled ? msg_true : msg_false;

        return m_tcp.send_raw(msg, strlen(msg));
    }

    Tcp::Result remote_exit()
    {
        const char* msg = "exit();";

        return m_tcp.send_raw(msg, strlen(msg));
    }

    void shutdown()
    {
        m_tcp.shutdown();
        //delete m_img;
        delete m_tex;
    }

    unsigned int get_texture_id()
    {
        if(nullptr == m_tex)
            return 0;

        return m_tex->textureId();
    }

    unsigned int get_texture_width()
    {
        if(nullptr == m_tex)
            return 0;

        return (unsigned int)m_tex->width();
    }

    unsigned int get_texture_height()
    {
        if(nullptr == m_tex)
            return 0;

        return (unsigned int)m_tex->height();
    }
};

FpvContext g_fpv_context;

void FpvService::init()
{
    g_fpv_context.init();
}

void FpvService::update()
{
    g_fpv_context.update();
}

void FpvService::shutdown()
{
    g_fpv_context.shutdown();
}

Tcp::State FpvService::get_state()
{
    return g_fpv_context.m_tcp.get_state();
}

Tcp::Result FpvService::disconnect()
{
    return g_fpv_context.m_tcp.disconnect();
}

Tcp::Result FpvService::connect(unsigned char in_ip[4], int in_port)
{
    auto ret = g_fpv_context.m_tcp.connect(in_ip, in_port);

    if (ret == Tcp::ret_succeeded)
    {
        set_capturing(g_fpv_context.m_capture);
    }

    return ret;
}

bool FpvService::is_capturing()
{
    return g_fpv_context.m_capture;
}

void FpvService::set_capturing(bool in_capture)
{
    g_fpv_context.set_capturing(in_capture);
}

void FpvService::remote_exit()
{
    g_fpv_context.remote_exit();
}

unsigned int FpvService::get_texture_id()
{
    return g_fpv_context.get_texture_id();
}

unsigned int FpvService::get_texture_width()
{
    return g_fpv_context.get_texture_width();
}

unsigned int FpvService::get_texture_height()
{
    return g_fpv_context.get_texture_height();
}

float FpvService::get_avg_recv_time()
{
    return g_fpv_context.m_avg_recv_time;
}



#if 0

#include <vpx/vpx_encoder.h>
#include <vpx/vpx_decoder.h>
#include <vpx/vp8cx.h>

#define VP9_FOURCC 0x30395056

    QImage* m_img;

    vpx_codec_ctx_t codec;

    bool vpx_init(unsigned int in_w, unsigned int in_h)
    {
        vpx_codec_err_t res;

        vpx_codec_enc_cfg_t cfg;

        res = vpx_codec_enc_config_default(vpx_codec_vp9_cx(), &cfg, 0);
        if (res)
            return false;

        cfg.g_w = in_w;
        cfg.g_h = in_h;
        cfg.g_timebase.num = 1;//info.time_base.numerator;
        cfg.g_timebase.den = 15;//info.time_base.denominator;
        cfg.rc_target_bitrate = 100;//kilobits per second
        cfg.g_error_resilient = (vpx_codec_er_flags_t)0;//strtoul(argv[7], NULL, 0);

        res = vpx_codec_enc_init(&codec, vpx_codec_vp9_cx(), &cfg, 0);

        if (res)
            return false;

        return true;
    }

    int vpx_encode_frame(vpx_image_t *img, int frame_index, int flags)
    {
        int got_pkts = 0;
        vpx_codec_iter_t iter = NULL;
        const vpx_codec_cx_pkt_t *pkt = NULL;
        const vpx_codec_err_t res = vpx_codec_encode(&codec, img, frame_index, 1, flags, VPX_DL_GOOD_QUALITY);
        if (res != VPX_CODEC_OK)
            return -1;

        while ((pkt = vpx_codec_get_cx_data(&codec, &iter)) != NULL)
        {
            got_pkts = 1;

            if (pkt->kind == VPX_CODEC_CX_FRAME_PKT)
            {
              const int keyframe = (pkt->data.frame.flags & VPX_FRAME_IS_KEY) != 0;

              //if (!vpx_video_writer_write_frame(writer, pkt->data.frame.buf,
              //                                  pkt->data.frame.sz,
              //                                  pkt->data.frame.pts)) {
              //  die_codec(codec, "Failed to write compressed frame");
              //}
              //printf(keyframe ? "K" : ".");
              fflush(stdout);
            }
        }

        return got_pkts;
    }

    void fill_test_pattern()
    {
        // QImage::Format_RGBA8888
        // The order of the colors is the same on any architecture if read as bytes 0xRR,0xGG,0xBB,0xAA.
        int bval = (int)(current_time_in_ms() * 0.25f) % 255;

        for(int y = 0; y < m_img->height(); ++y)
        {
            uchar* scan = m_img->scanLine(y);
            for(int x = 0; x < m_img->width(); ++x)
            {
                uchar* p = &scan[x * 4];
                uchar* r = p++;
                uchar* g = p++;
                uchar* b = p++;
                uchar* a = p++;

                *r = x;
                *g = y;
                *b = bval;
                *a = 255;
            }
        }
    }

#endif
