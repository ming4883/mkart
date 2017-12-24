#include "kart_commands.h"
#include <map>


class CmdDispatcher::Data
{
public:
  struct Entry
  {
    CmdDispatcher::CstorFunc cstor_func;
    CmdDispatcher::ExecFunc exec_func;
  };

  std::map<uint8_t, Entry> m_entries;
};


CmdDispatcher::CmdDispatcher()
  : m_data(new Data)
{

}

CmdDispatcher::~CmdDispatcher()
{
  delete m_data;
}

bool CmdDispatcher::register_command(uint8_t in_id, CstorFunc in_cstor_func, ExecFunc in_exec_func)
{
  if (m_data->m_entries.count(in_id))
    return false; // duplicated registration, reject

  Data::Entry entry {in_cstor_func, in_exec_func};
  m_data->m_entries[in_id] = entry;

  return true;
}


bool CmdDispatcher::dispatch_from_archive(InputArchive& in_archive)
{
  uint8_t sz;
  uint8_t id;

  if(!in_archive.read(sz))
  {
    return false;
  }

  if(!in_archive.read(id))
  {
    return false;
  }

  auto entry = m_data->m_entries.find(id);

  if (entry == m_data->m_entries.end())
    return false;

  Cmd* cmd = entry->second.cstor_func();
  cmd->unpack(in_archive);
  entry->second.exec_func(cmd);
  delete cmd;

  return false;
}
