#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sys/wait.h>
#include <sys/stat.h>
#include <vector>
#include <unistd.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctime>

using namespace std;

string trim(string input) {
  int ptr = 0;
  while(ptr < input.size() && input[ptr] == ' ') {
    ptr++;
  }
  if(ptr < input.size()) { input = input.substr(ptr);
  } else {
    return "";
  }

  ptr = input.size()- 1;
  while(ptr >= 0 && input[ptr] == ' ') {
    ptr--;
  }

  if(ptr >= 0) {
    input = input.substr(0,ptr+1);
  } else {
    return "";
  }

  return input;
}

char** vec_to_char_array(vector<string>& parts) {
  char** result = new char * [parts.size() + 1];
  for(int i = 0; i < parts.size(); i++) {
    result[i] = (char*) parts[i].c_str();
  }
  result[parts.size()] = NULL;
  return result;
}

vector<string> split(string line, string separator=" ") {
  vector<string> res;
  string temp = "";
  bool q = false;

  for(int i = 0; i < line.size(); i++) {
    if(line.substr(i, separator.size()) == separator && !q) {
      if(temp != "") {
        res.push_back(temp);
        temp = "";
      }
      i += separator.size()-1;
    } else {
      if(separator == " ") {
        if((int(line[i]) == 34 || int(line[i]) == 39) && q) {
          q = false;
        } else if(int(line[i]) == 34 || int(line[i]) == 39) {
          q = true;
        } else {
          temp += line[i];
        }
      } else {
        temp += line[i];
      } 
    }
  }

  if(temp!= "") {
    res.push_back(temp);
  }

  return res;

}

vector<string> split2(string line, string separator=" ") {
  vector<string> res;
  string temp = "";
  bool q = false;

  for(int i = 0; i < line.size(); i++) {
    if(line.substr(i, separator.size()) == separator && !q) {
      if(temp != "") {
        res.push_back(temp);
        temp = "";
      }
      i += separator.size()-1;
    } else {
      if(true) {
        if((int(line[i]) == 34 || int(line[i]) == 39) && q) {
          q = false;
        } else if(int(line[i]) == 34 || int(line[i]) == 39) {
          q = true;
        } else {
          temp += line[i];
        }
      } else {
        temp += line[i];
      } 
    }
  }

  if(temp!= "") {
    res.push_back(temp);
  }

  return res;

}

void logger(string s) {
  ofstream o;
  o.open("log.txt", ios::app);
  o << s << "\n" <<endl;
  o.close();
}

void execute(string inputline) {
    bool io = false;
    string curr = "";
    for(int i = 0; i < inputline.size(); i++) {
      if(inputline[i] == '<') {
        io = true;
        curr = trim(curr);
        string fileName = "";
        int temp = i+2;
        while(inputline[temp] !=' ' && temp < inputline.size()) {
          fileName += inputline[temp];
          temp++;
        }
      logger(curr);
      vector<string> arguments = split(curr);
      for(string arg : arguments) {
        logger(arg);
      }
      int fd = open(fileName.c_str(), O_RDONLY | S_IRUSR);
      dup2(fd, 0);

      char** args = vec_to_char_array(arguments);
      execvp(args[0], args);
      close(fd);

    } else if (inputline[i] == '>') {
      io = true;
      curr = trim(curr);
      string fileName = "";
      int temp = i+2;
      while(inputline[temp] !=' ' && temp < inputline.size()) {
        fileName += inputline[temp];
        temp++;
      }

      vector<string> arguments = split(curr);

      int fd = open(fileName.c_str(), O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
      dup2(fd, 1);
      char** args = vec_to_char_array(arguments);
      execvp(args[0], args);
      close(fd);
    } else {
      curr += inputline[i];
    }
  }

  // preparing the input command for execution
  if(!io) {
    vector<string> parts = split(inputline);
    char** args = vec_to_char_array(parts);
    execvp (args[0], args);
  }

  return;
}

string currDirectory() {
char temp[512];
return (getcwd(temp, sizeof(temp)) ? std::string( temp ) : std::string(""));
}

string getUsername() {
return getlogin();
}

string getDate() {
  time_t t = time(0);
  char* dt = ctime(&t);
  string date(dt);
  return date.substr(0,date.size()-1);
}

int main (){
  vector<int> bgs;
  string prevDir = currDirectory();

  int in = dup(0);
  int out = dup(1);

  while (true){

    dup2(in, 0);
    dup2(out, 1);

    for(int i = 0; i < bgs.size(); i++) {
      if(waitpid(bgs[i], 0, WNOHANG) == bgs[i]) {
        //cout<<"Process: " << bgs[i]<<" ended";
        bgs.erase(bgs.begin()+i);
        i--;
      }
    }

    //print prompt and gets input / trims input
    cout << getDate() << " " <<getUsername() << "$ ";
    string inputline;
    getline (cin, inputline); //get a line from standard input
    inputline = trim(inputline);

    vector<string> cmds = split(inputline);

    //handles exit
    if(inputline == ""){
      continue;
    }


    //Handles cd command
    if(cmds[0] == "cd") {
      if(cmds[1] == "-") {
        int rc = chdir(prevDir.c_str());
      } else {
        string temp = currDirectory(); 
        int rc = chdir(cmds[1].c_str());
        if(rc >= 0) {
          prevDir = temp;
        } else {
          cout<<"Error with cd";
        }
      }
      continue;
    }

    //int pid = fork ();

    bool bg = false;
    if(inputline[inputline.size()-1] == '&') {
      bg = true;
      inputline = inputline.substr(0, inputline.size()-1);
    }

    vector<string> c = split(inputline, "|");

    if(c.size() > 1) {
      for (int i=0; i< c.size(); i++){
        int fd [2];
        pipe (fd);
        int pid = fork();
        if (!pid){ // in the child process
          if(i < c.size()-1) {
            dup2(fd[1], 1);
          }
          c[i] = trim(c[i]);
          logger(c[i]);
          execute(c[i]);
        }else{ // in the parent process
          if(!bg) {
            waitpid (pid, 0, 0); //parent waits for child process
            dup2(fd[0], 0);
            close(fd[1]);
          } else {
            bgs.push_back(pid);
          }
        }
      }
    } else {
      int pid = fork();
      if (pid == 0){ //child process
        execute(inputline);
      }else{
        if(!bg) {
          waitpid (pid, 0, 0); //parent waits for child process
        } else {
          bgs.push_back(pid);
        }
      }
    }

  }
  
  return 0;
}
