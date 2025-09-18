
# SW Coding Standard

---

## Table of Contents
1. [Introduction](#introduction)
2. [Documents](#documents)
3. [Terms, Definitions and Abbreviations](#terms-definitions-and-abbreviations)
4. [Coding Standard Rules](#coding-standard-rules)

---

## 1. Introduction

### 1.1 Purpose of this Document
This document is the coding standard for C language. Its purpose is to standardize the way in which the C code is used and written in order to increase code readability, maintainability, reliability and security.

### 1.2 Scope of this Document
This document applies only to SW applications written in C language and doesn't apply to C++ or other languages. Its scope is only an excerpt from a real coding standard used in company projects.

Note that many of the recommendations given in this document are universal and could apply to any programming language.

## 2. Terms, Definitions and Abbreviations

| Acronym | Meaning                                   |
|---------|-------------------------------------------|
| CR      | Coding Rule                               |
| GCC     | GNU Compiler Collection                   |
| IDE     | Integrated Development Environment        |
| IEC     | International Electrotechnical Commission |
| ISO     | International Organization for Standardization |
| MISRA   | Motor Industry Software Reliability Association |
| PA      | Product Assurance                         |
| TBC     | To Be Confirmed                           |
| TBD     | To Be Defined/Determined                  |
| TBW     | To Be Written                             |

---

## 3. Coding Standard Rules

When a rule is stated using the word "shall", any violation to the rule must be properly documented (using a comment with a reason for the violation) and approved by PA. When a rule is worded using the word "should", the developer can take the liberty to break the rule if it benefits code clarity and readability.

The coding rules (CR) are numbered sequentially from CR-1.

### 3.1 General

- **CR-1:** Do not attempt to optimize the code until it is proven to be necessary. The code should be written to be as clear and readable as possible. Modern compilers are able to optimize inefficiently written code well.
- **CR-2:** Only syntax and language constructs which are valid in ISO C90 language standard shall be used. Exceptions are usage of double-slash comments (see section 4.3 Comments) and usage GNU extensions (variable or function attributes) when justified by performance needs or needed by target architecture. It is forbidden to use GNU extensions for bit-wise access to registers.
- **CR-3:** Typedefs that indicate size and signedness (e.g. int32_t) shall be used instead of basic types (e.g. int).
- **CR-4:** Function-like macros shall not be used, unless justified by performance needs.
- **CR-5:** Dynamic memory allocation (malloc) shall not be used, unless justified by performance needs. If dynamic memory allocation is used, the returned allocated pointer shall be checked whether it is not null.
- **CR-6:** "Magic numbers" shall not be present in code, all constants shall be defined using pre-processor macros at the top of the source file. Only exception is when implementing bit-wise operations; then using literals is encouraged as it makes code more readable. Example:

```c
/* Swap top and bottom four bits */
*inout_pu8Byte = (*inout_pu8Byte & 0xF0U) >> 4 | (*inout_pu8Byte & 0x0FU) << 4;
```

### 3.2 Naming

- **CR-7:** English language shall be used for naming of all variables, functions, files etc.
- **CR-8:** All names shall be easily understandable and descriptive of what the variable, function, module etc. is supposed to do. Do not abbreviate names unless the meaning remains clear for everyone.
- **CR-9:** Numbers shall not be used in identifiers to replace words (e.g. use intToArray instead of int2Array).
- **CR-10:** Variables shall use lower camel case. All variables shall have a prefix indicating their type. The list of possible prefixes is given in Table 4. Note that these prefixes can be combined. Example:

```c
uint32_t u32Variable;
uint16_t *pu16PointerToVariable;
uint8_t au8Buffer[BUFFER_SIZE];
```

- **CR-11:** Module names shall use upper camel case. Example:
	- ExampleModule.c
	- ExampleModule.h
- **CR-12:** Function names shall be composed of module name, underscore, and function name in upper camel case. Example:
	- `void ExampleFile_ExampleFunction(void);`
- **CR-13:** Function parameters shall have an additional prefix indicating whether they are an input parameter, output parameter or input/output parameter. These prefixes are in_, out_, and inout_ respectively. Example:
	- `void ExampleFile_ExampleFunction(uint32_t in_u32Input, uint8_t * const out_pu8OutputPointer);`
- **CR-14:** Macro names and preprocessor defines shall be uppercase, with underscores between words. For public defines (which are in header file), the name shall start with the module name. Example:
	- `#define HEADER_SIZE 12`
	- `#define EXAMPLE_MODULE_PUBLIC_DEFINE 14`
- **CR-15:** Variables which are static in a module (i.e. outside of functions) shall have prefix m_. Example:
	- `static uint32_t m_u32Counter = 0U;`
- **CR-16:** When defining new structures, unions, enums etc., keyword "typedef" shall be used (see rules below for examples).
- **CR-17:** Structure type names shall be upper camel case and have suffix _t. If the type is global (exported to other modules), it shall have the module name as prefix. This is also recommended for internal structures. Example:

```c
typedef struct {
	uint32_t u32Member1;
	uint8_t  u8Member2;
} LocalStructureName_t; /* Preferably ExampleModule_LocalStructureName_t */

typedef struct {
	uint32_t u32Member1;
	uint8_t  u8Member2;
} ExampleModule_GlobalStructureName_t;
```

- **CR-18:** Union naming shall be equivalent to structures, with _u suffix instead of _t.
- **CR-19:** Enum type naming shall be equivalent to structures, with _e suffix instead of _t. Names of enum members shall be upper case with underscores between words. They shall begin with prefix E_, followed by enum name, followed by member name. If the enum is public (in header file), the name of each member shall also contain the name of the module between E_ and the enum name. Example:

```c
typedef enum {
	E_COLOR_RED,
	E_COLOR_GREEN,
	E_COLOR_BLUE
} LocalColor_e; /* Preferably ExampleModule_LocalColor_e */

typedef enum {
	E_EXAMPLEMODULE_COLOR_RED,
	E_EXAMPLEMODULE_COLOR_GREEN,
	E_EXAMPLEMODULE_COLOR_BLUE
} ExampleModule_GlobalColor_e;
```

- **CR-20:** Using typedef to create other custom types should be avoided for code readability reasons, but if such a typedef is defined, the naming shall equivalent to structures, with _t suffix.

#### Table 4: List of variable prefixes

| Type         | Variable prefix |
|--------------|----------------|
| uint8_t      | u8             |
| int8_t       | s8             |
| uint16_t     | u16            |
| int16_t      | s16            |
| uint32_t     | u32            |
| int32_t      | s32            |
| uint64_t     | u64            |
| int64_t      | s64            |
| float32_t    | f32            |
| float64_t    | f64            |
| array        | a              |
| pointer      | p              |
| enum(*)      | e              |
| boolean      | b              |
| structure    | t              |
| function pointer | pFn           |

> (*) for enums which are essentially boolean use prefix "b".

- **CR-21:** Variables which are global in a project shall have prefix g_. Example:
	- `static I2C_HandleTypeDef g_tI2cHandle;`

### 4.3 Comments

- **CR-22:** All comments shall be in English language.
- **CR-23:** Every single function shall be commented. For simple functions, a short comment explaining the function at its start is sufficient.
- **CR-24:** All functions shall be commented using the Doxygen format in source file (not header file). Example below:

```c
/**
 * @brief This is a brief description of the function.
 *
 * This is a more detailed description of the function (if needed). In theory, after
 * reading this description, it should not be necessary to look into the function.
 *
 * @param in_u32Input       Description of input parameter (including valid range)
 * @param inout_au8Buffer   Description of the buffer
 * @param out_u32Result     Description of output parameter (including valid range)
 *
 * @return  E_TRUE  Function ended successfully
 * @return  E_FALSE Function encountered an unspecified error
 */
bool_e ExampleFile_ExampleFunction(uint32_t in_u32Input, uint8_t const inout_au8Buffer[BUFFER_SIZE], uint32_t * const out_u32Result)
{
		...
}
```


### 3.4 Includes

- **CR-25:** The first include in any source file shall be the corresponding header file.
- **CR-26:** Only include what is absolutely necessary. Keep coupling between modules minimal.

### 3.5 Variables

- **CR-26:** There shall be no global variables (available from outside of module scope) unless required by third-party libraries or required by design.
- **CR-27:** All variables shall be declared on their own line.
- **CR-28:** All variables (except arrays) shall be initialized on their declaration. If an array is initialized during declaration, it shall by initialized fully.
- **CR-29:** Variables which are static in a module (i.e. outside of functions) shall be declared static.
- **CR-30:** During initialization of unsigned variables, the unsigned literal "U" shall be used. This applies to hexadecimal representation as well. Example:

```c
int32_t s32Example1 = 1;
uint32_t u32Example2 = 1U;
uint32_t u32Example3 = 0x01U;
```

### 3.6 Functions

- **CR-31:** Each function should have only one return statement, which should be placed as the last line of the function. It is however also allowed to return at the start of a function in case some of the function parameters are out of permitted range.
- **CR-32:** Every function shall have (at least one) return statement, including void functions (so that breakpoint can be placed there).
- **CR-33:** All functions shall have a prototype, either in the header file (for public functions) or at the top of source file (for local functions). Local functions shall be declared as static.
- **CR-34:** Functions which have no parameters shall have the "void" keyword in their parameter list.
- **CR-35:** Recursion is forbidden; functions shall not call themselves.

### 4.7 Operators

- **CR-36:** All expressions which have more than two operands shall be parenthesized (the precedence of operators needs to be made explicit). Example:
	- `x = b + (c * d);`
- **CR-37:** Increment "++" or decrement "--" shall not be used in complex expressions. It shall be used only if it is the only operation on the line of code. Example:

```c
if (counter < MAX_COUNTER)
{
	counter++;
}
```

### 4.8 Control Statements

- **CR-38:** One-line if statements shall not be used. Always put the condition in parenthesis and the branch statement on next line.
- **CR-39:** There shall be no function calls in the control statement. All control statements shall be composed only of constants and variables, which shall not be modified in the control expression.
- **CR-40:** In "if" statement, the comparison shall be always explicit, i.e. when testing whether some expression is greater than zero, write the "> 0" explicitly.
- **CR-41:** For equality comparison "==" the constant shall be the first operand to prevent confusing "=" with "==". Example:

```c
if (3 == x)
{
	y = 2;
}
```

- **CR-42:** For 3 branches, use if - else if - else construct.
- **CR-43:** When there are more than 3 branches, use nested "if" statements (if the condition is complicated) or use switch statement.
- **CR-44:** In switch statement, every case should be terminated with a break statement. In case fall-through between cases is intended, it shall be explicitly stated in a comment placed instead of the break statement. Every switch statement shall have a default case. Variables shall not be declared inside a switch statement before the first case label.
- **CR-45:** There should be no more than one “break” statement terminating a cycle.


