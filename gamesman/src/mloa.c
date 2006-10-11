/************************************************************************
**
** NAME:        NAME OF FILE
**
** DESCRIPTION: Lines of Action
**
** AUTHOR:      Albert Shau
**
** DATE:        Started 9/18/2006
**
** UPDATE HIST: 2006.09.18 Coded InitializeGame and PrintPosition
**                         without hashes
**              2006.09.19 Fixed PrintPosition
**              2006.09.25 Fixed InitializeGame and PrintPosition to work
**                         with generic hash
**              2006.09.26 Coded Primitive and created isConnected to help
**                         Coded GenerateMoves and its helper functions
**              2006.10.07 Coded DoMove and debugged all previous functions
**                         fixed piecesInLineOfAction, got rid of board
**                         within functions in favor of gBoard everywhere
**                         to get rid of seg faults, fixed Primitive since
**                         everything was undecided 
**                         
**
** LAST CHANGE: $Id: mloa.c,v 1.2 2006-10-08 05:14:59 alb_shau Exp $
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>


/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING   kGameName            = "Lines of Action";
STRING   kAuthorName          = "Albert Shau";
STRING   kDBName              = "loa"; /* The name to store the database under */

BOOLEAN  kPartizan            = TRUE ;
BOOLEAN  kGameSpecificMenu    = FALSE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = FALSE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = TRUE ;

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = FALSE ; /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

void*	 gGameSpecificTclInit = NULL;

/**
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 * These help strings should be updated and dynamically changed using 
 * InitializeHelpStrings() 
 **/

STRING   kHelpGraphicInterface =
"Help strings not initialized!";

STRING   kHelpTextInterface =
"Help strings not initialized!"; 

STRING   kHelpOnYourTurn =
"Help strings not initialized!";

STRING   kHelpStandardObjective =
"Help strings not initialized!";

STRING   kHelpReverseObjective =
"Help strings not initialized!";

STRING   kHelpTieOccursWhen =
"Help strings not initialized!";

STRING   kHelpExample =
"Help strings not initialized!";



/*************************************************************************
**
** #defines and structs
**ccbb
**************************************************************************/
#define SIDELENGTH 4
#define BOARDSIZE SIDELENGTH*SIDELENGTH                              

#define PLAYERBLACK 1                             
#define PLAYERWHITE 2

#define BLANK ' '
#define BLACK 'B'
#define WHITE 'W'

typedef enum possibleDirections {
  UP, UPRIGHT, RIGHT, DOWNRIGHT, DOWN, DOWNLEFT, LEFT, UPLEFT
} Direction;


/*************************************************************************
**
** Global Variables
**
*************************************************************************/

char* gBoard;
int goInDirection[] = {-SIDELENGTH, -SIDELENGTH+1, 1, SIDELENGTH+1, SIDELENGTH, 
			SIDELENGTH-1, -1, -SIDELENGTH-1, 0};


/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/
MOVELIST* addPieceMoves(int boardSquare, int PlayerTurn, MOVELIST* moves);
MOVELIST* addMovesInDirection(Direction direction, int boardSquare, int PlayerTurn, MOVELIST* moves);
int piecesInLineOfAction(Direction direction, int boardSquare);
BOOLEAN onEdge(Direction direction, int boardSquare);
BOOLEAN isConnected(int boardSquare1, int boardSquare2);
BOOLEAN pieceIsolated(int boardSquare);
Direction oppositeDirection(Direction direction);
POSITION calcInitialPosition();
int numPiecesLeft(char color);


/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();

STRING                  MoveToString(MOVE move);

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
** 
************************************************************************/

