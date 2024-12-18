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

namespace trpc::mysql {

/// The error numbers are the same with MySQL
/// https://dev.mysql.com/doc/mysql-errors/8.0/en/error-reference-introduction.html except below

enum TrpcMysqlRetCode : int { TRPC_MYSQL_INVALID_HANDLE = 3502, TRPC_MYSQL_STMT_PARAMS_ERROR = 3503 };
}  // namespace trpc::mysql
