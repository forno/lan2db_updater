#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <utility>

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

namespace
{

constexpr auto default_host {"localhost"};
constexpr auto default_user {"lanwatcher"};
constexpr auto default_pass {"lanwatcherpass"};
constexpr auto default_db   {"lan"};

constexpr auto msg_for_desable {"disassociated"};
constexpr auto msg_for_enable {"associated"};
const std::regex mac_address_regex {R"([\da-f]{2}(:[\da-f]{2}){5})"};

class access_data
{
  bool enable {false};
  bool connecting_flg {false};
  std::string mac_address {};

public:
  access_data() = default;

  access_data(const std::string log)
    : enable {log.find(msg_for_enable) != std::string::npos},
      connecting_flg {enable && log.find(msg_for_desable) == std::string::npos}
  {
    if (!enable)
      return;
    std::sregex_iterator match_it {log.begin(), log.end(), mac_address_regex};
    if (match_it == std::sregex_iterator{}) {
      enable = false;
      return;
    }
    mac_address = match_it->str();

    assert(!mac_address.empty());
  }

  operator bool() const noexcept
  {
    return enable;
  }

  bool is_connecting() const noexcept
  {
    return connecting_flg;
  }

  std::string get_address() const noexcept
  {
    return mac_address;
  }
};

}

int main(int argc, char** argv)
{
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(nullptr);

  const std::string url      {argc >= 2 ? argv[1] : default_host};
  const std::string user     {argc >= 3 ? argv[2] : default_user};
  const std::string pass     {argc >= 4 ? argv[3] : default_pass};
  const std::string database {argc >= 5 ? argv[4] : default_db};

  std::string log;
  if (!std::getline(std::cin, log)) {
//    std::cerr << "Cannot read the log data";
    return EXIT_FAILURE;
  };

  access_data data {std::move(log)};
  if (!data) // invalid log
    return EXIT_SUCCESS;

  std::ostringstream sst;
  sst << "CALL update_mac_address('" << data.get_address() << "','" << std::boolalpha << data.is_connecting() << "')";

  try {
    sql::Driver* driver {get_driver_instance()};
    std::unique_ptr<sql::Connection> con {driver->connect(url, user, pass)};
    con->setSchema(database);
    std::unique_ptr<sql::Statement> stmt {con->createStatement()};

    stmt->execute(sst.str());
  } catch (sql::SQLException &e) {
//    std::cerr << "# ERR: " << e.what() <<
//      " (MySQL error code: " << e.getErrorCode() <<
//      ", SQLState: " << e.getSQLState() << " )" << std::endl;

    return EXIT_FAILURE;
  }
}