void InitializeGame ()
{
  //InitializeHelpStrings();
  int pieces[] = {BLANK, BOARDSIZE - 4*(SIDELENGTH-2), BOARDSIZE-2,
		  BLACK, 1, 2*(SIDELENGTH-2), WHITE, 1, 2*(SIDELENGTH-2), -1}; 
    
  gNumberOfPositions = generic_hash_init(BOARDSIZE, pieces, NULL);
  gBoard = (char*)SafeMalloc(sizeof(char) * (BOARDSIZE+1));
  gInitialPosition = calcInitialPosition();
}


/************************************************************************
**
** NAME:        InitializeHelpStrings
**
** DESCRIPTION: Sets up the help strings based on chosen game options.
**
** NOTES:       Should be called whenever the game options are changed.
**              (e.g., InitializeGame() and GameSpecificMenu())
**
************************************************************************/
void InitializeHelpStrings ()
{

kHelpGraphicInterface =
    "";

kHelpTextInterface =
   ""; 

kHelpOnYourTurn =
  "";

kHelpStandardObjective =
  "";

kHelpReverseObjective =
  ""; 

kHelpTieOccursWhen = 
  "A tie occurs when ...";

kHelpExample = 
  "";

    gMoveToStringFunPtr = &MoveToString;

}


/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Creates a linked list of every move that can be reached
**              from this position. Returns a pointer to the head of the
**              linked list.
** 
** INPUTS:      POSITION position : Current position to generate moves
**
** OUTPUTS:     (MOVELIST *)      : A pointer to the first item of
**                                  the linked list of generated moves
**
** CALLS:       MOVELIST *CreateMovelistNode();
**
************************************************************************/

MOVELIST *GenerateMoves (POSITION position)
{
  //printf("generateMoves starting... \n");
  int i;
  int playerTurn = whoseMove(position);
  char playerColor = (playerTurn == PLAYERBLACK ? BLACK : WHITE);
  MOVELIST *moves = NULL;
  generic_unhash(position, gBoard);

  for (i = 0; i < BOARDSIZE; i++) {
    if (gBoard[i] == playerColor)
      moves = addPieceMoves(i, playerTurn, moves);
  }

  return moves;
}


/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Applies the move to the position.
** 
** INPUTS:      POSITION position : The old position
**              MOVE     move     : The move to apply to the position
**
** OUTPUTS:     (POSITION)        : The position that results from move
**
** CALLS:       Some Board Hash Function
**              Some Board Unhash Function
**
*************************************************************************/

POSITION DoMove (POSITION position, MOVE move)
{

  //printf("doing move... \n");
  int start, end;
  int playerTurn = whoseMove(position);
  POSITION nextPosition;
  generic_unhash(position, gBoard);

 /* MOVE is just an int. 4 digits, the first two are the starting position
    and the second two are the ending position.  For example, 1234 means
    the piece at board[12] moves to board[34]. Something like 321 means
    board[3] moves to board[21].
 */
 end = move % 100;
 start = (move - end)/100;

 /* Move the piece */
 gBoard[end] = gBoard[start];
 gBoard[start] = BLANK;

 if (playerTurn == PLAYERBLACK)
   playerTurn = PLAYERWHITE;
 else
   playerTurn = PLAYERBLACK;
 
 nextPosition = generic_hash(gBoard, playerTurn);

 return nextPosition;
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Returns the value of a position if it fulfills certain
**              'primitive' constraints.
**
**              Example: Tic-tac-toe - Last piece already placed
**
**              Case                                  Return Value
**              *********************************************************
**              Current player sees three in a row    lose
**              Entire board filled                   tie
**              All other cases                       undecided
** 
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE)           : one of
**                                  (win, lose, tie, undecided)
**
** CALLS:       None              
**
************************************************************************/

