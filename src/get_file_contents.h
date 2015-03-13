#ifndef VV_GET_FILE_CONTENTS_H
#define VV_GET_FILE_CONTENTS_H

#include "vm/call_frame.h"

#include <string>

namespace vv {

class read_file_result {
public:
  read_file_result(const std::string& dir, const std::string& error)
    : m_successful {false},
      m_err {error},
      m_dir {dir}
  { }
  read_file_result(const std::string& dir,
                   std::vector<vm::command>&& instructions)
    : m_successful {true},
      m_result     {instructions},
      m_dir        {dir}
  { }

  bool successful() const noexcept { return m_successful; }

  const std::string& error() const { return m_err; }

  std::vector<vm::command>& result() { return m_result; }
  const std::vector<vm::command>& result() const { return m_result; }

  const std::string& file_directory() const { return m_dir; }

private:
  bool m_successful;
  std::string m_err;
  std::vector<vm::command> m_result;
  std::string m_dir;
};

read_file_result get_file_contents(const std::string& filename,
                                   const std::string& path = "");

}

#endif
