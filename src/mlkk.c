/************************************************************************
**
** NAME:        mlkk.c
**
** DESCRIPTION: Lau Kati Kata
**
** AUTHOR:      Alan Wang
**
** DATE:        2024-10-18
**
************************************************************************/

#include "gamesman.h"

CONST_STRING kAuthorName = "Alan Wang";
CONST_STRING kGameName = "Lau Kati Kata";  // Use this spacing and case
CONST_STRING kDBName = "lau kati kata";    // Use this spacing and case

/**
 * @brief An upper bound on the number of reachable positions.
 *
 * @details The hash value of every reachable position must be less
 * than `gNumberOfPositions`.
 */
POSITION gNumberOfPositions = 4782969; // 3^14 to represent each position (empty, black, white, plus turn)

/**
 * @brief The hash value of the initial position of the default
 * variant of the game.
 */
POSITION gInitialPosition = 0; // Will be set in InitializeGame()

/**
 * @brief Indicates whether this game is PARTIZAN, i.e. whether, given
 * a board, each player has a different set of moves 
 * available to them on their turn. If the game is impartial, this is FALSE.
 */
BOOLEAN kPartizan = TRUE;

/**
 * @brief Whether a tie or draw is possible in this game.
 */
BOOLEAN kTieIsPossible = TRUE;

/**
 * @brief Whether the game is loopy. It is TRUE if there exists a position
 * P in the game such that, there is a sequence of N >= 1 moves one can
 * make starting from P that allows them to revisit P.
 */
BOOLEAN kLoopy = TRUE;

/**
 * @brief Whether symmetries are supported, i.e., whether there is
 * at least one position P in the game that gCanonicalPosition() 
 * maps to a position Q such that P != Q. If gCanonicalPosition
 * (initialized in InitializeGame() in this file), is set to NULL,
 * then this should be set to FALSE.
 */
BOOLEAN kSupportsSymmetries = TRUE;

/**
 * @brief Useful for some solvers. Do not change this.
 */
POSITION kBadPosition = -1;

/**
 * @brief For debugging, some solvers print debug statements while
 * solving a game if kDebugDetermineValue is set to TRUE. If this
 * is set to FALSE, then those solvers do not print the debugging
 * statements.
 */
BOOLEAN kDebugDetermineValue = FALSE;

/**
 * @brief Declaration of optional functions.
 */
POSITION GetCanonicalPosition(POSITION);
void PositionToString(POSITION, char*);

/**
 * @brief If multiple variants are supported, set this
 * to true -- GameSpecificMenu() must be implemented.
 */
BOOLEAN kGameSpecificMenu = FALSE;

/**
 * @brief Set this to true if the DebugMenu() is implemented.
 */
BOOLEAN kDebugMenu = FALSE;

/**
 * @brief These variables are not needed for solving but if you have time
 * after you're done solving the game you should initialize them
 * with something helpful, for the TextUI.
 */
CONST_STRING kHelpGraphicInterface = "";
CONST_STRING kHelpTextInterface = "";
CONST_STRING kHelpOnYourTurn = "";
CONST_STRING kHelpStandardObjective = "";
CONST_STRING kHelpReverseObjective = "";
CONST_STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";
CONST_STRING kHelpExample = "";

/**
 * @brief Tcl-related stuff. Do not change if you do not plan to make a Tcl interface.
 */
void *gGameSpecificTclInit = NULL;
void SetTclCGameSpecificOptions(int theOptions[]) { (void)theOptions; }

#define BOARDSIZE 13
#define MAX_ADJACENTS 6 // Adjusted to match the maximum number of adjacents
#define MAX_CAPTURES 6  // Increased to accommodate node 6's captures
#define NUMSYMMETRIES 4 // Number of symmetries (identity, hFlip, vFlip, dFlip)

typedef enum {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
} Piece;

typedef struct {
    int adjacents[MAX_ADJACENTS];
    int numAdjacents;
    int captures[MAX_CAPTURES];
    int numCaptures;
} Node;

Node board[BOARDSIZE];

char pieceChar(Piece p) {
    if (p == BLACK) return 'B';
    if (p == WHITE) return 'W';
    return '-';
}

// Mapping of positions for symmetries
int symmetryMap[NUMSYMMETRIES][BOARDSIZE];

void InitializeSymmetryMaps(void);

void ApplySymmetry(Piece *srcBoard, Piece *destBoard, int symmetryIndex);

