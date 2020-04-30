#ifndef HANDLEINPUT_H
#define HANDLEINPUT_H
#include <cstdlib>
#include <string>

#include "handleBuiltIn.h"
#include "sh.h"
class Input {
 public:
  Input() {}
  ~Input() {}
  //parse input according to given rule
  bool parsecmd(std::string & line, shell & sh);
  //deal back slashes
  void handleBackSlash(shell & sh);
};
#endif
