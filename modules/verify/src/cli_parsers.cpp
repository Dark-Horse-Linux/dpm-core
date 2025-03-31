#include "../include/cli_parsers.hpp"

Command parse_command(const char* cmd_str) {
    if (cmd_str == nullptr || strlen(cmd_str) == 0) {
        return CMD_HELP;
    }

    if (strcmp(cmd_str, "help") == 0) {
        return CMD_HELP;
    }
    else if (strcmp(cmd_str, "checksum") == 0) {
        return CMD_CHECKSUM;
    }
    else if (strcmp(cmd_str, "signature") == 0) {
        return CMD_SIGNATURE;
    }
    else if (strcmp(cmd_str, "check") == 0) {
        return CMD_CHECK;
    }

    return CMD_UNKNOWN;
}
