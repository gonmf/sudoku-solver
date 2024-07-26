#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned short int u16;
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

static u8 bit_count(u16 i) {
	u8 ret = 0;
	while (i) {
		ret += i & 1;
		i /= 2;
	}
	return ret;
}

static u8 bit_counts[512];

static u8 bit_positions[] = {
    [1 << 0] = 0,
    [1 << 1] = 1,
    [1 << 2] = 2,
    [1 << 3] = 3,
    [1 << 4] = 4,
    [1 << 5] = 5,
    [1 << 6] = 6,
    [1 << 7] = 7,
    [1 << 8] = 8,
};

static void populate_bit_counts() {
    u16 i;
    for(i = 1; i < 512; ++i) {
        bit_counts[i] = bit_count(i);
    }
}

static bool calc_bitmaps_of_accepted_values(u16 bitmap[81], const u8 v[81], bool * impossible) {
    u8 i;
    for (i = 0; i < 81; ++i) {
        bitmap[i] = 0x1ff;
    }

    bool finished = TRUE;

    u8 j;
    u8 val;
    u16 mask;
    u8 l;
    u8 k;
    for (i = 0; i < 9; ++i) {
        for (j = 0; j < 9; ++j) {
            val = v[i * 9 + j];

            if (val == 0) {
                finished = FALSE;
            } else {
                mask = (~(1 << (val - 1)));

                //vertical & horizontal
                for (l = 0; l < 9; ++l) {
                    bitmap[l * 9 + j] &= mask;
                    bitmap[i * 9 + l] &= mask;
                }

                // square
                for (k = (i / 3) * 3; k < (i / 3) * 3 + 3; ++k) {
                    for (l = (j / 3) * 3; l < (j / 3) * 3 + 3; ++l) {
                        bitmap[k * 9 + l] &= mask;
                    }
                }
            }
        }
    }

    if (finished) {
        return TRUE;
    }

    for (i = 0; i < 81; ++i) {
        if (bitmap[i] == 0 && v[i] == 0) {
            *impossible = TRUE;
            break;
        }
    }

    return FALSE;
}

static bool board_filled(const u8 v[81]) {
	u8 i;
	for (i = 0; i < 81; ++i) {
		if (v[i] == 0) {
			return FALSE;
        }
    }
	return TRUE;
}

static u8 select_clear_position(const u8 v[81], const u16 bitmap[81], u8 * val) {
    u8 min_bits = 10;
    u8 ret = 255;
    u8 i;

    for (i = 0; i < 81; ++i) {
        if (v[i] == 0) {
            u8 bc = bit_counts[bitmap[i]];
            if (bc == 1) {
                *val = bit_positions[bitmap[i]] + 1;
                return i;
            }
            if (bc < min_bits) {
                min_bits = bc;
                ret = i;
            }
        }
    }

    return ret;
}

static bool solve(u8 v[81]) {
    u8 i;
    u16 bitmap[81];

    bool impossible = FALSE;
    bool finished = calc_bitmaps_of_accepted_values(bitmap, v, &impossible);
    if (impossible) {
        return FALSE;
    }
    if (finished) {
        return TRUE;
    }

    u8 val = 0;
    u8 selected = select_clear_position(v, bitmap, &val);
    if (val) {
        v[selected] = val;
        if (solve(v)) {
            return TRUE;
        }
        v[selected] = 0;
        return FALSE;
    }

    for (i = 0; i < 9; ++i) {
        if (bitmap[selected] & (1 << i)) {
            v[selected] = i + 1;
            if (solve(v)) {
                return TRUE;
            }
            v[selected] = 0;
        }
    }

    return FALSE;
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

    populate_bit_counts();

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

		solve(v);
		if (!board_filled(v)) {
			impossible++;
            if (verbose) {
    			fprintf(stderr, "Impossible puzzle\n");
            }
		} else {
			solved++;
            if (verbose) {
    			printf("Solution:\n");
    			print_board(v);
            }
		}
	}
	printf("Solved=%u\nImpossible=%u\nTotal=%u\n", solved, impossible, number);
	return EXIT_SUCCESS;
}