VALUE Primitive (POSITION position)
{
  int i, j, blackChainLength, whiteChainLength, blackPiecesLeft, whitePiecesLeft;
  VALUE result;
  BOOLEAN allWhiteConnected, allBlackConnected;
  int playerTurn = whoseMove(position);
  generic_unhash(position, gBoard);

  blackPiecesLeft = numPiecesLeft(BLACK);
  whitePiecesLeft = numPiecesLeft(WHITE);
  int chainOfBlacks[blackPiecesLeft+1], chainOfWhites[whitePiecesLeft];

  //printf("primitive starting...\n");
  // chains keep track of the boardSquare of every piece connected to that
  // chain of a certain color.  Initialize so that it's full of -1's except
  // for chain[0], which is the first piece we see on the board 
  i = 0;
  //printf("initializing chains ... blacksleft = %d, whitesleft = %d \n", blackPiecesLeft, whitePiecesLeft);
  while (gBoard[i] != BLACK) { i++;}
  chainOfBlacks[0] = i;
  for (i = 1; i < blackPiecesLeft; i++) {
    chainOfBlacks[i] = -1;
  }
  blackChainLength = 1;
  i = 0;
  while (gBoard[i] != WHITE) { i++;}
  chainOfWhites[0] = i;
  for (i = 1; i < whitePiecesLeft; i++) {
    chainOfWhites[i] = -1;
  }
  whiteChainLength = 1;
  //printf("chains initialized \n");

  // Check if pieces are not connected. Assume they are all connected
  // until we find a piece that is isolated and not the only one left
  allWhiteConnected = TRUE;
  allBlackConnected = TRUE;
  for (i = 0; i < BOARDSIZE; i++) {
    if (gBoard[i] == BLACK && blackPiecesLeft > 1) {
      //printf("checking to see if square %d is connected to chain\n", i);
      j = 0;
      do {
	if (isConnected(i, chainOfBlacks[j])) {
	  //printf("connected to chain! square %d is connected to %d\n", i, chainOfBlacks[j]);
	  chainOfBlacks[blackChainLength] = i;
	  blackChainLength++;
	  break;
	}
	j++;
      } while (chainOfBlacks[j] != -1);
    }     
    else if (gBoard[i] == WHITE && whitePiecesLeft > 1) {
      j = 0;
      do {
	if (isConnected(i, chainOfWhites[j])) {
	  chainOfWhites[whiteChainLength] = i;
	  whiteChainLength++;
	  break;
	}
	j++;
      } while (chainOfWhites[j] != -1);
    }
  }

  /*
  printf("printing chainOfBlacks ... \n");
  for (i = 0; i < blackPiecesLeft; i++) {
    printf("| %d ", chainOfBlacks[i]);
  }
  printf("\n");
  printf("printing chainOfWhites ... \n");
  for (i = 0; i < whitePiecesLeft; i++) {
    printf("| %d ", chainOfWhites[i]);
  }
  */

  // could do this in the big loop above.  Do it if you have time
  for (i = 0; i < blackPiecesLeft; i++) {
    if (chainOfBlacks[i] == -1) {
      allBlackConnected = FALSE;
      break;
    }
  }
  for (i = 0; i < whitePiecesLeft; i++) {
    if (chainOfWhites[i] == -1) {
      allWhiteConnected = FALSE;
      break;
    }
  }

   
  if (allBlackConnected && !allWhiteConnected)
    result = win;
  else if (allWhiteConnected && !allBlackConnected)
    result = lose;
  else if (allWhiteConnected && allBlackConnected)
    {
      if (playerTurn == PLAYERBLACK)
	result = lose;
      else
	result = win;
    }
  else
    result = undecided;

  return result;
}


/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Prints the position in a pretty format, including the
**              prediction of the game's outcome.
** 
** INPUTS:      POSITION position    : The position to pretty print.
**              STRING   playersName : The name of the player.
**              BOOLEAN  usersTurn   : TRUE <==> it's a user's turn.
**
** CALLS:       Unhash()
**              GetPrediction()      : Returns the prediction of the game
**
************************************************************************/

