#ifndef GMCORE_INTERACT_H
#define GMCORE_INTERACT_H
#include "types.h"
#include "constants.h"
#include "misc.h"
#include "openPositions.h"
#include "globals.h"
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

enum UWAPI_TURN {
	UWAPI_TURN_A = 'A',
	UWAPI_TURN_B = 'B'
};

STRING InteractReadPosition(STRING input, POSITION * result);
STRING InteractReadLong(STRING input, long * result);
STRING InteractReadBoardString(STRING input, char ** result);
STRING InteractValueCharToValueString(char value_char);
void InteractPrintJSONPositionValue(POSITION pos);
void InteractFreeBoardSting(STRING board);
void InteractCheckErrantExtra(STRING input, int max_words);
void ServerInteractLoop(void);
extern POSITION gInitialPosition;

char * MakeBoardString(char * first, ...);

BOOLEAN UWAPI_Regular2D_ParsePositionString(
	char const * str,
	enum UWAPI_TURN * turn,
	unsigned int * num_rows,
	unsigned int * num_columns,
	char ** board);
char * UWAPI_Regular2D_MakePositionString(
	enum UWAPI_TURN turn,
	unsigned int num_rows,
	unsigned int num_columns,
	char const * board);
char * UWAPI_Regular2D_MakeAddString(char piece, unsigned int to);
char * UWAPI_Regular2D_MakeMoveString(char piece, unsigned int to);

char * StringDup( char const * s );
void SafeFreeString(char * string);
char * StrFromI( long long i );
char * TierstringFromPosition(POSITION pos);
char * StringFormat(size_t max_size, char * format_str, ...);

typedef BOOLEAN (* get_value_func_t)( char *, void *);

#define GetValue(board_string, key, func, target) \
	(0 ? func("", target) : GetValueInner(board_string, key, (get_value_func_t) func, (void *) target))

BOOLEAN GetValueInner(char * board_string, char * key, get_value_func_t func, void * target);

BOOLEAN GetInt(char* value, int* placeholder);
BOOLEAN GetUnsignedLongLong(char* value, unsigned long long* placeholder);
BOOLEAN GetChar(char* value, char* placeholder);
void PrintLevel(POSITION pos, unsigned int depth, char * move_string);

#endif /* GMCORE_INTERACT_H */
