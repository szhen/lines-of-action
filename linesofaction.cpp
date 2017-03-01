/*
	Lines of Action
	Sui Zhen
*/
#include <limits.h>
#include <iostream>
#include <string>
#include <utility> // make_pair
#include <queue>
#include <vector>
#define ROW 5
#define COLUMN 5
#define MAX_DEPTH 3
using namespace std;

enum COLORS { WHITE = 2, BLACK = 1 };

void initBoard(int (*board)[COLUMN]);
void printBoard(int(*board)[COLUMN]);
void handleInput(COLORS, int(*board)[COLUMN]);
void handleMove(int(*board)[COLUMN], COLORS player, int x1, int y1, int x2, int y2);
void movePiece(int(*board)[COLUMN], int x1, int y1, int x2, int y2);
bool winningBoard(int(*board)[COLUMN], COLORS);
void absearch(int(*board)[COLUMN], COLORS player, COLORS ai);
pair<pair<int, int>, pair<int, int>> maxVal(int(*board)[COLUMN], COLORS player, COLORS ai);
int maxVal(int(*board)[COLUMN], int alpha, int beta, int depth, COLORS player, COLORS ai);
int minVal(int(*board)[COLUMN], int alpha, int beta, int depth, COLORS player, COLORS ai);
vector<pair<pair<int, int>, pair<int, int>>> allActions(int(*board)[COLUMN], COLORS player);
void applyAction(int(*board)[COLUMN], pair<pair<int, int>, pair<int, int>>);
int evaluate(int(*board)[COLUMN], COLORS ai);

// global variables
int maxDepth;
int cuts;
int nodesGenerated;

int main(int argc, char** argv) {
	maxDepth = 0; cuts = 0; nodesGenerated = 0;
	// state of the game
	COLORS p1, ai;
	string p1color;

	cout << "Choose a color: BLACK or WHITE" << endl;
	cin >> p1color;

	if (p1color == "BLACK") {
		p1 = BLACK;
		ai = WHITE;
	}
	else {
		p1 = WHITE;
		ai = BLACK;
	}

	int board[ROW][COLUMN];

	initBoard(board);

	while (true) {
		if (p1 == BLACK) {
			printBoard(board);
			handleInput(p1, board);
			printBoard(board);
			if (winningBoard(board, p1)) {
				cout << "GG you win!" << endl;
				break;
			}
			cout << "AI is thinking..." << endl;
			absearch(board, p1, ai);
			cout << "AI stats - Nodes generated: " << nodesGenerated << " | Cuts: " << cuts << " | max depth: " << maxDepth << endl;
			if (winningBoard(board, ai)) {
				printBoard(board);
				cout << "GG ai wins!" << endl;
				break;
			}
		}
		else {
			printBoard(board);
			cout << "AI is thinking..." << endl;
			absearch(board, p1, ai);
			cout << "AI stats - Nodes generated: " << nodesGenerated << " | Cuts: " << cuts << " | max depth: " << maxDepth << endl;
			if (winningBoard(board, ai)) {
				printBoard(board);
				cout << "GG ai wins!" << endl;
				break;
			}
			printBoard(board);
			handleInput(p1, board);
			if (winningBoard(board, p1)) {
				printBoard(board);
				cout << "GG you win!" << endl;
				break;
			}
		}
		nodesGenerated = 0; cuts = 0; maxDepth = 0;
	}
	
	return 0;
}

void initBoard(int (*board)[COLUMN]) {
	for (int i = 0; i < ROW; ++i) {
		for (int j = 0; j < COLUMN; ++j) {
			// top and bottom row excluding corner
			if (i == 0 && (j != 0 && j != COLUMN - 1)) board[i][j] = 1;
			else if (i == COLUMN - 1 && (j != 0 && j != COLUMN - 1)) board[i][j] = 1;
			// left and right column excluding corner
			else if (j == 0 && (i != 0 && i != ROW - 1)) board[i][j] = 2;
			else if (j == ROW - 1 && (i != 0 && i != ROW - 1)) board[i][j] = 2;
			else board[i][j] = 0;
		}
	}
}

