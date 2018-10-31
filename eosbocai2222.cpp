#include "eosbocai2222.hpp"

void eosbocai2222::reveal(const st_bet &bet)
{
    require_auth(_self);
    uint8_t random_roll = random(bet.player, bet.id);
    asset payout = asset(0, EOS_SYMBOL);
    if (random_roll < bet.roll_under)
    {
        payout = compute_payout(bet.roll_under, bet.amount);
        action(permission_level{_self, N(active)},
               N(eosio.token),
               N(transfer),
               make_tuple(_self, bet.player, payout, winner_memo(bet)))
            .send();
    }
    unlock(bet.amount);

    st_result result{.bet_id = bet.id,
                     .player = bet.player,
                     .referrer = bet.referrer,
                     .amount = bet.amount,
                     .roll_under = bet.roll_under,
                     .random_roll = random_roll,
                     .payout = payout};

    send_defer_action(permission_level{_self, N(active)},
                      LOG,
                      N(result),
                      result);
}

void eosbocai2222::transfer(const account_name &from,
                            const account_name &to,
                            const asset &quantity,
                            const string &memo)
{
    if (from == _self || to != _self)
    {
        return;
    }
    eostime playDiceStartat = 1540904400; //2018-10-30 21:00:00
    if ("buy token" == memo)
    {
        eosio_assert(playDiceStartat > now(), "Time is up");
        buytoken(from, quantity);
        return;
    }
    if (memo.substr(0, 4) != "dice")
    {
        return;
    }
    uint8_t roll_under;
    account_name referrer;
    parse_memo(memo, &roll_under, &referrer);
    eosio_assert(is_account(referrer), "referrer account does not exist");

    // //check quantity
    assert_quantity(quantity);

    // //check roll_under
    assert_roll_under(roll_under, quantity);

    // //check referrer
    eosio_assert(referrer != from, "referrer can not be self");

    //count player
    // iplay(from, quantity);

    //vip check
    // vipcheck(from, quantity);

    const st_bet _bet{.id = next_id(),
                      .player = from,
                      .referrer = referrer,
                      .amount = quantity,
                      .roll_under = roll_under,
                      .created_at = now()};

    lock(quantity);
    issue_token(from, quantity, "mining! eosdice.vip");
    action(permission_level{_self, N(active)},
           N(eosio.token),
           N(transfer),
           std::make_tuple(_self, DEV, compute_dev_reward(_bet), std::string("for dev")))
        .send();

    action(permission_level{_self, N(active)},
           N(eosio.token),
           N(transfer),
           std::make_tuple(_self, PRIZEPOOL, compute_pool_reward(_bet), std::string("for prize pool")))
        .send();

    action(permission_level{_self, N(active)},
           N(eosio.token),
           N(transfer),
           make_tuple(_self,
                      _bet.referrer,
                      compute_referrer_reward(_bet),
                      referrer_memo(_bet)))
        .send();

    send_defer_action(permission_level{_self, N(active)},
                      _self,
                      N(reveal),
                      _bet);
}

void eosbocai2222::init()
{
    require_auth(_self);
    st_global global = _global.get_or_default(
        st_global{.current_id = _bets.available_primary_key()});

    eosio_assert(global.initStatu != 1, "init ok");

    global.current_id += 1;
    global.nexthalve = 6336000000 * 1e4;
    global.eosperdice = 100;
    global.initStatu = 1;
    _global.set(global, _self);
}
