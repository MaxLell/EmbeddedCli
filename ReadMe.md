Build instructions for EmbeddedCLI: Copy this command to your terminal and hit enter

`rm -rf build && mkdir build && cd build && cmake .. && make && cd .. && ./build/firmware-cli`


## How to download, build & run
This repository includes a tiny example program (`host.c`) that shows how to
initialize the CLI, register commands and feed characters from a console.

Download the repo with and also download the sub-modules, which are refered in the repo.
`git clone https://github.com/MaxLell/EmbeddedCli.git && git submodule update --init --recursive`

Then build and run the example (with CMake):

```bash
rm -rf build && mkdir build && cd build && cmake .. && make && cd .. && ./build/firmware-cli
```

This will automatically build and run the demo, which can be run on a host computer (tested with Linux, but might also work with Windows / Mac).

The demo can be found in `example/host.c`. This should be fairly self-explanatory.

__Sample session (typed at console):__

```
==================================================
> help
help
--------------------------------------------------
* help: 
              List all commands
* clear: 
              Clear the screen
* reset: 
              Reset the CLI
* hello: 
              Say hello
* args: 
              Displays the given cli arguments
* echo: 
              Echoes the given string
--------------------------------------------------
Status -> [OK]   

==================================================
> args a b c d
args a b c d
--------------------------------------------------
argv[0] --> "args" 

argv[1] --> "a" 

argv[2] --> "b" 

argv[3] --> "c" 

argv[4] --> "d" 

--------------------------------------------------
Status -> [OK]   
==================================================
```

## How to test
For that you need to have ceedling (https://github.com/ThrowTheSwitch/Ceedling) installed on your machine.
You can run the tests by simply typing in the repo `ceedling`

```
-----------------------
✅ OVERALL TEST SUMMARY
-----------------------
TESTED:  17
PASSED:  17
FAILED:   0
IGNORED:  0
```