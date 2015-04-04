#ifndef VV_VALUE_FILE_H
#define VV_VALUE_FILE_H

#include "value/basic_object.h"

#include <fstream>

namespace vv {

namespace value {

struct file : public basic_object {
public:
  file(const std::string& filename);
  file();
  file(file&& other);

  struct value_type {
    std::string name;
    std::string cur_line;
    std::fstream val;
  };

  value_type value;
};

}

}

#endif
