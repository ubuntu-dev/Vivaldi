#ifndef VV_AST_LITERAL_H
#define VV_AST_LITERAL_H

#include "expression.h"

namespace vv {

namespace ast {

namespace literal {

class boolean : public expression {
public:
  boolean(bool val) : m_val{val} { }
  std::vector<vm::command> generate() const override;
private:
  bool m_val;
};

class character : public expression {
public:
  character(char val) : m_val{val} { }
  std::vector<vm::command> generate() const override;
private:
  char m_val;
};

class floating_point : public expression {
public:
  floating_point(double val) : m_val{val} { }
  std::vector<vm::command> generate() const override;
private:
  double m_val;
};

class integer : public expression {
public:
  integer(int64_t val) : m_val{val} { }
  std::vector<vm::command> generate() const override;
private:
  int64_t m_val;
};

class nil : public expression {
public:
  std::vector<vm::command> generate() const override;
};

class regex : public expression {
public:
  regex(const std::string& val) : m_val{val} { }
  std::vector<vm::command> generate() const override;
private:
  std::string m_val;
};

class string : public expression {
public:
  string(const std::string& val) : m_val{val} { }
  std::vector<vm::command> generate() const override;
private:
  std::string m_val;
};

class symbol : public expression {
public:
  symbol(vv::symbol val) : m_val{val} { }
  std::vector<vm::command> generate() const override;
private:
  vv::symbol m_val;
};

}

}

}

#endif
