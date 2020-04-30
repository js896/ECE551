#include "handleBuiltIn.h"

#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>

//tell which built-in command the input is
bool BuiltIn::handleBuiltIn(shell & sh) {
  if (sh.wholeCommand[0] == "cd") {
    if (handleCD(sh)) {
      return true;
    }
    else {
      return false;
    }
  }
  else if (sh.wholeCommand[0] == "set") {
    if (handleSET(sh)) {
      return true;
    }
    else {
      return false;
    }
  }
  else if (sh.wholeCommand[0] == "export") {
    if (handleExport(sh)) {
      return true;
    }
    else {
      return false;
    }
  }
  else if (sh.wholeCommand[0] == "rev") {
    if (handleRev(sh)) {
      return true;
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }
}

bool BuiltIn::handleCD(shell & sh) {
  //check if the command format is correct
  if (sh.wholeCommand.size() > 2) {
    std::cerr << "Please input exactly one argumnet" << std::endl;
    return false;
  }
  else {
    //deal with command "cd"
    if (sh.wholeCommand.size() == 1) {
      sh.wholeCommand.push_back(getenv("HOME"));
    }
    //check if the directory exists
    if (chdir(sh.wholeCommand[1].c_str()) == -1) {
      perror("cd");
      return false;
    }
    else {
      return true;
    }
  }
}

bool BuiltIn::handleSET(shell & sh) {
  //check input format
  if (sh.wholeCommand.size() != 3) {
    std::cerr << "Please input exactly two argument" << std::endl;
    return false;
  }
  //check the name of variable
  for (size_t i = 0; i < sh.wholeCommand[1].size(); ++i) {
    if (!isalpha(sh.wholeCommand[1][i]) && !isdigit(sh.wholeCommand[1][i]) &&
        !(sh.wholeCommand[1][i] == '_')) {
      std::cerr << "A variable name must be a combination of letters (case sensitive), "
                   "underscores, and numbers"
                << std::endl;
      return false;
    }
  }
  //search the varible and act according to wether the varibale exists or not
  std::map<std::string, std::string>::iterator it = sh.vars.find(sh.wholeCommand[1]);
  if (it != sh.vars.end()) {
    it->second = sh.wholeCommand[2];
  }
  else {
    sh.vars.insert(
        std::pair<std::string, std::string>(sh.wholeCommand[1], sh.wholeCommand[2]));
  }
  return true;
}

bool BuiltIn::handleRev(shell & sh) {
  //check command format
  if (sh.wholeCommand.size() != 2) {
    std::cerr << "Please input exactly one argument" << std::endl;
    return true;
  }
  //check if varibale exists
  std::map<std::string, std::string>::iterator it = sh.vars.find(sh.wholeCommand[1]);
  if (it == sh.vars.end()) {
    std::cerr << "Variable not found" << std::endl;
    return false;
  }
  //reverse variable value
  else {
    reverse(it->second.begin(), it->second.end());
    return true;
  }
}

bool BuiltIn::handleExport(shell & sh) {
  //check command format
  if (sh.wholeCommand.size() != 2) {
    std::cerr << "Please input exactly one argument" << std::endl;
    return false;
  }
  //check if variable exists
  std::map<std::string, std::string>::iterator it = sh.vars.find(sh.wholeCommand[1]);
  if (it == sh.vars.end()) {
    std::cerr << "Variable not found" << std::endl;
    return false;
  }
  //if exists, export it to environment
  else {
    setenv(sh.wholeCommand[1].c_str(), it->second.c_str(), 1);
    return true;
  }
}
