
#include <stdio.h>
#include <string.h>
#include <lo/lo_lowlevel.h>
#include "../src/types_internal.h"
#include "../src/mapper_internal.h"

int verbose = 1;

#define eprintf(format, ...) do {               \
    if (verbose)                                \
        fprintf(stdout, format, ##__VA_ARGS__); \
} while(0)

int main(int argc, char **argv)
{
    lo_arg *args[20], **a;
    mapper_message_t msg;
    int port=1234, src_length=4;
    float r[4] = {1.0, 2.0, -15.0, 25.0};
    int i, j, result = 0;
    const char *types;
    int length;

    // process flags for -v verbose, -h help
    for (i = 1; i < argc; i++) {
        if (argv[i] && argv[i][0] == '-') {
            int len = strlen(argv[i]);
            for (j = 1; j < len; j++) {
                switch (argv[i][j]) {
                    case 'h':
                        eprintf("testdb.c: possible arguments "
                                "-q quiet (suppress output), "
                                "-h help\n");
                        return 1;
                        break;
                    case 'q':
                        verbose = 0;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    eprintf("1: expected success\n");

    args[0]  = (lo_arg*)"@host";
    args[1]  = (lo_arg*)"127.0.0.1";
    args[2]  = (lo_arg*)"@srcMin";
    args[3]  = (lo_arg*)&r[0];
    args[4]  = (lo_arg*)&r[1];
    args[5]  = (lo_arg*)&r[2];
    args[6]  = (lo_arg*)&r[3];
    args[7]  = (lo_arg*)"@port";
    args[8]  = (lo_arg*)&port;
    args[9]  = (lo_arg*)"@srcType";
    args[10] = (lo_arg*)"f";
    args[11] = (lo_arg*)"@srcLength";
    args[12] = (lo_arg*)&src_length;

    int rc = mapper_msg_parse_params(&msg, "/test", "sssffffsiscsi", 13, args);
    if (!rc) {
        eprintf("1: Error parsing.\n");
        result = 1;
        goto done;
    }

    a = mapper_msg_get_param(&msg, AT_HOST, &types, &length);
    if (!a) {
        eprintf("1: Could not get @host param.\n");
        result = 1;
        goto done;
    }
    if (!types || types[0] != 's') {
        eprintf("1: Type error retrieving @host param.");
        result = 1;
        goto done;
    }
    if (length != 1) {
        eprintf("1: Length error retrieving @host param.");
        result = 1;
        goto done;
    }
    if (strcmp(&(*a)->s, "127.0.0.1")!=0)
        result |= 1;
    eprintf("1: @host = \"%s\" %s\n", &(*a)->s, result ? "WRONG" : "(correct)");
    if (result)
        goto done;

    a = mapper_msg_get_param(&msg, AT_PORT, &types, &length);
    if (!a) {
        eprintf("1: Could not get @port param.\n");
        result = 1;
        goto done;
    }
    if (!types || types[0] != 'i') {
        eprintf("1: Type error retrieving @port param.");
        result = 1;
        goto done;
    }
    if (length != 1) {
        eprintf("1: Length error retrieving @port param.");
        result = 1;
        goto done;
    }
    if ((*a)->i!=1234)
        result |= 1;
    eprintf("1: @port = %d %s\n", (*a)->i, result ? "WRONG" : "(correct)");
    if (result)
        goto done;

    a = mapper_msg_get_param(&msg, AT_SRC_MIN, &types, &length);
    if (!a) {
        eprintf("1: Could not get @src_min param.\n");
        result = 1;
        goto done;
    }
    if (!types || types[0] != 'f') {
        eprintf("1: Type error retrieving @src_min param.");
        result = 1;
        goto done;
    }
    if (length != 4) {
        eprintf("1: Length error retrieving @src_min param.");
        result = 1;
        goto done;
    }
    for (i=0; i<length; i++) {
        if (a[i]->f!=r[i])
            result = 1;
        eprintf("1: @src_min[%d] = %f %s\n", i, a[i]->f,
                result ? "WRONG" : "(correct)");
        if (result)
            goto done;
    }

    /*****/

    eprintf("2: deliberately malformed message\n");

    args[0] = (lo_arg*)"@port";
    args[1] = (lo_arg*)&port;
    args[2] = (lo_arg*)"@host";

    rc = mapper_msg_parse_params(&msg, "/test", "sis", 3, args);
    if (!rc) {
        eprintf("2: Error parsing.\n");
        result = 1;
        goto done;
    }

    a = mapper_msg_get_param(&msg, AT_PORT, &types, &length);
    if (!a) {
        eprintf("2: Could not get @port param.\n");
        result = 1;
        goto done;
    }
    if (!types || types[0] != 'i') {
        eprintf("2: Type error retrieving @port param.");
        result = 1;
        goto done;
    }
    if (length != 1) {
        eprintf("2: Length error retrieving @port param.");
        result = 1;
        goto done;
    }

    a = mapper_msg_get_param(&msg, AT_HOST, &types, &length);
    if (a) {
        eprintf("2: Error, should not have been able to retrieve @host param.\n");
        result = 1;
        goto done;
    }

    /*****/
done:
    if (!verbose)
        printf("..................................................");
    printf("Test %s.\n", result ? "FAILED" : "PASSED");
    return result;
}
