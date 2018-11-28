#include "utils.hpp"
#include <eosiolib/singleton.hpp>

#define EOS_SYMBOL S(4, EOS)
#define DICE_SYMBOL S(4, BOCAI)
#define LOG N(eosbocailogs)
#define DICETOKEN N(eosbocai1111)
#define DEV N(eosbocaidevv)
#define PRIZEPOOL N(eosbocai1111)
#define DICESUPPLY 88000000000000
#define FOMOTIME 24 * 60 * 60

typedef uint32_t eostime;
using eosio::extended_asset;
using eosio::singleton;

// @abi table bets i64
struct st_bet
{
    uint64_t id;
    account_name player;
    account_name referrer;
    extended_asset amount;
    uint8_t roll_under;
    uint64_t created_at;
    uint64_t primary_key() const { return id; }
};
// @abi table tokens i64
struct st_tokens
{
    account_name contract; // 合约账号
    symbol_type symbol;    // 代币名称
    uint64_t minAmout;     //最小允许投注的值
    uint64_t primary_key() const { return contract + symbol; }
};
typedef multi_index<N(tokens), st_tokens> tb_tokens;

// @abi table users1 i64
struct st_user1
{
    asset amount = asset(0, EOS_SYMBOL);
    uint32_t count = 0;
};

// @abi table users i64
struct st_user
{
    account_name owner;
    asset amount;
    uint32_t count;
    uint64_t primary_key() const { return owner; }
};
typedef singleton<N(users1), st_user1> tb_uesrs1;

struct st_result
{
    uint64_t bet_id;
    account_name player;
    account_name referrer;
    asset amount;
    uint8_t roll_under;
    uint8_t random_roll;
    asset payout;
};

// @abi table fundpool i64
struct st_fund_pool
{
    asset locked;
};

// @abi table global i64
struct st_global
{
    uint64_t current_id;
    double eosperdice;
    uint64_t nexthalve;
    uint64_t initStatu;
    account_name lastPlayer;
    eostime endtime;
    asset fomopool;
};

// typedef multi_index<N(users), st_user> tb_uesrs;
typedef multi_index<N(bets), st_bet> tb_bets;
typedef multi_index<N(users), st_user> tb_uesrs;

typedef singleton<N(fundpool), st_fund_pool> tb_fund_pool;
typedef singleton<N(global), st_global> tb_global;
