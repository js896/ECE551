#include "handleInput.h"

#include <cstdlib>
#include <iostream>

bool Input::parsecmd(std::string & line, shell & sh) {
  sh.command = line;
  sh.originalCommand = line;
  size_t numBSf = 0;
  size_t numBSb = 0;
  size_t wsPos;
  size_t qmPos;
  size_t i = 0;
  size_t j;
  size_t k;
  size_t qm_1st;
  size_t qmPos_pre = 0;
  size_t l;
  size_t num = 0;

  while (i < sh.command.size()) {
    //whitespace, do nothing
    if (sh.command[i] == 32) {
      ++i;
      continue;
    }
    //quotation mark
    else if (sh.command[i] == 34) {
      qm_1st = i;
      j = i - 1;
      while (sh.command[j] == '\\') {
        ++numBSf;
        --j;
      }
      //check if the first quotation mark in the possible pair is literal
      if (numBSf % 2 == 0) {
        while ((qmPos = sh.command.substr(i + 1).find('"')) != std::string::npos) {
          k = i + qmPos;
          while (sh.command[k] == '\\') {
            ++numBSb;
            --k;
          }
          //check if the second quotation mark in the possible pair is literal
          if (numBSb % 2 == 0) {
            sh.wholeCommand.push_back(sh.command.substr(qm_1st + 1, qmPos_pre + qmPos));
            break;
          }
          else {
            i = i + qmPos + 1;
            qmPos_pre = qmPos + 1 + qmPos_pre;
            numBSb = 0;
          }
        }
        if (qmPos == std::string::npos) {
          std::cerr << "unclosed quotation mark" << std::endl;
          return false;
        }

        i = qm_1st + qmPos_pre + qmPos + 2;
        numBSb = 0;
        numBSf = 0;
        qmPos_pre = 0;
      }
      else {
        sh.wholeCommand.push_back(sh.command.substr(i, 1));
        ++i;
      }
    }
    //when the character is neither backspace nor quotation mark
    else {
      wsPos = sh.command.substr(i).find(" ");
      if (wsPos == std::string::npos) {
        if (sh.command.back() == '"') {
          l = sh.command.size() - 2;
          while (sh.command[l] == '\\') {
            ++num;
            --l;
          }
          if (num % 2 == 0) {
            //unclosed quotation mark at the end of command line
            std::cerr << "unclosed quotation mark" << std::endl;
            return false;
          }
        }
        sh.wholeCommand.push_back(sh.command.substr(i, sh.command.size() - i));
        break;
      }
      sh.wholeCommand.push_back(sh.command.substr(i, wsPos));
      i = i + wsPos;
    }
  }
  return true;
}

void Input::handleBackSlash(shell & sh) {
  for (size_t i = 0; i < sh.wholeCommand.size(); ++i) {
    if (sh.wholeCommand[i].find('\\') != std::string::npos) {
      size_t j = 0;
      while (j < sh.wholeCommand[i].size() - 1) {
        //make character after backslash literal
        if (sh.wholeCommand[i][j] == '\\' &&
            ((sh.wholeCommand[i][j + 1] >= 0 && sh.wholeCommand[i][j + 1] <= 127))) {
          sh.wholeCommand[i].erase(j, 1);
          if (sh.wholeCommand[i][j] == '\\') {
            ++j;
          }
        }
        else {
          ++j;
        }
      }
    }
    else {
      continue;
    }
  }
}
