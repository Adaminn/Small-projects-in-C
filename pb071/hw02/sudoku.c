#include "sudoku.h"
#include "string.h"

/* ************************************************************** *
 *               Functions required by assignment                 *
 * ************************************************************** */
int decided_number(unsigned int x)
{
    for (int i = 0; i < 9; ++i) {
        if (x == (unsigned) (1U << i)) {
            return i + 1;
        }
    }
    return -1;
} /*
void dec_to_bin(unsigned int n){
    int a[16],i;
    for(i=0;n>0;i++)
    {
        a[i]=n%2;
        n=n/2;
    }

    for(i=i-1;i>=0;i--)
    {
        printf("%d",a[i]);
    }
}*/

bool eliminate_row(unsigned int sudoku[9][9], int row_index)
{
    unsigned value = 0;
    unsigned n;
    unsigned *undecided[10] = { 0 };
    bool changed = false;
    int undecided_index = 0;

    for (int x = 0; x < 9; x++) {
        n = sudoku[row_index][x];
        if (decided_number(n) != -1) {
            value = value | n;
        } else {
            undecided[undecided_index] = &sudoku[row_index][x];
            undecided_index++;
        }
    }
    value = ~value;

    for (int i = 0; i < undecided_index; i++) {
        if ((*(undecided[i]) & value) < *(undecided[i])) {
            changed = true;

            *(undecided[i]) = *(undecided[i]) & value;
        }
    }

    return changed; // todo
}

bool eliminate_col(unsigned int sudoku[9][9], int col_index)
{
    unsigned value = 0;
    unsigned n;
    unsigned *undecided[10] = { 0 };
    bool changed = false;
    int undecided_index = 0;

    for (int y = 0; y < 9; y++) {
        n = sudoku[y][col_index];
        if (decided_number(n) != -1) {
            value = value | n;
        } else {
            undecided[undecided_index] = &sudoku[y][col_index];

            undecided_index++;
        }
    }
    value = ~value;

    for (int i = 0; i < undecided_index; i++) {
        if ((*(undecided[i]) & value) < *(undecided[i])) {
            changed = true;

            *(undecided[i]) = *(undecided[i]) & value;
        }
    }

    return changed;
}

bool eliminate_box(unsigned int sudoku[9][9], int row_index, int col_index)
{
    unsigned value = 0;
    unsigned n;
    unsigned *undecided[10] = { 0 };
    bool changed = false;
    int undecided_index = 0;
    row_index = (row_index / 3) * 3;
    col_index = (col_index / 3) * 3;
    for (int y = col_index; y < col_index + 3; y++) {
        for (int x = row_index; x < row_index + 3; x++) {
            n = sudoku[y][x];
            if (decided_number(n) != -1) {
                value = value | n;
            } else {
                undecided[undecided_index] = &sudoku[y][x];
                undecided_index++;
            }
        }
    }
    value = ~value;
    for (int i = 0; i < undecided_index; i++) {
        if ((*(undecided[i]) & value) < *(undecided[i])) {
            changed = true;
            *(undecided[i]) = *(undecided[i]) & value;
        }
    }

    return changed; // todo
}

bool needs_solving(unsigned int sudoku[9][9])
{
    for (int y = 0; y < 9; y++) {
        for (int x = 0; x < 9; x++) {
            if (decided_number(sudoku[y][x]) == -1) {
                return true;
            }
        }
    }
    return false;
}

bool is_valid(unsigned int sudoku[9][9])
{
    int value;

    for (int y = 0; y < 9; y++) { // rows
        int used[9] = { -1 };
        for (int x = 0; x < 9; x++) {
            if (sudoku[y][x] == 0) {
                return false;
            }
            value = decided_number(sudoku[y][x]);
            if (value != -1) {
                for (int i = 0; i < x; i++) {
                    if (used[i] == value) {
                        return false;
                    }
                }
                used[x] = value;
            }
        }
    }

    for (int x = 0; x < 9; x++) { // cols
        int used[9] = { -1 };
        for (int y = 0; y < 9; y++) {
            value = decided_number(sudoku[y][x]);
            if (value != -1) {
                for (int i = 0; i < y; i++) {
                    if (used[i] == value) {
                        return false;
                    }
                }
                used[y] = value;
            }
        }
    }

    for (int squareY = 0; squareY < 3; squareY++) {     // squares
        for (int squareX = 0; squareX < 3; squareX++) { //
            int used[9] = { -1 };
            for (int y = 0; y < 3; y++) {     // square
                for (int x = 0; x < 3; x++) { //
                    value = decided_number(sudoku[squareY * 3 + y][squareX * 3 + x]);
                    if (value != -1) {
                        for (int i = 0; i < y * 3 + x; i++) {
                            if (used[i] == value) {
                                return false;
                            }
                        }
                        used[y * 3 + x] = value;
                    }
                }
            }
        }
    }
    return true;
}

bool solve(unsigned int sudoku[9][9])
{
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < 9; i++) {
            if (eliminate_row(sudoku, i)) {
                changed = true;
            }
        }

        for (int i = 0; i < 9; i++) {
            if (eliminate_col(sudoku, i)) {
                changed = true;
            }
        }

        for (int y = 0; y < 9; y += 3) {
            for (int x = 0; x < 9; x += 3) {
                if (eliminate_box(sudoku, x, y)) {
                    changed = true;
                }
            }
        }

        if (!is_valid(sudoku)) {
            return false;
        }
    }

    if (needs_solving(sudoku)) {
        return false;
    }

    return true;
}

