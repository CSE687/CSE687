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
