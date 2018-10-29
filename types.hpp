#include "utils.hpp"

#define EOS_SYMBOL S(4, EOS)
#define DICE_SYMBOL S(4, BOCAI)
#define LOG N(eosbocailogs)
#define DICETOKEN N(eosbocai1111)
#define DEV N(eosbocaidevv)
#define PRIZEPOOL N(eosbocai1111)
#define DICESUPPLY 88000000000000

typedef uint32_t eostime;

// @abi table bets i64
struct st_bet
{
    uint64_t id;
    account_name player;
    account_name referrer;
    asset amount;
    uint8_t roll_under;
    uint64_t created_at;
    uint64_t primary_key() const { return id; }
};

// @abi table users i64
struct st_user
{
    account_name owner;
    asset amount;
    uint32_t count;
    uint64_t primary_key() const { return owner; }
};

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
};

typedef multi_index<N(users), st_user> tb_uesrs;
typedef multi_index<N(bets), st_bet> tb_bets;
typedef singleton<N(fundpool), st_fund_pool> tb_fund_pool;
typedef singleton<N(global), st_global> tb_global;
