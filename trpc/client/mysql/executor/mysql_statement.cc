//
//
// Tencent is pleased to support the open source community by making tRPC available.
//
// Copyright (C) 2024 THL A29 Limited, a Tencent company.
// All rights reserved.
//
// If you have downloaded a copy of the tRPC source code from Tencent,
// please note that tRPC source code is licensed under the GNU General Public License Version 2.0 (GPLv2),
// A copy of the GPLv2 is included in this file.
//
//

#include "trpc/client/mysql/executor/mysql_statement.h"
#include <stdexcept>

namespace trpc::mysql {

MysqlStatement::MysqlStatement(MYSQL* conn)
                :mysql_stmt_(nullptr), mysql_(conn), field_count_(0) {
}


bool MysqlStatement::CloseStatement() {
  if(mysql_stmt_ != nullptr) {
    if(mysql_stmt_free_result(mysql_stmt_) != 0)
      return false;
    mysql_stmt_close(mysql_stmt_);
    mysql_stmt_ = nullptr;
  }

  return true;
}

bool MysqlStatement::Init(const std::string& sql) {
  mysql_stmt_ = mysql_stmt_init(mysql_);
  if (mysql_stmt_ == nullptr)
    return false;

  if(mysql_stmt_prepare(mysql_stmt_, sql.c_str(), sql.length()) != 0)
    return false;

  field_count_ = mysql_stmt_field_count(mysql_stmt_);
  params_count_ = mysql_stmt_param_count(mysql_stmt_);
  return true;
}

std::string MysqlStatement::GetErrorMessage() {
  if(mysql_stmt_ != nullptr)
    return std::string(mysql_stmt_error(mysql_stmt_));
  return "";
}

int MysqlStatement::GetErrorNumber() {
  if(mysql_stmt_ != nullptr)
    return mysql_stmt_errno(mysql_stmt_);
  return 0;
}

bool MysqlStatement::BindParam(std::vector<MYSQL_BIND> &bind_list) {

  return mysql_stmt_bind_param(mysql_stmt_, bind_list.data()) == 0? true : false;
}

MYSQL_RES* MysqlStatement::GetResultsMeta() {
  return mysql_stmt_result_metadata(mysql_stmt_);
}


}  // namespace trpc::mysql