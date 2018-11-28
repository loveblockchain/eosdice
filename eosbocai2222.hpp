#include <algorithm>
#include <eosiolib/transaction.hpp>
#include "eosio.token.hpp"
#include "types.hpp"
#include "pradata.hpp"

class eosbocai2222 : public contract
{
  public:
    eosbocai2222(account_name self)
        : contract(self),
          _bets(_self, _self),
          _users(_self, _self),
          _fund_pool(_self, _self),
          _tokens(_self, _self),
          _global(_self, _self){};

    // @abi action
    void transfer(account_name from,
                  account_name to,
                  asset quantity,
                  string memo);

    void onTransfer(account_name from,
                    account_name to,
                    extended_asset quantity,
                    string memo);

    // @abi action
    void reveal(const st_bet &bet);
    // @abi action
    void reveal1(const st_bet &bet);

    // @abi action
    void addtoken(account_name contract, asset quantity);

    // @abi action
    void init();

    void apply(account_name code, action_name action);

  private:
    tb_bets _bets;
    tb_uesrs _users;
    tb_fund_pool _fund_pool;
    tb_global _global;
    tb_tokens _tokens;

    void parse_memo(string memo,
                    uint8_t *roll_under,
                    account_name *referrer)
    {
        // remove space
        memo.erase(std::remove_if(memo.begin(),
                                  memo.end(),
                                  [](unsigned char x) { return std::isspace(x); }),
                   memo.end());

        size_t sep_count = std::count(memo.begin(), memo.end(), '-');
        eosio_assert(sep_count == 3, "invalid memo");

        size_t pos;
        string container;
        pos = sub2sep(memo, &container, '-', 0, true);
        pos = sub2sep(memo, &container, '-', ++pos, true);
        pos = sub2sep(memo, &container, '-', ++pos, true);
        eosio_assert(!container.empty(), "no roll under");
        *roll_under = stoi(container);
        container = memo.substr(++pos);
        if (container.empty())
        {
            *referrer = PRIZEPOOL;
        }
        else
        {
            *referrer = string_to_name(container.c_str());
        }
    }

    asset compute_referrer_reward(const st_bet &bet) { return bet.amount * 2 / 1000; } //10% for ref
    asset compute_dev_reward(const st_bet &bet) { return bet.amount * 4 / 1000; }      // 20% for dev
    asset compute_pool_reward(const st_bet &bet) { return bet.amount * 12 / 1000; }    // 60% for pool
    asset compute_fomopool_reward(const st_bet &bet) { return bet.amount * 2 / 1000; } // 10% for fomo pool

    uint64_t next_id()
    {
        st_global global = _global.get_or_default();
        global.current_id += 1;
        _global.set(global, _self);
        return global.current_id;
    }

    string referrer_memo(const st_bet &bet)
    {
        string memo = "bet id:";
        string id = uint64_string(bet.id);
        memo.append(id);
        memo.append(" player: ");
        string player = name{bet.player}.to_string();
        memo.append(player);
        memo.append(" referral reward! eosdice.vip");
        return memo;
    }

    string winner_memo(const st_bet &bet)
    {
        string memo = "bet id:";
        string id = uint64_string(bet.id);
        memo.append(id);
        memo.append(" player: ");
        string player = name{bet.player}.to_string();
        memo.append(player);
        memo.append(" winner! eosdice.vip");
        return memo;
    }

    void assert_quantity(const extended_asset &quantity)
    {
        auto itr = _tokens.find(quantity.contract + quantity.symbol);
        eosio_assert(itr != _tokens.end(), "Non-existent token");
        eosio_assert(quantity.is_valid(), "quantity invalid");
        eosio_assert(quantity.amount >= itr->minAmout, "transfer quantity must be greater than minimum");
    }
    bool iseostoken(const extended_asset &quantity)
    {
        if ((quantity.contract == N(eosio.token)) && (quantity.symbol == EOS_SYMBOL))
        {
            return true;
        }
        return false;
    }

