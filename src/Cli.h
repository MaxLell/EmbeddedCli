/**
 * MIT License
 *
 * Copyright (c) <2025> <Max Koell (maxkoell@proton.me)>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#if !defined(CLI_H)
#define CLI_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <stddef.h>
#include <stdint.h>

#define CLI_OK_STATUS                (0)
#define CLI_FAIL_STATUS              (-1)

#define CLI_MAX_NOF_CALLBACKS        (10)
#define CLI_MAX_CMD_NAME_LENGTH      (32)
#define CLI_MAX_HELPER_STRING_LENGTH (64)

#define CLI_MAX_RX_BUFFER_SIZE       (128)

#define CLI_GET_ARRAY_SIZE(arr)      (sizeof(arr) / sizeof(arr[0]))

    typedef int (*cli_cmd_fn)(int argc, char* argv[], void* context);

    typedef int (*cli_put_char_fn)(char c);

    typedef struct
    {
        const char name[CLI_MAX_CMD_NAME_LENGTH];
        cli_cmd_fn cmd_fn;
        void* context;
        const char help[CLI_MAX_HELPER_STRING_LENGTH];
    } cli_binding_t;

    typedef struct
    {
        uint32_t start_canary_word;
        cli_put_char_fn put_char_fn;
        uint8_t is_initialized;

        uint8_t nof_stored_chars_in_rx_buffer;
        char rx_char_buffer[CLI_MAX_RX_BUFFER_SIZE];
        uint32_t mid_canary_word;

        uint8_t nof_stored_cmd_bindings;
        cli_binding_t cmd_bindings_buffer[CLI_MAX_NOF_CALLBACKS];
        uint32_t end_canary_word;
    } cli_cfg_t;

    void cli_init(cli_cfg_t* const inout_module_cfg, cli_put_char_fn in_put_char_fn);

    void cli_register(const cli_binding_t* const in_binding);

    void cli_unregister(const char* const in_cmd_name);

    void cli_receive(char in_char);

    void cli_process(void);

    void cli_receive_and_process(char in_char);

    void cli_print(const char* const fmt, ...);

    void cli_deinit(cli_cfg_t* const inout_module_cfg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // CLI_H
