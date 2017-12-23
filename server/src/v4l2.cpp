// http://jwhsmith.net/2014/12/capturing-a-webcam-stream-using-v4l2/
// https://linuxtv.org/downloads/v4l-dvb-apis/uapi/v4l/capture.c.html

#include "v4l2.h"

#include <linux/videodev2.h>

#include <fcntl.h>              // low-level i/o
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>             // memset
#include <stdio.h>              // fopen

#include <loguru.hpp>
#include <dukglue/dukglue.h>
#include <vector>

#include "rpi_server.h"

#include "jpgd.h"
#include "jpge.h"

#define TARGET_LOGGING 0
#define TARGET_WIDTH 640 // in pixels
#define TARGET_FRAME_TIME 100 // in milli-seconds
#define TARGET_FORMAT V4L2_PIX_FMT_MJPEG
#define TARGET_USE_HW_JPEG 1


class V4L2Encoder
{
public:
    std::vector<unsigned char> m_bytes;
    int m_length;

    V4L2Encoder();
    void encode(const void* in_data, int in_w, int in_h, int in_nchannels);
    static bool is_jpeg(uint8_t* in_data);
};

int V4L2::xioctl(int fh, int request, void *arg)
{
    int r;

    do
    {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}

V4L2::V4L2()
    : m_fd(-1)
    , m_capture(false)
{
    m_frame_time = TARGET_FRAME_TIME;

    for(int i = 0; i < DEV_BUFFER_CNT; ++i)
    {
        m_dev_buf[i].sz = 0;
        m_dev_buf[i].ptr = nullptr;
    }

    m_dev_buf_idx = 0;
}

bool V4L2::startup(RpiServer& in_server)
{
    init_commands(in_server.script_context());

    set_capturing(false);

    m_fd = open("/dev/video0", O_RDWR);
    if (m_fd < 0)
    {
        LOG_F(ERROR, "open(\"/dev/video0\") failed\n");
        return false;
    }

    v4l2_capability cap;
    if (xioctl(m_fd, VIDIOC_QUERYCAP, &cap) < 0)
    {
        LOG_F(ERROR, "VIDIOC_QUERYCAP failed\n");
        return false;
    }

    if (0 == (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        LOG_F(ERROR, "device does not support video captrue\n");
        return false;
    }

    // v4l2-ctl -d /dev/video0 --list-formats-ext
    // enum all supported capture formats
    std::vector<v4l2_format> formats;

    for (int i = 0;; i++)
    {
        v4l2_fmtdesc fmtdesc;
        memset(&fmtdesc, 0, sizeof(fmtdesc));
        fmtdesc.index = i;
        fmtdesc.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (xioctl(m_fd, VIDIOC_ENUM_FMT, &fmtdesc) < 0)
            break;
        
        if (fmtdesc.pixelformat != TARGET_FORMAT)
            continue;
        
        LOG_F(INFO, "v4l2 supported format: %x, %s\n", fmtdesc.pixelformat, fmtdesc.description);

        v4l2_frmsizeenum frmsize;
        memset(&frmsize, 0, sizeof(frmsize));
        frmsize.pixel_format = fmtdesc.pixelformat;

        while (xioctl(m_fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0)
        {
            #if TARGET_LOGGING
            {
                LOG_F(INFO, "  size: %dx%d\n", frmsize.discrete.width, frmsize.discrete.height);
            }
            #endif

            v4l2_format format;
            memset(&format, 0, sizeof(format));
            format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            format.fmt.pix.pixelformat = fmtdesc.pixelformat;
            format.fmt.pix.width = frmsize.discrete.width;
            format.fmt.pix.height = frmsize.discrete.height;

            formats.push_back(format);

            frmsize.index++;
        }
    }

    if (formats.empty())
    {
        LOG_F(ERROR, "No compatible capture formats was found\n");
        return false;
    }

    // search for the closest format
    int target_index = -1;
    for(size_t i = 0; i < formats.size(); ++i)
    {
        if (target_index == -1)
        {
            target_index = (int)i;
        }
        else
        {
            int diff_a = formats[i].fmt.pix.width - TARGET_WIDTH;
            int diff_b = formats[target_index].fmt.pix.width - TARGET_WIDTH;

            if (diff_a * diff_a < diff_b * diff_b)
            {
                target_index = (int)i;
            }
        }
    }

    LOG_F(INFO, "v4l2 capturing with size: %dx%d\n", formats[target_index].fmt.pix.width, formats[target_index].fmt.pix.height);

    if (xioctl(m_fd, VIDIOC_S_FMT, &formats[target_index]) < 0)
    {
        LOG_F(ERROR, "VIDIOC_S_FMT failed\n");
        return false;
    }

    // request and query buffer info
    v4l2_requestbuffers bufrequest;
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = DEV_BUFFER_CNT;

    if (xioctl(m_fd, VIDIOC_REQBUFS, &bufrequest) < 0)
    {
        LOG_F(ERROR, "VIDIOC_REQBUFS failed\n");
        return false;
    }

    v4l2_buffer bufferinfo;
    memset(&bufferinfo, 0, sizeof(bufferinfo));
    
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;

    for(int i = 0; i < DEV_BUFFER_CNT; ++i)
    {
        bufferinfo.index = i;
        
        if(xioctl(m_fd, VIDIOC_QUERYBUF, &bufferinfo) < 0)
        {
            LOG_F(ERROR, "VIDIOC_QUERYBUF %d failed\n", i);
            return false;
        }

        void* buffer_start = mmap(
            NULL,
            bufferinfo.length,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            m_fd,
            bufferinfo.m.offset
        );
        
        if(buffer_start == MAP_FAILED)
        {
            LOG_F(ERROR, "mmap failed\n");
            return false;
        }

        m_dev_buf[i].ptr = buffer_start;
        m_dev_buf[i].sz = bufferinfo.length;

        LOG_F(INFO, "v4l2 buffer requested length = %d, offset = %d\n", bufferinfo.length, bufferinfo.m.offset);
    }
    
    m_dev_buf_idx = 0;

    // Activate streaming
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(m_fd, VIDIOC_STREAMON, &type) < 0)
    {
        LOG_F(ERROR, "VIDIOC_STREAMON failed\n");
        return false;
    }

    m_last_time = Clock::now();

    return true;
}

void V4L2::shutdown()
{
    // Deactivate streaming
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(m_fd, VIDIOC_STREAMOFF, &type) < 0)
    {
        LOG_F(ERROR, "VIDIOC_STREAMOFF failed\n");
    }

    for(int i = 0; i < DEV_BUFFER_CNT; ++i)
    {
        munmap(m_dev_buf[i].ptr, m_dev_buf[i].sz);
    }

    if (m_fd > 0)
        close(m_fd);
}

void V4L2::update()
{
    if (m_capture)
    {
        TimePt now = Clock::now();
        auto delta_t = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_time).count();
    
        if ((long)delta_t > m_frame_time)
        {
            if(!capture())
                set_capturing(false);
                
            m_last_time = Clock::now();
        }
    }
}