// Function prototypes
int GetMiddlePosition(int from, int to);

/*********** BEGIN SOLVING FUNCTIONS ***********/

/**
 * @brief Initialize any global variables.
 */
void InitializeGame(void) {
    gCanonicalPosition = GetCanonicalPosition;

    // Initialize the adjacency list for each position based on your provided adjacents

    // Initialize all nodes
    for (int i = 0; i < BOARDSIZE; i++) {
        board[i].numAdjacents = 0;
        board[i].numCaptures = 0;
    }

    // Node 0: adjacents [1, 3]; captures [2, 6]
    board[0].adjacents[board[0].numAdjacents++] = 1;
    board[0].adjacents[board[0].numAdjacents++] = 3;
    board[0].captures[board[0].numCaptures++] = 2;
    board[0].captures[board[0].numCaptures++] = 6;

    // Node 1: adjacents [0, 2, 4]; captures [6]
    board[1].adjacents[board[1].numAdjacents++] = 0;
    board[1].adjacents[board[1].numAdjacents++] = 2;
    board[1].adjacents[board[1].numAdjacents++] = 4;
    board[1].captures[board[1].numCaptures++] = 6;

    // Node 2: adjacents [1, 5]; captures [0, 6]
    board[2].adjacents[board[2].numAdjacents++] = 1;
    board[2].adjacents[board[2].numAdjacents++] = 5;
    board[2].captures[board[2].numCaptures++] = 0;
    board[2].captures[board[2].numCaptures++] = 6;

    // Node 3: adjacents [0, 4, 6]; captures [5, 9]
    board[3].adjacents[board[3].numAdjacents++] = 0;
    board[3].adjacents[board[3].numAdjacents++] = 4;
    board[3].adjacents[board[3].numAdjacents++] = 6;
    board[3].captures[board[3].numCaptures++] = 5;
    board[3].captures[board[3].numCaptures++] = 9;

    // Node 4: adjacents [1, 3, 5, 6]; captures [8]
    board[4].adjacents[board[4].numAdjacents++] = 1;
    board[4].adjacents[board[4].numAdjacents++] = 3;
    board[4].adjacents[board[4].numAdjacents++] = 5;
    board[4].adjacents[board[4].numAdjacents++] = 6;
    board[4].captures[board[4].numCaptures++] = 8;

    // Node 5: adjacents [2, 4, 6]; captures [3, 7]
    board[5].adjacents[board[5].numAdjacents++] = 2;
    board[5].adjacents[board[5].numAdjacents++] = 4;
    board[5].adjacents[board[5].numAdjacents++] = 6;
    board[5].captures[board[5].numCaptures++] = 3;
    board[5].captures[board[5].numCaptures++] = 7;

    // Node 6: adjacents [3, 4, 5, 7, 8, 9]; captures [1, 11, 0, 2, 10, 12]
    board[6].adjacents[board[6].numAdjacents++] = 3;
    board[6].adjacents[board[6].numAdjacents++] = 4;
    board[6].adjacents[board[6].numAdjacents++] = 5;
    board[6].adjacents[board[6].numAdjacents++] = 7;
    board[6].adjacents[board[6].numAdjacents++] = 8;
    board[6].adjacents[board[6].numAdjacents++] = 9;
    board[6].captures[board[6].numCaptures++] = 1;
    board[6].captures[board[6].numCaptures++] = 11;
    board[6].captures[board[6].numCaptures++] = 0;
    board[6].captures[board[6].numCaptures++] = 2;
    board[6].captures[board[6].numCaptures++] = 10;
    board[6].captures[board[6].numCaptures++] = 12;

    // Node 7: adjacents [6, 8, 10]; captures [5, 9]
    board[7].adjacents[board[7].numAdjacents++] = 6;
    board[7].adjacents[board[7].numAdjacents++] = 8;
    board[7].adjacents[board[7].numAdjacents++] = 10;
    board[7].captures[board[7].numCaptures++] = 5;
    board[7].captures[board[7].numCaptures++] = 9;

    // Node 8: adjacents [6, 7, 9, 11]; captures [4]
    board[8].adjacents[board[8].numAdjacents++] = 6;
    board[8].adjacents[board[8].numAdjacents++] = 7;
    board[8].adjacents[board[8].numAdjacents++] = 9;
    board[8].adjacents[board[8].numAdjacents++] = 11;
    board[8].captures[board[8].numCaptures++] = 4;

    // Node 9: adjacents [6, 8, 12]; captures [7, 3]
    board[9].adjacents[board[9].numAdjacents++] = 6;
    board[9].adjacents[board[9].numAdjacents++] = 8;
    board[9].adjacents[board[9].numAdjacents++] = 12;
    board[9].captures[board[9].numCaptures++] = 7;
    board[9].captures[board[9].numCaptures++] = 3;

    // Node 10: adjacents [7, 11]; captures [6, 12]
    board[10].adjacents[board[10].numAdjacents++] = 7;
    board[10].adjacents[board[10].numAdjacents++] = 11;
    board[10].captures[board[10].numCaptures++] = 6;
    board[10].captures[board[10].numCaptures++] = 12;

    // Node 11: adjacents [10, 12]; captures [6]
    board[11].adjacents[board[11].numAdjacents++] = 10;
    board[11].adjacents[board[11].numAdjacents++] = 12;
    board[11].captures[board[11].numCaptures++] = 8;
    board[11].captures[board[11].numCaptures++] = 6;

    // Node 12: adjacents [9, 11]; captures [6, 10]
    board[12].adjacents[board[12].numAdjacents++] = 9;
    board[12].adjacents[board[12].numAdjacents++] = 11;
    board[12].captures[board[12].numCaptures++] = 6;
    board[12].captures[board[12].numCaptures++] = 10;

    // Set the initial position
    // Black pieces occupy positions 0-5
    // White pieces occupy positions 7-12
    // Position 6 is empty (the central position)

    POSITION initialPosition = 0;
    int i;
    for (i = 0; i < BOARDSIZE; i++) {
        if (i == 6) {
            // Middle position is empty
            initialPosition = initialPosition * 3 + EMPTY;
        } else if (i < 6) {
            // Black pieces occupy positions 0-5
            initialPosition = initialPosition * 3 + BLACK;
        } else {
            // White pieces occupy positions 7-12
            initialPosition = initialPosition * 3 + WHITE;
        }
    }

    // The initial position is black's turn
    gInitialPosition = initialPosition * 3 + BLACK;

    // Use PositionToString function
    gPositionToStringFunPtr = &PositionToString;

    // Initialize symmetry maps
    InitializeSymmetryMaps();
}

