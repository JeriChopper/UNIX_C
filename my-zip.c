#include <stdio.h>
#include <stdlib.h>

void process_file(FILE *file, int *prev_char, int *count) {
    int current_char;

    while ((current_char = fgetc(file)) != EOF) {
        if (current_char == *prev_char) {
            (*count)++;
        } else {
            if (*prev_char != -1) {
                // Output previous character run
                fwrite(count, sizeof(int), 1, stdout);
                fputc(*prev_char, stdout);
            }
            // Start new run
            *prev_char = current_char;
            *count = 1;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }

    int prev_char = -1;
    int count = 0;

    for (int i = 1; i < argc; i++) {
        FILE *file = fopen(argv[i], "r");
        if (file == NULL) {
            printf("wzip: cannot open file \n");
            return 1;
        }
        process_file(file, &prev_char, &count);
        fclose(file);
    }

    // Output last character run
    if (prev_char != -1) {
        fwrite(&count, sizeof(int), 1, stdout);
        fputc(prev_char, stdout);
    }

    return 0;
}