    void assert_roll_under(const uint8_t &roll_under, const extended_asset &quantity)
    {
        eosio_assert(roll_under >= 2 && roll_under <= 96,
                     "roll under overflow, must be greater than 2 and less than 96");
        eosio_assert(
            max_payout(roll_under, quantity) <= max_bonus(quantity),
            "offered overflow, expected earning is greater than the maximum bonus");
    }

    void unlock(const asset &amount)
    {
        st_fund_pool pool = get_fund_pool();
        pool.locked -= amount;
        eosio_assert(pool.locked.amount >= 0, "fund unlock error");
        _fund_pool.set(pool, _self);
    }

    void lock(const asset &amount)
    {
        st_fund_pool pool = get_fund_pool();
        pool.locked += amount;
        _fund_pool.set(pool, _self);
    }

    asset compute_payout(const uint8_t &roll_under, const extended_asset &offer)
    {
        return min(max_payout(roll_under, offer), max_bonus(offer));
    }
    asset max_payout(const uint8_t &roll_under, const asset &offer)
    {
        const double ODDS = 98 / ((double)roll_under - 1.0);
        return asset(ODDS * offer.amount, offer.symbol);
    }

    asset max_bonus(const extended_asset &quantity) { return available_balance(quantity) / 10; } //Transfer balance to secure account

    asset available_balance(const extended_asset &quantity)
    {
        auto token = eosio::token(quantity.contract);
        const asset balance =
            token.get_balance(_self, symbol_type(quantity.symbol).name());
        asset available;
        if (iseostoken(quantity))
        {
            const asset locked = get_fund_pool().locked;
            available = balance - locked;
        }
        else
        {
            available = balance;
        }
        eosio_assert(available.amount >= 0, "fund pool overdraw");
        return available;
    }

    st_fund_pool get_fund_pool()
    {
        st_fund_pool fund_pool{.locked = asset(0, EOS_SYMBOL)};
        return _fund_pool.get_or_create(_self, fund_pool);
    }

    template <typename... Args>
    void send_defer_action(Args &&... args)
    {
        transaction trx;
        trx.actions.emplace_back(std::forward<Args>(args)...);
        trx.delay_sec = 1;
        trx.send(next_id(), _self, false);
    }

