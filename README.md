# CSE687

Winter 2023 class project


## Project 1

- Input: Input Directory, Output Directory, Temp Directory for Intermediates
  - Read Files
- Output: Single output file
  - list of words & count per word
- Can use https://www.boost.org
- Map, Reduce, Workflow, Executive

### Decisions

- Each class defined in a Header, Implementation file
- `main.cpp` includes each
- Classes
  - FileManager
    - Files
      - FileManager.h
      - FileManager.cpp
    - Dev: Chandler
      - Looking into a Singleton pattern for FileManager
        - [X] Separate components instantiating FileManager retrieve a shared object
        - [X] FileManager takes String/Filename and writes the string to the filename
  - Map
    - Files
      - Map.h
      - Map.cpp
    - Dev: Joe
      - Working on `main.cpp` to include original + Map test
  - Reduce
    - Files
      - Reduce.h
      - Reduce.cpp
    - Dev: Wes
  - Sorting
    - Files
      - Sort.h
      - Sort.cpp
      - test_Sort.cpp
    - Dev: Alice
  - Executive:
    - Dev: Wes
  - Workflow:
    - Dev: Wes
  - Get VSCode Debugging working without a lot of fuss
    - Dev: Wes

### Tasks

- [X] Boost compiling on MacOS
  - [X] `brew install boost`
- [X] Write a basic unit test with Boost
- [X] Log with Boost
- [X] Basic file with text separated on rows
  - [X] Read the text and count words
- [X] Output word counts to a file

### Meeting Plans

- [X] Get dev environments setup
- [X] Broke up implementation work & test cases
- [X] Next: Put it all together

## Phase 2

- Devs: Joe & Wes
  - [ ] Inheritance
    - [ ] Reduce & Map share a base class `Executor`
      - [X] Attributes
        - [X] `outputFilename`
        - [X] `fileManager`
      - [X] Pure Virtual Member Function
        - [X] `toString()`
      - [X] Reduce uses base class `Executor`
      - [ ] Map uses base class `Executor`
  - Polymorphism
    - [ ] (Cancelled) Workflow uses base class pointer to derived Reduce & Map class
    - [ ] (Cancelled) Reduce & Map both implement `execute` & `export` functions.
    - [X] Implement `toString()`
  - Templates
    - [ ] Use templates for declaration of `execute` and `export` functions
- Alice
  - Sort Class
  - Sort Optimization
- Chandler
  - [ ] How do DLL's work?
  - [ ] If Reduce & Map share a Base class - does the base class need a DLL?
  - [ ] Does it need to be in a shared DLL?
  - [ ] Explicit is preferred

## Phase 3

Due week after midterm

- [ ] Workflow handles
  - [ ] Partition input based on File
  - [ ] Creating thread pool
- [ ] Workflow assigns each file to an available thread
- [ ] Workflow waits for Mappers to complete before starting Reducers
- [ ] Workflow runs a Reducer in a thread for each file in temp directory
- [X] Sort incorporated into Reduce
  - Dev: Wes

### Meeting 01 - 2023-11-18

- [ ] Spawning Threads in `main`
  - Dev: Joe
- [ ] Workflow Method: executeReduce(filename: string, threadId: int)
  - filename: absolute path
  - Dev: Wes
- [X] Sort/Reduce Combined
  - Dev: Chandler
- [ ] Directly combine Sort/Reduce (map<string, vector<int>> -> map<string, int>)
  - Dev: Wes
- [ ] Documentation
  - Dev: Chandler

## Phase 4

Due 2 weeks after Phase 3