void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn)
{
  int i;
  int j;
  generic_unhash(position, gBoard);

  printf("\n         ");
  for (i = 0; i < SIDELENGTH; i++) {
    printf("  %d   ", i+1);
  }
  printf("\n         ");
  for (i = 1; i < SIDELENGTH; i++) {
    printf("______");
  }
  printf("_____\n");

  for (i = 0; i < SIDELENGTH; i++) {
    printf("        |");
    for (j = 0; j < SIDELENGTH; j++) {
      printf("     |");
    }
    printf("\n");
    printf("      %c |", i+65);
    for (j = 0; j < SIDELENGTH; j++) {
      printf("  %c  |", gBoard[SIDELENGTH*i+j]);
    }
    printf("\n");
    printf("        |");
    for (j = 0; j < SIDELENGTH; j++) {
      printf("_____|");
    }
    printf("\n");
  }

}


/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely formats the computers move.
** 
** INPUTS:      MOVE    computersMove : The computer's move. 
**              STRING  computersName : The computer's name. 
**
************************************************************************/

void PrintComputersMove (MOVE computersMove, STRING computersName)
{
    
}


/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Prints the move in a nice format.
** 
** INPUTS:      MOVE move         : The move to print. 
**
************************************************************************/

void PrintMove (MOVE move)
{
    STRING str = MoveToString( move );
    printf( "%s", str );
    SafeFree( str );
}


/************************************************************************
**
** NAME:        MoveToString
**
** DESCRIPTION: Returns the move as a STRING
** 
** INPUTS:      MOVE *move         : The move to put into a string.
**
************************************************************************/

STRING MoveToString (MOVE move)
{
  STRING moveString = (STRING) SafeMalloc(sizeof(char) * 4);
  int startSquare, endSquare;
  char letter, number;

  endSquare = move % 100;
  startSquare = (move - endSquare)/100;
  number = (startSquare % SIDELENGTH) + 49;
  letter = 65 + startSquare / SIDELENGTH;
  moveString[0] = letter;
  moveString[1] = number;
  number = (endSquare % SIDELENGTH) + 49;
  letter = 65 + endSquare / SIDELENGTH;
  moveString[2] = letter;
  moveString[3] = number;
  
  return moveString;
}


/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: Finds out if the player wishes to undo, abort, or use
**              some other gamesman option. The gamesman core does
**              most of the work here. 
**
** INPUTS:      POSITION position    : Current position
**              MOVE     *move       : The move to fill with user's move. 
**              STRING   playersName : Current Player's Name
**
** OUTPUTS:     USERINPUT          : One of
**                                   (Undo, Abort, Continue)
**
** CALLS:       USERINPUT HandleDefaultTextInput(POSITION, MOVE*, STRING)
**                                 : Gamesman Core Input Handling
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName)
{
    USERINPUT input;
    USERINPUT HandleDefaultTextInput();
    
    for (;;) {
        /***********************************************************
         * CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
         ***********************************************************/
	printf("%8s's move [(undo)/(startingSquare endingSquare (ex: A2C2))] : ", playersName);
	
	input = HandleDefaultTextInput(position, move, playersName);
	
	if (input != Continue)
		return input;
    }

    /* NOTREACHED */
    return Continue;
}


/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Rudimentary check to check if input is in the move form
**              you are expecting. Does not check if it is a valid move.
**              Only checks if it fits the move form.
**
**              Reserved Input Characters - DO NOT USE THESE ONE CHARACTER
**                                          COMMANDS IN YOUR GAME
**              ?, s, u, r, h, a, c, q
**                                          However, something like a3
**                                          is okay.
** 
**              Example: Tic-tac-toe Move Format : Integer from 1 to 9
**                       Only integers between 1 to 9 are accepted
**                       regardless of board position.
**                       Moves will be checked by the core.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN      : TRUE if the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput (STRING input)
{
  /* input should be like A2C2 */
 
  if ((input[0] > 'A'-1 && input[0] < 'A' + SIDELENGTH) &&
      (input[1] < '1' + SIDELENGTH && input[1] > '0') &&
      (input[2] > 'A'-1 && input[0] < 'A' + SIDELENGTH) &&
      (input[3] < '1' + SIDELENGTH && input[1] > '0'))
    return TRUE;

  return FALSE;
}