/**
 * @brief Initialize the symmetry maps for the board positions.
 */
void InitializeSymmetryMaps(void) {
    // Symmetry mappings based on rotations and reflections

    // Identity mapping
    int identityMap[BOARDSIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    // Horizontal flip
    int hFlipMap[BOARDSIZE] = {2, 1, 0, 5, 4, 3, 6, 9, 8, 7, 12, 11, 10};
    // Vertical flip
    int vFlipMap[BOARDSIZE] = {10, 11, 12, 7, 8, 9, 6, 3, 4, 5, 0, 1, 2};
    // Diagonal flip (swap top and bottom levels)
    int dFlipMap[BOARDSIZE] = {12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

    // For simplicity, let's consider 4 symmetries: identity, horizontal flip, vertical flip, and diagonal flip
    memcpy(symmetryMap[0], identityMap, sizeof(identityMap));
    memcpy(symmetryMap[1], hFlipMap, sizeof(hFlipMap));
    memcpy(symmetryMap[2], vFlipMap, sizeof(vFlipMap));
    memcpy(symmetryMap[3], dFlipMap, sizeof(dFlipMap));
}

/**
 * @brief Apply a symmetry to a board state.
 */
void ApplySymmetry(Piece *srcBoard, Piece *destBoard, int symmetryIndex) {
    int *map = symmetryMap[symmetryIndex];
    for (int i = 0; i < BOARDSIZE; i++) {
        destBoard[i] = srcBoard[map[i]];
    }
}

// Function to get the middle position for captures
int GetMiddlePosition(int from, int to) {
    // For predefined captures, we can define the middle positions
    // Manually define the middle positions for each capture
    if ((from == 0 && to == 2) || (from == 2 && to == 0)) return 1;
    if ((from == 0 && to == 6) || (from == 6 && to == 0)) return 3;
    if ((from == 1 && to == 6) || (from == 6 && to == 1)) return 4;
    if ((from == 2 && to == 6) || (from == 6 && to == 2)) return 5;
    if ((from == 3 && to == 5) || (from == 5 && to == 3)) return 4;
    if ((from == 3 && to == 9) || (from == 9 && to == 3)) return 6;
    if ((from == 4 && to == 8) || (from == 8 && to == 4)) return 6;
    if ((from == 5 && to == 7) || (from == 7 && to == 5)) return 6;
    if ((from == 6 && to == 11) || (from == 11 && to == 6)) return 8;
    if ((from == 7 && to == 9) || (from == 9 && to == 7)) return 8;
    if ((from == 8 && to == 4) || (from == 4 && to == 8)) return 6;
    if ((from == 9 && to == 7) || (from == 7 && to == 9)) return 8;
    if ((from == 9 && to == 3) || (from == 3 && to == 9)) return 6;
    if ((from == 10 && to == 6) || (from == 6 && to == 10)) return 7;
    if ((from == 10 && to == 12) || (from == 12 && to == 10)) return 11;
    if ((from == 11 && to == 6) || (from == 6 && to == 11)) return 8;
    if ((from == 12 && to == 6) || (from == 6 && to == 12)) return 9;
    if ((from == 12 && to == 10) || (from == 10 && to == 12)) return 11;
    // New captures involving 6
    if ((from == 6 && to == 0) || (from == 0 && to == 6)) return 3;
    if ((from == 6 && to == 2) || (from == 2 && to == 6)) return 5;
    if ((from == 6 && to == 10) || (from == 10 && to == 6)) return 7;
    if ((from == 6 && to == 12) || (from == 12 && to == 6)) return 9;
    return -1; // Not a valid capture move
}

/**
 * @brief Return the head of a list of the legal moves from the input position.
 */
void GenerateCaptureMoves(int from, Piece *boardState, Piece turn, MOVELIST **moves, int depth);

MOVELIST *GenerateMoves(POSITION position) {
    MOVELIST *moves = NULL;
    Piece boardState[BOARDSIZE];
    Piece turn;
    int i, j;

    // Decode the position
    POSITION tempPosition = position;
    turn = tempPosition % 3 == BLACK ? BLACK : WHITE;
    tempPosition /= 3;

    for (i = BOARDSIZE - 1; i >= 0; i--) {
        boardState[i] = tempPosition % 3;
        tempPosition /= 3;
    }

    // For each piece of the current player
    for (i = 0; i < BOARDSIZE; i++) {
        if (boardState[i] == turn) {
            // Check for adjacent empty positions to move to
            for (j = 0; j < board[i].numAdjacents; j++) {
                int adj = board[i].adjacents[j];
                if (boardState[adj] == EMPTY) {
                    // Encode move: from * 100 + to
                    MOVE move = i * 100 + adj;
                    moves = CreateMovelistNode(move, moves);
                }
            }

            // Check for possible captures (including multi-capture sequences)
            GenerateCaptureMoves(i, boardState, turn, &moves, 0);
        }
    }

    return moves;
}

/**
 * @brief Recursively generate all possible capture sequences from a given position.
 */
void GenerateCaptureMoves(int from, Piece *boardState, Piece turn, MOVELIST **moves, int depth) {
    int j;
    BOOLEAN foundCapture = FALSE;

    for (j = 0; j < board[from].numCaptures; j++) {
        int dest = board[from].captures[j];
        int over = GetMiddlePosition(from, dest);

        // Check if this is a valid capture move
        if (over != -1 && boardState[over] != EMPTY && boardState[over] != turn && boardState[dest] == EMPTY) {
            foundCapture = TRUE;

            // Create a temporary board state to simulate the move
            Piece tempBoardState[BOARDSIZE];
            memcpy(tempBoardState, boardState, sizeof(Piece) * BOARDSIZE);
            tempBoardState[from] = EMPTY;      // Moving from this position
            tempBoardState[over] = EMPTY;      // Captured opponent piece
            tempBoardState[dest] = turn;       // Moving to this position

            // Encode the current capture move: from * 10000 + over * 100 + dest
            MOVE move = from * 10000 + over * 100 + dest;

            // If this is the first capture, create a new move
            if (depth == 0) {
                *moves = CreateMovelistNode(move, *moves);
            } else {
                // If we are extending an existing capture sequence, add a new move node
                MOVELIST *newMoveNode = CreateMovelistNode(move, *moves);
                newMoveNode->next = *moves; // Insert at the beginning
                *moves = newMoveNode;
            }
            
            // Continue looking for additional captures from the new position
            GenerateCaptureMoves(dest, tempBoardState, turn, moves, depth + 1);
        }
    }

    // If no further captures are found and we are in a multi-capture sequence, add the move
    if (!foundCapture && depth > 0) {
        // This ensures that intermediate capture sequences are also added
        return;
    }
}


/**
 * @brief Return the child position reached when the input move
 * is made from the input position, including multi-captures.
 */
POSITION DoMove(POSITION position, MOVE move) {
    Piece boardState[BOARDSIZE];
    Piece turn;
    int i;

    // Decode the position
    POSITION tempPosition = position;
    turn = tempPosition % 3 == BLACK ? BLACK : WHITE;
    tempPosition /= 3;

    for (i = BOARDSIZE - 1; i >= 0; i--) {
        boardState[i] = tempPosition % 3;
        tempPosition /= 3;
    }

    // Apply the move (handle multi-capture)
    int currentMove = move;
    while (currentMove >= 10000) {
        int from = currentMove / 10000;
        int over = (currentMove % 10000) / 100;
        int to = currentMove % 100;

        boardState[to] = turn;
        boardState[from] = EMPTY;
        boardState[over] = EMPTY;

        currentMove = currentMove % 10000; // Remove the last applied move
    }

    // If there is a simple move left after multi-capture moves
    if (currentMove > 0) {
        int from = currentMove / 100;
        int to = currentMove % 100;

        boardState[to] = turn;
        boardState[from] = EMPTY;
    }

    // Switch turn after all captures or simple moves are applied
    turn = (turn == BLACK) ? WHITE : BLACK;

    // Encode the new position
    POSITION newPosition = 0;
    for (i = 0; i < BOARDSIZE; i++) {
        newPosition = newPosition * 3 + boardState[i];
    }
    newPosition = newPosition * 3 + turn;

    return newPosition;
}

/**
 * @brief Return win, lose, or tie if the input position
 * is primitive; otherwise return undecided.
 */
VALUE Primitive(POSITION position) {
    Piece boardState[BOARDSIZE];
    Piece turn;
    int i;

    // Decode the position
    POSITION tempPosition = position;
    turn = tempPosition % 3 == BLACK ? BLACK : WHITE;
    tempPosition /= 3;

    int blackCount = 0;
    int whiteCount = 0;

    for (i = BOARDSIZE - 1; i >= 0; i--) {
        boardState[i] = tempPosition % 3;
        tempPosition /= 3;

        if (boardState[i] == BLACK) {
            blackCount++;
        } else if (boardState[i] == WHITE) {
            whiteCount++;
        }
    }

    // Check for victory conditions
    if (blackCount == 0) {
        return (turn == BLACK) ? lose : win;
    }
    if (whiteCount == 0) {
        return (turn == WHITE) ? lose : win;
    }

    // Check if the current player has any legal moves
    MOVELIST *moves = GenerateMoves(position);
    if (moves == NULL) {
        return lose;
    } else {
        FreeMoveList(moves);
        return undecided;
    }
}

/**
 * @brief Given a position P, GetCanonicalPosition(P) shall return a 
 * position Q such that P is symmetric to P and 
 * GetCanonicalPosition(Q) also returns Q.
 */
POSITION GetCanonicalPosition(POSITION position) {
    Piece boardState[BOARDSIZE];
    Piece turn;
    int i;

    // Decode the position
    POSITION tempPosition = position;
    turn = tempPosition % 3 == BLACK ? BLACK : WHITE;
    tempPosition /= 3;

    for (i = BOARDSIZE - 1; i >= 0; i--) {
        boardState[i] = tempPosition % 3;
        tempPosition /= 3;
    }

    // Apply symmetries and find the minimum position hash
    POSITION minPosition = position;

    for (int s = 0; s < NUMSYMMETRIES; s++) {
        Piece symBoard[BOARDSIZE];
        ApplySymmetry(boardState, symBoard, s);

        // Encode the symBoard into a position
        POSITION symPosition = 0;
        for (i = 0; i < BOARDSIZE; i++) {
            symPosition = symPosition * 3 + symBoard[i];
        }
        symPosition = symPosition * 3 + turn;

        if (symPosition < minPosition) {
            minPosition = symPosition;
        }
    }

    return minPosition;
}

/*********** END SOLVING FUNCTIONS ***********/

/*********** BEGIN TEXTUI FUNCTIONS ***********/

/**
 * @brief Print the position in a pretty format, including the
 * prediction of the game's outcome.
 */
void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
    Piece boardState[BOARDSIZE];
    Piece turn;
    int i;

    // Decode the position
    POSITION tempPosition = position;
    turn = tempPosition % 3 == BLACK ? BLACK : WHITE;
    tempPosition /= 3;

    for (i = BOARDSIZE - 1; i >= 0; i--) {
        boardState[i] = tempPosition % 3;
        tempPosition /= 3;
    }

    // Print the current player
    printf("\nCurrent player: %s\n", (turn == BLACK) ? "Black" : "White");

    // Print the board with positions and pieces
    printf("\nBoard positions:\n\n");
    printf(" %2d:%c   %2d:%c   %2d:%c\n", 0, pieceChar(boardState[0]), 1, pieceChar(boardState[1]), 2, pieceChar(boardState[2]));
    printf(" %2d:%c   %2d:%c   %2d:%c\n", 3, pieceChar(boardState[3]), 4, pieceChar(boardState[4]), 5, pieceChar(boardState[5]));
    printf("      %2d:%c\n", 6, pieceChar(boardState[6]));
    printf(" %2d:%c   %2d:%c   %2d:%c\n", 7, pieceChar(boardState[7]), 8, pieceChar(boardState[8]), 9, pieceChar(boardState[9]));
    printf(" %2d:%c   %2d:%c   %2d:%c\n",10, pieceChar(boardState[10]),11, pieceChar(boardState[11]),12, pieceChar(boardState[12]));

    // Print prediction
    printf("\n%s\n", GetPrediction(position, playerName, usersTurn));
}

/**
 * @brief Find out if the player wants to undo, abort, or neither.
 * If so, return Undo or Abort and don't change `move`.
 * Otherwise, get the new `move` and fill the pointer up.
 */
USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
    char input[100];

    // Generate and print possible moves
    MOVELIST *moves = GenerateMoves(position);
    MOVELIST *current = moves;
    printf("\nPossible moves:\n");
    while (current != NULL) {
        char moveString[32];
        MoveToString(current->move, moveString);
        printf("%s\n", moveString);
        current = current->next;
    }

    do {
        printf("\nEnter your move, %s (format: from to): ", playerName);
        fgets(input, sizeof(input), stdin);

        // Check for undo or abort commands
        if (strncmp(input, "undo", 4) == 0) {
            FreeMoveList(moves);
            return Undo;
        }
        if (strncmp(input, "abort", 5) == 0) {
            FreeMoveList(moves);
            return Abort;
        }

        // Validate and convert the move
        if (ValidTextInput(input)) {
            *move = ConvertTextInputToMove(input);
            // Validate the move
            current = moves;
            while (current != NULL) {
                if (current->move == *move) {
                    FreeMoveList(moves);
                    return Continue;
                }
                current = current->next;
            }
            printf("Invalid move. Please try again.\n");
        } else {
            printf("Invalid input format. Please try again.\n");
        }
    } while (TRUE);
    FreeMoveList(moves);
    return (Continue); /* this is never reached, but lint is now happy */
}

