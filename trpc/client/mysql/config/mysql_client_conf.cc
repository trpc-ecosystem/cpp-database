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

#include "trpc/client/mysql/config/mysql_client_conf.h"

#include "trpc/util/log/logging.h"

namespace trpc::mysql {

void MysqlClientConf::Display() const {
  TRPC_LOG_DEBUG("user_name: " << user_name);
  TRPC_LOG_DEBUG("password: " << password);
  TRPC_LOG_DEBUG("dbname: " << dbname);
  TRPC_LOG_DEBUG("char_set: " << char_set);
  TRPC_LOG_DEBUG("thread_num: " << thread_num);
  TRPC_LOG_DEBUG("thread_bind_core: " << thread_bind_core);
  TRPC_LOG_DEBUG("num_shard_group: " << num_shard_group);
}

}  // namespace trpc::mysql
