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

#include "trpc/client/mysql/mysql_service_proxy.h"

#include "trpc/client/service_proxy_option.h"
#include "trpc/util/bind_core_manager.h"
#include "trpc/util/string/string_util.h"

#include "trpc/client/mysql/config/mysql_client_conf_parser.h"

namespace trpc::mysql {

bool MysqlServiceProxy::InitManager() {
  if (pool_manager_ != nullptr) return false;

  const ServiceProxyOption* option = GetServiceProxyOption();
  MysqlExecutorPoolOption pool_option;
  pool_option.max_size = option->max_conn_num;
  pool_option.max_idle_time = option->idle_time;
  pool_option.num_shard_group = mysql_conf_.num_shard_group;
  pool_option.username = mysql_conf_.user_name;
  pool_option.dbname = mysql_conf_.dbname;
  pool_option.password = mysql_conf_.password;
  pool_option.char_set = mysql_conf_.char_set;
  pool_manager_ = std::make_unique<MysqlExecutorPoolManager>(pool_option);
  return true;
}

bool MysqlServiceProxy::InitThreadPool() {
  if (thread_pool_ != nullptr) return false;

  ::trpc::ThreadPoolOption thread_pool_option;
  thread_pool_option.thread_num = mysql_conf_.thread_num;
  thread_pool_option.bind_core = !mysql_conf_.thread_bind_core.empty();
  thread_pool_ = std::make_unique<::trpc::ThreadPool>(std::move(thread_pool_option));
  BindCoreManager::ParseBindCoreGroup(mysql_conf_.thread_bind_core);
  thread_pool_->Start();
  BindCoreManager::ParseBindCoreGroup("");  // Reset config.
  return true;
}

void MysqlServiceProxy::SetServiceProxyOptionInner(const std::shared_ptr<ServiceProxyOption>& option) {
  ServiceProxy::SetServiceProxyOptionInner(option);
  SetConfigFromFile();
  mysql_conf_.Display();
  InitThreadPool();
  InitManager();
}

void MysqlServiceProxy::Destroy() {
  ServiceProxy::Destroy();
  thread_pool_->Join();
  pool_manager_->Destroy();
}

void MysqlServiceProxy::Stop() {
  ServiceProxy::Stop();
  thread_pool_->Stop();
  pool_manager_->Stop();
}

Status MysqlServiceProxy::Begin(const ClientContextPtr& context, TxHandlePtr& handle) {
  FillClientContext(context);
  auto filter_status = filter_controller_.RunMessageClientFilters(FilterPoint::CLIENT_PRE_RPC_INVOKE, context);
  if (filter_status == FilterStatus::REJECT) {
    TRPC_FMT_ERROR("service name:{}, filter execute failed.", GetServiceName());
    RunFilters(FilterPoint::CLIENT_POST_RPC_INVOKE, context);
    return context->GetStatus();
  }

  MysqlResults<OnlyExec> res;
  NodeAddr node_addr;

  // Bypass the selector to use or test the service_proxy independently
  // (since the selector might not be registered)
  if (context->GetIp().empty()) {
    ClientContextPtr temp_ctx = MakeRefCounted<ClientContext>(this->GetClientCodec());
    FillClientContext(temp_ctx);

    if (!SelectTarget(temp_ctx)) {
      TRPC_LOG_ERROR("select target failed: " << temp_ctx->GetStatus().ToString());
      context->SetStatus(temp_ctx->GetStatus());
      return context->GetStatus();
    }
    node_addr = temp_ctx->GetNodeAddr();
  } else {
    node_addr = context->GetNodeAddr();
  }

  MysqlExecutorPool* pool = this->pool_manager_->Get(node_addr);
  auto executor = pool->GetExecutor();

  Status status;
  if (!executor->IsConnected()) {
    std::string error_message =
        util::FormatString("service name:{}, connection failed. {}.", GetServiceName(), executor->GetErrorMessage());
    TRPC_LOG_ERROR(error_message);
    status.SetFrameworkRetCode(executor->GetErrorNumber());
    status.SetErrorMessage(error_message);
    context->SetStatus(std::move(status));
  } else {
    status = UnaryInvoke(context, executor, res, "begin");
  }

  if (context->GetStatus().OK()) {
    handle = MakeRefCounted<TransactionHandle>();
    handle->SetExecutor(std::move(executor));
    handle->SetState(TransactionHandle::TxState::kStarted);
  }

  RunFilters(FilterPoint::CLIENT_POST_RPC_INVOKE, context);
  return context->GetStatus();
}

Future<TxHandlePtr> MysqlServiceProxy::AsyncBegin(const ClientContextPtr& context) {
  FillClientContext(context);

  auto filter_status = filter_controller_.RunMessageClientFilters(FilterPoint::CLIENT_PRE_RPC_INVOKE, context);
  if (filter_status == FilterStatus::REJECT) {
    TRPC_FMT_ERROR("service name:{}, filter execute failed.", GetServiceName());
    context->SetRequestData(nullptr);
    const Status& result = context->GetStatus();
    auto exception_fut = MakeExceptionFuture<TxHandlePtr>(CommonException(result.ErrorMessage().c_str()));
    filter_controller_.RunMessageClientFilters(FilterPoint::CLIENT_POST_RPC_INVOKE, context);
    return exception_fut;
  }

  MysqlResults<OnlyExec> res;
  NodeAddr node_addr;

  // Bypass the selector to use or test the service_proxy independently
  // (since the selector might not be registered)
  if (context->GetIp().empty()) {
    ClientContextPtr temp_ctx = MakeRefCounted<ClientContext>(this->GetClientCodec());
    FillClientContext(temp_ctx);

    if (!SelectTarget(temp_ctx)) {
      TRPC_LOG_ERROR("select target failed: " << temp_ctx->GetStatus().ToString());
      context->SetStatus(temp_ctx->GetStatus());
      return MakeExceptionFuture<TxHandlePtr>(trpc::CommonException(temp_ctx->GetStatus().ToString().c_str()));
    }
    node_addr = temp_ctx->GetNodeAddr();

  } else {
    node_addr = context->GetNodeAddr();
  }

  MysqlExecutorPool* pool = this->pool_manager_->Get(node_addr);
  auto executor = pool->GetExecutor();
  if (!executor->IsConnected()) {
    std::string error_message =
        util::FormatString("service name:{}, connection failed. {}.", GetServiceName(), executor->GetErrorMessage());
    TRPC_LOG_ERROR(error_message);
    Status status;
    status.SetFrameworkRetCode(executor->GetErrorNumber());
    status.SetErrorMessage(error_message);
    context->SetStatus(status);
    return MakeExceptionFuture<TxHandlePtr>(CommonException(error_message.c_str()));
  }

  return AsyncUnaryInvoke<OnlyExec>(context, executor, "begin")
      .Then([executor](Future<MysqlResults<OnlyExec>>&& f) mutable {
        TxHandlePtr handle_ptr = MakeRefCounted<TransactionHandle>();
        if (f.IsFailed()) {
          return MakeExceptionFuture<TxHandlePtr>(f.GetException());
        }
        handle_ptr->SetState(TransactionHandle::TxState::kStarted);
        handle_ptr->SetExecutor(std::move(executor));
        return MakeReadyFuture(std::move(handle_ptr));
      });
}

Status MysqlServiceProxy::Commit(const ClientContextPtr& context, const TxHandlePtr& handle) {
  MysqlResults<OnlyExec> res;
  Status s = Execute(context, handle, res, "commit");

  if (!res.OK()) {
    Status status = kUnknownErrorStatus;
    status.SetErrorMessage(res.GetErrorMessage());
    context->SetStatus(status);
  } else {
    EndTransaction(handle, false);
  }

  return context->GetStatus();
}

Status MysqlServiceProxy::Rollback(const ClientContextPtr& context, const TxHandlePtr& handle) {
  MysqlResults<OnlyExec> res;
  Status s = Execute(context, handle, res, "rollback");

  if (!res.OK()) {
    Status status = kUnknownErrorStatus;
    status.SetErrorMessage(res.GetErrorMessage());
    context->SetStatus(status);
  } else {
    EndTransaction(handle, true);
  }

  return context->GetStatus();
}

Future<> MysqlServiceProxy::AsyncCommit(const ClientContextPtr& context, const TxHandlePtr& handle) {
  MysqlResults<OnlyExec> res;
  return AsyncQuery<OnlyExec>(context, handle, "commit")
      .Then([this, context, handle](Future<MysqlResults<OnlyExec>>&& f) {
        if (f.IsFailed()) {
          return MakeExceptionFuture<>(f.GetException());
        }
        EndTransaction(handle, false);
        return MakeReadyFuture<>();
      });
}

Future<> MysqlServiceProxy::AsyncRollback(const ClientContextPtr& context, const TxHandlePtr& handle) {
  MysqlResults<OnlyExec> res;
  return AsyncQuery<OnlyExec>(context, handle, "rollback")
      .Then([this, context, handle](Future<MysqlResults<OnlyExec>>&& f) {
        if (f.IsFailed()) {
          auto t = f.GetValue();
          return MakeExceptionFuture<>(f.GetException());
        }
        auto t = f.GetValue();
        EndTransaction(handle, true);
        return MakeReadyFuture<>();
      });
}

bool MysqlServiceProxy::EndTransaction(const TxHandlePtr& handle, bool rollback) {
  handle->SetState(rollback ? TransactionHandle::TxState::kRollBacked : TransactionHandle::TxState::kCommitted);
  auto executor = handle->GetExecutor();
  if (executor) {
    NodeAddr node_addr;
    node_addr.ip = executor->GetIp();
    node_addr.port = executor->GetPort();
    MysqlExecutorPool* pool = pool_manager_->Get(node_addr);
    pool->Reclaim(0, handle->TransferExecutor());
  }
  return true;
}

void MysqlServiceProxy::SetMysqlConfig(const MysqlClientConf& mysql_conf) {
  mysql_conf_ = mysql_conf;
  mysql_conf_.Display();
  thread_pool_->Stop();
  thread_pool_->Join();
  thread_pool_ = nullptr;
  pool_manager_->Stop();
  pool_manager_->Destroy();
  pool_manager_ = nullptr;

  // Reboot
  InitThreadPool();
  InitManager();
}

void MysqlServiceProxy::SetConfigFromFile() {
  YAML::Node node;
  ConfigHelper::GetInstance()->GetNode({"client", "service"}, node);
  for (auto&& idx : node) {
    if (idx["name"].as<std::string>() == GetServiceProxyOption()->name) {
      mysql_conf_ = idx["mysql"].as<::trpc::mysql::MysqlClientConf>();
    }
  }
}

}  // namespace trpc::mysql