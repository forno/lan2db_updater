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

constexpr auto default_host {"localhost"};
constexpr auto default_user {"lanwatcher"};
constexpr auto default_pass {"lanwatcherpass"};
constexpr auto default_db   {"lan"};

namespace
{

constexpr auto msg_for_desable {"disassociated"};
constexpr auto msg_for_enable {"associated"};
const std::regex mac_address_regex {R"([\da-f]{2}(:[\da-f]{2}){5})"};

class access_data
{
  bool connecting_flg {false};
  bool enable {false};
  std::string mac_address {};

public:
  access_data() = default;

  access_data(const std::string log)
    : connecting_flg {log.find(msg_for_desable) == std::string::npos},
      enable {connecting_flg || log.find(msg_for_enable) != std::string::npos}
  {
    if (!enable)
      return;
    std::sregex_iterator match_it {log.begin(), log.end(), mac_address_regex};
    if (match_it != std::sregex_iterator{})
      mac_address = match_it->str();

    assert(!mac_address.empty());
  }

  operator bool()
  {
    return enable;
  }

  bool is_connecting()
  {
    return connecting_flg;
  }

  std::string get_address()
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
    std::cerr << "Cannot read the log data";
    return EXIT_FAILURE;
  };

  access_data data {std::move(log)};
  if (!data) // invalid log
    return EXIT_SUCCESS;

  std::stringstream sql;
  sql << "CALL update_mac_address('" << data.get_address() << "','" << std::boolalpha << data.is_connecting() << "')";

  try {
    sql::Driver* driver {get_driver_instance()};
    std::unique_ptr<sql::Connection> con {driver->connect(url, user, pass)};
    con->setSchema(database);
    std::unique_ptr<sql::Statement> stmt {con->createStatement()};

    stmt->execute(sql.str());
  } catch (sql::SQLException &e) {
    std::cerr << "# ERR: SQLException in " << __FILE__;
    std::cerr << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
    std::cerr << "# ERR: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
