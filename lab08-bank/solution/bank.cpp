#include "bank.hpp"
#include <condition_variable>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace bank {

transaction::transaction(const user *cp, int delta, std::string cmt)
    : counterparty(cp), balance_delta_xts(delta), comment(std::move(cmt)) {
}

user::user(const std::string &name) : name_(name), balance_xts_(start_balance) {
    const std::unique_lock lock(user_mtx_);
    add_transaction(nullptr, start_balance, "Initial deposit for " + name);
}

std::string user::name() const noexcept {
    return name_;
}

int user::balance_xts() const {
    const std::unique_lock lock(user_mtx_);
    return balance_xts_;
}

void user::transfer(
    user &counterparty,
    int amount_xts,
    const std::string &comment
) {
    if (amount_xts <= 0) {
        throw transfer_error("Transfer amount must be positive");
    }

    if (&counterparty == this) {
        throw transfer_error("Cannot transfer to yourself");
    }

    const std::scoped_lock lock(user_mtx_, counterparty.user_mtx_);

    if (amount_xts > balance_xts_) {
        throw not_enough_funds_error(balance_xts_, amount_xts);
    }
    counterparty.balance_xts_ += amount_xts;
    balance_xts_ -= amount_xts;
    add_transaction(&counterparty, -amount_xts, comment);
    counterparty.add_transaction(this, amount_xts, comment);
}

[[nodiscard]] user_transactions_iterator user::monitor() const {
    const std::unique_lock lock(user_mtx_);
    return user_transactions_iterator{this, transactions_.size()};
}

void user::add_transaction(
    const bank::user *const counterparty,
    const int balance_delta_xts,
    const std::string &comment
) {
    transactions_.emplace_back(counterparty, balance_delta_xts, comment);
    new_transaction_cv_.notify_all();
}

user &ledger::get_or_create_user(const std::string &name) {
    const std::unique_lock lock(ledger_mtx_);
    if (!users_.contains(name)) {
        users_[name] = std::make_unique<user>(name);
    }
    return *users_[name];
}

user_transactions_iterator::user_transactions_iterator(
    const user *const user_,
    const std::size_t transaction_number
) noexcept
    : user_(user_), transaction_number(transaction_number) {
}

transaction user_transactions_iterator::wait_next_transaction() {
    std::unique_lock lock(user_->user_mtx_);
    user_->new_transaction_cv_.wait(lock, [this] {
        return transaction_number < user_->transactions_.size();
    });

    transaction last_transaction = user_->transactions_[transaction_number++];
    return last_transaction;
}

}  // namespace bank