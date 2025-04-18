#ifndef BANK_HPP
#define BANK_HPP

#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace bank {

class user;
struct transaction;
class ledger;
class user_transactions_iterator;

class transfer_error : public std::runtime_error {
public:
    transfer_error(  // cppcheck-suppress[noExplicitConstructor]
        const std::string &what_arg
    )
        : std::runtime_error(what_arg) {
    }
};

class not_enough_funds_error : public transfer_error {
public:
    not_enough_funds_error(const int available, const int requested)
        : transfer_error(
              "Not enough funds: " + std::to_string(available) +
              " XTS available, " + std::to_string(requested) + " XTS requested"
          ) {
    }
};

struct transaction {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    const user *const counterparty;
    const int balance_delta_xts;
    const std::string comment;
    // NOLINTEND(misc-non-private-member-variables-in-classes)
    transaction(const user *cp, int delta, std::string cmt);
};

class user_transactions_iterator {
    const user *user_;
    std::size_t transaction_number;

public:
    user_transactions_iterator(
        const user *user_,
        std::size_t transaction_number
    ) noexcept;
    transaction wait_next_transaction();
};

class user {
    const int start_balance = 100;
    std::string name_;
    int balance_xts_;
    std::vector<transaction> transactions_;
    mutable std::mutex user_mtx_;
    mutable std::condition_variable new_transaction_cv_;

public:
    friend class user_transactions_iterator;

    explicit user(const std::string &name);

    std::string name() const noexcept;

    void add_transaction(
        const bank::user *counterparty,
        int balance_delta_xts,
        const std::string &comment
    );

    int balance_xts() const;

    void
    transfer(user &counterparty, int amount_xts, const std::string &comment);

    template <typename Functor>
    user_transactions_iterator snapshot_transactions(Functor func) const {
        const std::unique_lock lock(user_mtx_);
        func(transactions_, balance_xts_);
        return user_transactions_iterator{this, transactions_.size()};
    }

    user_transactions_iterator monitor() const;
};

class ledger {
    std::unordered_map<std::string, std::unique_ptr<user>> users_;
    mutable std::mutex ledger_mtx_;

public:
    user &get_or_create_user(const std::string &name);
};

}  // namespace bank
#endif  // BANK.HPP