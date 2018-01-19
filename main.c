#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "unlocknvram.h"
#include "nonce.h"
#include "kutils.h"
#include "debug.h"

void printusage(void) {
    printf("-h this message\n");
    printf("-q stay quiet\n");
    printf("-v be more verbose\n");
    printf("-V even more verbose\n");
    printf("-U skip unlocking nvram\n");
    printf("-g print generator (when combined with s/d prints twice)\n");
    printf("-s [val] set generator (WARNING: NO VALIDATION PERFORMED)\n");
    printf("-d delete generator (conflicts with s)\n");
}

int gethelper(int setdel) {
    char *gen = getgen();
    if (gen != NULL) {
        printf("%s\n", gen);
        free(gen);
    } else {
        printf("nonce_not_set\n");
        if (!setdel) {
            return 1;
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int get, set, del;
    char *gentoset = NULL;
    get = set = del = 0;

    int nounlock = 0;

    char c;
    while ((c = getopt(argc, argv, "hqvVUrgds:")) != -1) {
        switch (c) {
            case 'h':
                printusage();
                return EXIT_SUCCESS;

            case 'q':
                loglevel = lvlNONE;
                break;
            case 'v':
                loglevel = lvlINFO;
                break;
            case 'V':
                loglevel = lvlDEBUG;
                break;

            case 'U':
                nounlock = 1;
                break;

            case 'g':
                get = 1;
                break;

            case 'd':
                del = 1;
                break;

            case 's':
                set = 1;
                gentoset = optarg;
                break;

            case '?':
                ERROR("Unknown option `-%c'", optopt);
                break;

            default:
                abort();
        }
    }

    if (!(get || set || del)) {
        ERROR("please specify g or s or d flag");
        printusage();
        return EXIT_FAILURE;
    }

    if (set && del) {
        ERROR("cant set and delete nonce at once");
        return EXIT_FAILURE;
    }

    if (init_tfpzero()) {
        ERROR("failed to init tfpzero");
        return EXIT_FAILURE;
    }

    int retval = EXIT_SUCCESS;

    if (!nounlock) {
        if (unlocknvram()) {
            ERROR("failed to unlock nvram, but trying anyway");
        }
    }

    if (get) {
        retval = gethelper(set || del);
        DEBUG("gethelper: %d", retval);
    }

    if (del) {
        retval = delgen();
        DEBUG("delgen: %d", retval);
    }

    if (set) {
        retval = setgen(gentoset);
        DEBUG("setgen: %d", retval);
    }

    if (get && (set || del)) {
        retval = gethelper(set || del);
        DEBUG("gethelper: %d", retval);
    }

    if (!nounlock) {
       if (locknvram()) {
            ERROR("failed to unlock nvram, cant do much about it");
        }
    }
    
    return retval;
}
