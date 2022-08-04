#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib_syscall.h"
#include "main.h"
#include <getopt.h>
#include <sys/file.h>

static cli_t cli;
static const char * promot = "sh >>";

static int do_help (int argc, char **argv) {
    const cli_cmd_t * start = cli.cmd_start;
    while (start < cli.cmd_end) {
        printf("%s %s\n", start->name, start->usage);
        start++;
    }
    return 0;
}

static int do_clear (int argc, char ** argv) {
    printf("%s", ESC_CLEAR_SCREEN);
    printf("%s", ESC_MOVE_CURSOR(0, 0));
    return 0;
}

static int do_echo (int argc, char ** argv) {
    if (argc == 1) {
        char msg_buf[128];
        fgets(msg_buf, sizeof(msg_buf), stdin);
        msg_buf[sizeof(msg_buf) - 1] = '\0';
        puts(msg_buf);
        return 0;
    }

    int count =1;
    int ch;
    while ((ch = getopt(argc, argv, "n:h")) != -1) {
        switch (ch) {
        case 'h':
            puts("echo any message");
            puts("Usage: echo [-n count] message");
            optind = 1;
            return 0;
        case 'n':
            count = atoi(optarg);
            break;
        case '?':
            if (optarg) {
                fprintf(stderr, "Unknown option: -%s\n", optarg);
            }
             optind = 1;
           return -1;
        default:
            break;
        }
    }

    if (optind > argc - 1) {
        fprintf(stderr, "Message is empty \n");
        optind = 1;
        return -1;
    }

    char * msg = argv[optind];
    for (int i = 0; i < count; i++) {
        puts(msg);
    }
    optind = 1;
    return 0;
}

static int do_exit (int argc, char ** argv) {
    exit(0);
    return 0;
}

static const cli_cmd_t cmd_list[] = {
    {
        .name = "help",
        .usage = "help -- list supported command",
        .do_func = do_help,
    },
    {
        .name = "clear",
        .usage = "clear -- clear screen",
        .do_func = do_clear,
    },
    {
        .name = "echo",
        .usage = "echo [-n count] msg -- echo something",
        .do_func = do_echo,
    },
    {
        .name = "quit",
        .usage = "quit from shell",
        .do_func = do_exit,
    }
};

static void show_promot (void) {
    printf("%s", cli.promot);
    fflush(stdout);
}

static const cli_cmd_t * find_builtin (const char * name) {
    for (const cli_cmd_t * cmd = cli.cmd_start; cmd < cli.cmd_end; cmd++) {
        if (strcmp(cmd->name, name) != 0) {
            continue;
        }

        return cmd;
    }

    return (const cli_cmd_t *)0;
}

static void run_builtin (const cli_cmd_t * cmd, int argc, char ** argv) {
    int ret = cmd->do_func(argc, argv);
    if (ret < 0) {
        fprintf(stderr, ESC_COLOR_ERROR"error: %d\n"ESC_COLOR_DEFAULT, ret);
    }
}

static void cli_init (const char * promot, const cli_cmd_t * cmd_list, int size) {
    cli.promot = promot;
    memset(cli.curr_input, 0, CLI_INPUT_SIZE);
    cli.cmd_start = cmd_list;
    cli.cmd_end = cmd_list + size;
}

// shell - ls - ls0, ls1
// shell  - ls0, ls1
static void run_exec_file(const char * path, int argc, char ** argv) {
    int pid = fork();
    if (pid < 0) {
        fprintf(stderr, "fork failed %s", path);
    } else if (pid == 0) {
        for (int i= 0; i < argc; i++) {
            msleep(1000);
            printf("arg %d = %s\n", i, argv[i]);
        }
        exit(-1);
    } else {
        int status;
        int pid = wait(&status);
        fprintf(stderr, "cmd %s result: %d, pid=%d\n", path, status, pid);
    }
}


int main (int argc, char **argv) {
	open(argv[0], O_RDWR);
    dup(0);     // 标准输出
    dup(0);     // 标准错误输出

    cli_init(promot, cmd_list, sizeof(cmd_list)/ sizeof(cmd_list[0]));
    for (;;) {
        show_promot();
        char * str = fgets(cli.curr_input, CLI_INPUT_SIZE, stdin);
        if (!str) {
            continue;
        }

        char * cr = strchr(cli.curr_input, '\n');
        if (cr) {
            *cr = '\0';
        }
        
        cr = strchr(cli.curr_input, '\r');
        if (cr) {
            *cr = '\0';
        }

        int argc = 0;
        char * argv[CLI_MAX_ARG_COUNT];
        memset(argv, 0, sizeof(argv));

        const char * space = " ";
        char * token = strtok(cli.curr_input, space);
        while (token) {
            argv[argc++] = token;
            token = strtok(NULL, space);
        }

        if (argc == 0) {
            continue;
        }

        const cli_cmd_t * cmd = find_builtin(argv[0]);
        if (cmd) {
            run_builtin(cmd, argc, argv);
            continue;
        }

        run_exec_file("", argc, argv);

        // exec
        fprintf(stderr, ESC_COLOR_ERROR"Unknown command: %s\n"ESC_COLOR_DEFAULT, cli.curr_input);
    }
}