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

#include "trpc/codec/client_codec.h"

#include "trpc/client/mysql/codec/mysql_protocol.h"
#include "trpc/client/client_context.h"

namespace trpc::mysql {

/// @brief MySQL client-side codec for encoding request messages and decoding response messages.
/// @details This is a dummy codec since we directly utilize the MySQL API, bypassing the transport layer.
/// The client context requires a codec instance from the service proxy. Therefore, this codec is implemented
/// to allow instantiation, although its members are not expected to be used in practice.
/// @note For internal use only; not intended for public APIs.
class MysqlClientCodec : public ClientCodec {
 public:
  /// @private For internal use purpose only.
  MysqlClientCodec() = default;

  /// @private For internal use purpose only.
  ~MysqlClientCodec() override = default;

  /// @brief Returns name of MySQL codec.
  /// @private For internal use purpose only.
  std::string Name() const override { return "mysql"; }

  /// @brief Decodes a complete MySQL protocol message from binary byte stream (zero-copy).
  ///
  /// @param conn is the connection object where the protocol integrity check is performed on the current binary bytes.
  /// @param in is a buffer contains input binary bytes read from the socket.
  /// @param out is a list of successfully decoded protocol message.
  /// @return Returns a number greater than or equal to 0 on success, less than 0 on failure.
  /// @private For internal use purpose only.
  int ZeroCopyCheck(const ConnectionPtr& conn, NoncontiguousBuffer& in, std::deque<std::any>& out) override;

  /// @brief Decodes a MySQL response protocol message object from a complete protocol message decoded from binary
  /// bytes (zero copy).
  ///
  /// @param ctx is client context for decoding.
  /// @param in is a complete protocol message decoded from binary bytes.
  ///           It is usually the output parameter of method "Check".
  /// @param out is MySQL response protocol message object.
  /// @return Returns true on success, false otherwise.
  /// @private For internal use purpose only.
  bool ZeroCopyDecode(const ClientContextPtr&, std::any&& in, ProtocolPtr& out) override;

  /// @brief Encodes a MySQL request protocol message object into a protocol message.
  ///
  /// @param ctx is client context for encoding.
  /// @param in  is MySQL request protocol message object.
  /// @param out is a complete protocol message.
  /// @return Returns true on success, false otherwise.
  /// @private For internal use purpose only.
  bool ZeroCopyEncode(const ClientContextPtr& context, const ProtocolPtr& in, NoncontiguousBuffer& out) override;

  /// @brief Fills the protocol object with the request message passed by user. If |body| is an IDL protocol message,
  /// you need to serialize it into binary data first, and then put it in the protocol message object.
  ///
  /// @param ctx is client context.
  /// @param in is MySQL request protocol message object.
  /// @param body is the request message passed by user.
  /// @return Returns true on success, false otherwise.
  /// @private For internal use purpose only.
  bool FillRequest(const ClientContextPtr& context, const ProtocolPtr& in, void* out) override;

  /// @brief Fills the response message with the protocol object. If |body| is an IDL protocol message,
  /// you need to deserialize binary data into a structure of response message first.
  ///
  /// @param ctx is client context.
  /// @param in is MySQL response protocol message object.
  /// @param body is the response message expected by user.
  /// @return Returns true on success, false otherwise.
  /// @private For internal use purpose only.
  bool FillResponse(const ClientContextPtr& context, const ProtocolPtr& in, void* out) override;

  /// @brief Creates a protocol object of MySQL request (unary call).
  /// @private For internal use purpose only.
  ProtocolPtr CreateRequestPtr() override;

  /// @brief Creates a protocol object of MySQL response (unary call).
  /// @private For internal use purpose only.
  ProtocolPtr CreateResponsePtr() override;
};

}  // namespace trpc::mysql
