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

#include "trpc/client/mysql/mysql_executor_pool_manager.h"

namespace trpc::mysql {

MysqlExecutorPoolManager::MysqlExecutorPoolManager(const MysqlExecutorPoolOption& option) : option_(option) {}

MysqlExecutorPool* MysqlExecutorPoolManager::Get(const NodeAddr& node_addr) {
  const int len = 64;
  std::string endpoint(len, 0x0);
  std::snprintf(const_cast<char*>(endpoint.c_str()), len, "%s:%d", node_addr.ip.c_str(), node_addr.port);

  MysqlExecutorPool* executor_pool{nullptr};
  bool ret = executor_pools_.Get(endpoint, executor_pool);

  if (ret) {
    return executor_pool;
  }

  MysqlExecutorPool* pool = CreateExecutorPool(node_addr);
  ret = executor_pools_.GetOrInsert(endpoint, pool, executor_pool);
  if (!ret) {
    return pool;
  }

  delete pool;
  pool = nullptr;

  return executor_pool;
}

MysqlExecutorPool* MysqlExecutorPoolManager::CreateExecutorPool(const NodeAddr& node_addr) {
  MysqlExecutorPool* new_pool{nullptr};
  new_pool = new MysqlExecutorPool(option_, node_addr);
  return new_pool;
}

void MysqlExecutorPoolManager::Stop() {
  executor_pools_.GetAllItems(pools_to_destroy_);

  for (auto& [key, pool] : pools_to_destroy_) pool->Stop();
}

void MysqlExecutorPoolManager::Destroy() {
  for (auto& [key, pool] : pools_to_destroy_) {
    pool->Destroy();
    delete pool;
    pool = nullptr;
  }

  executor_pools_.Reclaim();
  pools_to_destroy_.clear();
}

}  // namespace trpc::mysql
