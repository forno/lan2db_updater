#include <fstream>
#include <iostream>

int main(int argc, char** argv)
{
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(nullptr);

  std::string s;
  std::getline(std::cin, s);
  std::ofstream fs {"/tmp/syslog_develop.log"};
  if (!fs) {
    std::cerr << "can't open" << std::endl;
    return 1;
  }
  fs << s << '\n';
}
