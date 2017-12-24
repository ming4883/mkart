#ifndef KART_COMMANDS_H
#define KART_COMMANDS_H

#include "kart_archive.h"

#include <functional>

enum CommandId
{
  cmd_none,
  cmd_motor_set_speed,
};

class Cmd
{
public:
  virtual ~Cmd() {}

  virtual CommandId get_id() const = 0;
  virtual void pack(OutputArchive& inout_archive) const = 0;
  virtual void unpack(InputArchive& in_archive) = 0;
};

class CmdDispatcher
{
public:
  typedef std::function<Cmd* (void)> CstorFunc;
  typedef std::function<void (Cmd*)> ExecFunc;

  CmdDispatcher();
  ~CmdDispatcher();
  
  bool register_command(uint8_t in_id, CstorFunc in_cstor_func, ExecFunc in_exec_func);

  template<typename CMD>
  bool register_command(ExecFunc in_exec_func)
  {
    CstorFunc cstor = [] (void)
    {
      return new CMD;
    };

    return register_command(CMD::id, cstor, in_exec_func);
  }

  bool dispatch_from_archive(InputArchive& in_archive);

  class Data;
  Data* m_data;
};

class CmdNone : public Cmd
{
public:
  enum {id = cmd_none};

  CmdNone() {}

  CommandId get_id() const { return (CommandId)id; }  

  void pack(OutputArchive& inout_archive) const override
  {
      (void)inout_archive;
  }

  void unpack(InputArchive& in_archive) override
  {
      (void)in_archive;
  }
};

class CmdMotorSetSpeed : public Cmd
{
public:
  enum {id = cmd_motor_set_speed};

  int16_t l_speed;
  int16_t r_speed;

  CmdMotorSetSpeed() {}

  CmdMotorSetSpeed(int16_t in_l_speed, int16_t in_r_speed) : l_speed(in_l_speed), r_speed(in_r_speed) {}

  CommandId get_id() const { return (CommandId)id; }  

  void pack(OutputArchive& inout_archive) const override
  {
    inout_archive.write(l_speed);
    inout_archive.write(r_speed);
  }

  void unpack(InputArchive& in_archive) override
  {
    in_archive.read(l_speed);
    in_archive.read(r_speed);
  }
};

#endif // KART_COMMANDS_H
