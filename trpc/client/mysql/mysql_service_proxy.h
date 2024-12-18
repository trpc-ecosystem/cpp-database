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

#include "trpc/client/service_proxy.h"
#include "trpc/client/service_proxy_manager.h"
#include "trpc/coroutine/fiber_event.h"
#include "trpc/coroutine/fiber_latch.h"
#include "trpc/util/ref_ptr.h"
#include "trpc/util/thread/latch.h"
#include "trpc/util/thread/thread_pool.h"

#include "trpc/client/mysql/config/mysql_client_conf.h"
#include "trpc/client/mysql/mysql_executor_pool_manager.h"
#include "trpc/client/mysql/transaction.h"

namespace trpc::mysql {

class MysqlServiceProxy : public ServiceProxy {
  using ExecutorPtr = RefPtr<MysqlExecutor>;
  friend class trpc::ServiceProxyManager;

 public:
  MysqlServiceProxy() = default;

  ~MysqlServiceProxy() override = default;

  /// @brief Executes a SQL query and retrieves all resulting rows.
  ///
  ///  This function executes the provided SQL query with the specified input arguments.
  ///
  /// @param context
  /// @param res Which return the query results. Details in "class MysqlResults"
  /// @param sql_str The SQL query to be executed as a string which uses "?" as
  ///  placeholders (see [MySQL C API Documentation](https://dev.mysql.com/doc/c-api/8.0/en/mysql-stmt-prepare.html)).
  /// @param args The input arguments to be bound to the query placeholders. Could be empty if no placeholders in
  /// sql_str.
  /// @return  A trpc Status. Note that the Status and the error information in MysqlResults are independent of each
  /// other.
  ///  If an error occurs during the MySQL query, the error will be stored in MysqlResults, but the Status will still be
  ///  OK. If no error occurs in the MySQL query but another error occurs in the call, MysqlResults will contain no
  ///  error, but the Status will reflect the error.
  template <typename... OutputArgs, typename... InputArgs>
  Status Query(const ClientContextPtr& context, MysqlResults<OutputArgs...>& res, const std::string& sql_str,
               const InputArgs&... args);

  /// @brief Async method for executing a SQL query and retrieves all resulting rows.
  ///
  ///  This function executes the provided SQL query with the specified input arguments.
  ///
  /// @param context
  /// @param res Which return the query results. Details in "class MysqlResults"
  /// @param sql_str The SQL query to be executed as a string which uses "?" as
  ///  placeholders (see [MySQL C API Documentation](https://dev.mysql.com/doc/c-api/8.0/en/mysql-stmt-prepare.html)).
  /// @param args The input arguments to be bound to the query placeholders. Could be empty if no placeholders in
  /// sql_str.
  /// @return  Future containing the MysqlResults. If an error occurs during the MySQL query, the error will be stored
  ///  in MysqlResults, and the exception future will also contain the same error. If no error occurs during the MySQL
  ///  query (e.g., timeout), there will be no error in MysqlResults, and you can retrieve the error from the exception
  ///  future.
  template <typename... OutputArgs, typename... InputArgs>
  Future<MysqlResults<OutputArgs...>> AsyncQuery(const ClientContextPtr& context, const std::string& sql_str,
                                                 const InputArgs&... args);

  /// @brief "Execute" methods are completely the same with "Query" now.
  template <typename... OutputArgs, typename... InputArgs>
  Status Execute(const ClientContextPtr& context, MysqlResults<OutputArgs...>& res, const std::string& sql_str,
                 const InputArgs&... args);

  /// @brief "Execute" methods are completely the same with "Query" now.
  template <typename... OutputArgs, typename... InputArgs>
  Future<MysqlResults<OutputArgs...>> AsyncExecute(const ClientContextPtr& context, const std::string& sql_str,
                                                   const InputArgs&... args);

  /// @brief Transaction support for query. A TransactionHandle which has been called "Begin" is needed.
  template <typename... OutputArgs, typename... InputArgs>
  Status Query(const ClientContextPtr& context, const TxHandlePtr& handle, MysqlResults<OutputArgs...>& res,
               const std::string& sql_str, const InputArgs&... args);

  /// @brief Transaction support for query. A TransactionHandle which has been called "Begin" is needed.
  template <typename... OutputArgs, typename... InputArgs>
  Status Execute(const ClientContextPtr& context, const TxHandlePtr& handle, MysqlResults<OutputArgs...>& res,
                 const std::string& sql_str, const InputArgs&... args);

