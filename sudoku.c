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
static u8 calced_valid_plays[512];
static u8 calced_valid_play[512];

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

static u8 get_valid_plays(u16 val) {
    return calced_valid_plays[val];
}

static u8 pick_play(u16 val) {
    return calced_valid_play[val];
}

static void init() {
    for (u16 v = 0; v < 512; v++) {
        u8 bits = 0;
        for (u8 y = 0; y < 9; ++y) {
            if ((v & (1 << y)) == 0) {
                bits++;
                calced_valid_play[v] = y;
            }
        }
        calced_valid_plays[v] = bits;
    }
}

static bool search(u8 v[9][9]) {
    u8 x;
    u8 y;
    u8 best_valid_plays = 10;
    u8 best_pos_x;
    u8 best_pos_y;

    for (y = 0; y < 9; ++y) {
        for (x = 0; x < 9; ++x) {
            if (v[y][x] != 0) {
                continue;
            }

            u16 masked = rows[y] | columns[x] | squares[y / 3][x / 3];
            u8 valid_plays = get_valid_plays(masked);
            if (valid_plays == 0) {
                return FALSE;
            }

            if (valid_plays == 1) {
                u8 last_play = pick_play(masked);
                add_value(v, x, y, last_play);
                if (search(v)) {
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
    for (u8 play = 0; play < 9; ++play) {
        u16 mask = (1 << play);

        if ((rows[y] & mask) == 0 && (columns[x] & mask) == 0 && (squares[y / 3][x / 3] & mask) == 0) {
            add_value(v, x, y, play);
            if (search(v)) {
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

    for (y = 0; y < 9; ++y) {
        for (x = 0; x < 9; ++x) {
            if (v[y][x] != 0) {
                add_value(v, x, y, v[y][x] - 1);
            }
        }
    }

    return search(v);
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

    init();
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
