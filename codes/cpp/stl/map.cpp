#include <stdio.h>
#include <map>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{

  map<string, string> ss;
  ss.insert(pair<string, string> ("hello", "world"));

  map<string, string>::iterator it = ss.begin();
  for (; it != ss.end(); ++it) {
    printf("%s => %s\n", it->first.c_str(), it->second.c_str());
  }

  ss["cc"] = "linux";

  it = ss.find("hello");
  if (it != ss.end()) {
    printf("found key:%s value:%s\n", it->first.c_str(), it->second.c_str());
  }

  it = ss.find("world");
  if (it != ss.end()) {
    printf("found key:%s value:%s\n", it->first.c_str(), it->second.c_str());
  }

  it = ss.find("cc");
  if (it != ss.end()) {
    printf("found key:%s value:%s\n", it->first.c_str(), it->second.c_str());
  }



  return 0;
}
