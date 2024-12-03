
#pragma once

#include "trpc/common/status.h"

namespace trpc::mysql {

/// The error numbers are the same with MySQL https://dev.mysql.com/doc/mysql-errors/8.0/en/error-reference-introduction.html
/// except below

enum TrpcMysqlRetCode : int {
  TRPC_MYSQL_DB_CONNECTION_ERR = 3501,
  TRPC_MYSQL_INVALID_HANDLE = 3502
};
}
