#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Toggle standard I/O
#define STDOUT 1
// Toggle an extra, final verification, for debugging
#define VERIFY 0

typedef unsigned short int u16;
typedef unsigned char u8;
typedef char bool;
#define TRUE 1
#define FALSE 0

// Backtracking support
typedef struct __change_{
	u8 value;
	u8 position;
	bool guessed;
} change;
static change changes_list[81];
static u8 changes_idx;

// Board definition
static u16 bitmap[81];
static u8 value[81];

static char buffer[83];

#if VERIFY
static bool verify(u8 * b){
	u16 mask;
	u16 mask2;
	u8 i;
	u8 j;
	u8 k;
	u8 l;	
	for(i = 0; i < 9; ++i){
		mask = 0;
		mask2 = 0;
		for(j = 0; j < 9; ++j){
			mask |= 1 << (b[i * 9 + j] - '0' - 1);
			mask2 |= 1 << (b[j * 9 + i] - '0' - 1);
		}
		if(mask != 0x1ff || mask2 != 0x1ff)
			return FALSE;
	}
	for(i = 0; i < 3; ++i){
		for(j = 0; j < 3; ++j){
			mask = 0;
			for(k = 0; k < 3; ++k)
				for(l = 0; l < 3; ++l)
					mask |= 1 << (b[(i * 3 + k) * 9 + j * 3 + l] - '0' - 1);
			if(mask != 0x1ff)
				return FALSE;
		}
	}
	return TRUE;	
}
#endif

static void string_to_board(){
	u8 i;
	for(i = 0; i < 81; ++i){
		if(buffer[i] == '.' || buffer[i] == ' ' || buffer[i] == '0' || buffer[i] == '_'){
			bitmap[i] = 0x1ff;
			value[i] = 0;
		}else{
			bitmap[i] = 1 << (buffer[i] - '1');
			value[i] = buffer[i] - '0';
		}
	}
}

static void ruleout_bads(){
	u8 i;
	u8 j;
	u8 v;
	u16 mask;
	u8 l;
	u8 k;
	for(i = 0; i < 9; ++i){
		for(j = 0; j < 9; ++j){
			v = value[i * 9 + j];
			if(v != 0){
				mask = ~(1 << (v - 1));
				//vertical & horizontal
				for(l = 0; l < 9; ++l){
					if(l != i)
						bitmap[l * 9 + j] &= mask;
					if(l != j)
						bitmap[i * 9 + l] &= mask;
				}
				// square
				if((i % 3) == 0 && (j % 3) == 0){
					for(k = i; k < i + 3; ++k)
						for(l = j; l < j + 3; ++l)
							if(k != i && l != j)
								bitmap[k * 9 + l] &= mask;
				}
			}
		}
	}
}

static u8 bit_count(u16 i){
	u8 ret = 0;
	while(i){
		ret += i & 1;
		i /= 2;
	}
	return ret;
}

static u8 bit_position(u16 i){
	u8 ret = 0;
	while(1){
		if(i & 1)
			return ret;
		ret++;
		i /= 2;
	}
#if STDOUT
	// should never happen
	printf("error (1) unexpected\n");
#endif
	exit(EXIT_FAILURE); // should never happen
}

static bool mark_found(){
	bool ret = FALSE;
	u8 i;
	for(i = 0; i < 81; ++i){
		if(value[i] == 0 && bit_count(bitmap[i]) == 1){
			changes_list[++changes_idx].value = bit_position(bitmap[i]) + 1;
			changes_list[changes_idx].position = i;
			changes_list[changes_idx].guessed = FALSE;
			value[i] = bit_position(bitmap[i]) + 1;
			ret = TRUE;
		}
	}
	return ret;
}

static void clean_bads(){
	u8 i;
	for(i = 0; i < 81; ++i)
		bitmap[i] = 0x1ff;
}

static bool solved(){
	u8 i;
	for(i = 0; i < 81; ++i)
		if(value[i] == 0)
			return FALSE;
	return TRUE;
}

