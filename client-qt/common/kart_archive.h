#ifndef KART_ARCHIVE_H
#define KART_ARCHIVE_H

#include <cstddef>
#include <cstdint>
#include <vector>

class OutputArchive
{
public:
  ~OutputArchive() {}

  virtual void reset() = 0;
  virtual void get_buffer(const uint8_t*& out_ptr, size_t& out_size) = 0;
  virtual void write(uint8_t in_value) = 0;
  virtual void write(uint16_t in_value) = 0;
  virtual void write(uint32_t in_value) = 0;
  virtual void write(int8_t in_value) = 0;
  virtual void write(int16_t in_value) = 0;
  virtual void write(int32_t in_value) = 0;
};

class InputArchive
{
public:
  enum SeekMode
  {
    seek_beg,
    seek_end,
    seek_cur,
  };

  ~InputArchive() {}

  virtual bool seek(size_t in_pos) = 0;
  virtual bool seek(SeekMode in_mode, int32_t in_offset) = 0;
  virtual bool read(uint8_t& out_value) = 0;
  virtual bool read(uint16_t& out_value) = 0;
  virtual bool read(uint32_t& out_value) = 0;
  virtual bool read(int8_t& out_value) = 0;
  virtual bool read(int16_t& out_value) = 0;
  virtual bool read(int32_t& out_value) = 0;
};

class MemOutputArchive : public OutputArchive
{
public:
  std::vector<uint8_t> m_bytes;

  void reset() override
  {
    m_bytes.clear();
  }

  void get_buffer(const uint8_t*& out_ptr, size_t& out_size) override
  {
    out_ptr = &m_bytes[0];
    out_size = m_bytes.size();
  }

  void write(uint8_t in_value) override
  {
    write_bytes(in_value);
  }

  void write(uint16_t in_value) override
  {
    write_bytes(in_value);
  }

  void write(uint32_t in_value) override
  {
    write_bytes(in_value);
  }

  void write(int8_t in_value) override
  {
    write_bytes(in_value);
  }

  void write(int16_t in_value) override
  {
    write_bytes(in_value);
  }

  void write(int32_t in_value) override
  {
    write_bytes(in_value);
  }

protected:
  
  template<typename T>
  void to_bytes(T& in_t, uint8_t*& out_ptr, size_t& out_size)
  {
    out_ptr = (uint8_t*)(&in_t);
    out_size = sizeof(T);
  }

  template<typename T>
  void write_bytes(T& in_t)
  {
    uint8_t* ptr;
    size_t sz;
    to_bytes<T>(in_t, ptr, sz);
    for(size_t i = 0; i < sz; ++i)
    {
      m_bytes.push_back(ptr[i]);
    }
  }  
};


class MemInputArchive : public InputArchive
{
public:
  MemInputArchive(const void* in_buff_ptr, size_t in_buff_size)
    : m_buff_ptr((const uint8_t*)in_buff_ptr)
    , m_buff_size(in_buff_size)
    , m_pos(0)
  {
  }

  ~MemInputArchive()
  {
    
  }

  bool seek(size_t in_pos) override
  {
    (void)in_pos;
    return true;
  }

  bool seek(SeekMode in_mode, int32_t in_offset) override
  {
    (void)in_mode;
    (void)in_offset;
    return true;
  }

  bool read(uint8_t& out_value) override
  {
    return read<uint8_t>(out_value);
  }

  bool read(uint16_t& out_value) override
  {
    return read<uint16_t>(out_value);
  }
  
  bool read(uint32_t& out_value) override
  {
    return read<uint32_t>(out_value);
  }

  bool read(int8_t& out_value) override
  {
    return read<int8_t>(out_value);
  }

  bool read(int16_t& out_value) override
  {
    return read<int16_t>(out_value);
  }

  bool read(int32_t& out_value) override
  {
    return read<int32_t>(out_value);
  }

private:
  const uint8_t* m_buff_ptr;
  size_t m_buff_size;
  size_t m_pos;

  template<typename T>
  bool read(T& out_value, size_t in_size = sizeof(T))
  {
    if (m_pos + in_size > m_buff_size)
      return false;

    const T* ptr = (const T*)&m_buff_ptr[m_pos];
    out_value = *ptr;
    m_pos += in_size;
    return true;
  }

};


#endif // KART_ARCHIVE_H
