#include "eosbocai2222.hpp"

void eosbocai2222::reveal(const uint64_t &id)
{
    require_auth(_self);
    st_bet bet = find_or_error(id);
    uint8_t random_roll = random(bet.player, bet.id);
    asset payout = asset(0, bet.amount.symbol);
    if (random_roll < bet.roll_under)
    {
        payout = compute_payout(bet.roll_under, bet.amount);
        action(permission_level{_self, N(active)},
               bet.amount.contract,
               N(transfer),
               make_tuple(_self, bet.player, payout, winner_memo(bet)))
            .send();
    }
    if (iseostoken(bet.amount))
    {
        issue_token(bet.player, bet.amount, "mining! eosdice.vip");
        unlock(bet.amount);
    }

    st_result result{.bet_id = bet.id,
                     .player = bet.player,
                     .referrer = bet.referrer,
                     .amount = bet.amount,
                     .roll_under = bet.roll_under,
                     .random_roll = random_roll,
                     .payout = payout};

    action(permission_level{_self, N(active)},
           LOG,
           N(result),
           result)
        .send();
    action(permission_level{_self, N(active)},
           bet.amount.contract,
           N(transfer),
           std::make_tuple(_self, DEV, compute_dev_reward(bet), std::string("for dev")))
        .send();
    action(permission_level{_self, N(active)},
           bet.amount.contract,
           N(transfer),
           make_tuple(_self,
                      bet.referrer,
                      compute_referrer_reward(bet),
                      referrer_memo(bet)))
        .send();
    remove(bet.id);
}
void eosbocai2222::reveal1(const uint64_t &id)
{
    require_auth(_self);
    send_defer_action(permission_level{_self, N(active)},
                      _self,
                      N(reveal),
                      id);
}

void eosbocai2222::onTransfer(account_name from,
                              account_name to,
                              extended_asset quantity,
                              string memo)
{
    if (from == _self || to != _self)
    {
        return;
    }
    if (memo.substr(0, 4) != "dice")
    {
        return;
    }
    checkAccount1(from);
    uint8_t roll_under;
    account_name referrer;
    parse_memo(memo, &roll_under, &referrer);
    eosio_assert(is_account(referrer), "referrer account does not exist");
    // //check referrer
    eosio_assert(referrer != from, "referrer can not be self");

    // //check roll_under
    assert_roll_under(roll_under, quantity);
    // //check quantity
    assert_quantity(quantity);

    const st_bet _bet{.id = next_id(),
                      .player = from,
                      .referrer = referrer,
                      .amount = quantity,
                      .roll_under = roll_under,
                      .created_at = now()};
    save(_bet);
    if (iseostoken(quantity))
    {
        //count player
        iplay(from, quantity);

        //vip check
        vipcheck(from, quantity);

        lock(quantity);
        fomo(_bet);
    }

    action(permission_level{_self, N(active)},
           _bet.amount.contract,
           N(transfer),
           std::make_tuple(_self, N(eosbocai1111), compute_pool_reward(_bet), std::string("Dividing pool")))
        .send();
    send_defer_action(permission_level{_self, N(active)},
                      _self,
                      N(reveal1),
                      _bet.id);
}
void eosbocai2222::addtoken(account_name contract, asset quantity)
{
    require_auth(_self);
    _tokens.emplace(_self, [&](auto &r) {
        r.contract = contract;
        r.symbol = quantity.symbol;
        r.minAmout = quantity.amount;
    });
}
void eosbocai2222::init()
{
    require_auth(_self);
    st_global global = _global.get_or_default();

    global.current_id = 515789;
    global.nexthalve = 7524000000 * 1e4;
    global.eosperdice = 100;
    global.initStatu = 1;
    global.lastPlayer = N(eosbocai1111);
    global.endtime = now() + 60 * 5;
    global.fomopool = asset(0, EOS_SYMBOL);
    _global.set(global, _self);
}
