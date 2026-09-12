#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint16_t u16;
typedef uint8_t u8;
typedef char bool;
#define TRUE 1
#define FALSE 0

#define BOARD_SIZE 81
#define MASK_COMBINATIONS 512
#define LINE_BUFFER_SIZE 1024

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
static u8 calced_valid_plays_count[MASK_COMBINATIONS];
static u8 calced_valid_plays[MASK_COMBINATIONS][9];
static u8 squares_left_count;
static Pos squares_left[BOARD_SIZE];
static u8 v[9][9];
static char line[LINE_BUFFER_SIZE];

static void print_board(void) {
    unsigned int x;
    unsigned int y;

    for (y = 0; y < 9; ++y) {
        if (y > 0) {
            printf("\n");
        }
        for (x = 0; x < 9; ++x) {
            if (v[y][x] == 0) {
                printf(" -");
            } else {
                printf(" %u", (unsigned int)v[y][x]);
            }
        }
    }
    printf("\n\n");
}

static void add_value(unsigned int x, unsigned int y, unsigned int val) {
    u16 mask = (u16)(1u << val);
    bitmaps.rows[y] |= mask;
    bitmaps.columns[x] |= mask;
    bitmaps.squares[y / 3][x / 3] |= mask;
}

static void init_global(void) {
    memset(&start_bitmaps, 0, sizeof(Bitmaps));

    for (unsigned int mask = 0; mask < MASK_COMBINATIONS; mask++) {
        unsigned int count = 0;
        for (unsigned int play = 0; play < 9; ++play) {
            if ((mask & (1u << play)) == 0) {
                calced_valid_plays[mask][count] = (u8)play;
                count++;
            }
        }
        calced_valid_plays_count[mask] = (u8)count;
    }
}

static bool search(void) {
    u16 best_masked = 0;
    /* Above the 9 plays a fully empty cell has, so any cell can win. */
    unsigned int best_valid_plays = 10;
    unsigned int best_pos_x = 0;
    unsigned int best_pos_y = 0;
    unsigned int square_i = 0;

    if (squares_left_count == 0) {
        return TRUE;
    }

    for (unsigned int i = 0; i < squares_left_count; ++i) {
        unsigned int x = squares_left[i].x;
        unsigned int y = squares_left[i].y;

        u16 masked = bitmaps.rows[y] | bitmaps.columns[x] | bitmaps.squares[y / 3][x / 3];
        unsigned int valid_plays = calced_valid_plays_count[masked];
        if (valid_plays == 0) {
            return FALSE;
        }
        if (valid_plays == 1) {
            unsigned int play = calced_valid_plays[masked][0];
            u16 tmp1 = bitmaps.rows[y];
            u16 tmp2 = bitmaps.columns[x];
            u16 tmp3 = bitmaps.squares[y / 3][x / 3];
            squares_left_count--;
            squares_left[i] = squares_left[squares_left_count];
            add_value(x, y, play);
            if (search()) {
                v[y][x] = (u8)(play + 1);
                return TRUE;
            }
            bitmaps.rows[y] = tmp1;
            bitmaps.columns[x] = tmp2;
            bitmaps.squares[y / 3][x / 3] = tmp3;
            squares_left[squares_left_count] = squares_left[i];
            squares_left[i].x = (u8)x;
            squares_left[i].y = (u8)y;
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

    unsigned int x = best_pos_x;
    unsigned int y = best_pos_y;

    u16 tmp1 = bitmaps.rows[y];
    u16 tmp2 = bitmaps.columns[x];
    u16 tmp3 = bitmaps.squares[y / 3][x / 3];

    squares_left_count--;
    for (int i = (int)best_valid_plays - 1; i >= 0; --i) {
        unsigned int play = calced_valid_plays[best_masked][i];
        squares_left[square_i] = squares_left[squares_left_count];
        add_value(x, y, play);
        if (search()) {
            v[y][x] = (u8)(play + 1);
            return TRUE;
        }
        bitmaps.rows[y] = tmp1;
        bitmaps.columns[x] = tmp2;
        bitmaps.squares[y / 3][x / 3] = tmp3;
    }
    squares_left[square_i].x = (u8)x;
    squares_left[square_i].y = (u8)y;
    squares_left_count++;
    return FALSE;
}

static bool parse_line(const char * buffer, u8 * cells) {
    unsigned int count = 0;

    for (const char * p = buffer; *p != 0; ++p) {
        char c = *p;
        if (c >= '1' && c <= '9') {
            if (count == BOARD_SIZE) {
                return FALSE;
            }
            cells[count++] = (u8)(c - '0');
        } else if (c == '0' || c == '.') {
            if (count == BOARD_SIZE) {
                return FALSE;
            }
            cells[count++] = 0;
        }
    }

    return count == BOARD_SIZE;
}

static bool init_board(const u8 * cells) {
    memcpy(&bitmaps, &start_bitmaps, sizeof(Bitmaps));

    squares_left_count = 0;

    unsigned int i = 0;
    for (unsigned int y = 0; y < 9; ++y) {
        for (unsigned int x = 0; x < 9; ++x) {
            unsigned int vs = cells[i];

            if (vs == 0) {
                v[y][x] = 0;
                squares_left[squares_left_count].x = (u8)x;
                squares_left[squares_left_count].y = (u8)y;
                squares_left_count++;
            } else {
                u16 mask = (u16)(1u << (vs - 1));
                /* A value repeated in the same row, column or square makes
                   the puzzle impossible; OR-ing it in would hide that. */
                if ((bitmaps.rows[y] | bitmaps.columns[x] | bitmaps.squares[y / 3][x / 3]) & mask) {
                    return FALSE;
                }
                add_value(x, y, vs - 1);
                v[y][x] = (u8)vs;
            }

            i++;
        }
    }

    return search();
}

int main(int argc, char * argv[]) {
    const char * filename = NULL;
    bool verbose = FALSE;
    bool bad_usage = FALSE;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--verbose") == 0) {
            verbose = TRUE;
        } else if (filename == NULL) {
            filename = argv[i];
        } else {
            bad_usage = TRUE;
        }
    }

    if (filename == NULL || bad_usage) {
        printf("\nUSAGE\n\t%s puzzle_file [--verbose]\n\nARGUMENTS\n\tpuzzle_file - text file with sudoku puzzles one by line, with missing positions as ASCII zeroes or dots. All other characters will be ignored, and lines not holding exactly 81 positions will be skipped.\n\t--verbose - whether to print the result after each solved puzzle\n\nEXAMPLE\n\t%s puzzles.txt\n\n", argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    FILE * fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file for reading\n");
        return EXIT_FAILURE;
    }

    init_global();

    unsigned int number = 0;
    unsigned int solved = 0;
    u8 cells[BOARD_SIZE];

    while (fgets(line, LINE_BUFFER_SIZE, fp) != NULL) {
        size_t len = strlen(line);

        if (len + 1 == LINE_BUFFER_SIZE && line[len - 1] != '\n') {
            int c;
            while ((c = fgetc(fp)) != EOF && c != '\n') {
            }
            continue;
        }

        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = 0;
        }

        if (line[0] == '#' || !parse_line(line, cells)) {
            continue;
        }

        ++number;

        if (verbose) {
          printf("Starting puzzle #%u:\n%s\n\n", number, line);
        }

        bool board_solved = init_board(cells);
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

    fclose(fp);

    printf("Solved %u / %u\n", solved, number);
    return EXIT_SUCCESS;
}
