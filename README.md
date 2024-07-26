**Sudoku solver**

It uses a simple strategy of ruling out possibilities in a bitmap until there are none left. If the squares can't be filled they start being guessed from the possibilities.

Performance test with sample puzzles.txt file (29,072 puzzles):

| branch | time |
| :--- | ---: |
| master | 5:07.35 |
| option2 | 7:23.89 |
