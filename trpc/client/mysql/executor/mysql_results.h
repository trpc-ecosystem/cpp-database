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

#include "trpc/client/mysql/mysql_error_number.h"

namespace trpc::mysql {

constexpr size_t kDynamicBufferInitSize = 64;

struct MysqlResultsOption {
  // The initial size of the buffer used to store variable-length data
  // when fetching a row in BindType mode.
  // In many real-world applications, 64 bytes is sufficient to store common variable-length data
  size_t dynamic_buffer_init_size = kDynamicBufferInitSize;
};

///@brief Just specialize class MysqlResults
class OnlyExec {};

class NativeString {};

/// @brief Mode for `MysqlResults`
enum class MysqlResultsMode {
  // Bind query result data to tuples.
  BindType,
  // For SQL which will not return a result set data. But can get the
  // num of affected rows by GetAffectedRowNum()
  OnlyExec,
  // Return result data as vector of string_view.
  NativeString,
};

template <typename... Args>
struct ResultSetMapper {
  using type = std::vector<std::tuple<Args...>>;
  static constexpr MysqlResultsMode mode = MysqlResultsMode::BindType;
};

template <>
struct ResultSetMapper<NativeString> {
  using type = std::vector<std::vector<std::string_view>>;
  static constexpr MysqlResultsMode mode = MysqlResultsMode::NativeString;
};

template <>
struct ResultSetMapper<OnlyExec> {
  using type = std::vector<std::tuple<OnlyExec>>;
  static constexpr MysqlResultsMode mode = MysqlResultsMode::OnlyExec;
};

///
///@brief A class used to store the results of a MySQL query executed by the MysqlExecutor class.
///
/// The class template parameter `Args...` is used to define the types of data stored in the result set.
///
///- If `Args...` is `OnlyExec`, the class is intended for operations
///  that execute without returning a result set (e.g., INSERT, UPDATE).
///
///- If `Args...` is `NativeString`, the class handles operations that
///  return a `vector<vector<string_view>>>` result set.
///
///- If `Args...` includes common data types (e.g., int, std::string),
///  the class handles operations that return a `vector<tuple<Args...>>` result set.
///  The template args need to match the query field.  Notice that in this case, it will use
///  prepare statement to execute SQL. Pay attention to the bind type args. If the bind type args missmatch
///  the MySQL type in the table, it will be an undefined behaviour and will not raise an error message.
template <typename... Args>
class MysqlResults {
  friend class MysqlExecutor;

 public:
  static constexpr MysqlResultsMode mode = ResultSetMapper<Args...>::mode;

  using ResultSetType = typename ResultSetMapper<Args...>::type;

 public:
  MysqlResults();

  explicit MysqlResults(const MysqlResultsOption& option);

  MysqlResults(MysqlResults&& other) noexcept;

  MysqlResults& operator=(MysqlResults&& other) noexcept;

  MysqlResults(const MysqlResults& other) = delete;

  MysqlResults& operator=(const MysqlResults&) = delete;

  ~MysqlResults();

  auto& MutableResultSet();

  const auto& ResultSet() const;

  template <typename T>
  bool GetResultSet(T& res);

  const std::vector<std::vector<uint8_t>>& GetNullFlag();

  const std::vector<std::string>& GetFieldsName() const;

  const MysqlResultsOption& GetOption();

  size_t GetAffectedRowNum() const;

  bool IsValueNull(size_t row_index, size_t col_index) const;

  bool OK() const;

  void Clear();

  const std::string& GetErrorMessage();

  int GetErrorNumber() const;

 private:
  /// @brief Used for NativeString to the mysql_res_.
  /// @note  Must be called after Clear().
  void SetRawMysqlRes(MYSQL_RES* res);

  void SetFieldsName(MYSQL_RES* res);

  size_t SetAffectedRows(size_t n_rows);

  std::string& SetErrorMessage(const std::string& message);

  std::string& SetErrorMessage(std::string&& message);

  int SetErrorNumber(int error_number);

 private:
  MysqlResultsOption option_;

  ResultSetType result_set_;

  std::vector<std::string> fields_name_;

  std::vector<std::vector<uint8_t>> null_flags_;

  int error_number_{0};

  std::string error_message_;

  size_t affected_rows_;

  bool has_value_;

