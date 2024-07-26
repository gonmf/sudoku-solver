#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;
typedef char bool;
#define TRUE 1
#define FALSE 0

static void print_board(const u8 v[81]) {
    u8 i;
    for (i = 0; i < 81; ++i) {
        if (v[i] == 0) {
            printf(" -");
        } else {
            printf(" %u", v[i]);
        }

        if (((i + 1) % 9) == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

static void string_to_board(u8 v[81], const char * buffer) {
    u8 i;
    for (i = 0; i < 81; ++i) {
        if (buffer[i] == '.' || buffer[i] == ' ' || buffer[i] == '0' || buffer[i] == '_') {
            v[i] = 0;
        } else {
            v[i] = buffer[i] - '0';
        }
    }
}

static bool rows[9][9];
static bool columns[9][9];
static bool squares[3][3][9];

static bool solve1(u8 v[81]) {
    bool solved = TRUE;
    u8 x;
    u8 y;
    u8 i;
    u8 p;

    for (y = 0; y < 9; ++y) {
        for (x = 0; x < 9; ++x) {
            i = y * 9 + x;
            if (v[i] > 0) {
                continue;
            }
            solved = FALSE;

            for (p = 0; p < 9; ++p) {
                if (rows[y][p] && columns[x][p] && squares[y / 3][x / 3][p]) {
                    v[i] = p + 1;
                    rows[y][p] = FALSE;
                    columns[x][p] = FALSE;
                    squares[y / 3][x / 3][p] = FALSE;
                    bool result = solve1(v);
                    if (result) {
                        return TRUE;
                    }
                    v[i] = 0;
                    rows[y][p] = TRUE;
                    columns[x][p] = TRUE;
                    squares[y / 3][x / 3][p] = TRUE;
                }
            }
            return FALSE;
        }
    }

    return solved;
}

static bool solve(u8 v[81]) {
    u8 x;
    u8 y;
    u8 i;
    for (y = 0; y < 9; ++y) {
        for (x = 0; x < 9; ++x) {
            rows[y][x] = TRUE;
            columns[y][x] = TRUE;
        }
    }
    for (y = 0; y < 3; ++y) {
        for (x = 0; x < 3; ++x) {
            for (i = 0; i < 9; ++i) {
                squares[y][x][i] = TRUE;
            }
        }
    }

    for (y = 0; y < 9; ++y) {
        for (x = 0; x < 9; ++x) {
            i = y * 9 + x;
            if (v[i] == 0) {
                continue;
            }

            rows[y][v[i] - 1] = FALSE;
            columns[x][v[i] - 1] = FALSE;

            squares[y / 3][x / 3][v[i] - 1] = FALSE;
        }
    }

    return solve1(v);
}

int main(int argc, const char * argv[]) {
    if (argc != 2 && argc != 3) {
        printf("\nUSAGE\n\t%s puzzle_file [--verbose]\n\nARGUMENTS\n\tpuzzle_file - text file with sudoku puzzles one by line, with missing positions as ASCII zeroes or dots. All other characters will be ignored.\n\t--verbose - whether to print the result after each solved puzzle\n\nEXAMPLE\n\t%s puzzles.txt\n\n", argv[0], argv[0]);
        return 1;
    }
    unsigned int number = 0;
    unsigned int solved = 0;
    unsigned int impossible = 0;
    bool verbose = FALSE;

    const char * filename = argv[1];

    if (argc == 3) {
        if (strcmp(filename, "--verbose") == 0) {
            filename = argv[2];
            verbose = TRUE;
        } else if (strcmp(argv[2], "--verbose") == 0) {
            verbose = TRUE;
        }
    }

    FILE * fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file for reading\n");
        return EXIT_FAILURE;
    }

    char buffer[83];

    while (fgets(buffer, 82, fp) != NULL) {
        buffer[81] = 0;
        if (buffer[0] == '#' || strlen(buffer) < 81)
            continue;

        ++number;
        if (verbose) {
            printf("Starting puzzle #%d:\n%s\n\n", number, buffer);
        }

        u8 v[81];
        string_to_board(v, buffer);

        bool success = solve(v);
        if (success) {
            solved++;
            if (verbose) {
                printf("Solution:\n");
                print_board(v);
            }
        } else {
            impossible++;
            if (verbose) {
                fprintf(stderr, "Impossible puzzle\n");
            }
        }
    }
    printf("Solved=%u\nImpossible=%u\nTotal=%u\n", solved, impossible, number);
    return EXIT_SUCCESS;
}
