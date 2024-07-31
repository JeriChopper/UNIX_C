#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void process_file(FILE *file) {
    uint32_t count;
    int character;
    size_t result;

    while ((result = fread(&count, sizeof(uint32_t), 1, file)) == 1) {
        character = fgetc(file);
        if (character == EOF) {
            fprintf(stderr, "Error: Unexpected end of file.\n");
            exit(1);
        }
        for (uint32_t i = 0; i < count; i++) {
            putchar(character);
        }
    }

    if (result != 0) {
        fprintf(stderr, "Error: Incomplete data read.\n");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("wunzip: file1 [file2 ...]\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        FILE *file = fopen(argv[i], "rb");
        if (file == NULL) {
            perror(argv[i]);
            continue;
        }
        process_file(file);
        fclose(file);
    }

    return 0;
}
