**Fast command line sudoku solver**

It uses a simple strategy of ruling out possibilities in a bitmap until there are none left. If the squares can't be filled they start being guessed from the possibilities. Finally a list of all changes is used for backtracking and guessing forks. It is space constant and extremelly fast.

**Benchmarks**

This benchmark was run on the same hardware (of course) solving 29701 puzzles.

Program | Time elapsed | Space cost
---|---|---
*this one* | 0m5.352s | O(1) at ~2KiB
GSF's Sudocoo | 0m8.214s | ?

It is superior to the best of the best sudoku solvers like GSF's Sudocoo (http://forum.enjoysudoku.com/gsf-s-sudoku-solver-t30415.html).