/**
 * @brief Given a move that the user typed while playing a game in the
 * TextUI, return TRUE if the input move string is of the right "form"
 * and can be converted to a move hash.
 */
BOOLEAN ValidTextInput(STRING input) {
    int from, to;
    return sscanf(input, "%d %d", &from, &to) == 2 || sscanf(input, "%d %d %d", &from, &to, &to) == 3;
}

/**
 * @brief Convert the string input to the internal move representation,
 *        including multi-capture.
 */
MOVE ConvertTextInputToMove(STRING input) {
    int from, to, next;
    if (sscanf(input, "%d %d %d", &from, &to, &next) == 3) {
        // Multi-capture notation
        MOVE move = from * 10000 + GetMiddlePosition(from, to) * 100 + to;
        move = move * 10000 + GetMiddlePosition(to, next) * 100 + next;
        return move;
    } else {
        sscanf(input, "%d %d", &from, &to);
        // Determine if it's a simple move or a capture
        int over = GetMiddlePosition(from, to);
        if (over != -1) {
            return from * 10000 + over * 100 + to; // Capture move
        } else {
            return from * 100 + to; // Simple move
        }
    }
}

/**
 * @brief Write a short human-readable string representation
 *        of the move to the input buffer, now with multi-capture support.
 */
void MoveToString(MOVE move, char *moveStringBuffer) {
    if (move < 10000) {
        int from = move / 100;
        int to = move % 100;
        sprintf(moveStringBuffer, "%d %d", from, to);
    } else {
        char buffer[128] = "";
        while (move >= 10000) {
            int from = move / 10000;
            int to = move % 100;
            sprintf(buffer + strlen(buffer), "%d %d ", from, to);
            move = move / 10000;
        }
        buffer[strlen(buffer) - 1] = '\0'; // Remove trailing space
        strcpy(moveStringBuffer, buffer);
    }
}

