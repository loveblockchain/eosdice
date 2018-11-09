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
    eostime start = 1541768400; //UTC8 2018-11-9 21:00:00
    eostime end = 1541854800;   //UTC8 2018-11-10 21:00:00
    if (now() > start and now() < end)
    {
        if (random_roll == 8 or random_roll == 18 or random_roll == 28 or random_roll == 38 or random_roll == 48 or random_roll == 58 or random_roll == 68 or random_roll == 78 or random_roll == 88 or random_roll == 98)
        {
            issue_token(bet.player, bet.amount * 3, "mining! eosdice.vip");
        }
        else
        {
            issue_token(bet.player, bet.amount, "mining! eosdice.vip");
        }
    }
    else
    {
        issue_token(bet.player, bet.amount, "mining! eosdice.vip");
    }
    unlock(bet.amount);

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
           N(eosio.token),
           N(transfer),
           std::make_tuple(_self, DEV, compute_dev_reward(bet), std::string("for dev")))
        .send();
    action(permission_level{_self, N(active)},
           N(eosio.token),
           N(transfer),
           make_tuple(_self,
                      bet.referrer,
                      compute_referrer_reward(bet),
                      referrer_memo(bet)))
        .send();
}
void eosbocai2222::reveal1(const st_bet &bet)
{
    require_auth(_self);
    send_defer_action(permission_level{_self, N(active)},
                      _self,
                      N(reveal),
                      bet);
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
    iplay(from, quantity);

    //vip check
    vipcheck(from, quantity);

    const st_bet _bet{.id = next_id(),
                      .player = from,
                      .referrer = referrer,
                      .amount = quantity,
                      .roll_under = roll_under,
                      .created_at = now()};

    lock(quantity);

    fomo(_bet);

    action(permission_level{_self, N(active)},
           N(eosio.token),
           N(transfer),
           std::make_tuple(_self, N(eosbocaidivi), compute_pool_reward(_bet), std::string("make_profit")))
        .send();
    send_defer_action(permission_level{_self, N(active)},
                      _self,
                      N(reveal1),
                      _bet);
}

void eosbocai2222::init()
{
    require_auth(_self);
    st_global global = _global.get_or_default();

    global.current_id += 1;
    global.nexthalve = 7524000000 * 1e4;
    global.eosperdice = 100;
    global.initStatu = 1;
    global.lastPlayer = N(eosbocai1111);
    global.endtime = now() + 60 * 5;
    global.fomopool = asset(0, EOS_SYMBOL);
    _global.set(global, _self);
}
