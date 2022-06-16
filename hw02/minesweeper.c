#include <stdbool.h>
#include "minesweeper.h"
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#define UNUSED(A) (void) (A)

/* ************************************************************** *
 *                         HELP FUNCTIONS                         *
 * ************************************************************** */

bool is_flag(uint16_t cell)
{
    if (cell == 70 || cell == 170 || cell == 146|| cell == 87|| (cell < 160 && cell >= 150)|| (cell < 40 && cell >= 20)){
        return true;
    }
    return false;
}

bool is_mine(uint16_t cell)
{
    if (cell == 77 || cell == 177 || cell == 37 || cell == 170 || cell == 70){
        return true;
    }
    return false;
}

bool is_revealed(uint16_t cell)
{   if (cell == 87){
        return false;
    }
    if (cell == 88){
        return false;
    }
    if (cell > 99){
        return false;
    }
    if (cell >= 150  && cell < 160){
        return false;
    }
    if (cell >= 20  && cell < 40){
        return false;
    }
    return true;
}

int get_number(uint16_t cell){
    if (is_mine(cell)){
        return 0;
    }
    if (cell >= 150  && cell < 160){
        cell -= 150;
        return cell;
    }
    if (cell >= 20  && cell < 40){
        cell -= 20;
        return cell;
    }
    if (cell >= 100  && cell < 110){
        cell -= 100;
        return cell;
    }
    if (cell > 8){
        return 0;
    }
    return cell;
}

/* ************************************************************** *
 *                         INPUT FUNCTIONS                        *
 * ************************************************************** */

bool set_cell(uint16_t *cell, char val)
{   if (cell == NULL){
        return  false;
}   *cell = 0;
    if (val != 100){
    val = (char) toupper((int) val);}
    // whitelist
    if (val == '9'){
        return false;
    }
    if (!((isdigit(val))|| val == 'X'|| val == 'M'|| val == 'F' || val == 'W'|| val == '.' || (val < 100 && val > 20))){
        return false;
    }
    *cell =  (uint16_t) val;
    return true;
}

int load_board(size_t rows, size_t cols, uint16_t board[rows][cols])
{   int MineCounter = 0;
    /*(uint16_t[9])*(char*)board */
    int character;
    character = toupper(getchar());

    for (size_t index = 0; index < rows * cols;) {
        if (character == 0){
            continue;
        }
        uint16_t *cellpointer = *(board + 0) + index;
        if ((character != '.' &&character != 'X' && character != 'M' && character != 'W' && character != 'F' && !isdigit(character) )|| character == 9) {
            printf("Unexpected character: \'%c\' (%d)\n",character, character);
            character = toupper(getchar());
            continue;
        }
        set_cell(cellpointer, (char) character);
        if (is_mine(*cellpointer)){
            MineCounter++;
        }

        if ((uint16_t) index > rows * cols) {
            return -1;
        }
        ++index;
        character = toupper(getchar());
    }
    if (MineCounter < 1){
        return  -1;
    }
    return postprocess(rows, cols, board);
}