/**
 * @brief Nicely format the computers move.
 */
void PrintComputersMove(MOVE computersMove, STRING computersName) {
    char moveStringBuffer[32];
    MoveToString(computersMove, moveStringBuffer);
    printf("%s's move: %s\n", computersName, moveStringBuffer);
}

/**
 * @brief Menu used to debug internal problems. 
 * Does nothing if kDebugMenu == FALSE.
 */
void DebugMenu(void) {}

/*********** END TEXTUI FUNCTIONS ***********/

/*********** BEGIN VARIANT FUNCTIONS ***********/

/**
 * @return The total number of variants supported.
 */
int NumberOfOptions(void) {
    return 1;
}

/**
 * @return The current variant ID.
 */
int getOption(void) {
    return 0;
}

/**
 * @brief Set any global variables or data structures according to the
 * variant specified by the input variant ID.
 */
void setOption(int option) {
    (void)option;
}

/**
 * @brief Interactive menu used to change the variant, i.e., change
 * game-specific parameters, such as the side-length of a tic-tac-toe
 * board, for example. Does nothing if kGameSpecificMenu == FALSE.
 */
void GameSpecificMenu(void) {}

/*********** END VARIANT-RELATED FUNCTIONS ***********/

/**
 * @brief Convert the input position to a human-readable formal
 * position string.
 */