void printBoard(int(*board)[COLUMN]) {
	for (int i = 0; i < ROW; ++i) {
		cout << " ";
		for (int j = 0; j < COLUMN; ++j) {
			if (board[i][j] == 0) cout << " ";
			else if (board[i][j] == 1) cout << "b";
			else if (board[i][j] == 2) cout << "w";
			if (j != COLUMN - 1) cout << " | ";
		}
		if (i != ROW - 1) cout << "\n-------------------\n";
		else cout << endl;
	}
}

void handleInput(COLORS player, int(*board)[COLUMN]) {
		// process player input
		int x1, y1, x2, y2;
		cout << "Choose a piece location and destination, i.e. 1 2 2 2 to move 1,2 to 2,2:" << endl;
		cin >> x1 >> y1 >> x2 >> y2;
		cout << "x1: " << x1 << " y1: " << y1 << " x2: " << x2 << " y2: " << y2 << endl;
		if (x1 == x2 && y1 == y2) {
			cout << "must move to a new position\n";
			return;
		}
		handleMove(board, player, x1, y1, x2, y2);
}

void handleMove(int(*board)[COLUMN], COLORS player, int x1, int y1, int x2, int y2) {
	if ((x1 >= 1 && x1 <= COLUMN) && (y1 >= 1 && y1 <= ROW) && 
		(x2 >= 1 && x2 <= COLUMN) && (y2 >= 1 && y2 <= ROW)) {
		if (board[x1-1][y1-1] != 0) {
			// src and dst positions are okay (1 - row/COLUMN, not 0 - row/COLUMN-1)
			while (player != board[x1 - 1][y1 - 1]) {
				cout << "Invalid piece, try again..." << endl;
				cout << "Choose a piece location and destination, i.e. 1 2 2 2 to move 1,2 to 2,2:" << endl;
				cin >> x1 >> y1 >> x2 >> y2;
				cout << "x1: " << x1 << " y1: " << y1 << " x2: " << x2 << " y2: " << y2 << endl;
			}
			if (y1 == y2) {
				// move piece vertically
				int dist = abs(x2 - x1);
				// check if move is valid
				int pieces = 0;
				for (int i = 0; i < ROW; ++i) {
					if (board[i][y1-1] != 0) ++pieces;
				}
				if (pieces >= dist) {
					// >= pieces in the column
					// check if there is enemy piece blocking
					bool valid = true;
					if (x1 - x2 > 0) {
						// piece moved from bottom to top
						for (int i = x1 - 1, counter = 0; i != 0; --i, ++counter) {
							if (board[i][y1-1] != 0 && board[i][y1 - 1] != player && counter < dist) {
								// there is a conflict
								valid = false;
								break;
							}
						}
					}
					else {
						// top to bottom
						for (int i = x1 - 1, counter = 0; i < ROW; ++i, ++counter) {
							if (board[i][y1 - 1] != 0 && board[i][y1 - 1] != player && counter < dist) {
								// there is a conflict
								valid = false;
								break;
							}
						}
					}
					if (valid) movePiece(board, x1 - 1, y1 - 1, x2 - 1, y2 - 1);
					else {
						cout << "Invalid move..." << endl;
						return;
					}
				}
			}
			else if (x1 == x2) {
				// move piece horizontally
				int dist = abs(y2 - y1);
				int pieces = 0;
				for (int i = 0; i < COLUMN; ++i) {
					if (board[x1-1][i] != 0) ++pieces;
				}
				if (pieces >= dist) {
					// >= pieces in the column
					// check if there is enemy piece blocking
					bool valid = true;
					if (y1 - y2 > 0) {
						// piece moved from right to left
						for (int i = y1 - 1, counter = 0; i != 0; --i, ++counter) {
							if (board[x1 - 1][i] != 0 && board[x1 - 1][i] != player && counter < dist) {
								// there is a conflict
								valid = false;
								break;
							}
						}
					}
					else {
						// top to bottom
						for (int i = y1 - 1, counter = 0; i < COLUMN; ++i, ++counter) {
							if (board[x1 - 1][i] != 0 && board[x1 - 1][i] != player && counter < dist) {
								// there is a conflict
								valid = false;
								break;
							}
						}
					}
					if (valid) movePiece(board, x1 - 1, y1 - 1, x2 - 1, y2 - 1);
					else {
						cout << "Invalid move..." << endl;
						return;
					}
				}
			}
			else if (abs((y2 - y1) / (x2 - x1))) {
				// move piece diagonally
				int dist = abs(y2 - y1); // works with either x or y because slope is 1
				if (y1 - y2 > 0) {
					// moved from right to left
					if (x1 - x2 > 0) {
						// moved from bottom to top
						// check for how many pieces
						int pieces = 0;
						for (int i = x1 - 1, j = y1 - 1; i != 0, j != 0; --i, --j) {
							if (board[i][j] != 0) ++pieces;
						}
						if (pieces >= dist) {
							bool valid = true;
							for (int i = x1 - 1, j = y1 - 1, counter = 0; i != 0, j != 0; --i, --j, ++counter) {
								cout << board[i][j] << endl;
								if (board[i][j] != 0 && board[i][j] != player && counter < dist) {
									valid = false;
									break;
								}
							}
							if (valid) movePiece(board, x1 - 1, y1 - 1, x2 - 1, y2 - 1);
							else {
								cout << "Invalid move..." << endl;
								return;
							}
						}
					}
					else {
						// moved from top to bottom
						// check for how many pieces
						int pieces = 1;
						for (int i = x1 - 1, j = y1 - 1; i != ROW, j != 0; ++i, --j) {
							if (board[i][j] != 0) ++pieces;
						}
						if (pieces >= dist) {
							bool valid = true;
							for (int i = x1 - 1, j = y1 - 1, counter = 0; i != ROW, j != 0; ++i, --j, ++counter) {
								if (board[i][j] != 0 && board[i][j] != player && counter < dist) {
									valid = false;
									break;
								}
							}
							if (valid) movePiece(board, x1 - 1, y1 - 1, x2 - 1, y2 - 1);
							else {
								cout << "Invalid move..." << endl;
								return;
							}
						}
					}
				}
				else {
					// moved from left to right
					if (x1 - x2 > 0) {
						// moved from bottom to top
						// check for how many pieces
						int pieces = 1;
						for (int i = x1 - 1, j = y1 - 1; i != 0, j != ROW; --i, ++j) {
							if (board[i][j] != 0) ++pieces;
						}
						if (pieces >= dist) {
							bool valid = true;
							for (int i = x1 - 1, j = y1 - 1, counter = 0; i != 0, j != ROW; --i, ++j, ++counter) {
								if (board[i][j] != 0 && board[i][j] != player && counter < dist) {
									valid = false;
									break;
								}
							}
							if (valid) movePiece(board, x1 - 1, y1 - 1, x2 - 1, y2 - 1);
							else {
								cout << "Invalid move..." << endl;
								return;
							}
						}
					}
					else {
						// moved from top to bottom
						// check for how many pieces
						int pieces = 1;
						for (int i = x1 - 1, j = y1 - 1; i != COLUMN, j != ROW; ++i, ++j) {
							if (board[i][j] != 0) ++pieces;
						}
						if (pieces >= dist) {
							bool valid = true;
							for (int i = x1 - 1, j = y1 - 1, counter = 0; i != COLUMN, j != ROW; ++i, ++j, ++counter) {
								if (board[i][j] != 0 && board[i][j] != player && counter < dist) {
									valid = false;
									break;
								}
							}
							if (valid) movePiece(board, x1 - 1, y1 - 1, x2 - 1, y2 - 1);
							else {
								cout << "Invalid move..." << endl;
								return;
							}
						}
					}
				}
			}
			else {
				cout << "Invalid move..." << endl;
				return;
			}
		}
	}
}