bool load2(unsigned sudoku[9][9], int c)
{
    for (int i = 0; i < 13; ++i) {
        while (c == '\n' || c == 13) {
        c = getchar();
        }
        if (i % 4 == 0) { // should be +
            for (int j = 0; j < 25; ++j) {
                if (j % 8 == 0) {
                    if (c != '+') {
                        fprintf(stderr, "load error\n");
                        return false;
                    }
                } else {
                    if (c != '-') {
                        fprintf(stderr, "load error\n");
                        return false;
                    }
                }
                c = getchar();
            }
            while (c == '\n' || c == 13) {
                c = getchar();
            }
    
    
        } else { // should be |
            for (int j = 0; j < 25; ++j) {
                if (j % 8 == 0) {
                    if (c != '|') {
                        fprintf(stderr, "load error\n");
                        return false;
                    }
                } else if (j % 2 != 0) {
                    if (c != ' ') {
                        fprintf(stderr, "load error\n");
                        return false;
                    }
                } else {
                    if (c == '.') {
                        sudoku[i - 1 - i / 4][j / 2 - 1 - j / 9] = 511;
                    } else if (c > 48 && c < 58) {
                        sudoku[i - 1 - i / 4][j / 2 - 1 - j / 9] = 1 << (c - 49);
                    } else {
                        fprintf(stderr, "load error\n");
                        return false;
                    }
                }
                c = getchar();
            }

            while (c == '\n' || c == 13) {
                 c = getchar();
            }
       
        }
    }
    return true;
}

bool load(unsigned int sudoku[9][9])
{
    int c = getchar();

    while (c == '\n' || c == 13) {
        c = getchar();
    }
    if (c != '+') {
        for (int y = 0; y < 9; y++) {
            for (int x = 0; x < 9; x++) {
                if (c == 48) {
                    sudoku[y][x] = 511;
                } else if (c > 48 && c < 58) {
                    sudoku[y][x] = 1 << (c - 49);
                } else {
                    fprintf(stderr, "load error\n");
                    return false;
                }
                c = getchar();
            }
        }
    } else {
        return load2(sudoku, c);
    }

    return true;
}

void print(unsigned int sudoku[9][9])
{
    for (int y = 0; y < 13; y++) {
        if (y % 4 == 0) {
            printf("+-------+-------+-------+");
        } else {
            printf("|");
            for (int x = 1; x < 13; x++) {
                if (x % 4 == 0) {
                    printf(" |");
                } else if (decided_number(sudoku[y - 1 - y / 4][x - 1 - x / 4]) != -1) {
                    printf(" %d", decided_number(sudoku[y - 1 - y / 4][x - 1 - x / 4]));
                } else if (sudoku[y - 1 - y / 4][x - 1 - x / 4] == 0) {
                    printf(" !");
                } else {
                    printf(" .");
                }
            }
        }
        printf("\n");
    }
}

/* ************************************************************** *
 *                              Bonus                             *
 * ************************************************************** */

#ifdef BONUS_GENERATE
void generate(unsigned int sudoku[9][9])
{
    return; // todo
}
#endif

#ifdef BONUS_GENERIC_SOLVE
bool generic_solve(unsigned int sudoku[9][9])
{
    return false; // todo
}
#endif

/* ************************************************************** *
 *                 Adwised auxiliary functionns                   *
 * ************************************************************** */

/* TODO: comment-out #if 0 and correspoding endif to implement */

#if 0
/**
 * @brief Compute the bitset of all done numbers in the box.
 *
 * You might like a similar function for row and for column.
 *
 * @param sudoku 2D array of digit bitsets
 * @param row_index of the top most row in box, one of 0, 3, 6
 * @param col_index of the left most column in box, one of 0, 3, 6
 */
static int box_bitset(unsigned int sudoku[9][9], int row_index, int col_index) {
    return 0;
}

/**
 * @brief Add number into bit set
 *
 * This function encapsulates a bit ands, ors and whatever
 * other bint operations, that would flood the toplevel code
 * with implementation details.
 *
 * @param original  contents of the 2D sudoku cell.
 * @param number    to be added to the set
 * 
 * @return          new value of the cell with the number included
 */
static unsigned int bitset_add(unsigned int original, int number) {
    return 0;
}

/**
 * @brief  Drop number from bit set.
 *
 * For detailed description, see bitset_add.
 *
 * @param original  contents of the 2D sudoku cell.
 * @param number    to be dropped from the set
 * 
 * @return          new value of the cell without the number included
 */
static unsigned int bitset_drop(unsigned int original, int number) {
    return 0;
}

/**
 * @brief  Check whether given number is present in the set.
 *
 * @param original  contents of the 2D sudoku cell.
 * @param query     number which should be checked for presence
 * 
 * @return          true if set, false otherwise
 */
static bool bitset_is_set(unsigned int original, int query) {
	return false;
}

/**
 * @brief  Check whether given cell has a unique value assigned.
 *
 * @param original  bitset to check for single vs. multiple values.
 * 
 * @return          true if set, false otherwise
 */
static bool bitset_is_unique(unsigned int original) {
	return false;
}

/**
 * @brief Return next number present in bit set.
 *
 * This function encapsulates a bit ands, ors and whatever
 * other bint operations, that would flood the toplevel code
 * with implementation details.
 *
 * @param original  contents of the 2D sudoku cell.
 * @param previous  last known number present, 0 for start
 * 
 * @return          * next (higher) number than argument if such
                    such is present.
 *                  * -1 otherwise
 *
 * @note The value previous might not be in the bitset
 */
static int bitset_next(unsigned int bitset, int previous) {
    return 0;
}

#endif // if 0
