**Sudoku solver**

At the start it populates the unavailable values for every row/column square, then as it searches it sets/unsets their
availability, avoiding recalculating them for every square. It prefers searching based on lowest number of possible values for
each square, mixing in between the detection of impssible states and finished searches, for performance.

Tested with the sample puzzles.txt file (29,072 puzzles) on a 2020 MBP it solves all in 12.03s (0.41ms/puzzle average).
