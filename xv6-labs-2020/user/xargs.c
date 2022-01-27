// #include "kernel/defs.h"

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/pa_util.h"
#include "kernel/fs.h"

// #include "kernel/defs.h"
////////////////////////////////////////////////////////////////

//////////////////////////////////////////////
// Vec_char
typedef struct
{
    int len;
    char *buff;
} Vec_char;

Vec_char Vec_char_malloc(int size)
{
    Vec_char vc;
    vc.buff = (char *)malloc(size);
    vc.len = size;
    return vc;
}
void Vec_char_free(Vec_char vc)
{
    free(vc.buff);
}
//////////////////////////////////////////////
// Cmd

#define Cmd_max_arg_cnt (10)
typedef struct
{
    char *args[Cmd_max_arg_cnt + 1];
    uint8 arg_cnt;
    /* data */
} Cmd;
void Cmd_print(Cmd cmd);
Cmd Cmd_make(char *args[])
{
    Cmd cmd;
    memset(&cmd, 0, sizeof(Cmd));
    {
        // int i = 0;
        while (cmd.arg_cnt < Cmd_max_arg_cnt && args[cmd.arg_cnt])
        {
            cmd.args[cmd.arg_cnt] = args[cmd.arg_cnt];
            cmd.arg_cnt++;
        }
        cmd.args[cmd.arg_cnt] = 0;
    }
    // cmd.args = args;
    return cmd;
}

Cmd Cmd_append_make(Cmd cmd, char **append)
{
    while (cmd.arg_cnt < Cmd_max_arg_cnt && *append)
    {
        // printf("-----------append\n");
        // Cmd_print(cmd);
        // printf("-----------------\n");
        cmd.args[cmd.arg_cnt] = *append;
        append++;
        cmd.arg_cnt++;
    }
    cmd.args[cmd.arg_cnt] = 0;
    return cmd;
}
Cmd Cmd_append_one_make(Cmd cmd, char *append)
{
    char *arr[2];
    arr[0] = append;
    arr[1] = 0;
    return Cmd_append_make(cmd, arr);
}
void Cmd_print(Cmd cmd)
{
    printf("----------------\ncmd arg cnt %d\n", cmd.arg_cnt);
    //debug
    // {
    int i = 0;
    char **a =
        cmd.args;
    //argv;
    while (*a)
    {
        printf("%s ", *a);
        // argv++;/
        a++;
        i++;
    }
    printf("\n----------------\n");
    /////////////////////////////////////////////////////////
}
char Cmd_exec_in_child_process(Cmd cmd)
{
    if (cmd.args == 0 || *cmd.args == 0)
    {
        printf("invalid cmd\n");
        return 0; //invalid failed
    }
    int pid = fork();
    if (pid == 0)
    {
        exec(cmd.args[0], cmd.args);
        exit(0);
    }
    wait(0);

    return 1;
}

#define Mode_OneShot 0
#define Mode_EachLine 1
// void Cmd_run(Cmd cmd)
// {
//     exec(cmd.cmd, cmd.params);
// }
//////////////////////////////////////////////

void xargs(Cmd cmd, int mode, int fd)
{
    Vec_char vc = Vec_char_malloc(512);
    ///////////////////////////////////////////////////
    // int i, n;
    // int l, w, c, inword;

    // l = w = c = 0;
    // inword = 0;
    // Cmd_exec_in_child_process(cmd);
    int n, vc_i = 0, begin = 0;
    char buf = 0;

    Cmd cmd1 = cmd;
    while ((n = read(fd, &buf, sizeof(char))) > 0)
    { //read part to buff;
        // printf("read %c\n", buf);
        // if (vc_i)
        // {
        // }
        // if (mode == Mode_OneShot)
        // {//save all to buf
        // }
        // else if (mode == Mode_EachLine)
        // {
        // }

        if (buf == '\n' || buf == '*' || buf == ' ')
        {
            vc.buff[vc_i] = 0;
            vc_i++;
            // vc_i = 0;
            if (mode == Mode_EachLine)
            {
                if (buf == '\n' || buf == '*')
                {

                    cmd1 = Cmd_append_one_make(cmd1, &vc.buff[begin]);
                    // Cmd_print(cmd1);
                    Cmd_exec_in_child_process(cmd1);
                    cmd1 = cmd;
                }
                else if (vc_i - begin > 1)
                {
                    cmd1 = Cmd_append_one_make(cmd1, &vc.buff[begin]);
                }
            }
            // vc_i = 0;
            else if (mode == Mode_OneShot)
            {
                cmd = Cmd_append_one_make(cmd, &vc.buff[begin]);
                // Cmd_print(cmd);
            }
            begin = vc_i;
        }
        else //common word
        {
            vc.buff[vc_i] = buf;
            vc_i++;
        }

        // for (i = 0; i < n; i++)
        // {
        //     c++;
        //     if (vc.buff[i] == '\n')
        //     {
        //         if (mode == Mode_EachLine)
        //         {
        //             Cmd_exec_in_child_process(cmd);
        //         }
        //         l++;
        //     }
        //     if (strchr(" \r\t\n\v", vc.buff[i]))
        //         inword = 0;
        //     else
        //     {
        //         printf("%c", vc.buff[i]);
        //         if (!inword)
        //         {
        //             w++;
        //             inword = 1;
        //         }
        //     }
        // }
    }
    // printf("out of while \n");
    if (n < 0)
    {
        printf("wc: read error\n");
        exit(1);
    }
    if (mode == Mode_OneShot)
    {
        Cmd_exec_in_child_process(cmd);
    }
    else if (mode == Mode_EachLine)
    {
    }
    // printf("%d %d %d\n", l, w, c);
    ////////////////////////////////////////////////////////
    Vec_char_free(vc);
}

int main(int argc, char *argv[])
{
    Cmd cmd = Cmd_make(argv + 1);
    // Cmd cmd = Cmd_make(argv, argc - 1);
    // char valid = Cmd_exec_in_child_process(cmd);

    // printf("%s\n", valid ? "valid" : "invalid");
    // Cmd_print(cmd);
    xargs(cmd, Mode_OneShot, 0);

    // printf("no input\n");
    // }
    // else
    // {
    //     // printf("%s\n", argv[1]);
    // }

    // recur(40);
    exit(0);
}
