#include "sh.h"

#include <bits/stdc++.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

std::map<std::string, std::string> shell::vars;

bool shell::searchdir(std::string dirGonnaSearch) {
  DIR * dir;
  struct dirent * entry;
  //open the directory
  if ((dir = opendir(dirGonnaSearch.c_str())) == NULL) {
    std::cerr << "Can't open specified directory" << std::endl;
    closedir(dir);
    return false;
  }
  //look through all files in the directory
  while ((entry = readdir(dir)) != NULL) {
    std::string toBeComp(entry->d_name);
    if (toBeComp == "." || toBeComp == "..") {
      continue;
    }
    else if (entry->d_type == DT_REG) {
      //if we found a matching one, add its path to our command
      if (toBeComp.compare(wholeCommand[0]) == 0) {
        wholeCommand[0] = dirGonnaSearch + "/" + toBeComp;
        closedir(dir);
        return true;
      }
    }
  }
  closedir(dir);
  delete entry;
  return false;
}

void shell::parsePath() {
  //get the path to search for from ECE551PATH
  std::string ece551path = getenv("ECE551PATH");
  //divide the path into different directories by the seperating colons
  size_t begin = 0;
  size_t colonPos = ece551path.find(":");
  while (colonPos != std::string::npos) {
    parsedPath.push_back(ece551path.substr(begin, colonPos - begin));
    begin = colonPos + 1;
    colonPos = ece551path.find(":", begin);
  }
  parsedPath.push_back(ece551path.substr(begin, std::string::npos));
  //search in each directory for the command
  for (std::vector<std::string>::iterator it = parsedPath.begin(); it != parsedPath.end();
       ++it) {
    if (!searchdir(*it)) {
      continue;
    }
    else {
      return;
    }
  }
  //if it's built-in command, we already sloved it
  if (wholeCommand[0] == "cd" || wholeCommand[0] == "set" ||
      wholeCommand[0] == "export" || wholeCommand[0] == "rev") {
    return;
  }
  //command not found
  std::cerr << "Command " << originalCommand << " not found" << std::endl;
  exit(EXIT_FAILURE);
}

void shell::handleRequest() {
  //check if the command has already stated its directory
  if (wholeCommand[0][0] == '/' || wholeCommand[0][0] == '.' ||
      (wholeCommand[0][0] == '.' && wholeCommand[0][1] == '.')) {
    for (size_t i = 0; i < wholeCommand.size(); ++i) {
      cmd.push_back(const_cast<char *>(wholeCommand[i].c_str()));
    }
    cmd.push_back(NULL);
    execve(cmd[0], &cmd[0], environ);
    exit(EXIT_FAILURE);
  }
  else {
    //command comes with no directory, search for it
    parsePath();
    for (size_t i = 0; i < wholeCommand.size(); ++i) {
      cmd.push_back(const_cast<char *>(wholeCommand[i].c_str()));
    }
    cmd.push_back(NULL);
    execve(cmd[0], &cmd[0], environ);
    exit(EXIT_FAILURE);
  }
}

void shell::showPrompt() {
  char * directory;
  directory = getcwd(NULL, 0);
  //show the current directory
  std::cout << "ffosh:" << directory << " $ ";
  free(directory);
}

void shell::replaceVar() {
  for (size_t i = 0; i < wholeCommand.size(); ++i) {
    size_t Pos;
    size_t nextPos;
    size_t start = 0;
    /*Be greedy when substituting variables with their values, find the variable with the longest name after $*/
    while ((Pos = wholeCommand[i].substr(start).find("$")) != std::string::npos) {
      nextPos = 0;
      try {
        while (nextPos < (wholeCommand[i].size() - start - Pos - 1) &&
               (isalpha(wholeCommand[i].substr(start + Pos + 1)[nextPos]) ||
                isdigit(wholeCommand[i].substr(start + Pos + 1)[nextPos]) ||
                wholeCommand[i].substr(start + Pos + 1)[nextPos] == '_')) {
          ++nextPos;
          if (nextPos == (wholeCommand[i].size() - start - Pos - 1) &&
              vars.find(wholeCommand[i].substr(start + Pos + 1, nextPos)) == vars.end()) {
            //when indexing out of range, throw exception
            throw std::out_of_range("No Matching Variable");
          }
        }
      }
      //catch the exception
      catch (std::out_of_range & e) {
        std::cerr << "No Matching Variable" << std::endl;
        return;
      }
      //check it the varible exists
      if (vars.find(wholeCommand[i].substr(start + Pos + 1, nextPos)) != vars.end()) {
        //replace the variable
        wholeCommand[i].replace(start + Pos,
                                nextPos + 1,
                                vars[wholeCommand[i].substr(start + Pos + 1, nextPos)]);
        size_t sz = vars[wholeCommand[i].substr(start + Pos + 1, nextPos)].size();
        start = start + Pos + sz + 1;
      }
      else {
        start = start + Pos + nextPos + 2;
      }
    }
    continue;
  }
}

