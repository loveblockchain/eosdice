#include <eosiolib/crypto.h>
#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/types.hpp>
#include <iostream>

using namespace eosio;
using namespace std;

string uint64_string(uint64_t input)
{
    string result;
    uint8_t base = 10;
    do
    {
        char c = input % base;
        input /= base;
        if (c < 10)
            c += '0';
        else
            c += 'A' - 10;
        result = c + result;
    } while (input);
    return result;
}

size_t sub2sep(const string &input,
               string *output,
               const char &separator,
               const size_t &first_pos = 0,
               const bool &required = false)
{
    eosio_assert(first_pos != string::npos, "invalid first pos");
    auto pos = input.find(separator, first_pos);
    if (pos == string::npos)
    {
        eosio_assert(!required, "parse memo error");
        return string::npos;
    }
    *output = input.substr(first_pos, pos - first_pos);
    return pos;
}
