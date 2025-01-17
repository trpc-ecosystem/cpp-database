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


#include <iostream>
#include <memory>
#include <string>

#include <gflags/gflags.h>
#include "trpc/client/make_client_context.h"
#include "trpc/client/mysql/mysql_plugin.h"
#include "trpc/client/service_proxy.h"
#include "trpc/client/trpc_client.h"
#include "trpc/common/runtime_manager.h"
#include "trpc/log/trpc_log.h"
#include "trpc/future/future_utility.h"

using trpc::mysql::OnlyExec;
using trpc::mysql::NativeString;
using trpc::mysql::MysqlResults;
using trpc::mysql::MysqlTime;
using trpc::mysql::TransactionHandle;
using trpc::mysql::TxHandlePtr;
using trpc::Future;

DEFINE_string(client_config, "fiber_client_client_config.yaml", "trpc cpp framework client_config file");

void PrintResult(const std::vector<std::tuple<int, std::string>>& res_data) {
  for (const auto& tuple : res_data) {
    int id = std::get<0>(tuple);
    std::string username = std::get<1>(tuple);
    std::cout << "ID: " << id << ", Username: " << username << std::endl;
  }
}

void PrintResultTable(const MysqlResults<NativeString>& res) {
  std::vector<std::string> fields_name = res.GetFieldsName();
  bool flag = false;
  std::vector<size_t> column_widths;
  auto& res_set = res.ResultSet();

  for (auto& row : res_set) {
    if (!flag) {
      column_widths.resize(fields_name.size(), 0);
      for (size_t i = 0; i < fields_name.size(); ++i)
        column_widths[i] = std::max(column_widths[i], fields_name[i].length());
      flag = true;
    }

    size_t i = 0;
    for (auto field : row) {
      column_widths[i] = std::max(column_widths[i], field.length());
      ++i;
    }
  }

  for (size_t i = 0; i < fields_name.size(); ++i) {
    std::cout << std::left << std::setw(column_widths[i] + 2) << fields_name[i];
  }
  std::cout << std::endl;

  for (size_t i = 0; i < fields_name.size(); ++i) {
    std::cout << std::setw(column_widths[i] + 2) << std::setfill('-') << "";
  }
  std::cout << std::endl;
  std::cout << std::setfill(' ');

  for(int i = 0; i < res_set.size(); i++) {
    for(int j = 0; j < res_set[i].size(); j++) {
      std::cout << std::left << std::setw(column_widths[j] + 2) << (res.IsValueNull(i, j) ? "null" : res_set[i][j]);
    }
    std::cout << std::endl;
  }
}



void TestAsyncQuery(std::shared_ptr<trpc::mysql::MysqlServiceProxy>& proxy) {
  trpc::ClientContextPtr ctx = trpc::MakeClientContext(proxy);
  MysqlResults<int, std::string> res;

  using ResultType = MysqlResults<int, std::string>;
  auto future = proxy->AsyncQuery<int, std::string>(ctx, "select id, username from users where id = ?", 3)
                        .Then([](trpc::Future<ResultType>&& f){
                          if(f.IsReady()) {
                            auto res = f.GetValue0();
                            PrintResult(res.ResultSet());
                            return trpc::MakeReadyFuture();
                          }
                          return trpc::MakeExceptionFuture<>(f.GetException());
                        });
  std::cout << "do something\n";
  auto future_waited = trpc::future::BlockingGet(std::move(future));

  if(future_waited.IsFailed()) {
    TRPC_FMT_ERROR(future_waited.GetException().what());
    std::cerr << future_waited.GetException().what() << std::endl;
    return;
  }
}