void movePiece(int(*board)[COLUMN], int x1, int y1, int x2, int y2) {
	int player = board[x1][y1];
	board[x1][y1] = 0;
	board[x2][y2] = player;
}

bool winningBoard(int(*board)[COLUMN], COLORS player) {
	int bCount = 0;
	for (int i = 0; i < ROW; ++i) {
		for (int j = 0; j < COLUMN; ++j) {
			if (board[i][j] == player) ++bCount;		
		}
	}
	/* check if player won
	idea: look for first piece and perform bfs
	when bfs ends and the number of pieces found == bCount then board is won
	*/
	pair<int, int> bLoc;
	bool tmp = false;
	for (int i = 0; i < ROW; ++i) {
		for (int j = 0; j < COLUMN; ++j) {
			if (board[i][j] == player) {
				bLoc = make_pair(i, j);
				tmp = true;
				break;
			}
		}
		if (tmp) break;
	}
	queue<pair<int, int>> bQ;
	vector<pair<int, int>> visited;
	bQ.push(bLoc);
	int count = 0;
	while (!bQ.empty()) {
		pair<int, int> top = bQ.front();
		bQ.pop();
		// check if popped item is visited or not
		bool vis = false;

		for (int i = 0; i < visited.size(); ++i) {
			if (visited[i].first == top.first && visited[i].second == top.second) {
				// found
				vis = true;
				break;
			}
		}

		if (!vis) {
			// location was not visited add it to vector, check its neighbors and push it to queue
			++count;
			visited.push_back(top);
			// 8 locations to check
			if (top.first - 1 >= 0 && top.second - 1 >= 0) {
				// top left of current
				if (board[top.first - 1][top.second - 1] == player) {
					bQ.push(make_pair(top.first - 1, top.second - 1));
				}
			}
			if (top.second - 1 >= 0) {
				// top of current
				if (board[top.first][top.second - 1] == player) {
					bQ.push(make_pair(top.first, top.second - 1));
				}
			}
			if (top.first + 1 < COLUMN && top.second - 1 >= 0) {
				// top right of current
				if (board[top.first + 1][top.second - 1] == player) {
					bQ.push(make_pair(top.first + 1, top.second - 1));
				}
			}
			if (top.first + 1 < COLUMN) {
				// right of current
				if (board[top.first + 1][top.second] == player) {
					bQ.push(make_pair(top.first + 1, top.second));
				}
			}
			if (top.first + 1 < COLUMN && top.second + 1 < ROW) {
				// bottom right of current
				if (board[top.first + 1][top.second + 1] == player) {
					bQ.push(make_pair(top.first + 1, top.second + 1));
				}
			}
			if (top.second + 1 < ROW) {
				// bottom of current
				if (board[top.first][top.second + 1] == player) {
					bQ.push(make_pair(top.first, top.second + 1));
				}
			}
			if (top.first - 1 >= 0 && top.second + 1 < ROW) {
				// bottom left of current
				if (board[top.first - 1][top.second + 1] == player) {
					bQ.push(make_pair(top.first - 1, top.second + 1));
				}
			}
			if (top.first - 1 >= 0) {
				// left of current
				if (board[top.first - 1][top.second] == player) {
					bQ.push(make_pair(top.first - 1, top.second));
				}
			}
		}
	}

	if (count == bCount) {
		return true;
	}
	return false;
}