/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Converts the string input your internal move representation.
**              Gamesman already checked the move with ValidTextInput
**              and ValidMove.
** 
** INPUTS:      STRING input : The VALID string input from the user.
**
** OUTPUTS:     MOVE         : Move converted from user input.
**
************************************************************************/

MOVE ConvertTextInputToMove (STRING input)
{
  MOVE move;
  int startSquare, endSquare;
  
  startSquare = input[1] - 49 + SIDELENGTH*(input[0] - 65);
  endSquare = input[3] - 49 + SIDELENGTH*(input[2] - 65);
  move = endSquare + startSquare*100;

  return move;
}


/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Prints, receives, and sets game-specific parameters.
**
**              Examples
**              Board Size, Board Type
**
**              If kGameSpecificMenu == FALSE
**                   Gamesman will not enable GameSpecificMenu
**                   Gamesman will not call this function
** 
**              Resets gNumberOfPositions if necessary
**
************************************************************************/

void GameSpecificMenu ()
{
  InitializeHelpStrings();
}


/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
** 
************************************************************************/

void SetTclCGameSpecificOptions (int options[])
{
    
}
  
  
/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Called when the user wishes to change the initial
**              position. Asks the user for an initial position.
**              Sets new user defined gInitialPosition and resets
**              gNumberOfPositions if necessary
** 
** OUTPUTS:     POSITION : New Initial Position
**
************************************************************************/

POSITION GetInitialPosition ()
{
    return gInitialPosition;
}


/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of variants
**              your game supports.
**
** OUTPUTS:     int : Number of Game Variants
**
************************************************************************/

int NumberOfOptions ()
{
    return 0;
}


/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: A hash function that returns a number corresponding
**              to the current variant of the game.
**              Each set of variants needs to have a different number.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption ()
{
    /* If you have implemented symmetries you should
       include the boolean variable gSymmetries in your
       hash */
    return 0;
}


/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The corresponding unhash function for game variants.
**              Unhashes option and sets the necessary variants.
**
** INPUT:       int option : the number representation of the options.
**
************************************************************************/