bool V4L2::is_capturing()
{
    return m_capture;
}

void V4L2::set_capturing(bool in_enabled)
{
    m_capture = in_enabled;
    LOG_F(INFO, "V4L2 capturing set to %s", in_enabled ? "true" : "false");
}

bool V4L2::capture()
{
    static V4L2Encoder encoder;

    if(!RpiServer::get()->is_connected())
        return false;

    v4l2_buffer bufferinfo;

    // next buffer
    m_dev_buf_idx = (m_dev_buf_idx + 1) % DEV_BUFFER_CNT;

    HWBuffer& cur_buf = m_dev_buf[m_dev_buf_idx];
    
    memset(&bufferinfo, 0, sizeof(bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = m_dev_buf_idx; /* Queueing buffer index 0. */

    memset(cur_buf.ptr, 0, cur_buf.sz);
    
    // Put the buffer in the incoming queue.
    if (xioctl(m_fd, VIDIOC_QBUF, &bufferinfo) < 0)
    {
        LOG_F(ERROR, "VIDIOC_QBUF failed\n");
        return false;
    }
    
    // Dequeue the buffer.
    if (xioctl(m_fd, VIDIOC_DQBUF, &bufferinfo) < 0)
    {
        LOG_F(ERROR, "VIDIOC_DQBUF failed\n");
        return false;
    }

    if (!V4L2Encoder::is_jpeg((uint8_t*)cur_buf.ptr))
    {
        #if TARGET_LOGGING
        {
            LOG_F(INFO, "Frame captured, but not a valid JPEG!\n");
        }
        #endif
        return false;
    }
    
    #if TARGET_LOGGING
    {
        LOG_F(INFO, "Frame captured!\n");
    }
    #endif

    #if 1 == TARGET_USE_HW_JPEG
    {
        std::vector<uint8_t>& sent_buf = encoder.m_bytes;

        // write to sent buffer
        sent_buf.clear();
        sent_buf.resize(bufferinfo.bytesused + 8); // 4 bytes size + 4 bytes 4cc
        
        sent_buf[4] = 'J';
        sent_buf[5] = 'P';
        sent_buf[6] = 'E';
        sent_buf[7] = 'G';
        memcpy(&sent_buf[8], cur_buf.ptr, bufferinfo.bytesused);

        {
            uint32_t* ptr = (uint32_t*)&sent_buf[0];
            *ptr = (uint32_t)bufferinfo.bytesused + 4; // 4cc + data
        }

        if(RpiServer::get()->send(&sent_buf[0], sent_buf.size()))
        {
            #if TARGET_LOGGING
            {
                LOG_F(INFO, "Sent %d bytes\n", sent_buf.size());
            }
            #endif
        }
    }
    #else
    {
        jpgd::jpeg_decoder_mem_stream jpg_stm((const uint8_t*)cur_buf.ptr, cur_buf.sz);
        int jpg_w, jpg_h, jpg_c;
        
        auto jpg_p = jpgd::decompress_jpeg_image_from_stream(
            &jpg_stm, &jpg_w, &jpg_h, &jpg_c, 3);

        if (jpg_p)
        {
            encoder.encode(jpg_p, jpg_w, jpg_h, jpg_c);
            
            if(RpiServer::get()->send(&encoder.m_bytes[0], encoder.m_length))
            {
                #if TARGET_LOGGING
                {
                    LOG_F(INFO, "Sent %d bytes\n", encoder.m_length);
                }
                #endif
            }
        }
        else
        {
            #if TARGET_LOGGING
            {
                LOG_F(ERROR, "JPEG Decode failed");
            }
            #endif
        }
    }
    #endif

    return true;
}

void V4L2::init_commands(void* in_script_ctx)
{
    ScriptContext duk_ctx = (ScriptContext)in_script_ctx;

    struct Cmds
    {
        static void set_capturing(bool in_enabled)
        {
            if (!RpiServer::get())
                return;

            RpiServer::get()->v4l2().set_capturing(in_enabled);
        }
    };

    dukglue_register_function(duk_ctx, Cmds::set_capturing, "v4l2_set_capturing");
}

V4L2Encoder::V4L2Encoder()
{
    m_bytes.reserve(1024 * 10);
}

void V4L2Encoder::encode(const void* in_data, int in_w, int in_h, int in_nchannels)
{
    const int quality = 1; // 1 < 2 < 3

    m_bytes.clear();
    m_bytes.resize(in_w * in_h * 3);
    
    m_bytes[4] = 'J';
    m_bytes[5] = 'P';
    m_bytes[6] = 'E';
    m_bytes[7] = 'G';

    int byte_count = (int)m_bytes.size() - 8;
    jpge::compress_image_to_jpeg_file_in_memory(&m_bytes[8], byte_count, in_w, in_h, in_nchannels, (const uint8_t*) in_data);
    
    // message length
    uint32_t* ptr = (uint32_t*)&m_bytes[0];
    *ptr = (uint32_t)byte_count + 4; // image length + 4cc

    m_length = byte_count + 8;
}

/*
void V4L2Encoder::tje_write_func(void* in_context, void* in_data, int in_size)
{
    auto inst = (V4L2Encoder*)in_context;
    unsigned char* ptr = (unsigned char*)in_data;

    for(int i = 0; i < in_size; ++i)
    {
        inst->m_bytes.push_back(ptr[i]);
    }
}
*/

bool V4L2Encoder::is_jpeg(uint8_t* in_data)
{
    if (   in_data[0] == 0xFF 
        && in_data[1] == 0xD8 
        && in_data[2] == 0xFF 
        && in_data[3] == 0xE0)
        return true;

    if (   in_data[0] == 0xFF 
        && in_data[1] == 0xD8 
        && in_data[2] == 0xFF 
        && in_data[3] == 0xDB)
        return true;
    
    LOG_F(INFO, "v4l2 unknown header %2x %2x %2x %2x ...\n", in_data[0], in_data[1], in_data[2], in_data[3]);

    return false;
}
