#include "server_helper.h"

int main(int argc, char *argv[]) {
    struct options_server opts;

    check_root_user(argc, argv);
    options_init_server(&opts);
    parse_arguments_server(argc, argv, &opts);
    confirm_server_info(&opts);
    options_process_server(&opts);
//    cleanup_server(&opts);
    return EXIT_SUCCESS;
}
