#ifndef SH_H
#define SH_H
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

class shell {
 private:
  std::string command;
  std::string originalCommand;
  std::vector<std::string> parsedPath;
  std::vector<char *> cmd;
  static std::map<std::string, std::string> vars;
  std::vector<std::string> wholeCommand;

 public:
  ~shell() {}
  void showPrompt();
  bool parsecmd(std::string & line);
  void execute();
  void handleRequest();
  void parsePath();
  void print();
  bool searchdir(std::string dirGonnaSearch);
  void handleBackSlash();
  void replaceVar();
  void handleRedirection();
  void redirection(char oper, std::string redirObject);
  friend class BuiltIn;
  friend class Input;
};
#endif
