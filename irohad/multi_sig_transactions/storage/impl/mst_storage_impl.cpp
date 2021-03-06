/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "multi_sig_transactions/storage/mst_storage_impl.hpp"

namespace iroha {
  // ------------------------------| private API |------------------------------

  auto MstStorageStateImpl::getState(
      const shared_model::crypto::PublicKey &target_peer_key) {
    auto target_state_iter = peer_states_.find(target_peer_key);
    if (target_state_iter == peer_states_.end()) {
      return peer_states_.insert({target_peer_key, MstState::empty(completer_)})
          .first;
    }
    return target_state_iter;
  }
  // -----------------------------| interface API |-----------------------------

  MstStorageStateImpl::MstStorageStateImpl(const CompleterType &completer)
      : MstStorage(),
        completer_(completer),
        own_state_(MstState::empty(completer_)) {}

  auto MstStorageStateImpl::applyImpl(
      const shared_model::crypto::PublicKey &target_peer_key,
      const MstState &new_state)
      -> decltype(apply(target_peer_key, new_state)) {
    auto target_state_iter = getState(target_peer_key);
    target_state_iter->second += new_state;
    return own_state_ += new_state;
  }

  auto MstStorageStateImpl::updateOwnStateImpl(const DataType &tx)
      -> decltype(updateOwnState(tx)) {
    return own_state_ += tx;
  }

  auto MstStorageStateImpl::getExpiredTransactionsImpl(
      const TimeType &current_time)
      -> decltype(getExpiredTransactions(current_time)) {
    return own_state_.eraseByTime(current_time);
  }

  auto MstStorageStateImpl::getDiffStateImpl(
      const shared_model::crypto::PublicKey &target_peer_key,
      const TimeType &current_time)
      -> decltype(getDiffState(target_peer_key, current_time)) {
    auto target_current_state_iter = getState(target_peer_key);
    auto new_diff_state = own_state_ - target_current_state_iter->second;
    new_diff_state.eraseByTime(current_time);
    return new_diff_state;
  }

  auto MstStorageStateImpl::whatsNewImpl(ConstRefState new_state) const
      -> decltype(whatsNew(new_state)) {
    return new_state - own_state_;
  }

  bool MstStorageStateImpl::batchInStorageImpl(const DataType &batch) const {
    return own_state_.contains(batch);
  }

}  // namespace iroha
