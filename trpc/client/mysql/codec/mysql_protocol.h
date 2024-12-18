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

#include "trpc/codec/protocol.h"
#include "trpc/util/buffer/noncontiguous_buffer.h"

namespace trpc::mysql {

/// @brief MySQL request protocol message is mainly used to package MySQL request to make the code consistent
/// @details This is a dummy protocol as we directly utilizes the MySQL API, bypassing the transport layer.
/// @private For internal use purpose only.
class MySQLRequestProtocol : public trpc::Protocol {
 public:
  /// @private For internal use purpose only.
  MySQLRequestProtocol() = default;

  /// @private For internal use purpose only.
  ~MySQLRequestProtocol() override = default;

  /// @brief Decodes or encodes MySQL request protocol message (zero copy).
  /// @private For internal use purpose only.
  bool ZeroCopyDecode(NoncontiguousBuffer& buff) override;
  /// @brief Encodes MySQL request protocol message (zero copy).
  /// @private For internal use purpose only.
  bool ZeroCopyEncode(NoncontiguousBuffer& buff) override;

 public:
  // Placeholder for MySQL request specific data
  std::string mysql_req_data;
};

/// @brief MySQL response protocol message is mainly used to package MySQL response to make the code consistent
/// @private For internal use purpose only.
class MySQLResponseProtocol : public trpc::Protocol {
 public:
  /// @private For internal use purpose only.
  MySQLResponseProtocol() = default;
  /// @private For internal use purpose only.
  ~MySQLResponseProtocol() override = default;

  /// @brief Decodes or encodes MySQL response protocol message (zero copy).
  /// @private For internal use purpose only.
  bool ZeroCopyDecode(NoncontiguousBuffer& buff) override;
  /// @brief Encodes MySQL response protocol message (zero copy).
  /// @private For internal use purpose only.
  bool ZeroCopyEncode(NoncontiguousBuffer& buff) override;

 public:
  // Placeholder for MySQL response specific data
  std::string mysql_rsp_data;
};

using MySQLRequestProtocolPtr = std::shared_ptr<MySQLRequestProtocol>;
using MySQLResponseProtocolPtr = std::shared_ptr<MySQLResponseProtocol>;

}  // namespace trpc::mysql
