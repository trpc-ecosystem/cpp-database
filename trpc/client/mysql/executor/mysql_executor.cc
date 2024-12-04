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

#include "trpc/client/mysql/executor/mysql_executor.h"
#include "trpc/util/log/logging.h"

namespace trpc::mysql {

std::mutex MysqlExecutor::mysql_mutex;
constexpr unsigned int TRPC_MYSQL_API_TIMEOUT = 5;
constexpr int RECONNECT_INIT_RETRY_INTERVAL = 100;
constexpr int RECONNECT_MAX_RETRY = 5;


MysqlExecutor::MysqlExecutor(const MysqlConnOption& option)
    : is_connected(false),
      option_(option) {
  {
    std::lock_guard<std::mutex> lock(mysql_mutex);
    mysql_ = mysql_init(nullptr);
  }
  mysql_set_character_set(mysql_, option_.char_set.c_str());

  unsigned int timeout = TRPC_MYSQL_API_TIMEOUT;
  mysql_options(mysql_, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
  mysql_options(mysql_, MYSQL_OPT_READ_TIMEOUT, &timeout);
  mysql_options(mysql_, MYSQL_OPT_WRITE_TIMEOUT, &timeout);

}

bool MysqlExecutor::Connect() {

  if(is_connected)
      return true;

  if(mysql_ == nullptr) {
    std::lock_guard<std::mutex> lock(mysql_mutex);
    mysql_ = mysql_init(nullptr);
  }

  MYSQL* ret = mysql_real_connect(mysql_, option_.hostname.c_str(), option_.username.c_str(),
                                  option_.password.c_str(), option_.database.c_str(),
                                  option_.port, nullptr, 0);

  if (nullptr == ret) {
    mysql_close(mysql_);
    is_connected = false;
    return false;
  }

  is_connected = true;
  return true;
}

MysqlExecutor::~MysqlExecutor() {
  // Usually it will call Close() before destructor
  TRPC_ASSERT(is_connected == false);
  Close();
}

void MysqlExecutor::Close() {
  if (mysql_ != nullptr && is_connected) {
    mysql_close(mysql_);
    mysql_ = nullptr;
  }
  is_connected = false;
}

Status MysqlExecutor::ExecuteStatement(std::vector<MYSQL_BIND>& output_binds, MysqlStatement& statement) {
  Status s;
  if (!CheckAlive()) {
    if (!StartReconnect()) {
      // Get error from mysql_ not statement.
      s.SetFrameworkRetCode(GetErrorNumber());
      s.SetErrorMessage(GetErrorMessage());
      return s;
    }
  }

  if (mysql_stmt_bind_result(statement.STMTPointer(), output_binds.data()) != 0) {
    s.SetFrameworkRetCode(statement.GetErrorNumber());
    s.SetErrorMessage(statement.GetErrorMessage());
    return s;
  }

  if (mysql_stmt_execute(statement.STMTPointer()) != 0) {
    s.SetFrameworkRetCode(statement.GetErrorNumber());
    s.SetErrorMessage(statement.GetErrorMessage());
    return s;
  }

  return s;
}

Status MysqlExecutor::ExecuteStatement(MysqlStatement& statement) {
  Status s;
  if (!CheckAlive()) {
    if (!StartReconnect()) {
      // Get error from mysql_ not statement.
      s.SetFrameworkRetCode(GetErrorNumber());
      s.SetErrorMessage(GetErrorMessage());
      return s;
    }
  }
  if (mysql_stmt_execute(statement.STMTPointer()) != 0) {
    s.SetFrameworkRetCode(statement.GetErrorNumber());
    s.SetErrorMessage(statement.GetErrorMessage());
    return s;
  }

  return s;
}

uint64_t MysqlExecutor::GetAliveTime() const {
  uint64_t now = trpc::GetSteadyMilliSeconds();
  uint64_t alive_time = now - m_alivetime;
  return alive_time;
}

void MysqlExecutor::RefreshAliveTime() { m_alivetime = trpc::GetSteadyMilliSeconds(); }

bool MysqlExecutor::StartReconnect() {
  int retry_interval = RECONNECT_INIT_RETRY_INTERVAL;
  bool reconnected = false;
  for (int i = 0; i < RECONNECT_MAX_RETRY; ++i) {
    if (Reconnect()) {
      reconnected = true;
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval));
    retry_interval = std::min(retry_interval * 2, 3000);
  }
  return reconnected;
}

bool MysqlExecutor::Reconnect() { return Connect(); }

bool MysqlExecutor::CheckAlive() {
  if(!is_connected)
      return false;

  if (mysql_ != nullptr && mysql_ping(mysql_) == 0) {
    return true;
  } else {
    is_connected = false;
    return false;
  }
}

size_t MysqlExecutor::ExecuteInternal(const std::string& query, MysqlResults<OnlyExec>& mysql_results) {
  if (mysql_real_query(mysql_, query.c_str(), query.length()) != 0) {
    mysql_results.SetErrorMessage(GetErrorMessage());
    mysql_results.SetErrorNumber(GetErrorNumber());
    return 0;
  }

  return mysql_affected_rows(mysql_);
}

void MysqlExecutor::SetExecutorId(uint64_t eid) {
  executor_id_ = eid;
}

uint64_t MysqlExecutor::GetExecutorId() const {
  return executor_id_;
}

std::string MysqlExecutor::GetIp() const {
  return option_.hostname;
}

uint16_t MysqlExecutor::GetPort() const {
  return option_.port;
}

int MysqlExecutor::GetErrorNumber() {
  return mysql_errno(mysql_);
}

std::string MysqlExecutor::GetErrorMessage() {
  return mysql_error(mysql_);
}

bool MysqlExecutor::Autocommit(bool mode) {
  unsigned mode_n = mode ? 1 : 0;

  // Sets autocommit mode on if mode_n is 1, off if mode is 0.
  if(mysql_autocommit(mysql_, mode_n) != 0)
    return false;

  auto_commit_ = mode;
  return true;
}

bool MysqlExecutor::IsConnected() {
  return is_connected;
}

}  // namespace trpc::mysql
