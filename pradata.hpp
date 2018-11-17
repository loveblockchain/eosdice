#pragma once
#include <eosiolib/eosio.hpp>
#include <string>
#include <eosiolib/contract.hpp>

namespace prochain
{

// @abi table trating i64
struct Rating
{
    account_name account;         //eos account name
    uint8_t account_type;         //enum_account_type: 0, normal account; 1, code account;
    uint8_t normal_account_level; //rating level for normal account, from 0 to 10
    uint8_t code_account_level;   //rating level for code account, from 0 to 10

    uint64_t primary_key() const { return account; }
    EOSLIB_SERIALIZE(Rating, (account)(account_type)(normal_account_level)(code_account_level));
};
typedef eosio::multi_index<N(trating), Rating> rating_index;

//account type
enum enum_account_type
{
    normal_account = 0, // 0: normal account
    code_account,       // 1: code account
    account_type_count
};

//level defined for both normal and code account
uint8_t BP_BLACKLIST = 0;

//level defined for normal account
uint8_t PRABOX_BLACKLIST = 2;
uint8_t PRABOX_GREYLIST = 3;
uint8_t PRABOX_AUTH_VERYFIED = 6;
uint8_t PRABOX_KYC_VERYFIED = 6;

//level defined for code account
uint8_t MALICIOUS_CODE_ACCOUNT = 0;

//call back results
std::string RESULT_FOUND = "FOUND";
std::string RESULT_NOTFOUND = "NOTFOUND";

} // namespace prochain
