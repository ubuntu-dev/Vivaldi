#ifndef VV_VALUE_FILE_H
#define VV_VALUE_FILE_H

#include "value.h"

#include <fstream>

namespace vv {

namespace value {

struct file : public base {
public:
  file(const std::string& filename);
  file();
  file(file&& other);

  std::string value() const override;

  std::string name;
  std::string cur_line;
  std::unique_ptr<std::fstream> val;
};

}

}

#endif
