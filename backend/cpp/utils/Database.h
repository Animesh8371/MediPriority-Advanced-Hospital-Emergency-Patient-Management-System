#ifndef MEDIPRIORITY_DATABASE_H
#define MEDIPRIORITY_DATABASE_H

#include "Config.h"
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <memory>
#include <string>

namespace medipriority {

/*
 * Thin wrapper around MySQL Connector/C++'s classic JDBC-style API
 * (sql::Driver / sql::Connection / sql::PreparedStatement).
 *
 * One Database instance owns one live sql::Connection. Repositories take a
 * Database& and call prepareStatement()/connection() to run parameterized
 * queries -- never raw string-concatenated SQL, to avoid injection.
 *
 * NOTE: this is intentionally a single-connection wrapper suitable for an
 * academic MVP demonstrating correct connection management (open once at
 * startup, reuse, close on shutdown) rather than a production connection
 * pool. Known limitation, documented in docs/architecture.md.
 */
class Database {
private:
    std::unique_ptr<sql::Connection> connection_;

public:
    explicit Database(const Config &config) {
        sql::Driver *driver = get_driver_instance();
        std::string url = "tcp://" + config.dbHost + ":" + config.dbPort;
        connection_.reset(driver->connect(url, config.dbUser, config.dbPassword));
        connection_->setSchema(config.dbName);
    }

    sql::Connection *connection() { return connection_.get(); }

    std::unique_ptr<sql::PreparedStatement> prepare(const std::string &sql) {
        return std::unique_ptr<sql::PreparedStatement>(connection_->prepareStatement(sql));
    }

    void beginTransaction() { connection_->setAutoCommit(false); }
    void commit() { connection_->commit(); connection_->setAutoCommit(true); }
    void rollback() { connection_->rollback(); connection_->setAutoCommit(true); }
};

} // namespace medipriority

#endif // MEDIPRIORITY_DATABASE_H
