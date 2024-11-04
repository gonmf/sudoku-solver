#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint16_t u16;
typedef uint8_t u8;
typedef char bool;
#define TRUE 1
#define FALSE 0


typedef struct __Pos {
   u8 x;
   u8 y;
} Pos;

typedef struct __Bitmaps {
    u16 rows[9];
    u16 columns[9];
    u16 squares[3][3];
} Bitmaps;

static Bitmaps start_bitmaps;
static Bitmaps bitmaps;
static u8 calced_valid_plays_count[512];
static u8 calced_valid_plays[512][8];
static u8 squares_left_count;
static Pos squares_left[64];
static u8 v[9][9];

static void print_board() {
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
    printf("\n\n");
}

static void add_value(u8 x, u8 y, u8 val) {
    u16 mask = (1 << val);
    bitmaps.rows[y] |= mask;
    bitmaps.columns[x] |= mask;
    bitmaps.squares[y / 3][x / 3] |= mask;
}

static void init_global() {
    u8 x;
    u8 y;

    for (y = 0; y < 9; ++y) {
        start_bitmaps.rows[y] = 0;
        start_bitmaps.columns[y] = 0;
    }
    for (y = 0; y < 3; ++y) {
        for (x = 0; x < 3; ++x) {
            start_bitmaps.squares[y][x] = 0;
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

static bool search() {
    u8 x;
    u8 y;
    u16 best_masked;
    u8 best_valid_plays = 9;
    u8 best_pos_x;
    u8 best_pos_y;
    u8 square_i;

    if (squares_left_count == 0) {
        return TRUE;
    }

    for (u8 i = 0; i < squares_left_count; ++i) {
        u8 x = squares_left[i].x;
        u8 y = squares_left[i].y;

        u16 masked = bitmaps.rows[y] | bitmaps.columns[x] | bitmaps.squares[y / 3][x / 3];
        u8 valid_plays = calced_valid_plays_count[masked];
        if (valid_plays == 0) {
            return FALSE;
        }
        if (valid_plays == 1) {
            u8 play = calced_valid_plays[masked][0];
            u16 tmp1 = bitmaps.rows[y];
            u16 tmp2 = bitmaps.columns[x];
            u16 tmp3 = bitmaps.squares[y / 3][x / 3];
            squares_left_count--;
            squares_left[i].x = squares_left[squares_left_count].x;
            squares_left[i].y = squares_left[squares_left_count].y;
            add_value(x, y, play);
            if (search()) {
                v[y][x] = play + 1;
                return TRUE;
            }
            bitmaps.rows[y] = tmp1;
            bitmaps.columns[x] = tmp2;
            bitmaps.squares[y / 3][x / 3] = tmp3;
            squares_left[squares_left_count].x = squares_left[i].x;
            squares_left[squares_left_count].y = squares_left[i].y;
            squares_left[i].x = x;
            squares_left[i].y = y;
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

    u16 tmp1 = bitmaps.rows[y];
    u16 tmp2 = bitmaps.columns[x];
    u16 tmp3 = bitmaps.squares[y / 3][x / 3];

    squares_left_count--;
    for (char i = best_valid_plays - 1; i >= 0; --i) {
        u8 play = calced_valid_plays[best_masked][(u8)i];
        squares_left[square_i].x = squares_left[squares_left_count].x;
        squares_left[square_i].y = squares_left[squares_left_count].y;
        add_value(x, y, play);
        if (search()) {
            v[y][x] = play + 1;
            return TRUE;
        }
        bitmaps.rows[y] = tmp1;
        bitmaps.columns[x] = tmp2;
        bitmaps.squares[y / 3][x / 3] = tmp3;
    }
    squares_left[square_i].x = x;
    squares_left[square_i].y = y;
    squares_left_count++;
    return FALSE;
}

static bool init_board(const char * buffer) {
    memcpy(&bitmaps, &start_bitmaps, sizeof(Bitmaps));

    squares_left_count = 0;

    u8 i = 0;
    for (u8 y = 0; y < 9; ++y) {
        for (u8 x = 0; x < 9; ++x) {
            if (buffer[i] == '.') {
                v[y][x] = 0;
                squares_left[squares_left_count].x = x;
                squares_left[squares_left_count].y = y;
                squares_left_count++;
            } else {
                u8 vs = buffer[i] - '0';
                add_value(x, y, vs - 1);
                v[y][x] = vs;
            }

            i++;
        }
    }

    return search();
}

int main(int argc, char * argv[]) {
    if (argc != 2 && argc != 3) {
        printf("\nUSAGE\n\t%s puzzle_file [--verbose]\n\nARGUMENTS\n\tpuzzle_file - text file with sudoku puzzles one by line, with missing positions as ASCII zeroes or dots. All other characters will be ignored.\n\t--verbose - whether to print the result after each solved puzzle\n\nEXAMPLE\n\t%s puzzles.txt\n\n", argv[0], argv[0]);
        return 1;
    }
    unsigned int number = 0;
    unsigned int solved = 0;
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

        bool board_solved = init_board(buffer);
        if (board_solved) {
            solved++;
            if (verbose) {
                printf("Solution:\n");
                print_board();
            }
        } else {
            if (verbose) {
                fprintf(stderr, "Impossible puzzle\n");
            }
        }
    }
    printf("Solved %u / %u\n", solved, number);
    return EXIT_SUCCESS;
}