int postprocess(size_t rows, size_t cols, uint16_t board[rows][cols])
{   size_t ParCount = 0;
    int MinesCounter;

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            if (isdigit(board[i][j])){
                ParCount++;
            }
            else if (is_mine(board[i][j])){
                ParCount++;
            }
            else if (is_flag(board[i][j])){
                ParCount++;
            }
            else if (board[i][j] == 'X' || board[i][j] == '.'){
                ParCount++;
            }
        }
    }
    if (ParCount != rows*cols){
        return  -1;
    }
    int MinesCounterAll = 0;
    if ((rows > MAX_SIZE )||(cols > MAX_SIZE )){
        return -1;
    }
    if ((rows < MIN_SIZE )||(cols < MIN_SIZE )){
        return -1;

    }
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {

            MinesCounter = 0;
            if ((char) board[i][j] == 'M'){
                MinesCounterAll++;
            }
            if ((char) board[i][j] == 'F'){
                MinesCounterAll++;
            }
            if ((char) board[i][j] == 'W'){
                MinesCounter += 50;
            }
            if ((char) board[i][j] == '.'){
                MinesCounter = -100;
            }
            if (isdigit(board[i][j])){
                MinesCounter = -100;
            }
            if (((char)  board[i][j] == 'X' )||(char) board[i][j] == '.'||(char)  board[i][j] == 'W'||isdigit(board[i][j])){
                if (i != 0) {
                    if (is_mine(board[i - 1][j])) { ++MinesCounter; }
                }
                if (i != rows -1) {
                    if (is_mine(board[i + 1][j])) { ++MinesCounter; }
                }
                if (j != cols-1) {
                    if (is_mine(board[i][j + 1])) { ++MinesCounter; }
                    if (i != rows -1) {
                        if (is_mine(board[i + 1][j + 1])) { ++MinesCounter; }
                    }
                    if (i != 0){
                    if (is_mine(board[i - 1][j + 1])) { ++MinesCounter; }
                    }
                }
                if (j  != 0) {
                    if (is_mine(board[i][j - 1])) { ++MinesCounter; }
                    if (i != 0){
                    if (is_mine(board[i - 1][j - 1])) { ++MinesCounter; }}
                    if (i != rows -1){
                    if (is_mine(board[i + 1][j - 1])) { ++MinesCounter; }}
                }
                if ((isdigit(board[i][j]) && board[i][j]-148 != MinesCounter)) {
                    return -1;
                }
                    board[i][j] = MinesCounter;

            }
        }
    }
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
                board[i][j] = board[i][j] + 100;
        }

    }
    if (is_mine(board[0][0]) || is_mine(board[0][cols - 1]) || is_mine(board[rows - 1][0]) || is_mine(board[rows - 1][cols - 1])){
        return -1;
    }
    if (MinesCounterAll == 0){
        return -1;
    }
    return MinesCounterAll;
}

/* ************************************************************** *
 *                        OUTPUT FUNCTIONS                        *
 * ************************************************************** */

int print_board(size_t rows, size_t cols, uint16_t board[rows][cols])
{   printf("   ");
    for (size_t i = 0; i < cols; ++i) {
        if (i > 9 ){
            printf(" %zu ",i);

        }else{
        printf("  %zu ",i);}
    }
        printf("\n");
    for (size_t i = 0; i < rows; ++i) {
        printf("   +");
        for (size_t j = 0; j < cols; ++j) {
            printf("---+");
        }
        printf("\n");
        if (i > 9 ){
            printf("%zu ",i);} else{
        printf(" %zu ",i);}
        printf("|");
        for (size_t j = 0; j < cols; ++j) {
            if (show_cell(board[i][j])=='F'){
                printf("_F_");
            }
            else if (show_cell(board[i][j])=='X'){
                printf("XXX");
            }
            else if (show_cell(board[i][j])==' '){
                printf("   ");
            }
            else if (show_cell(board[i][j])=='M'){
                printf(" M ");
            }
            else {
                printf(" %hu ",board[i][j]);
            }
            printf("|");
        }
        printf("\n");


    }
        printf("   +");
        for (size_t j = 0; j < cols; ++j) {
            printf("---+");
        }
    printf("\n");
    return 0;
}

char show_cell(uint16_t cell)
{
    if (cell == 170 || cell == 187 || cell == 'W' || cell == 'F' ||(cell >= 20 && cell < 40 )||(cell >= 150 && cell < 160 )){
        return 'F';
    }
    else if (cell >= 100 || cell == 'X'){
        return 'X';
    }
    else if (cell == 77){
        return 'M';
    }
    else if (cell == 0){
        return ' ';
    }
    else{
        /*if (!isdigit(cell) || cell == '9'){
            printf("CHYBA");
            return 0;
        }*/
        return (char) cell;
    }
}

/* ************************************************************** *
 *                    GAME MECHANIC FUNCTIONS                     *
 * ************************************************************** */

int reveal_cell(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col)
{   uint16_t cell = board[row][col];
    if (is_flag(cell)){
        return  -1;
    }
    if (cell < 20){
        return  -1;
    }
    if (row > rows){
        return -1;
    }
    if (col > cols){
        return -1;
    }
    if (is_mine(cell)){
        reveal_single(*(board + row)+col);
        return 1;
    }

    if (cell == 100){
        board[row][col] = cell -100;
        reveal_floodfill(rows, cols, board, row ,col);
        return 0;
    }


    if (cell < 110 && cell > 100 ){
        reveal_single(*(board + row)+col);
        return 0;
    }
    return 0;

}

