#include <cstdlib>
#include <iostream>
#include <memory>

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

constexpr auto default_host {"localhost"};
constexpr auto default_user {"worlduser"};
constexpr auto default_pass {"worldpass"};
constexpr auto default_db {"world"};

int main(int argc, char** argv)
{
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(nullptr);

  const std::string url      {argc >= 2 ? argv[1] : default_host};
  const std::string user     {argc >= 3 ? argv[2] : default_user};
  const std::string pass     {argc >= 4 ? argv[3] : default_pass};
  const std::string database {argc >= 5 ? argv[4] : default_db};

  try {
    sql::Driver* driver {get_driver_instance()};
    std::unique_ptr<sql::Connection> con {driver->connect(url, user, pass)};
    con->setSchema(database);
    std::unique_ptr<sql::Statement> stmt {con->createStatement()};

    std::unique_ptr<sql::ResultSet> res {stmt->executeQuery("SELECT count(*) from country")};
    res->next();
    std::cout << "count = " << res->getInt(1) << std::endl;
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
