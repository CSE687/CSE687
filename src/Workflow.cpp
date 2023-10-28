#include "Workflow.hpp"

#include <map>
#include <regex>
#include <string>

#include "FileManager.hpp"
#include "Map.hpp"
#include "Reduce.hpp"
// #include "WordToken.hpp"

using namespace std;

#ifdef DEBUG
#define DEBUG_MSG(str)       \
    do {                     \
        cout << str << endl; \
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
    Map mapper;  // unimplemented; set temp dir.
    for (string currfile : input_files) {
        vector<string> contents;
        try {
            contents = fileManager->readFile(currfile);
        } catch (exception& e) {
            skippedFiles.push_back(currfile);
            continue;
        }
        if (contents.size() == 0) {
            cout << "File '" << currfile << "' was empty; no mapping done." << endl;
            skippedFiles.push_back(currfile);
        }
        int wordcount = 0;
        int lineNum = 0;
        int numLines = contents.size();
        for (string currline : contents) {
            wordcount += mapper.map(currfile, currline, numLines, lineNum);  // unimplemented; tokenize, exportData, and return num tokens.
            lineNum++;
        }
#ifdef DEBUG
        DEBUG_MSG("Mapper tokenized " + to_string(wordcount) + " words from " + currfile);
#endif
    }
    cout << "Mapper completed intermediate files." << endl;
}
// Sorts into a multimap:
// vector<string> tempFiles = fileManager->getDirectoryFileList(fileManager->getTempDirectory());
// Reduce reducer = Reduce(fileManager->getOutputDirectory());  // maybe this shuold go in the above for-loop?
// for (string currfile : tempFiles) {
//    vector<string> contents;
//    try {
//        contents = fileManager->readFile(currfile);
//    } catch (exception& e) {
//        cerr << "Exception while reading map " << currfile << ": " << e.what() << endl;
//        continue;
//    }
//    vector<WordToken> tokens;
//    sortme(contents, &tokens);
//}  // tokens is aggregated and sorted; sending to Reduce item by item.

// for (WordToken i : tokens) {
// }
//}

/*
 * This helper function reads in a vector of strings that look like:
 *  ("a", 1); --> ("a", {1, 1, 1, 1, 1})
 *  ("the", 1)
 *  ("is", 1)
 *  ("the", 1)
 *  ("that", 1)
 *  ("ish", 1)
 * ...and sorts and aggregates their counts. The resulting vector gets passed to Reducer.



void sortme(vector<string> lines, vector<WordToken>* tokens) {
    // grab data from each line
    // create WordToken, append to tokens vector
    // sort vector by length of counts
    smatch m;
    regex regexp("^\\(\"([a-z]+)\", (\\d+)\\)$");

    for (string i : lines) {
        regex_search(i, m, regexp);
        if (m.size() < 2) {
            cerr << "NOMATCH Line: " << i << endl;
        } else {
            auto f = map->find(m[1]);
            if (f != map->end()) {
                f->second.push_back(m[2]);
            } else {
                map->insert();
            }
        }
    }
}
*/