  template <typename... OutputArgs, typename... InputArgs>
  Future<MysqlResults<OutputArgs...>> AsyncQuery(const ClientContextPtr& context, const TxHandlePtr& handle,
                                                 const std::string& sql_str, const InputArgs&... args);

  template <typename... OutputArgs, typename... InputArgs>
  Future<MysqlResults<OutputArgs...>> AsyncExecute(const ClientContextPtr& context, const TxHandlePtr& handle,
                                                   const std::string& sql_str, const InputArgs&... args);

  /// @brief Begin a transaction. An empty handle is needed.
  Status Begin(const ClientContextPtr& context, TxHandlePtr& handle);

  /// @brief Commit a transaction.
  Status Commit(const ClientContextPtr& context, const TxHandlePtr& handle);

  /// @brief Rollback a transaction.
  Status Rollback(const ClientContextPtr& context, const TxHandlePtr& handle);

  /// @brief Begin a transaction.
  /// @return The refcounted handle pointer of this transaction.
  Future<TxHandlePtr> AsyncBegin(const ClientContextPtr& context);

  /// @brief Commit a transaction.
  Future<> AsyncCommit(const ClientContextPtr& context, const TxHandlePtr& handle);

  /// @brief Rollback a transaction.
  Future<> AsyncRollback(const ClientContextPtr& context, const TxHandlePtr& handle);

  void Stop() override;

  void Destroy() override;

  /// @brief Set the MySQL configuration. Because MysqlClientConf is independent of ServiceProxyOption,
  /// it cannot be set via `GetProxy(const std::string& name, const ServiceProxyOption& option)`.
  /// Therefore, if you want to set the configuration using parameters at runtime instead of YAML,
  /// @details This will destroy the thread pool and executor pool manager.
  /// This is because when using YAML for configuration, calling
  /// `GetProxy(const std::string& name)` completes the initialization directly without requiring an additional
  /// initialization function.
  void SetMysqlConfig(const MysqlClientConf& mysql_conf);

 protected:
  /// @brief Init pool manager and thread pool.
  void SetServiceProxyOptionInner(const std::shared_ptr<ServiceProxyOption>& option) override;

 private:
  /// @brief set mysql_conf from yaml file.
  void SetConfigFromFile();

  /// @brief pool_manager_ only can be inited after the service option has been set.
  bool InitManager();

  /// @brief thread_pool_ only can be inited after the service option has been set.
  bool InitThreadPool();

  /// @param context
  /// @param executor If executor is nullptr, it will get a executor from executor manager.
  /// @param res
  /// @param sql_str
  /// @param args
  /// @return
  template <typename... OutputArgs, typename... InputArgs>
  Status UnaryInvoke(const ClientContextPtr& context, const ExecutorPtr& executor, MysqlResults<OutputArgs...>& res,
                     const std::string& sql_str, const InputArgs&... args);

  template <typename... OutputArgs, typename... InputArgs>
  Future<MysqlResults<OutputArgs...>> AsyncUnaryInvoke(const ClientContextPtr& context, const ExecutorPtr& executor,
                                                       const std::string& sql_str, const InputArgs&... args);

  /// @brief Set the handle state and reclaim its executor.
  /// @param rollback set the state to rollback otherwise commited.
  /// @return true if success.
  bool EndTransaction(const TxHandlePtr& handle, bool rollback);

 private:
  std::unique_ptr<ThreadPool> thread_pool_{nullptr};

  std::unique_ptr<MysqlExecutorPoolManager> pool_manager_{nullptr};