vector<pair<pair<int, int>, pair<int, int>>> allActions(int(*board)[COLUMN], COLORS player) {
	vector<pair<pair<int, int>, pair<int, int>>> actions;
	vector<pair<int, int>> pieceLoc;
	for (int i = 0; i < ROW; ++i) {
		for (int j = 0; j < COLUMN; ++j) {
			if (board[i][j] == player) pieceLoc.push_back(make_pair(i, j));
		}
	}
	for (int i = 0; i < pieceLoc.size(); ++i) {
		// horizontal moves
		int pieces = 0;
		for (int j = 0; j < COLUMN; ++j) {
			if (board[pieceLoc[i].first][j] != 0) ++pieces;
		}
		int y = pieceLoc[i].second;
		int counter = 1;
		bool noLeft = false, noRight = false;
		while (counter <= pieces) {
			if (y + counter < ROW) {
				if (board[pieceLoc[i].first][y + counter] != player && board[pieceLoc[i].first][y + counter] != 0 && !noRight) {
					// enemy piece, can capture, but cannot jump over
					noRight = true;
					actions.push_back(make_pair(pieceLoc[i], make_pair(pieceLoc[i].first, y + counter)));
				}
				if (board[pieceLoc[i].first][y + counter] == 0 && !noRight) {
					// free piece and can move here
					actions.push_back(make_pair(pieceLoc[i], make_pair(pieceLoc[i].first, y + counter)));
				}
			}
			else if (y - counter >= 0) {
				if (board[pieceLoc[i].first][y - counter] != player && board[pieceLoc[i].first][y - counter] != 0 && !noLeft) {
					// enemy piece, can capture, but cannot jump over
					noLeft = true;
					actions.push_back(make_pair(pieceLoc[i], make_pair(pieceLoc[i].first, y - counter)));
				}
				if (board[pieceLoc[i].first][y - counter] == 0 && !noLeft) {
					// free piece and can move here
					actions.push_back(make_pair(pieceLoc[i], make_pair(pieceLoc[i].first, y - counter)));
				}
			}
			if (noLeft && noRight) break;
			++counter;
		}

		// vertical moves
		pieces = 0;
		for (int j = 0; j < ROW; ++j) {
			if (board[j][pieceLoc[i].second] != 0) ++pieces;
		}
		int x = pieceLoc[i].first;
		counter = 1;
		bool noUp = false, noDown = false;
		while (counter <= pieces) {
			if (x + counter < COLUMN) {
				if (board[x+counter][y] != player && board[x + counter][y] != 0 && !noDown) {
					// enemy piece, can capture, but cannot jump over
					noDown = true;
					actions.push_back(make_pair(pieceLoc[i], make_pair(pieceLoc[i].first + counter, y)));
				}
				if (board[x + counter][y] == 0 && !noDown) {
					// free piece and can move here
					actions.push_back(make_pair(pieceLoc[i], make_pair(pieceLoc[i].first + counter, y)));
				}
			}
			else if (x - counter >= 0) {
				if (board[x - counter][y] != player && board[x - counter][y] != 0 && !noUp) {
					// enemy piece, can capture, but cannot jump over
					noUp = true;
					actions.push_back(make_pair(pieceLoc[i], make_pair(pieceLoc[i].first - counter, y)));
				}
				if (board[pieceLoc[i].first][y - counter] == 0 && !noLeft) {
					// free piece and can move here
					actions.push_back(make_pair(pieceLoc[i], make_pair(pieceLoc[i].first - counter, y)));
				}
			}
			if (noUp && noDown) break;
			++counter;
		}

		// diagonal moves
		// y = -x diagonal
		pieces = 0;
		int tmp = min(x, y);
		for (int j = x - tmp; j < ROW; ++j) {
			for (int k = y - tmp; k < COLUMN; ++k) {
				if (board[j][k] != 0) ++pieces;
			}
		}
		counter = 1;
		noDown = false, noUp = false;
		while (counter <= pieces) {
			if (x + counter < COLUMN && y + counter < ROW) {
				if (board[x + counter][y+counter] != player && board[x + counter][y+counter] != 0 && !noDown) {
					// enemy piece, can capture, but cannot jump over
					noDown = true;
					actions.push_back(make_pair(pieceLoc[i], make_pair(pieceLoc[i].first + counter, y + counter)));
				}
				if (board[x + counter][y+counter] == 0 && !noDown) {
					// free piece and can move here
					actions.push_back(make_pair(pieceLoc[i], make_pair(pieceLoc[i].first + counter, y + counter)));
				}
			}
			else if (x - counter >= 0 && y - counter >= 0) {
				if (board[x - counter][y- counter] != player && board[x - counter][y - counter] != 0 && !noUp) {
					// enemy piece, can capture, but cannot jump over
					noUp = true;
					actions.push_back(make_pair(pieceLoc[i], make_pair(pieceLoc[i].first - counter, y - counter)));
				}
				if (board[pieceLoc[i].first][y - counter] == 0 && !noLeft) {
					// free piece and can move here
					actions.push_back(make_pair(pieceLoc[i], make_pair(pieceLoc[i].first - counter, y - counter)));
				}
			}
			if (noUp && noDown) break;
			++counter;
		}

		// y = -x diagonal
		pieces = 0;
		for (int j = 0; j < ROW; ++j) {
			for (int k = y + x; k >= 0; --k) {
				if (board[j][k] != 0) ++pieces;
			}
		}
		counter = 1;
		noDown = false, noUp = false;
		while (counter <= pieces) {
			if (x - counter >= 0 && y + counter < ROW) {
				if (board[x - counter][y + counter] != player && board[x - counter][y + counter] != 0 && !noUp) {
					// enemy piece, can capture, but cannot jump over
					noUp = true;
					actions.push_back(make_pair(pieceLoc[i], make_pair(pieceLoc[i].first - counter, y + counter)));
				}
				if (board[x - counter][y + counter] == 0 && !noDown) {
					// free piece and can move here
					actions.push_back(make_pair(pieceLoc[i], make_pair(pieceLoc[i].first - counter, y + counter)));
				}
			}
			else if (x + counter < COLUMN && y - counter >= 0) {
				if (board[x + counter][y - counter] != player && board[x + counter][y - counter] != 0 && !noDown) {
					// enemy piece, can capture, but cannot jump over
					noDown = true;
					actions.push_back(make_pair(pieceLoc[i], make_pair(pieceLoc[i].first + counter, y - counter)));
				}
				if (board[x+counter][y - counter] == 0 && !noLeft) {
					// free piece and can move here
					actions.push_back(make_pair(pieceLoc[i], make_pair(pieceLoc[i].first + counter, y - counter)));
				}
			}
			if (noUp && noDown) break;
			++counter;
		}
	}
	return actions;
}

