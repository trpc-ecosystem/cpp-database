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

#include "trpc/client/mysql/executor/mysql_executor.h"

namespace trpc::mysql {

class TransactionHandle : public RefCounted<TransactionHandle> {
 public:
  /// @note When the handle has been moved, it's state would be set to kInValid
  enum class TxState { kNotInited, kStarted, kRollBacked, kCommitted, kInValid };

  explicit TransactionHandle(RefPtr<MysqlExecutor>&& executor) : executor_(std::move(executor)) {}

  TransactionHandle() : executor_(nullptr) {}

  TransactionHandle& operator=(TransactionHandle&& other) noexcept {
    state_ = other.state_;
    executor_ = std::move(other.executor_);
    other.state_ = TxState::kInValid;
    other.executor_ = nullptr;
    return *this;
  }

  TransactionHandle(TransactionHandle&& other) noexcept {
    state_ = other.state_;
    executor_ = std::move(other.executor_);
    other.state_ = TxState::kInValid;
    other.executor_ = nullptr;
  }

  TransactionHandle& operator=(const TransactionHandle& other) = delete;

  TransactionHandle(const TransactionHandle& other) = delete;

  ~TransactionHandle() {
    // usually executor_ would be nullptr(be reclaimed)
    if (executor_) {
      TRPC_FMT_ERROR("TransactionHandle destructed but executor is not reclaimed.");
      executor_->Close();
    }
    state_ = TxState::kInValid;
  }

  void SetState(TxState state) { state_ = state; }

  TxState GetState() { return state_; }

  bool SetExecutor(RefPtr<MysqlExecutor>&& executor) {
    if (executor_) return false;
    executor_ = std::move(executor);
    return true;
  }

  RefPtr<MysqlExecutor> GetExecutor() { return executor_; }

  RefPtr<MysqlExecutor>&& TransferExecutor() { return std::move(executor_); }

 private:
  RefPtr<MysqlExecutor> executor_{nullptr};
  TxState state_{TxState::kNotInited};
};

using TxHandlePtr = RefPtr<TransactionHandle>;
}  // namespace trpc::mysql
