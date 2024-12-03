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

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include "trpc/client/mysql/mysql_executor_pool.h"
#include "trpc/transport/common/transport_message_common.h"
#include "trpc/util/concurrency/lightly_concurrent_hashmap.h"

namespace trpc::mysql {

class MysqlExecutorPoolManager {
 public:
  explicit MysqlExecutorPoolManager(const MysqlExecutorPoolOption& option);

  MysqlExecutorPool* Get(const NodeAddr& node_addr);

  void Stop();

  void Destroy();

 private:
  MysqlExecutorPool* CreateExecutorPool(const NodeAddr& node_addr);

 private:
  concurrency::LightlyConcurrentHashMap<std::string, MysqlExecutorPool*> executor_pools_;
  std::unordered_map<std::string, MysqlExecutorPool*> pools_to_destroy_;
  MysqlExecutorPoolOption option_;
};

}