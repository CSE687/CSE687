# CSE687

Winter 2023 class project


## Quickstart

Of course, you can name things anything you want, but the following is an example.
Starting from the project root folder, this will build a docker image called `ubu`,
and then mounts some volumes that you can access in the container's `/home/vscode`
folder.

```
docker build -t ubu .
docker run --rm -it -v $(pwd)/src:/home/vscode -v $(pwd)/tests:/home/vscode/tests ubu /bin/bash
```

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
    - Dev:
  - Map
    - Files
      - Map.h
      - Map.cpp
    - Dev: Joe
  - Reduce
    - Files
      - Reduce.h
      - Reduce.cpp
    - Dev: Wes
  - Sorting
    - Files
      - Sort.h
      - Sort.cpp
    - Dev: Alice

### Tasks

- [ ] Boost compiling on MacOS
  - [X] `brew install boost`
- [ ] Write a basic unit test with Boost
- [X] Log with Boost
- [ ] Basic file with text separated on rows
  - [ ] Read the text and count words
- [ ] Output word counts to a file

### Meeting Plans

1. Now
  - Get dev environment setup
2. Next meeting
  - Figure out what header files look like
  - Break up implementation work & test cases
    - Individual work
3. Next
  - Put it all together
