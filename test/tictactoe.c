/* simple tic tac toe game, should span over multiple .text pages */

#include "uconst.h"
#include "ulib.e"
#include "ulibuarm.e"

#define CROSS 1
#define CIRCLE 2

#define TURNPLAYER 0
#define TURNAI 1

#define AIROW 0
#define AICOL 1
#define AIDIAG 2

#define PLWIN CROSS
#define PLLOSE CIRCLE
#define DRAW 0

void  printSymbol(int s) {
	switch(s) {
		case 0:
			print_term("|   "); break;
		case CROSS:
			print_term("| X "); break;
		case CIRCLE:
			print_term("| O "); break;
	}
}

void printLine(char *h, int *row) {
	print_term(h);
	printSymbol(row[0]);
	printSymbol(row[1]);
	printSymbol(row[2]);
	print_term("|\n");
}


void printBattlefield(int *game[]) {
	print_term("   1   2   3\n");
	print_term(" +---+---+---+\n");
	printLine("a", game[0]);
	print_term(" +---+---+---+\n");
	printLine("b", game[1]);
	print_term(" +---+---+---+\n");
	printLine("c", game[2]);
	print_term(" +---+---+---+\n");
}

int readCol(char col){
	switch(col){
		case 'a':
		case 'A':
			return 0;
		case 'b':
		case 'B':
			return 1;
		case 'c':
		case 'C':
			return 2;
	}
	return -1;
}

int readRow(char row){
	switch(row){
		case '1':
			return 0;
		case '2':
			return 1;
		case '3':
			return 2;
	}
	return -1;
}

int rowScore(int row, int *game[]){
	int cCount = 0, xCount = 0, i;
	for(i = 0; i < 3; i++){
		if(game[row][i] == CIRCLE){
			cCount ++;
		} else if(game[row][i] == CROSS){
			xCount ++;
		}
	}
	if(xCount == 1 || (cCount + xCount) == 3){
		cCount = -1;
	} else if(xCount == 2){
		cCount = 3;
	}
	return cCount;
}

int colScore(int col, int *game[]){
	int cCount = 0, xCount = 0, i;
	for(i = 0; i < 3; i++){
		if(game[i][col] == CIRCLE){
			cCount ++;
		} else if(game[i][col] == CROSS){
			xCount ++;
		}
	}
	if(xCount == 1 || (cCount + xCount) == 3){
		cCount = -1;
	} else if(xCount == 2){
		cCount = 3;
	}
	return cCount;
}

int diagScore(int diag, int *game[]){
	int cCount = 0, xCount = 0, i, j;
	for(i = 0; i < 3; i++){
		j = (diag == 0 ? i : (2-i));
		if(game[j][i] == CIRCLE){
			cCount ++;
		} else if(game[j][i] == CROSS){
			xCount ++;
		}
	}
	if(xCount == 1 || (cCount + xCount) == 3){
		cCount = -1;
	} else if(xCount == 2){
		cCount = 3;
	}
	return cCount;
}

int checkRow(int row, int *game[]){
	int s, i, winner;
	s = game[row][0];
	winner = s;
	for(i = 0; i < 3 && winner != 0; i++){
		if(game[row][i] != s){
			winner = 0;
		}
	}
	return winner;
}

int checkCol(int col, int *game[]){
	int s, i, winner;
	s = game[0][col];
	winner = s;
	for(i = 0; i < 3 && winner != 0; i++){
		if(game[i][col] != s){
			winner = 0;
		}
	}
	return winner;
}

int checkDiag(int diag, int *game[]){
	int s, i, j, winner;
	j = (diag == 0 ? 0 : 2);
	s = game[j][0];
	winner = s;
	for(i = 0; i < 3 && winner != 0; i++){
		j = (diag == 0 ? i : (2-i));
		if(game[j][i] != s){
			winner = 0;
		}
	}
	return winner;
}

int checkWinner(int *game[]){
	int i, winner;
	winner = 0;
	for(i = 0; i < 3 && winner == 0; i++){
		winner = checkRow(i, game);
	}
	for(i = 0; i < 3 && winner == 0; i++){
		winner = checkCol(i, game);
	}
	for(i = 0; i < 2 && winner == 0; i++){
		winner = checkDiag(i, game);
	}
	return winner;
}

