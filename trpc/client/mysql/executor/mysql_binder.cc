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

#include "mysql_binder.h"


namespace trpc::mysql {


#define MYSQL_OUTPUT_TYPE_MAP(c_type, ...) \
const std::unordered_set<enum_field_types> MysqlOutputType<c_type>::types = { __VA_ARGS__ };


MYSQL_OUTPUT_TYPE_MAP(int8_t, MYSQL_TYPE_TINY)
MYSQL_OUTPUT_TYPE_MAP(uint8_t, MYSQL_TYPE_TINY)
MYSQL_OUTPUT_TYPE_MAP(int16_t , MYSQL_TYPE_SHORT)
MYSQL_OUTPUT_TYPE_MAP(uint16_t , MYSQL_TYPE_SHORT)
MYSQL_OUTPUT_TYPE_MAP(int32_t, MYSQL_TYPE_LONG, MYSQL_TYPE_INT24)
MYSQL_OUTPUT_TYPE_MAP(uint32_t, MYSQL_TYPE_LONG, MYSQL_TYPE_INT24)
MYSQL_OUTPUT_TYPE_MAP(int64_t, MYSQL_TYPE_LONGLONG)
MYSQL_OUTPUT_TYPE_MAP(uint64_t, MYSQL_TYPE_LONGLONG)
MYSQL_OUTPUT_TYPE_MAP(float, MYSQL_TYPE_FLOAT)
MYSQL_OUTPUT_TYPE_MAP(double, MYSQL_TYPE_DOUBLE)
MYSQL_OUTPUT_TYPE_MAP(MysqlTime, MYSQL_TYPE_TIME, MYSQL_TYPE_DATE, MYSQL_TYPE_DATETIME, MYSQL_TYPE_TIMESTAMP)
MYSQL_OUTPUT_TYPE_MAP(std::string, MYSQL_TYPE_TIME, MYSQL_TYPE_DATE, MYSQL_TYPE_DATETIME, MYSQL_TYPE_TIMESTAMP,\
                                  MYSQL_TYPE_STRING, MYSQL_TYPE_VAR_STRING, MYSQL_TYPE_TINY_BLOB, MYSQL_TYPE_BLOB, MYSQL_TYPE_MEDIUM_BLOB,\
                                  MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_BIT, MYSQL_TYPE_NEWDECIMAL)

MYSQL_OUTPUT_TYPE_MAP(MysqlBlob, MYSQL_TYPE_TINY_BLOB, MYSQL_TYPE_BLOB, MYSQL_TYPE_MEDIUM_BLOB,\
                                  MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_BIT)


}