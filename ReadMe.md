## How to download, build & run
This repository includes a tiny example program (`host.c`) that shows how to
initialize the CLI, register commands and feed characters from a console.

Download the repo and its sub-modules with the following command
```bash
git clone https://github.com/MaxLell/EmbeddedCli.git && cd EmbeddedCli && git submodule update --init --recursive
```

Then build and run the example (with CMake installed):

```bash
rm -rf build && mkdir build && cd build && cmake .. && make && cd .. && ./build/firmware-cli
```

This will automatically build and run the demo, which can be run on a host computer (tested with Linux, but might also work with Windows / Mac).

The demo can be found in `example/host.c`. This should be fairly self-explanatory. This demo covers all functionality of the EmbeddedCli

## Explanation on the demo

Once you launched the demo, you can enter your command and hit enter. For a simple start: enter `help`, then the following output will be generated

You can also enter `cle` and then hit the `Tab` key, and the cli autocompletes your command string (for the current example of `cle` the command is completed to `clear`). You can now press enter and everything works the same as if you would have entered `clear`.

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

For more details on the workings of the cli, please follow along in the `example/host.c` file. There is more documentation provided on how to use the EmbeddedCli

---

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