void randomPlay(int *game[]){
	unsigned int i, j, ti, tj, exit;
	unsigned int seedX, seedY;
	seedX = (get_TOD() & 3);
	seedX = (seedX == 3 ? 0 : seedX);
	seedY = (get_TOD() & 3);
	seedY = (seedY == 3 ? 0 : seedY);

	for(i = 0, exit = 0; i < 3 && exit == 0; i++){
		for(j = 0; j < 3 && exit == 0; j++){
			ti = i + seedX - (i+seedX < 3 ? 0 : 3);
			tj = j + seedY - (j+seedY < 3 ? 0 : 3);
			if(game[ti][tj] == 0){
				game[ti][tj] = CIRCLE;
				exit = 1;
			}
		}
	}
}

int main(){
	int row0[3] = {0,0,0};
	int row1[3] = {0,0,0};
	int row2[3] = {0,0,0};
	int *game[3] = {row0, row1, row2};
	int round, sRow, sCol, inputlen, winner, turn;
	int aiMax, aiPos, aiMove, aiScore, i, j;
	unsigned int seed;
	char b[3];

	winner = DRAW;

	for(round = 0, turn = 0; round < 9 && winner == 0; round ++, turn = 1 - turn){
		printBattlefield(game);

		if(turn == TURNPLAYER){
			sRow = sCol = inputlen = -1;
			while(sRow < 0 || sRow > 2 || sCol < 0 || sCol > 2){
				if(round == 0){
					print_term("Select square (e.g. a2): ");
				} else {
					print_term("Select square: ");
				}
				inputlen = read_term(b);
				sCol = readCol(b[0]);
				if(sCol < 0){
					sCol = readCol(b[1]);
					sRow = readRow(b[0]);
				} else {
					sRow = readRow(b[1]);
				}
				if(inputlen != 3 || sCol < 0 || sRow < 0 || game[sCol][sRow] != 0){
					print_term("Invalid selection!\n");
					sRow = sCol = -1;
				}
			}
			game[sCol][sRow] = CROSS;

		} else {

			print_term("playing");
			print_term(".");
			//delay(1);
			print_term(".\n");

			seed = (get_TOD() & 0XFF);
			if(seed < 0X22){
				randomPlay(game);
			} else {
				if(game[1][1] == 0){
					game[1][1] = CIRCLE;
				} else {
					aiMax = aiPos = aiMove = -1;
					seed = (get_TOD() & 7);
					for(i = 0; i < 8 && aiMax < 2; i++){
						j = seed + i - (seed+i < 8 ? 0 : 8);
						if(j < 3){
							if((aiScore = rowScore(j, game)) > aiMax){
								aiPos = j;
								aiMove = AIROW;
								aiMax = aiScore;
							}
						} else if(j < 6){
							if((aiScore = colScore(j-3, game)) > aiMax){
								aiPos = j-3;
								aiMove = AICOL;
								aiMax = aiScore;
							}
						} else {
							if((aiScore = diagScore(j-6, game)) > aiMax){
								aiPos = j-6;
								aiMove = AIDIAG;
								aiMax = aiScore;
							}
						}
					}
					if(aiMax >= 0){
						switch(aiMove){
							case AIROW:
								for(i = 0; i < 3; i++){
									if(game[aiPos][i] == 0){
										game[aiPos][i] = CIRCLE;
										break;
									}
								}
								break;
							case AICOL:
								for(i = 0; i < 3; i++){
									if(game[i][aiPos] == 0){
										game[i][aiPos] = CIRCLE;
										break;
									}
								}

								break;
							case AIDIAG:
								for(i = 0; i < 3; i++){
									if(game[(aiPos*2)-i][i] == 0){
										game[(aiPos*2)-i][i] = CIRCLE;
										break;
									}
								}

								break;
						}
					} else {
						randomPlay(game);
					}
				}
			}
		}

		winner = checkWinner(game);
	}

	printBattlefield(game);

	switch(winner){
		case DRAW:
			print_term("Draw.\n"); break;
		case PLWIN:
			print_term("You Win!\n"); break;
		case PLLOSE:
			print_term("You Lose.\n"); break;
	}

	return 0;
}
