#include <_types/_intmax_t.h>
#include <iostream>
#include <ios>
#include <sys/_types/_timespec.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <string>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctime>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <time.h>
#include <locale.h>
#include <filesystem>

using namespace std;

std::string numToMonth(int num) {
  switch(num) {
    case 1:
      return "Jan";
      break;
    case 2:
      return "Feb";
      break;
    case 3:
      return "Mar";
      break;
    case 4:
      return "Apr";
      break;
    case 5:
      return "May";
      break;
    case 6:
      return "Jun";
      break;
    case 7:
      return "Jul";
      break;
    case 8:
      return "Aug";
      break;
    case 9:
      return "Sep";
      break;
    case 10:
      return "Oct";
      break;
    case 11:
      return "Nov";
      break;
    case 12:
      return "Dec";
      break;
    default:
      return "Jan";
      break;
  }
}

std::string getSize(struct stat sb) {
  return std::to_string((intmax_t)sb.st_size);
}

std::string getPerms(struct stat sb) {
  std::string res = "";
  mode_t perm = sb.st_mode;
  switch (sb.st_mode & S_IFMT) {
    case S_IFDIR: 
      res += "d";
      break;
    default:
      res += "-";
      break;
  }
  res += (perm & S_IRUSR) ? 'r' : '-';
  res += (perm & S_IWUSR) ? 'w' : '-';
  res += (perm & S_IXUSR) ? 'x' : '-';
  res += (perm & S_IRGRP) ? 'r' : '-';
  res += (perm & S_IWGRP) ? 'w' : '-';
  res += (perm & S_IXGRP) ? 'x' : '-';
  res += (perm & S_IROTH) ? 'r' : '-';
  res += (perm & S_IWOTH) ? 'w' : '-';
  res += (perm & S_IXOTH) ? 'x' : '-';
  res += '\0';

  return res;
}

std::string getLinkCount(struct stat sb) {
  return std::to_string(sb.st_nlink);
}

std::string getUser(struct stat sb) {
  struct passwd *pw = getpwuid(sb.st_uid);
  return pw->pw_name;
}

std::string getGroup(struct stat sb) {
  struct group *g = getgrgid(sb.st_gid);
  return g->gr_name;
}

std::string getMonth(struct stat sb) {
  struct timespec tm;
  tm = sb.st_mtimespec; 
  char buff[100];
  strftime(buff, 100, "%D %T", gmtime(&tm.tv_sec));
  std::string mon = "";
  mon += buff[0];
  mon += buff[1];

  return numToMonth(stoi(mon));
}

std::string getDay(struct stat sb) {
  struct timespec tm;
  tm = sb.st_mtimespec; 
  char buff[100];
  strftime(buff, 100, "%D %T", gmtime(&tm.tv_sec));
  std::string mon = "";
  mon += buff[3];
  mon += buff[4];

  return mon;
}

std::string getTime(struct stat sb) {
  struct timespec tm;
  tm = sb.st_mtimespec; 
  char buff[100];
  strftime(buff, 100, "%D %T", gmtime(&tm.tv_sec));
  std::string mon = "";
  mon += buff[9];
  mon += buff[10];
  mon += buff[11];
  mon += buff[12];
  mon += buff[13];

  return mon;
}

int main(int argc, char *argv[]) {

  struct stat sb;
  std::string pathname = "./";

  bool isDir = false;

  if(argc != 2) {
    pathname = ".";
  } else {
    pathname = argv[1];

  } 

  stat((char*)pathname.c_str(), &sb);

  switch (sb.st_mode & S_IFMT) {
    case S_IFDIR:  isDir = true; break;
    default:       break;
  }

  if(isDir) {
    if(pathname[pathname.size()-1] != '/') {
      pathname += "/";
    }
  }

  std::string perms = getPerms(sb);
  std::string links = getLinkCount(sb);
  std::string user = getUser(sb);
  std::string group = getGroup(sb);
  std::string size = getSize(sb);
  std::string month = getMonth(sb);
  std::string day = getDay(sb);
  std::string time = getTime(sb);
  std::string name = pathname; 


  if(!isDir) {
    printf("%-*s %-*s %-*s %-*s %-*s %-3s %-2s %-5s %-*s\n", (int)perms.size(), perms.c_str(), (int)links.size(), links.c_str(), (int)user.size()+1, user.c_str(), (int)group.size()+1, group.c_str(), (int)size.size(), size.c_str(), month.c_str(), day.c_str(), time.c_str(), (int)name.size(), name.c_str());
  } else {
    string one = ".";
    string two = "..";
    DIR *dir;
    struct dirent *d;
    if((dir = opendir(pathname.c_str())) != NULL) {
      while((d = readdir(dir)) != NULL) {
        string path = pathname + d->d_name;
        if(path[path.size()-1] != '.' || path == ".") {
          stat((char*)path.c_str(), &sb);
          std::string perms = getPerms(sb);
          std::string links = getLinkCount(sb);
          std::string user = getUser(sb);
          std::string group = getGroup(sb);
          std::string size = getSize(sb);
          std::string month = getMonth(sb);
          std::string day = getDay(sb);
          std::string time = getTime(sb);
          std::string name = d->d_name;
          printf("%-*s %-*s %-*s %-*s %-*s %-3s %-2s %-5s %-*s\n", (int)perms.size(), perms.c_str(), (int)links.size(), links.c_str(), (int)user.size()+1, user.c_str(), (int)group.size()+1, group.c_str(), 5, size.c_str(), month.c_str(), day.c_str(), time.c_str(), (int)name.size(), name.c_str());
        }
      }
    }
  }


  return 0;
}