void setOption (int option)
{
    /* If you have implemented symmetries you should
       include the boolean variable gSymmetries in your
       hash */
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Game Specific Debug Menu (Gamesman comes with a default
**              debug menu). Menu used to debug internal problems.
**
**              If kDebugMenu == FALSE
**                   Gamesman will not display a debug menu option
**                   Gamesman will not call this function
** 
************************************************************************/

void DebugMenu ()
{
    
}


/************************************************************************
**
** Everything specific to this module goes below these lines.
**
** Things you want down here:
** Move Hasher
** Move Unhasher
** Any other function you deem necessary to help the ones above.
** 
************************************************************************/

/************************************************************************
 ** returns TRUE if boardSquare2 is connected to boardSquare1.  Diagonals
 ** count as connected. FALSE if they're the same. Called by Primitive
 ***********************************************************************/
BOOLEAN isConnected(int boardSquare1, int boardSquare2)
{
  Direction d;

  for (d = UP; d <= UPLEFT; d++) {
    if ((!onEdge(d, boardSquare1)) &&
	(boardSquare1 + goInDirection[d] == boardSquare2))
      return TRUE;
  }

  return FALSE;
}

/************************************************************************
 ** this function returns TRUE is the piece specified has no other pieces
 ** of the same color touching it in any direction. diagonals included
 ***********************************************************************/
BOOLEAN pieceIsolated(int boardSquare)
{
  Direction d;
  
  for (d = UP; d <= UPLEFT; d++) {
    if ((!onEdge(d, boardSquare)) && (gBoard[boardSquare + goInDirection[d]] == gBoard[boardSquare])) {
      return FALSE;
    }
  }
  return TRUE;
}

/************************************************************************
 ** adds moves to the movelist given a square on the board that has a 
 ** piece on it. Called by GenerateMoves so assume position has been
 ** unhashed and is stored correctly in gPosition.
 ***********************************************************************/
MOVELIST* addPieceMoves(int boardSquare, int playerTurn, MOVELIST* moves)
{
  //printf("starting addPieceMoves...\n");
  Direction d;

  for (d = UP; d <= UPLEFT; d++) {
    //printf(" checking to add move in direction = %d from boardSquare = %d\n", d, boardSquare);
    moves = addMovesInDirection(d, boardSquare, playerTurn, moves);
  }
  return moves;
}

/************************************************************************
 ** Adds moves to the movelist when given the square that a piece is on
 ** and the direction you're looking in. Called by addPieceMoves
 ***********************************************************************/
MOVELIST* addMovesInDirection(Direction direction, int boardSquare, int playerTurn, MOVELIST* moves) 
{
  int moveLength, startSquare, endSquare, i;
  char otherPlayerColor = (playerTurn == PLAYERBLACK ? WHITE : BLACK);
  //printf("starting addMovesInDirection...\n");
  

  moveLength = piecesInLineOfAction(direction, boardSquare);
  startSquare = boardSquare;
  endSquare = boardSquare + moveLength*goInDirection[direction];
  i = 0;
    
  /* go in that direction one step at a time.  If you hit an edge or a
     piece of the opposite color, you cannot move in this direction */
  while (i < moveLength && !onEdge(direction, boardSquare))
    {
      //printf("   i = % d, moveLength = %d, \n", i, moveLength);
      boardSquare += goInDirection[direction];
      if (gBoard[boardSquare] != otherPlayerColor)
	i++;
      else
	break;
    }
  if (boardSquare == endSquare)
    {
      //printf("direction = %d, startSquare = %d, moveLength = %d \n", direction, startSquare, moveLength);
      moves = CreateMovelistNode(startSquare*100 + endSquare, moves);
    }

  return moves;
}

/************************************************************************
 ** Returns the number of pieces in a specific line of action, which is
 ** specified by the direction.  Note that it's line of action and not
 ** direction.  This means this function will return the same thing
 ** if the direction is RIGHT or LEFT, Up or DOWN, UpLEFT or DOWNRIGHT,
 ** UPRIGHT or DOWNLEFT. 
 ***********************************************************************/
int piecesInLineOfAction(Direction direction, int boardSquare)
{
  int piecesSoFar = 1;
  int tempBoardSquare = boardSquare;

  /* Go opposite direction until you hit an edge */
  while (!onEdge(oppositeDirection(direction), tempBoardSquare)) {
    tempBoardSquare += goInDirection[oppositeDirection(direction)];
    if (gBoard[tempBoardSquare] != BLANK)
      piecesSoFar++;
  }
  /* Go in direction, counting all pieces as you go */
  while (!onEdge(direction, boardSquare)) {
    boardSquare += goInDirection[direction];
    if (gBoard[boardSquare] != BLANK)
      piecesSoFar++;
  }


  return piecesSoFar;
}

/*************************************************************************
 ** I'm defining an Edge a bit differently than normal.  An edge depends 
 ** on which direction you're looking at.  For example, if you're looking
 ** right and you're on the top most row, you're still not on an edge.  
 ** You're only on an edge if you cannot go any further in the specified
 ** direction.  If you're going right, you're only on an edge if you're
 ** on the rightmost column of the board.
 ************************************************************************/
BOOLEAN onEdge(Direction direction, int boardSquare)
{
  switch (direction) 
    {
    case UP:
      if (boardSquare >= 0 && boardSquare < SIDELENGTH)
	return TRUE;
      else
	return FALSE;
    case UPRIGHT:
      if ((boardSquare >= 0 && boardSquare < SIDELENGTH) ||
	  (boardSquare % SIDELENGTH == SIDELENGTH-1))
	return TRUE;
      else
	return FALSE;
    case RIGHT:
      if (boardSquare % SIDELENGTH == SIDELENGTH-1)
	return TRUE;
      else
	return FALSE;
    case DOWNRIGHT:
      if ((boardSquare % SIDELENGTH == SIDELENGTH-1) ||
	  (boardSquare >= BOARDSIZE-SIDELENGTH && boardSquare < BOARDSIZE))
	return TRUE;
      else
	return FALSE;
    case DOWN:
      if (boardSquare >= BOARDSIZE-SIDELENGTH && boardSquare < BOARDSIZE)
	return TRUE;
      else
	return FALSE;
    case DOWNLEFT:
      if ((boardSquare >= BOARDSIZE-SIDELENGTH && boardSquare < BOARDSIZE) ||
	  (boardSquare % SIDELENGTH == 0))
	return TRUE;
      else
	return FALSE;
    case LEFT:
      if (boardSquare % SIDELENGTH == 0)
	return TRUE;
      else 
	return FALSE;
    case UPLEFT:
      if ((boardSquare >= 0 && boardSquare < SIDELENGTH) ||
	  (boardSquare % SIDELENGTH == 0))
	return TRUE;
      else
	return FALSE;
    }      
  
  /* This return should never happen but I put this in to get rid of a warning */
  return FALSE;
}

Direction oppositeDirection(Direction direction)
{
  if (direction == UP)
    return DOWN;
  else if (direction == DOWN)
    return UP;
  else if (direction == RIGHT)
    return LEFT;
  else if (direction == LEFT)
    return RIGHT;
  else if (direction == UPRIGHT)
    return DOWNLEFT;
  else if (direction == DOWNLEFT)
    return UPRIGHT;
  else if (direction == UPLEFT)
    return DOWNRIGHT;
  else  //(direction == DOWNRIGHT)
    return UPLEFT;
}


/*************************************************************************
 ** returns initial position using BOARDSIZE and SIDELENGTH.  Called from
 ** InitializeGame().
 ************************************************************************/
POSITION calcInitialPosition()
{
  int i;
  POSITION initialPos = 0;
  char* board = (char*)SafeMalloc(sizeof(char) * (BOARDSIZE+1));

  for (i = 0; i < BOARDSIZE; i++) {
    if ((i > 0 && i < SIDELENGTH-1) || (i < BOARDSIZE-1 && i > BOARDSIZE-SIDELENGTH))
      board[i] = BLACK;
    else if ((i % SIDELENGTH == 0) || (i % SIDELENGTH == SIDELENGTH-1))
      board[i] = WHITE;
    else
      board[i] = BLANK;
  }
  board[0] = BLANK;
  board[SIDELENGTH-1] = BLANK;
  board[BOARDSIZE-SIDELENGTH] = BLANK;
  board[BOARDSIZE-1] = BLANK;
  board[BOARDSIZE] = '\0';  

  initialPos = generic_hash(board, PLAYERBLACK);

  SafeFree(board);
  return initialPos;
}

/***************************************************************************
 ** Given a position and a color, returns the number of pieces of that same
 ** color on the board. Only called by Primitive so assume gBoard is correct
 **************************************************************************/
int numPiecesLeft(char color)
{
  int i, piecesSoFar;
  piecesSoFar = 0;

  for (i = 0; i < BOARDSIZE; i++) {
    if (gBoard[i] == color)
      piecesSoFar++;
  }

  return piecesSoFar;
}


/************************************************************************
 ** Changelog
 **
 ** $Log: not supported by cvs2svn $
 ** Revision 1.1  2006/09/26 23:01:28  alb_shau
 ** *** empty log message ***
 **
 ** 
 **
 ************************************************************************/