void shell::redirection(char oper, std::string redirObject) {
  int fd;
  if (oper == '<') {
    //0764 stands for: user can read, write and execute, group can read and write, others can read
    fd = open(redirObject.c_str(), O_RDWR, 0764);
  }
  //for output and error redirection, create the file when it does not exist
  else {
    fd = open(redirObject.c_str(), O_RDWR | O_CREAT, 0764);
  }
  if (fd == -1) {
    //swap to deal with mutiple output redirection
    std::vector<std::string>().swap(wholeCommand);
    std::cerr << "file does not exist" << std::endl;
    exit(EXIT_FAILURE);
  }
  int new_fd = 0;
  //input redirection
  if (oper == '<') {
    new_fd = dup2(fd, 0);
  }
  //output redirection
  else if (oper == '>') {
    new_fd = dup2(fd, 1);
  }
  //error redirection
  else if (oper == '2') {
    new_fd = dup2(fd, 2);
  }
  if (new_fd == -1) {
    //    std::vector<std::string>().swap(wholeCommand);
    std::cerr << "Can't redirect to specified file" << std::endl;
    exit(EXIT_FAILURE);
  }
  close(fd);
}

void shell::handleRedirection() {
  size_t i = 0;
  while (i < wholeCommand.size()) {
    //when redirection character is an isolated command line argument
    if (wholeCommand[i] == ">" || wholeCommand[i] == "<" || wholeCommand[i] == "2>") {
      if (i == wholeCommand.size() - 1) {
        std::cerr << "Please specify redirection object" << std::endl;
        std::vector<std::string>().swap(wholeCommand);
        exit(EXIT_FAILURE);
      }
      redirection(wholeCommand[i][0], wholeCommand[i + 1]);
      std::vector<std::string>::iterator it = wholeCommand.begin() + i;
      wholeCommand.erase(it, it + 1);
    }
    //when redirection character is at the end of an argument
    else if (wholeCommand[i].back() == '>' || wholeCommand[i].back() == '<') {
      if (i == wholeCommand.size() - 1) {
        std::cerr << "Please specify redirection object" << std::endl;
        std::vector<std::string>().swap(wholeCommand);
        exit(EXIT_FAILURE);
      }
      if (wholeCommand[i].back() == '<') {
        redirection('<', wholeCommand[i + 1]);
        wholeCommand[i].erase(wholeCommand[i].size() - 1);
      }
      else if (wholeCommand[i][wholeCommand.size() - 2] == '2') {
        redirection('2', wholeCommand[i + 1]);
        wholeCommand[i].erase(wholeCommand[i].size() - 2);
      }
      else {
        redirection('>', wholeCommand[i + 1]);
        wholeCommand[i].erase(wholeCommand[i].size() - 1);
      }
      std::vector<std::string>::iterator it = wholeCommand.begin() + i + 1;
      wholeCommand.erase(it);
    }
    //when redirection character is at the beginning of an argument
    else if (wholeCommand[i][0] == '>' || wholeCommand[i][0] == '<' ||
             (wholeCommand[i][0] == '2' && wholeCommand[i][1] == '>')) {
      if (wholeCommand[i][0] == '>' || wholeCommand[i][0] == '<') {
        redirection(wholeCommand[i][0], wholeCommand[i].substr(1));
      }
      else {
        redirection('2', wholeCommand[i].substr(2));
      }
      std::vector<std::string>::iterator it = wholeCommand.begin() + i;
      wholeCommand.erase(it);
    }
    //none of previous patterns
    else {
      ++i;
      continue;
    }
  }
}

void shell::execute() {
  //fork current process
  pid_t pid = fork();
  pid_t wpid;
  int status;
  //deal with fork error
  if (pid == -1) {
    std::cerr << "fork failed";
    exit(EXIT_FAILURE);
  }
  //child process
  else if (pid == 0) {
    handleRedirection();
    handleRequest();
  }
  //parent process
  else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
      if (wpid == -1) {
        std::cerr << "waitpid";
        exit(EXIT_FAILURE);
      }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    if (WIFEXITED(status)) {
      if (WEXITSTATUS(status) == 0) {
        std::cout << "Program was successful" << std::endl;
      }
      else {
        std::cout << "Program failed with code " << WEXITSTATUS(status) << std::endl;
      }
    }
    if (WIFSIGNALED(status)) {
      std::cout << "Terminated by signal " << WTERMSIG(status) << std::endl;
    }
  }
}