static u8 calc_next_guess(){
	u8 i;
	for(i = changes_list[changes_idx].value + 1; i < 10; ++i){
		if(bitmap[changes_list[changes_idx].position] & (1 << (i - 1))){
			return i;
		}
	}
	return 255;
}

static unsigned char * solve(){
	changes_idx = 255;
	while(1){
		// Fill board as best as possible
		do{
			ruleout_bads();
		}while(mark_found());
		if(solved()){
			return value;
		}

		// Guess
		u8 least_possibilities_pos;
		u8 least_possibilities_num = 10;
		u8 i;
		u8 bc;
		for(i = 0; i < 81; ++i){
			if(value[i] == 0){
				bc = bit_count(bitmap[i]);
				if(bc > 1 && bc < least_possibilities_num){
					least_possibilities_num = bc;
					least_possibilities_pos = i;
				}
			}
		}
		if(least_possibilities_num < 10){
			changes_list[++changes_idx].value = 0;
			changes_list[changes_idx].position = least_possibilities_pos;
			changes_list[changes_idx].guessed = TRUE;
			changes_list[changes_idx].value = calc_next_guess();
			bitmap[least_possibilities_pos] = 1 << (changes_list[changes_idx].value - 1);
			value[least_possibilities_pos] = changes_list[changes_idx].value;
			continue;
		}
		
		// Backtracking until a guessing point
		while(changes_idx != 255){

			if(changes_list[changes_idx].guessed == TRUE){
				clean_bads();
				ruleout_bads();
				u8 next_guess = calc_next_guess();
				if(next_guess != 255){ // Continue guessing
					bitmap[changes_list[changes_idx].position] = 1 << (next_guess - 1);
					value[changes_list[changes_idx].position] = next_guess;
					changes_list[changes_idx].value = next_guess;
					break;
				}else{ // Backtrack
					value[changes_list[changes_idx--].position] = 0;
				}
			}else{ // Backtrack
				value[changes_list[changes_idx--].position] = 0;
			}
		}
	}
	
	return NULL; // impossible to get here
}

int main(int argc, char * argv[]){
	if(argc != 2){
#if STDOUT
		printf("\nUSAGE\n\t%s puzzle_file\n\nARGUMENTS\n\tpuzzle_file - text file with sudoku puzzles one by line, with missing positions as ASCII zeroes or dots. All other characters will be ignored.\n\nEXAMPLE\n\t%s puzzles.txt\n\n", argv[0], argv[0]);
#endif
		return 1;
	}
	unsigned int number = 0;
	unsigned int solved = 0;
	unsigned int impossible = 0;
	
	FILE * fp = fopen(argv[1], "r");
	if(fp == NULL){
		fprintf(stderr, "Could not open file for reading\n");
		return EXIT_FAILURE;
	}
	while(fgets(buffer, 82, fp) != NULL){
		if(strlen(buffer) < 81)
			continue;

		++number;
#if STDOUT
		printf("Starting puzzle #%d:\n%s\n", number, buffer);
#endif
		string_to_board();
		
		u8 * ret = solve();
		if(ret == NULL){
			impossible++;
#if STDOUT	
			fprintf(stderr, "Impossible puzzle\n");
#endif
		}else{
			solved++;
#if STDOUT
			printf("Answer:\n");
			u8 i;
			for(i = 0; i < 81; ++i){
				printf(" %d", ret[i]);
				if(((i + 1) % 9) == 0)
					printf("\n");
			}
#endif
#if VERIFY
			if(verify(ret)){
				fprintf(stderr, "Puzzle solved incorrectly, aborting\n");
				exit(EXIT_FAILURE);
			}
#endif
		}
	}
#if STDOUT
	printf("Solved=%u\nImpossible=%u\nTotal=%u\n", solved, impossible, number);
#endif
	return EXIT_SUCCESS;
}