void absearch(int(*board)[COLUMN], COLORS player, COLORS ai) {
	pair<pair<int, int>, pair<int, int>> action = maxVal(board, player, ai);
	movePiece(board, action.first.first, action.first.second, action.second.first, action.second.second);
}

pair<pair<int, int>, pair<int, int>> maxVal(int(*board)[COLUMN], COLORS player, COLORS ai) {
	vector<pair<pair<int, int>, pair<int, int>>> actions = allActions(board, ai);
	int val = INT_MIN, alpha = INT_MIN, beta = INT_MAX;
	pair<pair<int, int>, pair<int, int>> currentBest;
	for (int i = 0; i < actions.size(); ++i) {
		++nodesGenerated;
		int tmp[ROW][COLUMN];
		for (int r = 0; r < ROW; ++r) {
			for (int c = 0; c < COLUMN; ++c) {
				tmp[r][c] = board[r][c];
			}
		}
		int oldVal = val;
		applyAction(tmp, actions[i]);
		val = max(val, minVal(tmp, alpha, beta, 0 + 1, player, ai));
		if (val != oldVal) {
			// val updated so update best move too
			currentBest = actions[i];
		}
		if (val >= beta) return currentBest;
		alpha = max(alpha, val);
	}
	return currentBest;
}

int maxVal(int(*board)[COLUMN], int alpha, int beta, int depth, COLORS player, COLORS ai) {
	if (winningBoard(board, ai)) return 100;
	else if (winningBoard(board, player)) return -100;
	if (depth > MAX_DEPTH) return evaluate(board, ai);
	// update stats
	++nodesGenerated;
	maxDepth = max(maxDepth, depth);
	int val = INT_MIN;
	vector<pair<pair<int, int>, pair<int, int>>> actions = allActions(board, ai);
	for (int i = 0; i < actions.size(); ++i) {
		int tmp[ROW][COLUMN];
		for (int r = 0; r < ROW; ++r) {
			for (int c = 0; c < COLUMN; ++c) {
				tmp[r][c] = board[r][c];
			}
		}
		applyAction(tmp, actions[i]);
		val = max(val, minVal(tmp, alpha, beta, depth + 1, player, ai));
		if (val >= beta) {
			// cut
			++cuts;
			return val;
		}
		alpha = max(alpha, val);
	}
	return val;
}

