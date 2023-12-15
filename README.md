# CSE687

Winter 2023 class project

** The link to our demo video for Phase 3 is [here](https://zoom.us/rec/share/96nE1av1Y8jKv_emqxavAJyixQlyCP5MXH02kXmOoTpU20qP2mDwe3wuto7RtsNJ.b9EsYLQ3xEzsVtvx?startTime=1701641692000). **


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

- 3 Stubs
  - 1 stub does mapping
  - 2 stubs for reduce
- Run mapping on all files
- Run reduce on all files
- Message Format
```
{
  task_id: int
  task: str ["map", "reduce"]
  input_file: str
  output_directory: str
}

# SOLD
{  # From Controller to Stub: Do this task
  "message_type": "map_task" / "reduce_task"
  "batch_id": int
  "files": "<List[fileName]>"
}

# SOLD
{ # From Stub to Controller: completed task
  "message_type": "batch_status"
  "batch_id": int
  "status": "InProgress", "Complete", "Error"
}

// Stub preference is List<fileName>
// How many files per map/reduce task?
// map: # files / # stubs
// reduce: # files / # stubs
```

1. Controller iterates through files in `input_directory`
2. Controller creates Message Format for each file in `input_directory`
3. Controller tracks messages sent to which stub
4. Controller sends Message Format to specific stub
5. Controller waits for Complete/Success message from Stub for specific `task_id`
6. Controller sends next `task_id` for next task to stub

- Controller Class
  - Dev: Wes
  - Needs to track state
    - Files processed
      - Tasks processed per file
    - Tasks submitted to each stub
    - Connected Stubs
      - Last Heartbeat per Stub
- Stub Class
  - Dev: Chandler
  - Stub has implementations of
    - Map
    - Reduce
  - Listens for messags from Controller
    - Launches new thread based on parameters received in message from Controller
    - When the Thread finishes execution:
      - send message back to Controller communicating Success
  - Tracks state of connection with Controller
    - Last Heartbeat time, etc

Controller CLI Command

```
./bin/project-03 controller 9000 workdir/input workdir/output workdir/temp 9001 9002 9003
```

Stub CLI Command

```
./bin/project-03 stub -p 9001
./bin/project-03 stub -p 9002
./bin/project-03 stub -p 9003
```

- What to do if process dies
  - Options:
    - Just log it on controller after X missed heartbeats with stub
    - Stub Status: Ded