    uint64_t getDiceSupply()
    {
        auto eos_token = eosio::token(DICETOKEN);
        auto supply = eos_token.get_supply(symbol_type(DICE_SYMBOL).name());
        return supply.amount;
    }
    uint8_t random(account_name name, uint64_t game_id)
    {
        auto mixd = tapos_block_prefix() * tapos_block_num() + name + game_id - current_time();
        const char *mixedChar = reinterpret_cast<const char *>(&mixd);
        checksum256 result;
        sha256((char *)mixedChar, sizeof(mixedChar), &result);

        uint64_t random_num = *(uint64_t *)(&result.hash[0]) + *(uint64_t *)(&result.hash[8]) + *(uint64_t *)(&result.hash[16]) + *(uint64_t *)(&result.hash[24]);
        return (uint8_t)(random_num % 100 + 1);
    }
    void issue_token(account_name to,
                     asset quantity,
                     string memo)
    {
        st_global global = _global.get_or_default();
        auto supply = getDiceSupply();
        auto nexthalve = global.nexthalve;
        if ((DICESUPPLY - supply) <= nexthalve)
        {
            global.nexthalve = global.nexthalve * 95 / 100;
            global.eosperdice = global.eosperdice * 3 / 4;
            _global.set(global, _self);
        }

        asset sendAmout = asset(quantity.amount * global.eosperdice, DICE_SYMBOL);
        action(permission_level{_self, N(active)},
               DICETOKEN,
               N(issue),
               make_tuple(to, sendAmout, memo))
            .send();
    }
    void iplay(account_name from, asset quantity)
    {
        tb_uesrs1 _users1(_self, from);
        auto v = _users1.get_or_create(_self, st_user1{});
        v.amount += quantity;
        v.count += 1;
        _users1.set(v, _self);
    }
    void fomo(const st_bet &bet)
    {
        st_global global = _global.get_or_default();
        global.fomopool += compute_fomopool_reward(bet);
        if (bet.amount.amount > global.fomopool.amount / 1000)
        {
            if (now() > global.endtime) //winner winner chicken dinner
            {
                action(permission_level{_self, N(active)},
                       N(eosio.token),
                       N(transfer),
                       std::make_tuple(_self, global.lastPlayer, global.fomopool / 2, std::string("Congratulations, win the fomo award! eosdice.vip ")))
                    .send();
                global.fomopool /= 2;
            }

            global.lastPlayer = bet.player;
            global.endtime = now() + 60 * 60;
        }
        _global.set(global, _self);
    }
    void vipcheck(account_name from, asset quantity)
    {
        tb_uesrs1 _users1(_self, from);
        auto v = _users1.get_or_create(_self, st_user1{});
        uint64_t amount = v.amount.amount / 1e4;
        asset checkout = asset(0, EOS_SYMBOL);
        if (amount < 1000)
        {
            return;
        }
        else if (amount < 5000)
        {
            checkout = asset(quantity.amount * 1 / 10000, EOS_SYMBOL);
        }
        else if (amount < 10000)
        {
            checkout = asset(quantity.amount * 2 / 10000, EOS_SYMBOL);
        }
        else if (amount < 50000)
        {
            checkout = asset(quantity.amount * 3 / 10000, EOS_SYMBOL);
        }
        else if (amount < 100000)
        {
            checkout = asset(quantity.amount * 4 / 10000, EOS_SYMBOL);
        }
        else if (amount < 500000)
        {
            checkout = asset(quantity.amount * 5 / 10000, EOS_SYMBOL);
        }
        else if (amount < 1000000)
        {
            checkout = asset(quantity.amount * 7 / 10000, EOS_SYMBOL);
        }
        else if (amount < 5000000)
        {
            checkout = asset(quantity.amount * 9 / 10000, EOS_SYMBOL);
        }
        else if (amount < 10000000)
        {
            checkout = asset(quantity.amount * 11 / 10000, EOS_SYMBOL);
        }
        else if (amount < 50000000)
        {
            checkout = asset(quantity.amount * 13 / 10000, EOS_SYMBOL);
        }
        else
        {
            checkout = asset(quantity.amount * 15 / 10000, EOS_SYMBOL);
        }
        action(permission_level{_self, N(active)},
               N(eosio.token),
               N(transfer),
               std::make_tuple(_self, from, checkout, std::string("for vip! eosdice.vip")))
            .send();
    }
    void checkAccount1(account_name from)
    {
        auto db = prochain::rating_index(N(rating.pra), N(rating.pra));
        auto me = db.find(from);
        if (me != db.end())
        {
            auto account_type = me->account_type;
            eosio_assert(account_type == 0, "Human only");
        }
    }
};
struct st_transfer
{
    account_name from;
    account_name to;
    asset quantity;
    string memo;
};

void eosbocai2222::apply(account_name code, action_name action)
{
    auto &thiscontract = *this;

    if (action == N(transfer))
    {
        auto transfer_data = unpack_action_data<st_transfer>();
        onTransfer(transfer_data.from, transfer_data.to, extended_asset(transfer_data.quantity, code), transfer_data.memo);
        return;
    }

    if (code != _self)
        return;
    switch (action)
    {
        EOSIO_API(eosbocai2222, (reveal)(init)(reveal1)(addtoken));
    };
}

extern "C"
{
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        eosbocai2222 p(receiver);
        p.apply(code, action);
        eosio_exit(0);
    }
}
