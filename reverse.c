/// Käyttöjärjestelmät ja systeemiohjelmointi
/// reverse.c 
/// Jericho Koskinen
/// https://stackoverflow.com/questions/66101547/get-line-from-text-and-put-into-a-linked-list


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> // ssize_t needs this included
#include <sys/stat.h>


// Define a structure for the linked list node
typedef struct Node {
    char *line;
    struct Node *next;
} Node;

// Function to create a new node
Node* createNode(char *line) {
    Node *newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
    newNode->line = line;
    newNode->next = NULL;
    return newNode;
}

// Function to read lines from a file, reversing the order and storing them in a linked list
Node* readFileToList(FILE *file) {
    Node *head = NULL;
    Node *tail = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1) {
        char *lineCopy = strdup(line);
        if (lineCopy == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            exit(1);
        }
        Node *newNode = createNode(lineCopy);
        newNode->next = head;  // Insert at the beginning to reverse the order
        head = newNode;       
    }

    free(line);
    return head;
}

// Function to write the linked list contents to a file (Content is already reversed in linked list)
void writeListToFile(Node *head, FILE *file) {
    Node *current = head;
    while (current != NULL) {
        fprintf(file, "%s", current->line);
        current = current->next;
    }
}

// Function to free the linked list
void freeList(Node *head) {
    Node *current = head;
    while (current != NULL) {
        Node *next = current->next;
        free(current->line);
        free(current);
        current = next;
    }
}



int main(int argc, char *argv[]) {
    // Default input and output files
    FILE *input = stdin;
    FILE *output = stdout;

  // Error handling for command-line arguments
    if (argc > 3) {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }

    if (argc >= 2) {
        // Open input file if specified
        input = fopen(argv[1], "r");
        if (input == NULL) {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[1]);
            exit(1);
        }
       
    }

    if (argc == 3) {
        // Open output file if specified
        if (strcmp(argv[1], argv[2]) == 0) {
            fprintf(stderr, "reverse: input and output file must differ\n");
            exit(1);
        }

        output = fopen(argv[2], "w");
        if (output == NULL) {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[2]);
            exit(1);
        }
    }

    // Read lines from input file into a linked list
    Node *listHead = readFileToList(input);

    // Close the input file if it was opened
    if (input != stdin) fclose(input);

    // Write lines from the linked list to the output file
    writeListToFile(listHead, output);

    // Close the output file if it was opened
    if (output != stdout) fclose(output);

    // Free the linked list
    freeList(listHead);

    return 0;
}