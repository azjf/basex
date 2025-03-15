#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <utility>

#include <boost/program_options.hpp>

#include <basex.h>

using std::string, std::cin, std::cout, std::cerr, std::endl, std::ifstream, std::istreambuf_iterator, std::ostreambuf_iterator;
namespace po = boost::program_options;

using basex::bxencode, basex::bxdecode;
namespace b64 = basex::b64;
namespace b58 = basex::b58;

void parse_arg(int argc, char **argv, string &algo, string &input_file, bool &is_decode)
{
    po::options_description generic(R""""(Usage: basex [OPTION]... [FILE]
Basex encode or decode FILE, or standard input, to standard output.

With no FILE, or when FILE is -, read standard input.

Mandatory arguments to long options are mandatory for short options too)"""");
    // clang-format off
    generic.add_options()
        ("algorithm,a", po::value<string>(&algo)->default_value("b64"), "algorithm: b64 (RFC 4648), b58 (Bitcoin), or codec digits for the number base convertion")
        ("decode,d", "decode data")
        ("help,h", "display this help and exit");
    // clang-format on

    po::options_description hidden("Hidden options");
    hidden.add_options()("input-file", po::value<string>(), "input file");
    po::positional_options_description p;
    p.add("input-file", -1);

    po::options_description cmdline_options;
    cmdline_options.add(generic).add(hidden);

    try {
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << generic;
            exit(0);
        }

        is_decode = vm.count("decode");

        if (vm.count("input-file")) { input_file = vm["input-file"].as<string>(); }
    } catch (const po::error &e) {
        cerr << "basex: " << e.what() << "\nTry 'basex --help' for more information." << endl;
        exit(1);
    }
}

int main(int argc, char **argv)
{
    string algo, input_file;
    bool is_decode = false;
    parse_arg(argc, argv, algo, input_file, is_decode);

    istreambuf_iterator<char> beg;
    ifstream f;
    if (!input_file.empty()) {
        auto err = [&file = std::as_const(input_file)](const string &msg) {
            cerr << "basex: " << file << ": " << msg << endl;
            return 1;
        };
        if (!std::filesystem::is_regular_file(input_file)) { return err("No such file"); }
        f = ifstream(input_file, std::ios::binary);
        if (!f.good()) { return err("Permission denied"); }
        beg = istreambuf_iterator<char>(f);
    } else {
        beg = istreambuf_iterator<char>(cin);
    }

    istreambuf_iterator<char> end;
    ostreambuf_iterator<char> result(cout);
    const std::map<string, std::array<decltype(result) (*)(decltype(beg), decltype(beg), decltype(result)), 2>> funcs{{"b64", {b64::encode, b64::decode}},
                                                                                                                      {"b58", {b58::encode, b58::decode}}};
    if (funcs.contains(algo)) {
        funcs.at(algo)[is_decode](beg, end, result);
    } else {
        if (algo.size() < 2) {
            cerr << "basex: " << algo << ": Invalid algorithm" << endl;
            return 1;
        }
        if (!is_decode) {
            bxencode(beg, end, result, algo);
        } else {
            bxdecode(beg, end, result, algo);
        }
    }
    return 0;
}
