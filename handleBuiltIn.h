#include <cstdlib>
#include <map>
#include <string>
#include <vector>

#include "sh.h"
class BuiltIn {
 public:
  BuiltIn() {}
  ~BuiltIn() {}
  //handleBuiltin tells the input is which built-in command
  bool handleBuiltIn(shell & sh);
  bool handleCD(shell & sh);
  bool handleSET(shell & sh);
  bool handleRev(shell & sh);
  bool handleExport(shell & sh);
};