  /// @brief For NativeString, it will represent real data.
  MYSQL_RES* mysql_res_{nullptr};
};

template <typename... Args>
const std::vector<std::string>& MysqlResults<Args...>::GetFieldsName() const {
  return fields_name_;
}

template <typename... Args>
void MysqlResults<Args...>::SetFieldsName(MYSQL_RES* res) {
  if (!res) return;

  MYSQL_FIELD* fields_meta = mysql_fetch_fields(res);
  unsigned long fields_num = mysql_num_fields(res);

  for (unsigned long i = 0; i < fields_num; ++i) fields_name_.emplace_back(fields_meta[i].name);
}

template <typename... Args>
MysqlResults<Args...>& MysqlResults<Args...>::operator=(MysqlResults&& other) noexcept {
  if (this != &other) {
    option_ = std::move(other.option_);
    result_set_ = std::move(other.result_set_);
    fields_name_ = std::move(other.fields_name_);
    null_flags_ = std::move(other.null_flags_);
    error_message_ = std::move(other.error_message_);
    affected_rows_ = other.affected_rows_;
    has_value_ = other.has_value_;
    mysql_res_ = other.mysql_res_;

    other.mysql_res_ = nullptr;
  }
  return *this;
}

template <typename... Args>
MysqlResults<Args...>::MysqlResults(MysqlResults&& other) noexcept
    : option_(std::move(other.option_)),
      result_set_(std::move(other.result_set_)),
      fields_name_(std::move(other.fields_name_)),
      null_flags_(std::move(other.null_flags_)),
      error_message_(std::move(other.error_message_)),
      affected_rows_(other.affected_rows_),
      has_value_(other.has_value_),
      mysql_res_(other.mysql_res_) {
  other.mysql_res_ = nullptr;
}

template <typename... Args>
void MysqlResults<Args...>::SetRawMysqlRes(MYSQL_RES* res) {
  TRPC_ASSERT(mysql_res_ == nullptr);
  mysql_res_ = res;
}

template <typename... Args>
MysqlResults<Args...>::~MysqlResults() {
  if (mysql_res_) {
    mysql_free_result(mysql_res_);
    mysql_res_ = nullptr;
  }
}

template <typename... Args>
std::string& MysqlResults<Args...>::SetErrorMessage(std::string&& message) {
  error_message_ = std::move(message);
  return error_message_;
}

template <typename... Args>
std::string& MysqlResults<Args...>::SetErrorMessage(const std::string& message) {
  error_message_ = message;
  return error_message_;
}

template <typename... Args>
const std::string& MysqlResults<Args...>::GetErrorMessage() {
  return error_message_;
}

template <typename... Args>
int MysqlResults<Args...>::GetErrorNumber() const {
  return error_number_;
}

template <typename... Args>
int MysqlResults<Args...>::SetErrorNumber(int error_number) {
  return error_number_ = error_number;
}

template <typename... Args>
MysqlResults<Args...>::MysqlResults() : affected_rows_(0), has_value_(false), mysql_res_(nullptr) {
  // dummy
}

template <typename... Args>
MysqlResults<Args...>::MysqlResults(const MysqlResultsOption& option) : MysqlResults() {
  option_ = option;
}

template <typename... Args>
auto& MysqlResults<Args...>::MutableResultSet() {
  return result_set_;
}

template <typename... Args>
const auto& MysqlResults<Args...>::ResultSet() const {
  return result_set_;
}

template <typename... Args>
template <typename T>
bool MysqlResults<Args...>::GetResultSet(T& res) {
  if (!has_value_) return false;

  if constexpr (mode == MysqlResultsMode::NativeString) {
    res.clear();
    for (const auto& row : result_set_) {
      res.emplace_back(row.begin(), row.end());
    }

    return true;
  } else {
    res = std::move(result_set_);
    has_value_ = false;
    return true;
  }
}

template <typename... Args>
const std::vector<std::vector<uint8_t>>& MysqlResults<Args...>::GetNullFlag() {
  return null_flags_;
}

template <typename... Args>
const MysqlResultsOption& MysqlResults<Args...>::GetOption() {
  return option_;
}

template <typename... Args>
size_t MysqlResults<Args...>::GetAffectedRowNum() const {
  return affected_rows_;
}

template <typename... Args>
size_t MysqlResults<Args...>::SetAffectedRows(size_t n_rows) {
  affected_rows_ = n_rows;
  return affected_rows_;
}

template <typename... Args>
bool MysqlResults<Args...>::IsValueNull(size_t row_index, size_t col_index) const {
  if (null_flags_.empty()) return false;

  if (row_index >= null_flags_.size() || col_index >= null_flags_[0].size()) return false;

  return null_flags_[row_index][col_index];
}

template <typename... Args>
bool MysqlResults<Args...>::OK() const {
  return error_number_ == 0;
}

template <typename... Args>
void MysqlResults<Args...>::Clear() {
  null_flags_.clear();
  error_message_.clear();
  fields_name_.clear();
  has_value_ = false;
  affected_rows_ = 0;
  MutableResultSet().clear();

  if (mysql_res_) {
    mysql_free_result(mysql_res_);
    mysql_res_ = nullptr;
  }
}

}  // namespace trpc::mysql