int reveal_single(uint16_t *cell)
{    if (cell==NULL){
        return -1;
    }
    if (is_flag(*cell)){
        return -1;
    }
    if (is_mine(*cell)){
        *cell = 'M';
        return 1;
    }

    if (*cell < 100){
        return -1;
    }
    *cell = *cell - 100;
    return 0;
}
int flag_cleaner(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col){
    uint16_t cell = board [row][col];
    if (cell > 20 && cell < 30){
        board[row][col] = get_number(cell);
        reveal_single(*(board + row)+col);
        return 0;
    }
    if (cell > 150 && cell < 160){
        board[row][col] = get_number(cell);
        reveal_single(*(board + row)+col);
        return 0;
    }
    if (cell == 150){
        board[row][col] = cell -150;
        reveal_floodfill(rows, cols, board, row ,col);
    }
    return reveal_cell(rows,cols,board,row, col);

}

void reveal_floodfill(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col)
{
    if (row != 0) {
        flag_cleaner(rows, cols ,board, row - 1, col);
    }
    if (row != rows -1) {
        flag_cleaner(rows, cols ,board, row + 1, col);
    }
        if (col != cols-1) {
            flag_cleaner(rows, cols, board, row, col + 1);
            if (row != rows -1) {
                flag_cleaner(rows, cols, board, row + 1, col + 1);}
            if (row != 0) {
                flag_cleaner(rows, cols, board, row - 1, col + 1);}
        }
            if (col  != 0) {
                flag_cleaner(rows, cols ,board, row, col - 1);
                if (row != 0) {
                    flag_cleaner(rows, cols, board, row - 1, col - 1);}
                if (row != rows -1) {
                    flag_cleaner(rows, cols ,board, row + 1, col - 1);}
            }


    }

int FlaggReturn(size_t rows, size_t cols, uint16_t board[rows][cols]){
    int Minescounter = 0;
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            if (is_mine(board[i][j])){
                Minescounter ++;
            }
            if (is_flag(board[i][j])){
                Minescounter --;
            }
        }
    }
    return  Minescounter;
}

int flag_cell(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col)
{   uint16_t cell = board[row][col];
    if (row > rows){
        printf("Provided row is invalid \"%zu\" from input:\n",row);
        return FlaggReturn(rows, cols, board);
    }
    if (col > cols){
        printf("Provided row is invalid \"%zu\" from input:\n",col);
        return FlaggReturn(rows, cols, board);
    }

    if (cell > 187 || cell < 20){
        printf("Unable to flag the cell at position [%zu, %zu]\n",row,col);
        return FlaggReturn(rows, cols, board);
    }
    if (cell >= 150 && cell <= 160){
        board[row][col] = board[row][col] - 50;
    }
    if (cell < 40 && cell >= 20){
        if (is_mine(cell)){
            board[row][col] = board[row][col] + 140;
            return FlaggReturn(rows, cols, board);
        }
        board[row][col] = board[row][col] + 80;
        return FlaggReturn(rows, cols, board);
    }
    if (cell == 170){
        board[row][col] = 177;
        return FlaggReturn(rows, cols, board);
    }
    if (cell == 177){
        board[row][col] = 37;
        return FlaggReturn(rows, cols, board);
    }
    else if (cell == 37){
        board[row][col] = 177;
        return FlaggReturn(rows, cols, board);
    }
    else if (cell>= 100 &&cell <= 110){
        set_cell(*(board + row)+col,(char)(cell - 80));
    }
    return FlaggReturn(rows, cols, board);
}

bool is_solved(size_t rows, size_t cols, uint16_t board[rows][cols])
{   int MinerCount = 0;
    int Unrevealed = 0;
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            if (is_mine(board[i][j])) {
                MinerCount++;
            }
            if (!is_revealed(board[i][j])) {
                Unrevealed++;
            }
        }
    }
    if (Unrevealed == MinerCount){
        return true;
    }
    return false;
}

/* ************************************************************** *
 *                         BONUS FUNCTIONS                        *
 * ************************************************************** */

int generate_random_board(size_t rows, size_t cols, uint16_t board[rows][cols], size_t mines)
{
    // TODO: Implement me
    UNUSED(mines);

    // The postprocess function should be called at the end of the generate random board function
    return postprocess(rows, cols, board);
}

int find_mines(size_t rows, size_t cols, uint16_t board[rows][cols])
{
    // TODO: Implement me
    UNUSED(rows);
    UNUSED(cols);
    UNUSED(board);
    return -1;
}
