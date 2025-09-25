Build instructions for EmbeddedCLI: Copy this command to your terminal and hit enter

`rm -rf build && mkdir build && cd build && cmake .. && make && cd .. && ./build/firmware-cli`

Generating API documentation with Doxygen

If you have Doxygen installed you can generate the HTML API docs with:

```bash
doxygen Doxyfile
```

The documentation will be written to the `docs/html` folder. See `Doxyfile`
in the project root for the configuration used by the generation step.

## How to use (example)

This repository includes a tiny example program (`main.c`) that shows how to
initialize the CLI, register commands and feed characters from a console.

Build and run the example:

```bash
rm -rf build && mkdir build && cd build && cmake .. && make && cd .. && ./build/firmware-cli
```

Example commands registered by the example program:

- `hello` — prints "Hello World!"
- `display_args` — prints each argument passed to the command
- `clear` — clears the console screen (ANSI escape sequence)
- `echo` — an example echo command (this binding is unregistered by the example)

The example command handlers are defined in `main.c`. The bindings used are:

```c
static Cli_Binding_t atCliBindings[] = {
		{ "hello", Cli_HelloWorld, NULL, "Say hello" },
		{ "display_args", Cli_DisplayArgs, NULL, "Displays the given cli arguments" },
		{ "clear", Cli_ClearScreen, NULL, "Clears the screen" },
		{ "echo", Cli_EchoString, NULL, "Echoes the given string" },
};
```

The example registers all bindings, then unregisters the `echo` command to
demonstrate unregistering:

```c
for( size_t i = 0; i < CLI_GET_ARRAY_SIZE( atCliBindings ); i++ )
{
		Cli_RegisterBinding( &atCliBindings[i] );
}

Cli_UnregisterBinding( "echo" );
```

Sample session (typed at console):

```
> hello
[OK] Hello World!
> display_args one two
argv[0] --> "display_args"
argv[1] --> "one"
argv[2] --> "two"
> clear
# (screen cleared)
> help
* hello: 
							Say hello
* display_args: 
							Displays the given cli arguments
* clear: 
							Clears the screen
```

Notes
- The CLI prints prompts and messages using the output handler you provide to
	`Cli_Init` (e.g. `putchar`).
- To generate API docs, run `doxygen Doxyfile` and open `docs/html/index.html`.

## Init and polling

Initialization and feeding characters to the CLI are intentionally simple so
the library can be used on constrained embedded targets.

1) Prepare a `Cli_Config_t` instance and call `Cli_Init` with a character
	 output function (for example `putchar` wrapped to match the signature):

```c
static Cli_Config_t tCliCfg = { 0 };

int Console_PutCharacter(char c)
{
		return putchar(c);
}

// Initialize the CLI once at startup
Cli_Init(&tCliCfg, Console_PutCharacter);
```

2) Register command bindings. The CLI copies the binding structures, so you
	 may pass a stack or static array and don't need to keep the original alive
	 afterwards:

```c
static Cli_Binding_t atCliBindings[] = {
		{ "hello", Cli_HelloWorld, NULL, "Say hello" },
		{ "display_args", Cli_DisplayArgs, NULL, "Displays args" },
};

for (size_t i = 0; i < CLI_GET_ARRAY_SIZE(atCliBindings); ++i) {
		Cli_RegisterBinding(&atCliBindings[i]);
}
```

3) Feeding characters (polling / ISR):

- Polling loop (example from `main.c`):

```c
while (1) {
		char c = Console_GetCharacter(); // blocking or non-blocking getchar
		Cli_ReceiveCharacter(c);
}
```

- From a UART receive ISR: call `Cli_ReceiveCharacter()` for each received
	byte. The library itself is small and uses simple buffer manipulations. If
	your platform requires it, make sure `Cli_ReceiveCharacter()` is called in
	a context that is safe for the used output function and that mutual
	exclusion is provided when the main code and ISR could both touch the CLI
	state. In many embedded setups the output function used by the CLI is
	non-reentrant (e.g. blocking `putchar`), so prefer to use a small ring
	buffer in the ISR and call `Cli_ReceiveCharacter()` from a non-ISR
	context if possible.

Notes on concurrency and safety
- `Cli_Config_t` must remain valid for the lifetime of the CLI (don't free it).
- If you call `Cli_ReceiveCharacter()` from both ISR and main context, protect
	access to the CLI state (for example disable interrupts briefly or use a
	lock-free single-writer approach). The simplest safe pattern is: from the
	ISR push bytes to a tiny circular buffer and in the main loop call
	`Cli_ReceiveCharacter()` for bytes popped from that buffer.
