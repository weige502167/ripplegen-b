///////////////////////////////////////////////////////////////////////////////////
//
// ripplegen.cpp 
//
// Copyright (c) 2013 Eric Lombrozo
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// Portions of the source were taken from the ripple and bitcoin reference
// implementations. Their licensing terms and conditions must be respected.
// Please see accompanying LICENSE file for details.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "RippleAddress.h"
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <thread>
#include <openssl/rand.h>

#define UPDATE_ITERATIONS 10000

using namespace std;


void getRand(unsigned char *buf, int num)
{
    if (RAND_bytes(buf, num) != 1)
    {
        assert(false);
        throw std::runtime_error("Entropy pool not seeded");
    }
}

static bool startsWith(const string& input, const string& pattern)
{
    for (int i = 0; i < pattern.size(); i++)
    {
        if (input[i] != pattern[i])
            return false;
    }

    return true;
}

void LoopThread(unsigned int n, vector<string> sourcePatterns)
{
    RippleAddress naSeed;
    RippleAddress naAccount;
    string        account_id;

    char fname[128];
    sprintf_s(fname, 128, "result_%i.dat", n);

    uint128 key;
    getRand(key.begin(), key.size());

    uint64_t start_time = time(NULL);
    uint64_t count = 0;

    //TODO: I'm not sure how these things work in C++ :-(  Better make own copy of the vector to avoid concurrency problems
    size_t numOfPatterns = sourcePatterns.size();
    vector<string> patterns;
    for (int i = 0; i < numOfPatterns; i++)
    {
        string copy = sourcePatterns[i].c_str();
        patterns.push_back(copy);
    }

    do {
        naSeed.setSeed(key);
        RippleAddress naGenerator = createGeneratorPublic(naSeed);
        naAccount.setAccountPublic(naGenerator.getAccountPublic(), 0);
        account_id = naAccount.humanAccountID();
        count++;

        for (size_t i = 0; i < numOfPatterns; i++)
        {
            if (startsWith(account_id, patterns[i]))
            {
                Beep(750, 500);
                string secret = naSeed.humanSeed();
                cout << endl << "secret: " << secret << "\t public: " << account_id << "\t (pattern: " << patterns[i] << ")" << endl << endl;

                FILE* f = fopen(fname, "at");
                if (f)
                {
                    fprintf(f, "%s;%s;%s\n", account_id.c_str(), naSeed.humanSeed().c_str(), patterns[i].c_str());
                    fclose(f);
                }

                cout << "DEBUG: key=" << key.ToString() << endl << "===============================" << endl;
            }
        }

        if (count % UPDATE_ITERATIONS == 0)
        {
            uint64_t nSecs = time(NULL) - start_time;
            start_time = time(NULL);
            cout << "(thread " << n << ") Another " << UPDATE_ITERATIONS << " items tested (took " << nSecs << "sec, last " << account_id << ")" << endl;
        }
        key++;
    } while (true);

//What would this be good for?    string pmaster_seed_hex = naSeed.getSeed().ToString();
}


// isPatternValid can be changed depending on encoding being used.
bool isPatternValid(const string& pattern, string& msg)
{
    if (pattern[0] != 'r' || pattern.find("0") != string::npos || pattern.find("l") != string::npos ||
        pattern.find("I") != string::npos || pattern.find("O") != string::npos)
    {
        msg = "Pattern must begin with an 'r' and must not contain any of '0', 'l', 'I', 'O'.";
        return false;
    }
    return true;
}

void usage()
{
    cout << "#" << endl
        << "# Usage: ripplegen.exe [--threads=<number_of_threads_to_run>] --input=<file_with_patterns> ... " << endl
        << "# Patterns are only used for prefix check, i.e. matching addresses will start with a pattern." << endl
        << "# Available letters: rpshnaf39wBUDNEGHJKLM4PQRST7VWXYZ2bcdeCg65jkm8oFqi1tuvAxyz" << endl
        << "# Input file must contain one prefix pattern per line" << endl
        << "#" << endl;
}

int main(int argc, char* argv[])
{
    if (argc < 1)
    {
        usage();
        return 0;
    }

    string inputFilename;
    unsigned int threads = 0;

    for (int i = 1; i < argc; i++)
    {
        char* arg = argv[i];

        if (startsWith(arg, "--threads="))
        {
            threads = std::stoi(arg + 10);
        }
        if (startsWith(arg, "--input="))
        {
            inputFilename = arg + 8;
        }
    }

    if (inputFilename.empty())
    {
        usage();
        return 0;
    }
    ifstream infile(inputFilename);
    if (!infile.good())
    {
        //File not found
        usage();
        return 0;
    }

    if (0 == threads)
    {
        unsigned int cpus = std::thread::hardware_concurrency();
        cout << "Thread count not specified. Found " << cpus << " logical units, will use them all" << endl;
        threads = cpus;
    }

    vector<string> patterns;
    string line;
    string msg;

    while (infile >> line)
    {
        if (line.empty())
        {
            continue;
        }
        if (isPatternValid(line, msg))
        {
            patterns.push_back(line);
        }
        else
        {
            cout << "Bad input '" << line << "'. " << msg << endl;
        }
    }

    if (patterns.size() == 0)
    {
        cout << "No valid patterns provided. Good bye" << endl;
        return 0;
    }
    else
    {
        cout << "#" << endl
             << "# Running " << threads << " thread" << (threads == 1 ? "" : "s") << "." << endl
             << "#" << endl
             << "# Generating seeds for " << patterns.size() << " patterns" << endl << endl;
    }

    vector<std::thread> vpThreads(threads);
    for (unsigned int i = 0; i < threads; i++)
    {
        vpThreads[i] = std::thread(LoopThread, i, patterns);
    }

    for (unsigned int i = 0; i < threads; i++)
    {
        vpThreads[i].join();
    }

    return 0;
}
