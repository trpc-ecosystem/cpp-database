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

#include "trpc/client/mysql/mysql_executor_pool.h"
#include <stdexcept>
#include <thread>
#include "trpc/client/mysql/executor/mysql_executor.h"

namespace trpc {
namespace mysql {

MysqlExecutorPool::MysqlExecutorPool(const MysqlExecutorPoolOption& option, const NodeAddr& node_addr)
    : pool_option_(option), target_((node_addr)){
  executor_shards_ = std::make_unique<Shard[]>(option.num_shard_group);
  max_num_per_shard_ = std::ceil(pool_option_.max_size / option.num_shard_group);
}

RefPtr<MysqlExecutor> MysqlExecutorPool::GetOrCreate() {
  RefPtr<MysqlExecutor> executor{nullptr};
  RefPtr<MysqlExecutor> idle_executor{nullptr};

  uint32_t shard_id = shard_id_gen_.fetch_add(1);
  int retry_num = 3;

  while (retry_num > 0) {
    auto& shard = executor_shards_[shard_id % pool_option_.num_shard_group];

    {
      std::scoped_lock _(shard.lock);
      if (shard.mysql_executors.size() > 0) {
        executor = shard.mysql_executors.back();
        TRPC_ASSERT(executor != nullptr);

        shard.mysql_executors.pop_back();

        if (executor->IsConnectionValid()) {
          if (!IsIdleTimeout(executor))
            return executor;
          else
            idle_executor = executor;
        }
        else {
          idle_executor = executor;
        }
      } else {
        break;
      }
    }

    if (idle_executor != nullptr) {
      idle_executor->Close();
    }

    --retry_num;
  }

  executor = CreateExecutor(shard_id);

  if(executor->Connect()) {
    executor_num_.fetch_add(1, std::memory_order_relaxed);
    return executor;
  }

  return nullptr;
}

RefPtr<MysqlExecutor> MysqlExecutorPool::CreateExecutor(uint32_t shard_id) {
  uint64_t executor_id = static_cast<uint64_t>(shard_id) << 32;
  executor_id |= executor_id_gen_.fetch_add(1, std::memory_order_relaxed);

  MysqlConnOption conn_option;
  conn_option.hostname = target_.ip;
  conn_option.port = target_.port;
  conn_option.username = pool_option_.username;
  conn_option.database = pool_option_.dbname;
  conn_option.password = pool_option_.password;
  conn_option.char_set = pool_option_.char_set;

  auto executor = MakeRefCounted<MysqlExecutor>(conn_option);
  executor->SetExecutorId(executor_id);
  return executor;
}

void MysqlExecutorPool::Reclaim(int ret, RefPtr<MysqlExecutor>&& executor) {

  if(ret == 0) {
    uint32_t shard_id = (executor->GetExecutorId() >> 32);
    auto& shard = executor_shards_[shard_id % pool_option_.num_shard_group];

    std::scoped_lock _(shard.lock);
    if ((shard.mysql_executors.size() <= max_num_per_shard_) &&
        (executor_num_.load(std::memory_order_relaxed) <= pool_option_.max_size)) {
      executor->RefreshAliveTime();
      shard.mysql_executors.push_back(std::move(executor));
      return;
    }
  }

  executor_num_.fetch_add(1, std::memory_order_relaxed);
  executor->Close();
}

void MysqlExecutorPool::Stop() {
  for (uint32_t i = 0; i != pool_option_.num_shard_group; ++i) {
    auto&& shard = executor_shards_[i];

    std::list<RefPtr<MysqlExecutor>> mysql_executors;
    {
      std::scoped_lock _(shard.lock);
      mysql_executors = shard.mysql_executors;
    }

    for (const auto& executor : mysql_executors) {
      TRPC_ASSERT(executor != nullptr);
      executor->Close();
    }
  }
}

void MysqlExecutorPool::Destroy() {
  for (uint32_t i = 0; i != pool_option_.num_shard_group; ++i) {
    auto&& shard = executor_shards_[i];

    std::list<RefPtr<MysqlExecutor>> mysql_executors;
    {
      std::scoped_lock _(shard.lock);
      mysql_executors.swap(shard.mysql_executors);
    }

    // Just destruct the MysqlExecutor by Refcount.
  }
}

RefPtr<MysqlExecutor> MysqlExecutorPool::GetExecutor() {
  return GetOrCreate();
}

bool MysqlExecutorPool::IsIdleTimeout(RefPtr<MysqlExecutor> executor) {
  if (executor != nullptr) {
    if (pool_option_.max_idle_time == 0 ||
        executor->GetAliveTime() < pool_option_.max_idle_time) {
      return false;
    }
    return true;
  }
  return false;
}


}  // namespace mysql
}  // namespace trpc