int minVal(int(*board)[COLUMN], int alpha, int beta, int depth, COLORS player, COLORS ai) {
	if (winningBoard(board, ai)) return 100;
	else if (winningBoard(board, player)) return -100;
	if (depth > MAX_DEPTH) return -1*evaluate(board, player);
	// update stats
	++nodesGenerated;
	maxDepth = max(maxDepth, depth);
	int val = INT_MAX;
	vector<pair<pair<int, int>, pair<int, int>>> actions = allActions(board, player);
	for (int i = 0; i < actions.size(); ++i) {
		int tmp[ROW][COLUMN];
		for (int r = 0; r < ROW; ++r) {
			for (int c = 0; c < COLUMN; ++c) {
				tmp[r][c] = board[r][c];
			}
		}
		applyAction(tmp, actions[i]);
		val = min(val, maxVal(tmp, alpha, beta, depth + 1, player, ai));
		if (val <= alpha) {
			// cuts
			++cuts;
			return val;
		}
		beta = min(beta, val);
	}
	return val;
}

void applyAction(int(*board)[COLUMN], pair<pair<int, int>, pair<int, int>> action) {
	movePiece(board, action.first.first, action.first.second, action.second.first, action.second.second);
}

int evaluate(int(*board)[COLUMN], COLORS ai) {
	vector<pair<int, int>> pieceLoc;
	for (int i = 0; i < ROW; ++i) {
		for (int j = 0; j < COLUMN; ++j) {
			if (board[i][j] == ai) pieceLoc.push_back(make_pair(i, j));
		}
	}
	int longestContig = 0;
	for (int i = 0; i < pieceLoc.size(); ++i) {
		queue<pair<int, int>> bQ;
		vector<pair<int, int>> visited;
		bQ.push(pieceLoc[i]);
		int count = 0;
		while (!bQ.empty()) {
			pair<int, int> top = bQ.front();
			bQ.pop();
			// check if popped item is visited or not
			bool vis = false;

			for (int i = 0; i < visited.size(); ++i) {
				if (visited[i].first == top.first && visited[i].second == top.second) {
					// found
					vis = true;
					break;
				}
			}

			if (!vis) {
				// location was not visited add it to vector, check its neighbors and push it to queue
				++count;
				visited.push_back(top);
				// 8 locations to check
				if (top.first - 1 >= 0 && top.second - 1 >= 0) {
					// top left of current
					if (board[top.first - 1][top.second - 1] == ai) {
						bQ.push(make_pair(top.first - 1, top.second - 1));
					}
				}
				if (top.second - 1 >= 0) {
					// top of current
					if (board[top.first][top.second - 1] == ai) {
						bQ.push(make_pair(top.first, top.second - 1));
					}
				}
				if (top.first + 1 < COLUMN && top.second - 1 >= 0) {
					// top right of current
					if (board[top.first + 1][top.second - 1] == ai) {
						bQ.push(make_pair(top.first + 1, top.second - 1));
					}
				}
				if (top.first + 1 < COLUMN) {
					// right of current
					if (board[top.first + 1][top.second] == ai) {
						bQ.push(make_pair(top.first + 1, top.second));
					}
				}
				if (top.first + 1 < COLUMN && top.second + 1 < ROW) {
					// bottom right of current
					if (board[top.first + 1][top.second + 1] == ai) {
						bQ.push(make_pair(top.first + 1, top.second + 1));
					}
				}
				if (top.second + 1 < ROW) {
					// bottom of current
					if (board[top.first][top.second + 1] == ai) {
						bQ.push(make_pair(top.first, top.second + 1));
					}
				}
				if (top.first - 1 >= 0 && top.second + 1 < ROW) {
					// bottom left of current
					if (board[top.first - 1][top.second + 1] == ai) {
						bQ.push(make_pair(top.first - 1, top.second + 1));
					}
				}
				if (top.first - 1 >= 0) {
					// left of current
					if (board[top.first - 1][top.second] == ai) {
						bQ.push(make_pair(top.first - 1, top.second));
					}
				}
			}
		}
		longestContig = max(longestContig, count);
	}
	return longestContig * 10;
}