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

static u8 rows[9][9];
static u8 columns[9][9];
static u8 squares[3][3][9];

static void add_value(u8 v[81], u8 pos, u8 play) {
    v[pos] = play + 1;

    u8 x = pos % 9;
    u8 y = pos / 9;

    rows[y][play]++;
    columns[x][play]++;
    squares[y / 3][x / 3][play]++;
}

static void rem_value(u8 v[81], u8 pos, u8 play) {
    v[pos] = 0;

    u8 x = pos % 9;
    u8 y = pos / 9;

    rows[y][play]--;
    columns[x][play]--;
    squares[y / 3][x / 3][play]--;
}

static bool solve1(u8 v[81]) {
    u8 i;
    u8 x;
    u8 y;
    u8 best_valid_plays = 10;
    u8 best_pos;
    u8 last_play;
    u8 valid_plays;
    u8 play;
    bool finished = TRUE;

    for (i = 0; i < 81; ++i) {
        if (v[i] != 0) {
            continue;
        }

        finished = FALSE;
        x = i % 9;
        y = i / 9;

        last_play = 0;
        valid_plays = 0;
        for (play = 0; play < 9; ++play) {
            if (rows[y][play] == 0 && columns[x][play] == 0 && squares[y / 3][x / 3][play] == 0) {
                valid_plays++;
                last_play = play;
            }
        }

        if (valid_plays == 0) {
            return FALSE;
        }
        if (valid_plays == 1) {
            add_value(v, i, last_play);
            if (solve1(v)) {
                return TRUE;
            }
            rem_value(v, i, last_play);
            return FALSE;
        }
        if (best_valid_plays > valid_plays) {
            best_valid_plays = valid_plays;
            best_pos = i;
        }
    }

    if (finished) {
        return TRUE;
    }

    x = best_pos % 9;
    y = best_pos / 9;
    for (play = 0; play < 9; ++play) {
        if (rows[y][play] == 0 && columns[x][play] == 0 && squares[y / 3][x / 3][play] == 0) {
            add_value(v, best_pos, play);
            if (solve1(v)) {
                return TRUE;
            }
            rem_value(v, best_pos, play);
        }
    }
    return FALSE;
}

static bool solve(u8 v[81]) {
    u8 i;
    u8 j;
    u8 k;

    for (i = 0; i < 9; ++i) {
        for (j = 0; j < 9; ++j) {
            rows[i][j] = 0;
            columns[i][j] = 0;
        }
    }
    for (i = 0; i < 3; ++i) {
        for (j = 0; j < 3; ++j) {
            for (k = 0; k < 9; ++k) {
                squares[i][j][k] = 0;
            }
        }
    }

    for (i = 0; i < 81; ++i) {
        if (v[i] != 0) {
            add_value(v, i, v[i] - 1);
        }
    }

    return solve1(v);
}

int main(int argc, char * argv[]) {
    if (argc != 2 && argc != 3) {
        printf("\nUSAGE\n\t%s puzzle_file [--verbose]\n\nARGUMENTS\n\tpuzzle_file - text file with sudoku puzzles one by line, with missing positions as ASCII zeroes or dots. All other characters will be ignored.\n\t--verbose - whether to print the result after each solved puzzle\n\nEXAMPLE\n\t%s puzzles.txt\n\n", argv[0], argv[0]);
        return 1;
    }
    unsigned int number = 0;
    unsigned int solved = 0;
    unsigned int impossible = 0;
    bool verbose = FALSE;

    const char * filename = argv[1];

    if (argc > 2) {
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

        bool board_solved = solve(v);
        if (board_solved) {
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