void PositionToString(POSITION position, char *positionStringBuffer) {
    Piece boardState[BOARDSIZE];
    Piece turn;
    int i;

    // Decode the position
    POSITION tempPosition = position;
    turn = tempPosition % 3 == BLACK ? BLACK : WHITE;
    tempPosition /= 3;

    for (i = BOARDSIZE - 1; i >= 0; i--) {
        boardState[i] = tempPosition % 3;
        tempPosition /= 3;
    }

    // Build the position string
    char boardStr[BOARDSIZE + 1];
    for (i = 0; i < BOARDSIZE; i++) {
        if (boardState[i] == EMPTY) boardStr[i] = '-';
        else if (boardState[i] == BLACK) boardStr[i] = 'B';
        else boardStr[i] = 'W';
    }
    boardStr[BOARDSIZE] = '\0';

    sprintf(positionStringBuffer, "%d_%s", (turn == BLACK) ? 1 : 2, boardStr);
}

/**
 * @brief Convert the input position string to
 * the in-game integer representation of the position.
 */
POSITION StringToPosition(char *positionString) {
    int turn;
    char *boardStr;
    if (ParseStandardOnelinePositionString(positionString, &turn, &boardStr)) {
        if (strlen(boardStr) != BOARDSIZE) return NULL_POSITION;
        POSITION position = 0;
        int i;
        for (i = 0; i < BOARDSIZE; i++) {
            char c = boardStr[i];
            int value;
            if (c == '-') value = EMPTY;
            else if (c == 'B') value = BLACK;
            else if (c == 'W') value = WHITE;
            else return NULL_POSITION;
            position = position * 3 + value;
        }
        position = position * 3 + ((turn == 1) ? BLACK : WHITE);
        return position;
    }
    return NULL_POSITION;
}

/**
 * @brief Write an AutoGUI-formatted position string for the given position 
 * (which tells the frontend application how to render the position) to the
 * input buffer.
 */
void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
    PositionToString(position, autoguiPositionStringBuffer);
}

/**
 * @brief Write an AutoGUI-formatted move string for the given move 
 * (which tells the frontend application how to render the move as button) to the
 * input buffer.
 */
void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
    (void)position;
    if (move < 10000) {
        int from = move / 100;
        int to = move % 100;
        AutoGUIMakeMoveButtonStringM(from, to, 'm', autoguiMoveStringBuffer);
    } else {
        int from = move / 10000;
        int to = move % 100;
        AutoGUIMakeMoveButtonStringM(from, to, 'x', autoguiMoveStringBuffer);
    }
}
