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

#include "trpc/client/mysql/codec/mysql_protocol.h"

namespace trpc::mysql {

bool MySQLRequestProtocol::ZeroCopyDecode(NoncontiguousBuffer& buff) { return true; }

bool MySQLRequestProtocol::ZeroCopyEncode(NoncontiguousBuffer& buff) { return true; }

bool MySQLResponseProtocol::ZeroCopyDecode(NoncontiguousBuffer& buff) { return true; }

bool MySQLResponseProtocol::ZeroCopyEncode(NoncontiguousBuffer& buff) { return true; }
}  // namespace trpc::mysql