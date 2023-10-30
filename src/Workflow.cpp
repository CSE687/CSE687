///////////////////////////////////////////////////////////////////////
//  Workflow.cpp    -   manages the execution of the entire pipeline //
//  Language        -   C++                                          //
///////////////////////////////////////////////////////////////////////
#include "Workflow.hpp"

#include <ctime>
#include <map>
#include <regex>

#include "Map.hpp"
#include "Reduce.hpp"

using namespace std;

#ifdef DEBUG
#define DEBUG_MSG(str)                    \
    do {                                  \
        cout << "DEBUG: " << str << endl; \
    } while (false)
#else
#define DEBUG_MSG(str) void() 0  // treat DEBUG_MSG as a no-op if not in DEBUG mode
#endif

Workflow::Workflow(FileManager* filemgr) {
    fileManager = filemgr;
}

void Workflow::execute() {
    input_files = fileManager->getDirectoryFileList(fileManager->getInputDirectory());

#ifdef DEBUG
    string debugmsg = "All Files in Input Directory:\n";
    for (string i : input_files) {
        debugmsg.append("\t");
        debugmsg.append(i);
        debugmsg.append("\n");
    }
    DEBUG_MSG(debugmsg);
#endif
    // Maps all the files:
    cout << "[+] Starting Mapper to parse input files..." << endl;
    Map mapper;
    for (string currfile : input_files) {
        vector<string> contents;
        try {
            contents = fileManager->readFile(currfile);
        } catch (exception& e) {
            cout << "File '" << currfile << "' could not be opened; skipping." << endl;
            skippedFiles.push_back(currfile);
            continue;
        }
        if (contents.size() == 0) {
            cout << "File '" << currfile << "' was empty; no mapping done." << endl;
            skippedFiles.push_back(currfile);
            continue;
        }
        int wordcount = 0;
        int lineNum = 0;
        int numLines = contents.size();
        for (string currline : contents) {
            wordcount += mapper.map(currfile, currline, numLines, lineNum);
            lineNum++;
        }
#ifdef DEBUG
        DEBUG_MSG("Mapper tokenized " + to_string(wordcount) + " words from " + currfile);
#endif
    }
    cout << "[+] Mapper complete." << endl;
    cout << "[+] Sorting and reducing tokens in intermediate files..." << endl;

#ifdef DEBUG
    time_t start_time;
    time_t end_time;
#endif

    input_files = this->fileManager->getDirectoryFileList(this->fileManager->getTempDirectory());

    // Sort & Reduce all of the files output by Mapper
    for (int i = 0; i < input_files.size(); i++) {
#ifdef DEBUG
        DEBUG_MSG("Processing temporary file " + input_files[i]);
        time(&start_time);
#endif
        map<string, vector<int> > sorted_words;
        vector<string> file_lines = this->fileManager->readFile(input_files[i]);
        for (string j : file_lines) {
            regex key_rgx("^\\(([a-z]+),");
            regex value_rgx(", (\\d+)\\)");
            smatch key, value_match;
            regex_search(j, key, key_rgx);
            regex_search(j, value_match, value_rgx);
            if (key.size() < 2 || value_match.size() < 2) {
                cout << "SKIP: No matches found in " << j << endl;
                continue;
            }
            int value = stoi(value_match.str(1));
            map<std::string, std::vector<int> >::iterator find_iter = sorted_words.find(key.str(1));
            if (find_iter == sorted_words.end()) {
                sorted_words.insert({key.str(1), {value}});
            } else {
                vector<int> value_vector = find_iter->second;
                value_vector.insert(value_vector.end(), value);
                find_iter->second = value_vector;
            }
        }

        // Initialize Reducer with output file name
        Reduce reducer = Reduce(fileManager->getFileStem(input_files[i]) + ".txt");

        // Loop over keys in sorted_words and reduce
        for (auto& key : sorted_words) {
            reducer.reduce(key.first, key.second);
        }

        // Write SUCCESS file to output directory
        fileManager->writeFile(fileManager->getOutputDirectory(), reducer.outputFilename + "-SUCCESS", "");

        // remove temp directory
        fileManager->remove(input_files[i]);

#ifdef DEBUG
        time(&end_time);
        DEBUG_MSG("File " + input_files[i] + " complete. (" + to_string(i + 1) + "/" + to_string(input_files.size()) + ") Time: " + to_string(end_time - start_time) + " sec");
#endif
    }
    cout << "[+] Completed sorting and reducing tokens." << endl;
    cout << "[+] Workflow complete." << endl;
}
