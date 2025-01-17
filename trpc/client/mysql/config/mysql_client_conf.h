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

#include "yaml-cpp/yaml.h"

namespace trpc::mysql {

/// @brief Client config for accessing mysql
/// Mainly contains authentication information
struct MysqlClientConf {
  /// @brief User nmae
  std::string user_name;

  /// @brief user password
  std::string password;

  /// @brief db name
  std::string dbname;

  std::string char_set{"utf8mb4"};

  // Thread num for thread pool
  size_t thread_num{4};

  // thread_bind_core for thread pool
  // An input example of '1,5-7' will be converted to a list of [1, 5, 6, 7].
  std::string thread_bind_core;

  /// Only For MysqlExecutorPoolImpl
  uint32_t num_shard_group{4};

  void Display() const;
};

}  // namespace trpc::mysql
