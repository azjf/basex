#include <iostream>

#include <basex.h>

using std::cout, std::endl, std::ios, std::ifstream;
using std::string;

namespace b64 = basex::b64;
namespace b58 = basex::b58;

void test()
{
    string data = "test";
    // data = "A";
    // data = "BC";
    // data = "Man";

    string encoded;
    b64::encode(data.cbegin(), data.cend(), back_inserter(encoded));
    cout << encoded << endl;
    string decoded;
    b64::decode(encoded.cbegin(), encoded.cend(), back_inserter(decoded));
    cout << decoded << endl;

    encoded = "";
    b58::encode(data.cbegin(), data.cend(), back_inserter(encoded));
    cout << encoded << endl;
    decoded = "";
    b58::decode(encoded.cbegin(), encoded.cend(), back_inserter(decoded));
    cout << decoded << endl;
}

int main(int argc, char **argv)
{
    test();
    return 0;
}