void TestAsyncTx(std::shared_ptr<trpc::mysql::MysqlServiceProxy>& proxy) {
  MysqlResults<NativeString> query_res;
  TxHandlePtr handle = nullptr;
  trpc::ClientContextPtr ctx = trpc::MakeClientContext(proxy);
  proxy->Query(ctx, query_res, "select * from users");

  // Do two query separately in the same one transaction.
  auto fut = proxy->AsyncBegin(ctx)
          .Then([&handle](Future<TxHandlePtr>&& f) mutable {
            if(f.IsFailed())
              return trpc::MakeExceptionFuture<>(f.GetException());

            // Get the ref counted handle ptr here.
            // Or you can return Future<TxHandlePtr> and get the handle outside.
            handle = f.GetValue0();
            return trpc::MakeReadyFuture<>();
          });

  trpc::future::BlockingGet(std::move(fut));

  auto fut2 = proxy
          ->AsyncQuery<NativeString>(ctx, handle, "select username from users where username = ?", "alice")
          .Then([](Future<MysqlResults<NativeString>>&& f) mutable {
            if(f.IsFailed())
              return trpc::MakeExceptionFuture<>(f.GetException());
            auto t = f.GetValue();
            auto res = std::move(std::get<0>(t));

            std::cout << "\n>>> select username from users where username = alice\n";
            PrintResultTable(res);
            return trpc::MakeReadyFuture<>();
          });

  auto fut3 = trpc::future::BlockingGet(std::move(fut2));

  if(fut3.IsFailed()) {
    TRPC_FMT_ERROR(fut3.GetException().what());
    std::cerr << fut3.GetException().what() << std::endl;
    return;
  }

  // Do query in "Then Chain" and rollback
  MysqlTime mtime;
  mtime.SetYear(2024).SetMonth(9).SetDay(10);
  auto fut4 = proxy
          ->AsyncExecute<OnlyExec>(ctx, handle,
                                   "insert into users (username, email, created_at)"
                                   "values (\"jack\", \"jack@abc.com\", ?)", mtime)
          .Then([proxy, ctx, &handle](Future<MysqlResults<OnlyExec>>&& f) {
            if(f.IsFailed())
              return trpc::MakeExceptionFuture<MysqlResults<OnlyExec>>(f.GetException());
            auto t = f.GetValue();
            auto res = std::move(std::get<0>(t));
            std::cout << "\n>>> "
                      << "insert into users (username, email, created_at)\n"
                      << "values (\"jack\", \"jack@abc.com\", \"2024-9-10\")\n\n"
                      << "affected rows: "
                      << res.GetAffectedRowNum()
                      << "\n";

            return proxy
                    ->AsyncQuery<OnlyExec>(ctx, handle,
                                           "update users set email = ? where username = ? ", "jack@gmail.com", "jack");
          })
          .Then([proxy, ctx, &handle](Future<MysqlResults<OnlyExec>>&& f) {
            if(f.IsFailed())
              return trpc::MakeExceptionFuture<MysqlResults<NativeString>>(f.GetException());
            auto t = f.GetValue();
            auto res = std::move(std::get<0>(t));
            std::cout << "\n>>> "
                      << "update users set email = \"jack@gmail.com\" where username = \"jack\"\n\n"
                      << "affected rows: "
                      << res.GetAffectedRowNum()
                      << "\n";

            return proxy->AsyncQuery<NativeString>(ctx, handle, "select * from users");
          })
          .Then([proxy, ctx, &handle](Future<MysqlResults<NativeString>>&& f){
            if(f.IsFailed())
              return trpc::MakeExceptionFuture<MysqlResults<OnlyExec>>(f.GetException());
            auto t = f.GetValue();
            auto res = std::move(std::get<0>(t));

            std::cout << "\n>>> select * from users\n";
            PrintResultTable(res);
            std::cout << "\n\n";

            return proxy
                    ->AsyncQuery<OnlyExec>(ctx, handle,
                                           "update unknown_table set email = ? where username = ? ", "jack@gmail.com", "jack");
          })
          .Then([proxy, ctx, &handle](Future<MysqlResults<OnlyExec>>&& f) {
            // The last sql query should return an error, and we roll back the transaction.
            if(f.IsFailed()) {
              TRPC_LOG_ERROR(f.GetException().what());
              return proxy->AsyncRollback(ctx, handle);
            }
            return trpc::MakeReadyFuture();
          })
          .Then([proxy, ctx](Future<>&& f){
            if(f.IsFailed())
              return trpc::MakeExceptionFuture<>(f.GetException());

            std::cout << "\n>>> rollback\n"
                      << "transaction end\n"
                      << "\n>>> select * from users\n";
            MysqlResults<NativeString> query_res;
            proxy->Query(ctx, query_res, "select * from users");
            PrintResultTable(query_res);

            return trpc::MakeReadyFuture<>();
          });

  trpc::future::BlockingGet(std::move(fut4));

}

int Run() {
  auto proxy = ::trpc::GetTrpcClient()->GetProxy<::trpc::mysql::MysqlServiceProxy>("mysql_server");
  TestAsyncQuery(proxy);
  TestAsyncTx(proxy);
  return 0;
}

void ParseClientConfig(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::CommandLineFlagInfo info;
  if (GetCommandLineFlagInfo("client_config", &info) && info.is_default) {
    std::cerr << "start client with client_config, for example: " << argv[0]
              << " --client_config=/client/client_config/filepath" << std::endl;
    exit(-1);
  }
  std::cout << "FLAGS_client_config:" << FLAGS_client_config << std::endl;

  int ret = ::trpc::TrpcConfig::GetInstance()->Init(FLAGS_client_config);
  if (ret != 0) {
    std::cerr << "load client_config failed." << std::endl;
    exit(-1);
  }
}

int main(int argc, char* argv[]) {
  ParseClientConfig(argc, argv);
  ::trpc::mysql::InitPlugin();

  std::cout << "*************************************\n"
            << "************future_client************\n"
            << "*************************************\n\n";
  // If the business code is running in trpc pure client mode,
  // the business code needs to be running in the `RunInTrpcRuntime` function
  return ::trpc::RunInTrpcRuntime([]() { return Run(); });
}