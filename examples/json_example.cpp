//
// json_example.cpp — Example program: format analyzeRuby output as JSON
//
// Build (with JUMANLRC_BUILD_EXAMPLES=ON):
//   mkdir build && cd build
//   cmake .. -DJUMANLRC_BUILD_EXAMPLES=ON
//   cmake --build . --config Release
//
// Usage:
//   json_example --model <path> [--katakana] --text <text> [--output <path>]
//   json_example --model <path> [--katakana] --input <path> [--output <path>]
//
// Examples:
//   json_example --model jumandic.jppmdl --text "私は学生です"
//   json_example --model jumandic.jppmdl --katakana --text "ラーメンを食べる"
//   json_example --model jumandic.jppmdl --katakana --input input.txt --output result.json
//
// Output format:
//   [
//     {"surface": ["readings..."]},
//     ...
//   ]
//   Each element is {surface: [mora1, mora2, ...]}.
//   readings is empty if the unit is pure kana (surface == reading).
//

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#endif

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "jumanlrc_lib.h"

using json = nlohmann::json;
namespace jpp = jumanlrc::lib;

struct Options
{
    std::string modelPath;
    std::string text;
    std::string inputPath;
    std::string outputPath;
    bool katakana = false;
};

static void printUsage(const char *prog)
{
    std::cerr << "Usage:\n"
              << "  " << prog << " --model <path> [--katakana] --text <text> [--output <path>]\n"
              << "  " << prog << " --model <path> [--katakana] --input <path> [--output <path>]\n";
}

static bool parseOptions(int argc, char *argv[], Options &opts)
{
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--model" && i + 1 < argc)
            opts.modelPath = argv[++i];
        else if (arg == "--text" && i + 1 < argc)
            opts.text = argv[++i];
        else if (arg == "--input" && i + 1 < argc)
            opts.inputPath = argv[++i];
        else if (arg == "--output" && i + 1 < argc)
            opts.outputPath = argv[++i];
        else if (arg == "--katakana")
            opts.katakana = true;
        else
        {
            std::cerr << "Unknown option: " << arg << "\n";
            return false;
        }
    }

    if (opts.modelPath.empty())
    {
        std::cerr << "Error: --model is required.\n";
        return false;
    }
    if (opts.text.empty() && opts.inputPath.empty())
    {
        std::cerr << "Error: either --text or --input is required.\n";
        return false;
    }
    if (!opts.text.empty() && !opts.inputPath.empty())
    {
        std::cerr << "Error: use either --text or --input, not both.\n";
        return false;
    }

    return true;
}

static std::string readFile(const std::string &path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open())
    {
        std::cerr << "Error: cannot open file: " << path << "\n";
        return {};
    }
    return std::string((std::istreambuf_iterator<char>(ifs)),
                       std::istreambuf_iterator<char>());
}

static json rubyResultToJson(const std::vector<jpp::RubyUnit> &ruby)
{
    json arr = json::array();
    for (const auto &u : ruby)
    {
        json reads = json::array();
        for (const auto &r : u.readings)
            reads.push_back(r);

        json entry;
        entry[u.surface] = reads;
        arr.push_back(std::move(entry));
    }
    return arr;
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);

    // Convert command-line arguments from system code page to UTF-8
    // (Linux/macOS pass UTF-8 natively; Windows uses the active code page)
    std::vector<std::string> utf8Args;
    std::vector<char *> utf8Ptrs;
    {
        LPWSTR *wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (wargv)
        {
            utf8Args.resize(argc);
            utf8Ptrs.resize(argc);
            for (int i = 0; i < argc; i++)
            {
                int len = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1,
                                              nullptr, 0, nullptr, nullptr);
                utf8Args[i].resize(len - 1);
                WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1,
                                    &utf8Args[i][0], len, nullptr, nullptr);
                utf8Ptrs[i] = &utf8Args[i][0];
            }
            LocalFree(wargv);
        }
        argv = utf8Ptrs.data();
    }
#endif

    Options opts;
    if (!parseOptions(argc, argv, opts))
    {
        printUsage(argv[0]);
        return 1;
    }

    // Read input text
    std::string inputText;
    if (!opts.text.empty())
        inputText = opts.text;
    else
    {
        inputText = readFile(opts.inputPath);
        if (inputText.empty())
            return 1;
    }

    // Run analysis
    jpp::JumanppLib jpp;
    jpp.setKatakanaAnnotation(opts.katakana);

    if (!jpp.loadModel(opts.modelPath))
    {
        std::cerr << "Error: " << jpp.error() << "\n";
        return 1;
    }

    if (!jpp.analyzeRuby(inputText))
    {
        std::cerr << "Error: " << jpp.error() << "\n";
        return 1;
    }

    // Output JSON
    json result = rubyResultToJson(jpp.lastRubyResult());
    std::string jsonStr = result.dump() + "\n";

    if (!opts.outputPath.empty())
    {
        std::ofstream ofs(opts.outputPath);
        if (!ofs.is_open())
        {
            std::cerr << "Error: cannot write file: " << opts.outputPath << "\n";
            return 1;
        }
        ofs << jsonStr;
    }
    else
    {
        std::cout << jsonStr;
    }

    return 0;
}
