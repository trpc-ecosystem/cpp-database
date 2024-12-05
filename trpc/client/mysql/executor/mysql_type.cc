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

#include <sstream>
#include "trpc/client/mysql/executor/mysql_type.h"
#include "trpc/util/time.h"
#include "trpc/util/log/logging.h"

namespace trpc::mysql {


MysqlTime::MysqlTime() {
  mt_.year = 2024;
  mt_.month = 1;
  mt_.day = 1;
  mt_.hour = 0;
  mt_.minute = 0;
  mt_.second = 0;
  mt_.second_part = 0;
  mt_.time_type = MYSQL_TIMESTAMP_DATETIME;
  mt_.neg = 0;
}

MysqlTime::MysqlTime(MYSQL_TIME my_time) {
  mt_ = my_time;
}


MysqlTime &MysqlTime::SetYear(unsigned int year) {
  mt_.year = year;
  return *this;
}

MysqlTime &MysqlTime::SetMonth(unsigned int month) {
  if(month > 0 && month <= 12) {
    mt_.month = month;
  } else {
    TRPC_FMT_ERROR("MysqlTime::SetMonth ({}) Failed.", month);
  }
  return *this;
}

MysqlTime &MysqlTime::SetDay(unsigned int day) {
  mt_.day = day;
  return *this;
}

MysqlTime &MysqlTime::SetHour(unsigned int hour) {
  if(hour <= 24) {
    mt_.hour = hour;
  } else {
    TRPC_FMT_ERROR("MysqlTime::SetHour ({}) Failed.", hour);
  }
  return *this;
}

MysqlTime &MysqlTime::SetMinute(unsigned int minute) {
  if(minute <= 60) {
    mt_.minute = minute;
  } else {
    TRPC_FMT_ERROR("MysqlTime::SetMinute ({}) Failed.", minute);
  }
  return *this;
}

MysqlTime &MysqlTime::SetSecond(unsigned int second) {
  if(second <= 60) {
    mt_.second = second;
  } else {
    TRPC_FMT_ERROR("MysqlTime::SetSecond ({}) Failed.", second);
  }
  return *this;
}

MysqlTime &MysqlTime::SetSecondPart(unsigned long second_part) {
  mt_.second_part = second_part;
  return *this;
}


MysqlTime &MysqlTime::SetTimeType(enum_mysql_timestamp_type time_type) {
  mt_.time_type = time_type;
  return *this;
}

unsigned int MysqlTime::GetYear() const { return mt_.year; }

unsigned int MysqlTime::GetMonth() const { return mt_.month; }

unsigned int MysqlTime::GetDay() const { return mt_.day; }

unsigned int MysqlTime::GetHour() const { return mt_.hour; }

unsigned int MysqlTime::GetMinute() const { return mt_.minute; }

unsigned int MysqlTime::GetSecond() const { return mt_.second; }

unsigned long MysqlTime::SetSecondPart() const { return mt_.second_part; }

enum_mysql_timestamp_type MysqlTime::GetTimeType() const { return mt_.time_type; }

std::string MysqlTime::ToString() const {
  return time::StringFormat("%04d-%02d-%02d %02d:%02d:%02d", mt_.year, mt_.month,
                            mt_.day, mt_.hour, mt_.minute, mt_.second);
}

void MysqlTime::FromString(const std::string &timeStr) {
  std::istringstream iss(timeStr);
  char delimiter;
  iss >> mt_.year >> delimiter >> mt_.month >> delimiter >> mt_.day
      >> mt_.hour >> delimiter >> mt_.minute >> delimiter >> mt_.second;
}

const char *MysqlTime::DataConstPtr() const {
  return reinterpret_cast<const char *>(&mt_);
}



MysqlBlob::MysqlBlob(MysqlBlob &&other) noexcept:
        data_(std::move(other.data_)
        ) {
}


MysqlBlob::MysqlBlob(const std::string &data) : data_(data) {}

MysqlBlob::MysqlBlob(std::string &&data) noexcept: data_(std::move(data)) {}

MysqlBlob::MysqlBlob(const char *data, std::size_t length) : data_(data, length) {}

MysqlBlob &MysqlBlob::operator=(const MysqlBlob &other) {
  if (this != &other) {
    data_ = other.data_;
  }
  return *this;
}

MysqlBlob &MysqlBlob::operator=(MysqlBlob &&other)
noexcept {
  if (this != &other) {
    data_ = std::move(other.data_);
  }
  return *this;
}

bool MysqlBlob::operator==(const MysqlBlob &other) const {
  return data_ == other.data_;
}

const char *MysqlBlob::DataConstPtr() const {
  return data_.data();
}

size_t MysqlBlob::Size() const {
  return data_.size();
}

std::string_view MysqlBlob::AsStringView() {
  return data_;
}
}