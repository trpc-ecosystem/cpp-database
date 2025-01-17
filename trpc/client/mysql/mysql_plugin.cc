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

#include "mysql_plugin.h"

#include "trpc/codec/codec_manager.h"

#include "trpc/client/mysql/codec/mysql_client_codec.h"

namespace trpc::mysql {

bool InitPlugin() {
  return ::trpc::codec::InitCodecPlugins<MysqlClientCodec>();
}

}