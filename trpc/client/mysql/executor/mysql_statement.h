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

#pragma once

#include "mysqlclient/mysql.h"
#include "trpc/util/log/logging.h"

namespace trpc::mysql {

class MysqlStatement {
 public:
  MysqlStatement(MYSQL* conn);

  ~MysqlStatement() { TRPC_ASSERT(mysql_stmt_ == nullptr); }

  MysqlStatement(MysqlStatement&& rhs) = default;

  MysqlStatement(MysqlStatement& rhs) = delete;

  bool Init(const std::string& sql);

  std::string GetErrorMessage();

  int GetErrorNumber();

  bool BindParam(std::vector<MYSQL_BIND>& bind_list);

  bool CloseStatement();

  unsigned int GetFieldCount() { return field_count_; }

  unsigned long GetParamsCount() { return params_count_; }

  MYSQL_RES* GetResultsMeta();

  MYSQL_STMT* STMTPointer() { return mysql_stmt_; }

  bool IsValid() { return mysql_stmt_ == nullptr; }

 private:
  MYSQL_STMT* mysql_stmt_;

  MYSQL* mysql_;

  unsigned int field_count_;

  unsigned long params_count_;
};

}  // namespace trpc::mysql
