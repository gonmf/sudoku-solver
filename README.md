**Sudoku solver**

At each iteration it populates a bitmap of the options left for each square, and continues the search from the square with the
least options left. It precalculates the conversion of the bitmap values where a single option is left, the counting of number
of bits for every combination of options left, and enumerated explicitly some bitwise operations to avoid expensive mathematical
instructions.

Performance test with sample puzzles.txt file (29,072 puzzles):

| branch | time |
| :--- | ---: |
| master | 3:00.72 |
| option2 | 7:23.89 |
