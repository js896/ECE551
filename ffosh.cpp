
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "handleInput.h"

extern char ** environ;

int main(void) {
  //initialize ECE551PATH
  setenv("ECE551PATH", getenv("PATH"), 1);
  while (true) {
    std::string command;
    shell ffosh;
    ffosh.showPrompt();
    //read input
    std::getline(std::cin, command);
    std::string ext = "exit";
    //check if input is EOF or exit
    if (std::cin.eof() || (strcmp(ext.c_str(), command.c_str()) == 0)) {
      return EXIT_SUCCESS;
    }
    if (command.empty()) {
      continue;
    }
    Input in;
    //parse input
    if (!in.parsecmd(command, ffosh)) {
      continue;
    }
    in.handleBackSlash(ffosh);
    ffosh.replaceVar();
    BuiltIn bt;
    //deal with built-in commands and cd
    if (bt.handleBuiltIn(ffosh)) {
      continue;
    }
    //deal with other commands
    ffosh.execute();
  }
  return EXIT_SUCCESS;
}
