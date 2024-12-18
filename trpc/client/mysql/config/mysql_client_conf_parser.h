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

#include "trpc/client/mysql/config/mysql_client_conf.h"

namespace YAML {

template <>
struct convert<trpc::mysql::MysqlClientConf> {
  static YAML::Node encode(const trpc::mysql::MysqlClientConf& mysql_conf) {
    YAML::Node node;
    node["user_name"] = mysql_conf.user_name;
    node["password"] = mysql_conf.password;
    node["dbname"] = mysql_conf.dbname;
    node["char_set"] = mysql_conf.char_set;
    node["thread_num"] = mysql_conf.thread_num;
    node["thread_bind_core"] = mysql_conf.thread_bind_core;
    node["num_shard_group"] = mysql_conf.num_shard_group;
    return node;
  }

  static bool decode(const YAML::Node& node, trpc::mysql::MysqlClientConf& mysql_conf) {
    if (node["user_name"]) {
      mysql_conf.user_name = node["user_name"].as<std::string>();
    }
    if (node["password"]) {
      mysql_conf.password = node["password"].as<std::string>();
    }
    if (node["dbname"]) {
      mysql_conf.dbname = node["dbname"].as<std::string>();
    }
    if (node["char_set"]) {
      mysql_conf.char_set = node["char_set"].as<std::string>();
    }
    if (node["thread_num"]) {
      mysql_conf.thread_num = node["thread_num"].as<size_t>();
    }
    if (node["thread_bind_core"]) {
      mysql_conf.thread_bind_core = node["thread_bind_core"].as<std::string>();
    }
    if (node["num_shard_group"]) {
      mysql_conf.num_shard_group = node["num_shard_group"].as<uint32_t>();
    }

    return true;
  }
};

}  // namespace YAML
