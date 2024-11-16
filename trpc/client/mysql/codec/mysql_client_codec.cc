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

#include "trpc/client/mysql/codec/mysql_client_codec.h"

namespace trpc::mysql {

int MysqlClientCodec::ZeroCopyCheck(const ConnectionPtr& conn, NoncontiguousBuffer& in, std::deque<std::any>& out) {
  return 1;
}

bool MysqlClientCodec::ZeroCopyDecode(const ClientContextPtr&, std::any&& in, ProtocolPtr& out) { return true; }

bool MysqlClientCodec::ZeroCopyEncode(const ClientContextPtr& context, const ProtocolPtr& in,
                                      NoncontiguousBuffer& out) {
  return true;
}

bool MysqlClientCodec::FillRequest(const ClientContextPtr& context, const ProtocolPtr& in, void* out) { return true; }

bool MysqlClientCodec::FillResponse(const ClientContextPtr& context, const ProtocolPtr& in, void* out) { return true; }

ProtocolPtr MysqlClientCodec::CreateRequestPtr() { return std::make_shared<MySQLRequestProtocol>(); }

ProtocolPtr MysqlClientCodec::CreateResponsePtr() { return std::make_shared<MySQLResponseProtocol>(); }

}  // namespace trpc
