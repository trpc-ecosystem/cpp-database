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

#include <type_traits>
#include <unordered_set>

#include "mysqlclient/mysql.h"
#include "trpc/util/string_util.h"

#include "trpc/client/mysql/executor/mysql_type.h"

namespace trpc::mysql {

template <typename T>
void* BindPointerCast(const T* ptr) {
  return const_cast<void*>(static_cast<const void*>(ptr));
}

///@brief map type information for mysql api
template <typename T>
struct MysqlInputType {
  static constexpr enum_field_types value = MYSQL_TYPE_NULL;
  static constexpr bool is_unsigned = false;
};

#define MYSQL_INPUT_TYPE_SPECIALIZATION(c_type, mysql_type, is_unsigned_type) \
  template <>                                                                 \
  struct MysqlInputType<c_type> {                                             \
    static constexpr enum_field_types value = mysql_type;                     \
    static constexpr bool is_unsigned = is_unsigned_type;                     \
  };

// Used for `StepInputBind(MYSQL_BIND& bind, const T& value)`
MYSQL_INPUT_TYPE_SPECIALIZATION(int8_t, MYSQL_TYPE_TINY, false)
MYSQL_INPUT_TYPE_SPECIALIZATION(uint8_t, MYSQL_TYPE_TINY, true)
MYSQL_INPUT_TYPE_SPECIALIZATION(int16_t, MYSQL_TYPE_SHORT, false)
MYSQL_INPUT_TYPE_SPECIALIZATION(uint16_t, MYSQL_TYPE_SHORT, true)
MYSQL_INPUT_TYPE_SPECIALIZATION(int32_t, MYSQL_TYPE_LONG, false)
MYSQL_INPUT_TYPE_SPECIALIZATION(uint32_t, MYSQL_TYPE_LONG, true)
MYSQL_INPUT_TYPE_SPECIALIZATION(int64_t, MYSQL_TYPE_LONGLONG, false)
MYSQL_INPUT_TYPE_SPECIALIZATION(uint64_t, MYSQL_TYPE_LONGLONG, true)
MYSQL_INPUT_TYPE_SPECIALIZATION(float, MYSQL_TYPE_FLOAT, false)
MYSQL_INPUT_TYPE_SPECIALIZATION(double, MYSQL_TYPE_DOUBLE, false)
MYSQL_INPUT_TYPE_SPECIALIZATION(std::string, MYSQL_TYPE_STRING, true)
MYSQL_INPUT_TYPE_SPECIALIZATION(MysqlTime, MYSQL_TYPE_DATETIME, true)
MYSQL_INPUT_TYPE_SPECIALIZATION(MysqlBlob, MYSQL_TYPE_BLOB, true)

template <typename T>
struct MysqlOutputType {
  static const std::unordered_set<enum_field_types> types;
};

#define MYSQL_OUTPUT_TYPE_SPECIALIZATION(c_type)             \
  template <>                                                \
  struct MysqlOutputType<c_type> {                           \
    static const std::unordered_set<enum_field_types> types; \
  };

MYSQL_OUTPUT_TYPE_SPECIALIZATION(int8_t)
MYSQL_OUTPUT_TYPE_SPECIALIZATION(uint8_t)
MYSQL_OUTPUT_TYPE_SPECIALIZATION(int16_t)
MYSQL_OUTPUT_TYPE_SPECIALIZATION(uint16_t)
MYSQL_OUTPUT_TYPE_SPECIALIZATION(int32_t)
MYSQL_OUTPUT_TYPE_SPECIALIZATION(uint32_t)
MYSQL_OUTPUT_TYPE_SPECIALIZATION(int64_t)
MYSQL_OUTPUT_TYPE_SPECIALIZATION(uint64_t)
MYSQL_OUTPUT_TYPE_SPECIALIZATION(float)
MYSQL_OUTPUT_TYPE_SPECIALIZATION(double)
MYSQL_OUTPUT_TYPE_SPECIALIZATION(MysqlTime)
MYSQL_OUTPUT_TYPE_SPECIALIZATION(std::string)
MYSQL_OUTPUT_TYPE_SPECIALIZATION(MysqlBlob)

template <typename T>
bool OutputTypeValid(enum_field_types mysql_type) {
  return MysqlOutputType<T>::types.count(mysql_type) > 0;
}

// ***********
// Input Bind
// ***********

/// @brief enable_if to bypass types convertible to std::string_view like char*.
template <typename T, typename = std::enable_if_t<!std::is_convertible<T, std::string_view>::value>>
void StepInputBind(MYSQL_BIND& bind, const T& value) {
  std::memset(&bind, 0, sizeof(bind));
  bind.buffer_type = MysqlInputType<T>::value;
  bind.buffer = BindPointerCast(&value);
  bind.is_unsigned = MysqlInputType<T>::is_unsigned;
}

inline void StepInputBind(MYSQL_BIND& bind, const MysqlBlob& value) {
  std::memset(&bind, 0, sizeof(bind));
  bind.buffer_type = MYSQL_TYPE_BLOB;
  bind.buffer = BindPointerCast(value.DataConstPtr());
  bind.buffer_length = value.Size();
  bind.length = &bind.buffer_length;
  bind.is_unsigned = false;
}

/// @brief Overload for MysqlTime. This avoids relying on the general template
/// to prevent issues if the MysqlTime class changes and value.DataConstPtr() != &value.
inline void StepInputBind(MYSQL_BIND& bind, const MysqlTime& value) {
  std::memset(&bind, 0, sizeof(bind));
  bind.buffer_type = MYSQL_TYPE_DATETIME;
  bind.buffer = BindPointerCast(value.DataConstPtr());
  bind.is_unsigned = true;
}

///@brief use string_view to handle "string", "char *, char[N]"
inline void StepInputBind(MYSQL_BIND& bind, std::string_view value) {
  std::memset(&bind, 0, sizeof(bind));
  bind.buffer_type = MYSQL_TYPE_STRING;
  bind.buffer = BindPointerCast(value.data());
  bind.buffer_length = value.length();
  bind.length = &bind.buffer_length;
  bind.is_unsigned = false;
}

template <typename... InputArgs>
void BindInputImpl(std::vector<MYSQL_BIND>& binds, const InputArgs&... args) {
  binds.resize(sizeof...(InputArgs));
  int i = 0;
  (StepInputBind(binds[i++], args), ...);
}

// ***********
// Output Bind
// ***********

constexpr int TRPC_BIND_BUFFER_MIN_SIZE = 32;

/// @note the buffer.buffer_type has been set before this function according to the fields meta.
template <typename T>
void StepOutputBind(MYSQL_BIND& bind, std::vector<std::byte>& buffer, uint8_t& null_flag) {
  buffer.resize(sizeof(T));
  bind.buffer = buffer.data();
  bind.is_null = reinterpret_cast<bool*>(&null_flag);
}

template <>
inline void StepOutputBind<std::string>(MYSQL_BIND& bind, std::vector<std::byte>& buffer, uint8_t& null_flag) {
  // Strings are a special case. If the output is received as a string, the buffer type must be explicitly set
  // to MYSQL_TYPE_STRING. For example, if the field is of a date type, its buffer type is initially set to
  // the corresponding date type. However, if the user intends to receive the field as a string, we must
  // override the buffer type to MYSQL_TYPE_STRING to avoid errors.
  bind.buffer_type = MYSQL_TYPE_STRING;
  if (buffer.empty()) {
    buffer.resize(TRPC_BIND_BUFFER_MIN_SIZE);  // buffer size will usually be set by MysqlExecutor::QueryHandle
                                               // according to the MysqlResultsOption
  }
  bind.buffer = buffer.data();
  bind.is_null = reinterpret_cast<bool*>(&null_flag);
  bind.buffer_length = buffer.size();
}

template <>
inline void StepOutputBind<MysqlBlob>(MYSQL_BIND& bind, std::vector<std::byte>& buffer, uint8_t& null_flag) {
  if (buffer.empty()) {
    buffer.resize(TRPC_BIND_BUFFER_MIN_SIZE);  // buffer size will usually be set by MysqlExecutor::QueryHandle
                                               // according to the MysqlResultsOption
  }
  bind.buffer = buffer.data();
  bind.is_null = reinterpret_cast<bool*>(&null_flag);
  bind.buffer_length = buffer.size();
}

template <typename... OutputArgs>
void BindOutputImpl(std::vector<MYSQL_BIND>& output_binds, std::vector<std::vector<std::byte>>& output_buffers,
                           std::vector<uint8_t>& null_flag_buffer) {
  size_t i = 0;
  ((StepOutputBind<OutputArgs>(output_binds[i], output_buffers[i], null_flag_buffer[i]), i++), ...);
}

template <typename... Args>
std::string CheckFieldsOutputArgs(MYSQL_RES* res) {
  std::string error;
  unsigned int num_fields = mysql_num_fields(res);
  if (num_fields != sizeof...(Args)) {
    error = util::FormatString("The query field count is {}, but you give {} OutputArgs.", num_fields, sizeof...(Args));
    return error;
  }

  MYSQL_FIELD* fields_meta = mysql_fetch_fields(res);

  unsigned long i = 0;
  std::vector<unsigned long> failed_index;

  ((OutputTypeValid<Args>(fields_meta[i].type) ? (void)i++ : failed_index.push_back(i++)), ...);

  if (!failed_index.empty()) {
    error = "Bind output type warning for fields: (";
    error += std::string(fields_meta[failed_index[0]].name);
    for (i = 1; i < failed_index.size(); i++) error.append(", ").append(fields_meta[failed_index[i]].name);
    error.append(").");
    return error;
  }
  return "";
}

// ****************
// Result Tuple Set
// ****************

template <typename T>
void StepTupleSet(T& value, const MYSQL_BIND& bind) {
  value = *static_cast<const decltype(&value)>(bind.buffer);
}

inline void StepTupleSet(MysqlTime& value, const MYSQL_BIND& bind) { value = {*static_cast<MysqlTime*>(bind.buffer)}; }

inline void StepTupleSet(std::string& value, const MYSQL_BIND& bind) {
  if ((*bind.is_null) == 0) value.assign(static_cast<const char*>(bind.buffer), *(bind.length));
}

inline void StepTupleSet(MysqlBlob& value, const MYSQL_BIND& bind) {
  if ((*bind.is_null) == 0) value = MysqlBlob(static_cast<const char*>(bind.buffer), *(bind.length));
}

template <typename... OutputArgs>
void SetResultTuple(std::tuple<OutputArgs...>& result, const std::vector<MYSQL_BIND>& output_binds) {
  std::apply(
      [&output_binds](auto&... args) {
        size_t i = 0;
        ((StepTupleSet(args, output_binds[i++])), ...);
      },
      result);
}

}  // namespace trpc::mysql
