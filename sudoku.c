#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint16_t u16;
typedef uint8_t u8;
typedef char bool;
#define TRUE 1
#define FALSE 0

static void print_board(const u8 v[9][9]) {
    u8 x;
    u8 y;

    for (y = 0; y < 9; ++y) {
        if (y > 0) {
            printf("\n");
        }
        for (x = 0; x < 9; ++x) {
            if (v[y][x] == 0) {
                printf(" -");
            } else {
                printf(" %u", v[y][x]);
            }
        }
    }
    printf("\n");
}

static void string_to_board(u8 v[9][9], const char * buffer) {
    u8 i;
    for (i = 0; i < 81; ++i) {
        u8 x = i % 9;
        u8 y = i / 9;
        if (buffer[i] == '.' || buffer[i] == ' ' || buffer[i] == '0' || buffer[i] == '_') {
            v[y][x] = 0;
        } else {
            v[y][x] = buffer[i] - '0';
        }
    }
}

static u16 rows[9];
static u16 columns[9];
static u16 squares[3][3];

static void add_value(u8 v[9][9], u8 x, u8 y, u8 val) {
    v[y][x] = val + 1;
    u16 mask = (1 << val);
    rows[y] |= mask;
    columns[x] |= mask;
    squares[y / 3][x / 3] |= mask;
}

static void rem_value(u8 v[9][9], u8 x, u8 y, u8 val) {
    v[y][x] = 0;
    u16 mask = ~(1 << val);
    rows[y] &= mask;
    columns[x] &= mask;
    squares[y / 3][x / 3] &= mask;
}

static bool search_upper(u8 v[9][9]) {
    u8 x;
    u8 y;
    u8 best_valid_plays = 10;
    u8 best_pos_x;
    u8 best_pos_y;
    u8 last_play;
    u8 valid_plays;
    u8 play;

    for (y = 0; y < 9; ++y) {
        for (x = 0; x < 9; ++x) {
            if (v[y][x] != 0) {
                continue;
            }

            last_play = 0;
            valid_plays = 0;
            for (play = 0; play < 9; ++play) {
                u16 mask = (1 << play);

                if ((rows[y] & mask) == 0 && (columns[x] & mask) == 0 && (squares[y / 3][x / 3] & mask) == 0) {
                    valid_plays++;
                    last_play = play;
                }
            }

            if (valid_plays == 0) {
                return FALSE;
            }
            if (valid_plays == 1) {
                add_value(v, x, y, last_play);
                if (search_upper(v)) {
                    return TRUE;
                }
                rem_value(v, x, y, last_play);
                return FALSE;
            }
            if (best_valid_plays > valid_plays) {
                best_valid_plays = valid_plays;
                best_pos_x = x;
                best_pos_y = y;
            }
        }
    }

    if (best_valid_plays == 10) {
        return TRUE;
    }

    x = best_pos_x;
    y = best_pos_y;
    for (play = 0; play < 9; ++play) {
        u16 mask = (1 << play);

        if ((rows[y] & mask) == 0 && (columns[x] & mask) == 0 && (squares[y / 3][x / 3] & mask) == 0) {
            add_value(v, x, y, play);
            if (search_upper(v)) {
                return TRUE;
            }
            rem_value(v, x, y, play);
        }
    }
    return FALSE;
}

static bool search_lower(u8 v[9][9]) {
    u8 x;
    u8 y;
    u8 best_valid_plays = 10;
    u8 best_pos_x;
    u8 best_pos_y;
    u8 last_play;
    u8 valid_plays;
    char play;

    for (y = 0; y < 9; ++y) {
        for (x = 0; x < 9; ++x) {
            if (v[y][x] != 0) {
                continue;
            }

            last_play = 0;
            valid_plays = 0;
            for (play = 8; play >= 0; --play) {
                u16 mask = (1 << play);

                if ((rows[y] & mask) == 0 && (columns[x] & mask) == 0 && (squares[y / 3][x / 3] & mask) == 0) {
                    valid_plays++;
                    last_play = play;
                }
            }

            if (valid_plays == 0) {
                return FALSE;
            }
            if (valid_plays == 1) {
                add_value(v, x, y, last_play);
                if (search_lower(v)) {
                    return TRUE;
                }
                rem_value(v, x, y, last_play);
                return FALSE;
            }
            if (best_valid_plays > valid_plays) {
                best_valid_plays = valid_plays;
                best_pos_x = x;
                best_pos_y = y;
            }
        }
    }

    if (best_valid_plays == 10) {
        return TRUE;
    }

    x = best_pos_x;
    y = best_pos_y;
    for (play = 8; play >= 0; --play) {
        u16 mask = (1 << play);

        if ((rows[y] & mask) == 0 && (columns[x] & mask) == 0 && (squares[y / 3][x / 3] & mask) == 0) {
            add_value(v, x, y, play);
            if (search_lower(v)) {
                return TRUE;
            }
            rem_value(v, x, y, play);
        }
    }
    return FALSE;
}

static bool setup(u8 v[9][9]) {
    u8 x;
    u8 y;

    for (y = 0; y < 9; ++y) {
        rows[y] = 0;
        columns[y] = 0;
    }
    for (y = 0; y < 3; ++y) {
        for (x = 0; x < 3; ++x) {
            squares[y][x] = 0;
        }
    }

    u8 upper = 0;
    u8 lower = 0;

    for (y = 0; y < 9; ++y) {
        for (x = 0; x < 9; ++x) {
            if (v[y][x] != 0) {
                if (v[y][x] < 5) {
                    lower++;
                } else {
                    upper++;
                }
                add_value(v, x, y, v[y][x] - 1);
            }
        }
    }

    if (upper < lower) {
        return search_lower(v);
    } else {
        return search_upper(v);
    }
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

        u8 v[9][9];
        string_to_board(v, buffer);

        bool board_solved = setup(v);
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
