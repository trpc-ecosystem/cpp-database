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

#include <string>

#include "mysqlclient/mysql.h"

namespace trpc::mysql {

/// @brief A common type for MySQL time.
class MysqlTime {
 public:
  MysqlTime();

  explicit MysqlTime(MYSQL_TIME my_time);

  MysqlTime& SetYear(unsigned int year);

  MysqlTime& SetMonth(unsigned int month);

  MysqlTime& SetDay(unsigned int day);

  MysqlTime& SetHour(unsigned int hour);

  MysqlTime& SetMinute(unsigned int minute);

  MysqlTime& SetSecond(unsigned int second);

  MysqlTime& SetSecondPart(unsigned long second_part);

  MysqlTime& SetTimeType(enum_mysql_timestamp_type time_type);

  unsigned int GetYear() const;

  unsigned int GetMonth() const;

  unsigned int GetDay() const;

  unsigned int GetHour() const;

  unsigned int GetMinute() const;

  unsigned int GetSecond() const;

  unsigned long SetSecondPart() const;

  enum_mysql_timestamp_type GetTimeType() const;

  /// @brief Converts the MYSQL_TIME object to a string representation.
  /// The format of the string is "YYYY-MM-DD HH:MM:SS".
  std::string ToString() const;

  /// @brief Parses a string in the format "YYYY-MM-DD HH:MM:SS"
  /// and updates the MYSQL_TIME object accordingly.
  /// The input string must match the expected format.
  void FromString(const std::string& time_str);

  /// @brief For mysql_binder.h
  const char* DataConstPtr() const;

 private:
  MYSQL_TIME mt_{};
};

class MysqlBlob {
 public:
  MysqlBlob() = default;

  MysqlBlob(const MysqlBlob& other) = default;

  MysqlBlob(MysqlBlob&& other) noexcept;

  explicit MysqlBlob(const std::string& data);

  explicit MysqlBlob(std::string&& data) noexcept;

  MysqlBlob(const char* data, std::size_t length);

  MysqlBlob& operator=(const MysqlBlob& other);

  MysqlBlob& operator=(MysqlBlob&& other) noexcept;

  bool operator==(const MysqlBlob& other) const;

  const char* DataConstPtr() const;

  size_t Size() const;

  std::string_view AsStringView();

 private:
  std::string data_;
};

}  // namespace trpc::mysql
