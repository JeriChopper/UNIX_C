/// Käyttöjärjestelmät ja systeemiohjelmointi
/// my-cat.c 
/// Jericho Koskinen
/// https://www.geeksforgeeks.org/c-program-print-contents-file/


#include <stdio.h>
#include <stdlib.h>

void print_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("wcat: cannot open file\n");
        exit(1);
    }

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file)) {
        printf("%s", buffer);
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        print_file(argv[i]);
    }

    return 0;
}