  MysqlClientConf mysql_conf_;
};

template <typename... OutputArgs, typename... InputArgs>
Status MysqlServiceProxy::Query(const ClientContextPtr& context, MysqlResults<OutputArgs...>& res,
                                const std::string& sql_str, const InputArgs&... args) {
  FillClientContext(context);

  auto filter_status = filter_controller_.RunMessageClientFilters(FilterPoint::CLIENT_PRE_RPC_INVOKE, context);
  if (filter_status == FilterStatus::REJECT)
    TRPC_FMT_ERROR("service name:{}, filter execute failed.", GetServiceName());
  else
    UnaryInvoke(context, nullptr, res, sql_str, args...);

  RunFilters(FilterPoint::CLIENT_POST_RPC_INVOKE, context);
  return context->GetStatus();
}

template <typename... OutputArgs, typename... InputArgs>
Future<MysqlResults<OutputArgs...>> MysqlServiceProxy::AsyncQuery(const ClientContextPtr& context,
                                                                  const std::string& sql_str,
                                                                  const InputArgs&... args) {
  FillClientContext(context);

  auto filter_status = filter_controller_.RunMessageClientFilters(FilterPoint::CLIENT_PRE_RPC_INVOKE, context);
  if (filter_status == FilterStatus::REJECT) {
    TRPC_FMT_ERROR("service name:{}, filter execute failed.", GetServiceName());
    context->SetRequestData(nullptr);
    const Status& result = context->GetStatus();
    auto exception_fut =
        MakeExceptionFuture<MysqlResults<OutputArgs...>>(CommonException(result.ErrorMessage().c_str()));
    filter_controller_.RunMessageClientFilters(FilterPoint::CLIENT_POST_RPC_INVOKE, context);
    return exception_fut;
  }

  return AsyncUnaryInvoke<OutputArgs...>(context, nullptr, sql_str, args...)
      .Then([this, context](Future<MysqlResults<OutputArgs...>>&& f) {
        if (f.IsFailed()) {
          RunFilters(FilterPoint::CLIENT_POST_RPC_INVOKE, context);
          return MakeExceptionFuture<MysqlResults<OutputArgs...>>(f.GetException());
        }
        RunFilters(FilterPoint::CLIENT_POST_RPC_INVOKE, context);
        return MakeReadyFuture<MysqlResults<OutputArgs...>>(f.GetValue0());
      });
}

template <typename... OutputArgs, typename... InputArgs>
Status MysqlServiceProxy::Execute(const ClientContextPtr& context, MysqlResults<OutputArgs...>& res,
                                  const std::string& sql_str, const InputArgs&... args) {
  return Query(context, res, sql_str, args...);
}

template <typename... OutputArgs, typename... InputArgs>
Future<MysqlResults<OutputArgs...>> MysqlServiceProxy::AsyncExecute(const ClientContextPtr& context,
                                                                    const std::string& sql_str,
                                                                    const InputArgs&... args) {
  return AsyncQuery<OutputArgs...>(context, sql_str, args...);
}

template <typename... OutputArgs, typename... InputArgs>
Status MysqlServiceProxy::Query(const ClientContextPtr& context, const TxHandlePtr& handle,
                                MysqlResults<OutputArgs...>& res, const std::string& sql_str,
                                const InputArgs&... args) {
  Status status;
  FillClientContext(context);

  auto filter_status = filter_controller_.RunMessageClientFilters(FilterPoint::CLIENT_PRE_RPC_INVOKE, context);

  if (filter_status == FilterStatus::REJECT) {
    TRPC_FMT_ERROR("service name:{}, filter execute failed.", GetServiceName());
  } else if (handle->GetState() != TransactionHandle::TxState::kStarted) {
    TRPC_FMT_ERROR("service name:{}, query in an invalid transaction.", GetServiceName());
    status.SetFrameworkRetCode(TrpcMysqlRetCode::TRPC_MYSQL_INVALID_HANDLE);
    status.SetErrorMessage(util::FormatString("Invalid transaction state code: {}.", int(handle->GetState())));
    context->SetStatus(std::move(status));

  } else if (!handle->GetExecutor()->CheckAlive()) {
    // If the Connection lost the transaction will be rollback automatically. (some exception?)
    TRPC_FMT_ERROR("service name:{}, transaction connection lost.", GetServiceName());
    handle->SetState(TransactionHandle::TxState::kRollBacked);
    status.SetFrameworkRetCode(TrpcRetCode::TRPC_CLIENT_CONNECT_ERR);
    status.SetErrorMessage("Connect error. Rollback.");
    context->SetStatus(status);

  } else {
    UnaryInvoke(context, handle->GetExecutor(), res, sql_str, args...);
  }

  RunFilters(FilterPoint::CLIENT_POST_RPC_INVOKE, context);
  return context->GetStatus();
}

template <typename... OutputArgs, typename... InputArgs>
Status MysqlServiceProxy::Execute(const ClientContextPtr& context, const TxHandlePtr& handle,
                                  MysqlResults<OutputArgs...>& res, const std::string& sql_str,
                                  const InputArgs&... args) {
  return Query(context, handle, res, sql_str, args...);
}

template <typename... OutputArgs, typename... InputArgs>
Future<MysqlResults<OutputArgs...>> MysqlServiceProxy::AsyncQuery(const ClientContextPtr& context,
                                                                  const TxHandlePtr& handle, const std::string& sql_str,
                                                                  const InputArgs&... args) {
  FillClientContext(context);
  auto filter_status = filter_controller_.RunMessageClientFilters(FilterPoint::CLIENT_PRE_RPC_INVOKE, context);
  if (filter_status == FilterStatus::REJECT) {
    TRPC_FMT_ERROR("service name:{}, filter execute failed.", GetServiceName());
    context->SetRequestData(nullptr);
    const Status& result = context->GetStatus();
    auto exception_fut =
        MakeExceptionFuture<MysqlResults<OutputArgs...>>(CommonException(result.ErrorMessage().c_str()));
    filter_controller_.RunMessageClientFilters(FilterPoint::CLIENT_POST_RPC_INVOKE, context);
    return exception_fut;
  }

  if (handle->GetState() != TransactionHandle::TxState::kStarted) {
    TRPC_FMT_ERROR("service name:{}, invalid handle state.");
    context->SetStatus(Status(TrpcMysqlRetCode::TRPC_MYSQL_INVALID_HANDLE, "Invalid handle."));
    filter_controller_.RunMessageClientFilters(FilterPoint::CLIENT_POST_RPC_INVOKE, context);
    return MakeExceptionFuture<MysqlResults<OutputArgs...>>(CommonException("Invalid handle."));
  }

  if (!handle->GetExecutor()->CheckAlive()) {
    handle->SetState(TransactionHandle::TxState::kRollBacked);
    Status status;
    status.SetFrameworkRetCode(TrpcRetCode::TRPC_CLIENT_CONNECT_ERR);
    status.SetErrorMessage("Connect error. Rollback.");
    context->SetStatus(status);
    auto exception_fut = MakeExceptionFuture<MysqlResults<OutputArgs...>>(CommonException("Connect error. Rollback."));
    filter_controller_.RunMessageClientFilters(FilterPoint::CLIENT_POST_RPC_INVOKE, context);
    return exception_fut;
  }

  auto executor = handle->GetExecutor();

  return AsyncUnaryInvoke<OutputArgs...>(context, executor, sql_str, args...)
      .Then([this, context](Future<MysqlResults<OutputArgs...>>&& f) mutable {
        if (f.IsFailed()) {
          RunFilters(FilterPoint::CLIENT_POST_RPC_INVOKE, context);
          return MakeExceptionFuture<MysqlResults<OutputArgs...>>(f.GetException());
        }

        RunFilters(FilterPoint::CLIENT_POST_RPC_INVOKE, context);
        return MakeReadyFuture<MysqlResults<OutputArgs...>>(f.GetValue0());
      });
}

template <typename... OutputArgs, typename... InputArgs>
Future<MysqlResults<OutputArgs...>> MysqlServiceProxy::AsyncExecute(const ClientContextPtr& context,
                                                                    const TxHandlePtr& handle,
                                                                    const std::string& sql_str,
                                                                    const InputArgs&... args) {
  return AsyncQuery<OutputArgs...>(context, handle, sql_str, args...);
}

template <typename... OutputArgs, typename... InputArgs>
Status MysqlServiceProxy::UnaryInvoke(const ClientContextPtr& context, const ExecutorPtr& executor,
                                      MysqlResults<OutputArgs...>& res, const std::string& sql_str,
                                      const InputArgs&... args) {
  if (CheckTimeout(context)) return context->GetStatus();

  int filter_ret = RunFilters(FilterPoint::CLIENT_PRE_SEND_MSG, context);

  if (filter_ret != 0) {
    ProxyStatistics(context);
    RunFilters(FilterPoint::CLIENT_POST_RECV_MSG, context);
    return context->GetStatus();
  }

  FiberEvent e;
  thread_pool_->AddTask([this, &context, &executor, &e, &res, &sql_str, &args...]() {
    ExecutorPtr conn{nullptr};
    MysqlExecutorPool* pool{nullptr};

    if (executor == nullptr) {
      NodeAddr node_addr;
      node_addr.ip = context->GetIp();
      node_addr.port = context->GetPort();
      pool = this->pool_manager_->Get(node_addr);
      conn = pool->GetExecutor();
    } else
      conn = executor;

    if (!conn->IsConnected()) {
      std::string error_message =
          util::FormatString("service name:{}, connection failed. {}.", GetServiceName(), conn->GetErrorMessage());
      TRPC_LOG_ERROR(error_message);
      Status status;
      status.SetFrameworkRetCode(conn->GetErrorNumber());
      status.SetErrorMessage(error_message);
      context->SetStatus(std::move(status));
    } else {
      if constexpr (MysqlResults<OutputArgs...>::mode == MysqlResultsMode::OnlyExec)
        conn->Execute(res, sql_str, args...);
      else
        conn->QueryAll(res, sql_str, args...);

      if (pool != nullptr) pool->Reclaim(0, std::move(conn));
    }
    e.Set();
  });

  e.Wait();

  if (!res.OK()) {
    Status s;
    s.SetErrorMessage(res.GetErrorMessage());
    s.SetFrameworkRetCode(res.GetErrorNumber());
    context->SetStatus(std::move(s));
  }

  ProxyStatistics(context);
  RunFilters(FilterPoint::CLIENT_POST_RECV_MSG, context);

  return context->GetStatus();
}

template <typename... OutputArgs, typename... InputArgs>
Future<MysqlResults<OutputArgs...>> MysqlServiceProxy::AsyncUnaryInvoke(const ClientContextPtr& context,
                                                                        const ExecutorPtr& executor,
                                                                        const std::string& sql_str,
                                                                        const InputArgs&... args) {
  Promise<MysqlResults<OutputArgs...>> pr;
  auto fu = pr.GetFuture();

  if (CheckTimeout(context)) {
    const Status& result = context->GetStatus();
    return MakeExceptionFuture<MysqlResults<OutputArgs...>>(
        CommonException(result.ErrorMessage().c_str(), result.GetFrameworkRetCode()));
  }

  int filter_ret = RunFilters(FilterPoint::CLIENT_PRE_SEND_MSG, context);

  if (filter_ret != 0) {
    RunFilters(FilterPoint::CLIENT_POST_RECV_MSG, context);
    RunFilters(FilterPoint::CLIENT_POST_RPC_INVOKE, context);
    return MakeExceptionFuture<MysqlResults<OutputArgs...>>(
        CommonException(context->GetStatus().ErrorMessage().c_str()));
  }

  thread_pool_->AddTask([p = std::move(pr), this, executor, context, sql_str, args...]() mutable {
    MysqlResults<OutputArgs...> res;
    NodeAddr node_addr;

    MysqlExecutorPool* pool{nullptr};
    ExecutorPtr conn{nullptr};

    if (executor == nullptr) {
      node_addr = context->GetNodeAddr();
      pool = this->pool_manager_->Get(node_addr);
      conn = pool->GetExecutor();
    } else
      conn = executor;

    if (TRPC_UNLIKELY(!conn->IsConnected())) {
      std::string error_message =
          util::FormatString("service name:{}, connection failed. {}.", GetServiceName(), conn->GetErrorMessage());
      TRPC_LOG_ERROR(error_message);
      Status status;
      status.SetFrameworkRetCode(conn->GetErrorNumber());
      status.SetErrorMessage(error_message);

      context->SetStatus(status);
      p.SetException(CommonException(status.ErrorMessage().c_str()));
      return;
    }

    if constexpr (MysqlResults<OutputArgs...>::mode == MysqlResultsMode::OnlyExec)
      conn->Execute(res, sql_str, args...);
    else
      conn->QueryAll(res, sql_str, args...);

    if (pool != nullptr) pool->Reclaim(0, std::move(conn));

    ProxyStatistics(context);

    if (res.OK())
      p.SetValue(std::move(res));
    else
      p.SetException(CommonException(res.GetErrorMessage().c_str()));
  });

  return fu.Then([context, this](Future<MysqlResults<OutputArgs...>>&& fu) {
    if (fu.IsFailed()) {
      RunFilters(FilterPoint::CLIENT_POST_RECV_MSG, context);
      return MakeExceptionFuture<MysqlResults<OutputArgs...>>(fu.GetException());
    }
    auto mysql_res = fu.GetValue0();
    RunFilters(FilterPoint::CLIENT_POST_RECV_MSG, context);
    return MakeReadyFuture<MysqlResults<OutputArgs...>>(std::move(mysql_res));
  });
}

}  // namespace trpc::mysql
