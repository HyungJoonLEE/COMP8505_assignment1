#include "client_helper.h"
#include "common.h"

int main(int argc, char *argv[]) {
    struct options_client opts;

    check_root_user(argc, argv);
    options_init_client(&opts);
    parse_arguments_client(argc, argv, &opts);
    confirm_client_info(&opts);
    options_process_client(&opts);
    return EXIT_SUCCESS;
}
