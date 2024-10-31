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

static u16 start_rows[9];
static u16 start_columns[9];
static u16 start_squares[3][3];
static u16 rows[9];
static u16 columns[9];
static u16 squares[3][3];
static u8 calced_valid_plays_count[512];
static u8 calced_valid_plays[512][8];
static u8 squares_left_count;
static u8 squares_left_x[81];
static u8 squares_left_y[81];

static void add_value(u8 v[9][9], u8 x, u8 y, u8 val) {
    v[y][x] = val + 1;
    u16 mask = (1 << val);
    rows[y] |= mask;
    columns[x] |= mask;
    squares[y / 3][x / 3] |= mask;
}

static void init_global() {
    u8 x;
    u8 y;

    for (y = 0; y < 9; ++y) {
        start_rows[y] = 0;
        start_columns[y] = 0;
    }
    for (y = 0; y < 3; ++y) {
        for (x = 0; x < 3; ++x) {
            start_squares[y][x] = 0;
        }
    }

    for (u16 v = 0; v < 512; v++) {
        calced_valid_plays_count[v] = 0;
        for (u8 y = 0; y < 9; ++y) {
            if ((v & (1 << y)) == 0) {
                if (calced_valid_plays_count[v] < 8) {
                    calced_valid_plays[v][calced_valid_plays_count[v]] = y;
                }
                calced_valid_plays_count[v]++;
            }
        }
    }
}

static bool search(u8 v[9][9]) {
    u8 x;
    u8 y;
    u16 best_masked;
    u8 best_valid_plays = 10;
    u8 best_pos_x;
    u8 best_pos_y;
    u8 square_i;

    if (squares_left_count == 0) {
        return TRUE;
    }

    for (u8 i = 0; i < squares_left_count; ++i) {
        u8 x = squares_left_x[i];
        u8 y = squares_left_y[i];

        u16 masked = rows[y] | columns[x] | squares[y / 3][x / 3];
        u8 valid_plays = calced_valid_plays_count[masked];
        if (valid_plays == 0) {
            return FALSE;
        }
        if (valid_plays == 1) {
            u8 last_play = calced_valid_plays[masked][0];
            u16 tmp1 = rows[y];
            u16 tmp2 = columns[x];
            u16 tmp3 = squares[y / 3][x / 3];
            squares_left_count--;
            squares_left_x[i] = squares_left_x[squares_left_count];
            squares_left_y[i] = squares_left_y[squares_left_count];
            add_value(v, x, y, last_play);
            if (search(v)) {
                return TRUE;
            }
            v[y][x] = 0;
            rows[y] = tmp1;
            columns[x] = tmp2;
            squares[y / 3][x / 3] = tmp3;
            squares_left_x[squares_left_count] = squares_left_x[i];
            squares_left_y[squares_left_count] = squares_left_y[i];
            squares_left_x[i] = x;
            squares_left_y[i] = y;
            squares_left_count++;
            return FALSE;
        }
        if (best_valid_plays > valid_plays) {
            best_masked = masked;
            best_valid_plays = valid_plays;
            best_pos_x = x;
            best_pos_y = y;
            square_i = i;
        }
    }

    x = best_pos_x;
    y = best_pos_y;

    u16 tmp1 = rows[y];
    u16 tmp2 = columns[x];
    u16 tmp3 = squares[y / 3][x / 3];

    if (best_valid_plays == 9) {
        for (u8 play = 0; play < 9; ++play) {
            add_value(v, x, y, play);
            squares_left_count--;
            squares_left_x[square_i] = squares_left_x[squares_left_count];
            squares_left_y[square_i] = squares_left_y[squares_left_count];
            if (search(v)) {
                return TRUE;
            }
            rows[y] = tmp1;
            columns[x] = tmp2;
            squares[y / 3][x / 3] = tmp3;
            squares_left_x[squares_left_count] = squares_left_x[square_i];
            squares_left_y[squares_left_count] = squares_left_y[square_i];
            squares_left_x[square_i] = x;
            squares_left_y[square_i] = y;
            squares_left_count++;
        }
        v[y][x] = 0;
        return FALSE;
    }

    for (char i = best_valid_plays - 1; i >= 0; --i) {
        u8 play = calced_valid_plays[best_masked][(u8)i];
        squares_left_count--;
        squares_left_x[square_i] = squares_left_x[squares_left_count];
        squares_left_y[square_i] = squares_left_y[squares_left_count];
        add_value(v, x, y, play);
        if (search(v)) {
            return TRUE;
        }
        rows[y] = tmp1;
        columns[x] = tmp2;
        squares[y / 3][x / 3] = tmp3;
        squares_left_x[squares_left_count] = squares_left_x[square_i];
        squares_left_y[squares_left_count] = squares_left_y[square_i];
        squares_left_x[square_i] = x;
        squares_left_y[square_i] = y;
        squares_left_count++;
    }
    v[y][x] = 0;
    return FALSE;
}

static bool init_board(u8 v[9][9]) {
    memcpy(rows, start_rows, sizeof(rows));
    memcpy(columns, start_columns, sizeof(columns));
    memcpy(squares, start_squares, sizeof(squares));

    squares_left_count = 0;

    for (u8 y = 0; y < 9; ++y) {
        for (u8 x = 0; x < 9; ++x) {
            if (v[y][x] == 0) {
                squares_left_x[squares_left_count] = x;
                squares_left_y[squares_left_count] = y;
                squares_left_count++;
            } else {
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

    init_global();
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

        bool board_solved = init_board(v);
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
