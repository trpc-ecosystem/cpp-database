//
//
// Tencent is pleased to support the open source community by making tRPC available.
//
// Copyright (C) 2023 THL A29 Limited, a Tencent company.
// All rights reserved.
//
// If you have downloaded a copy of the tRPC source code from Tencent,
// please note that tRPC source code is licensed under the  Apache 2.0 License,
// A copy of the Apache 2.0 License is included in this file.
//
//


#pragma once

#include <cstdint>
#include <string>

namespace trpc::mysql {

struct MysqlExecutorPoolOption {
  /// @brief User nmae
  std::string user_name;

  /// @brief user password
  std::string password;

  /// @brief db name
  std::string dbname;

  std::string char_set;

  uint32_t max_size{0};       // Maximum number of connections in the pool
  uint64_t max_idle_time{0};  // Maximum idle time for connections
  uint32_t num_shard_group{4};

};

}  // namespace trpc::mysql