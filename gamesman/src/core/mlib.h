#ifndef _MLIB_H_
#define _MLIB_H_

/*function prototypes*/

typedef enum possibleBoardPieces {
  Blank, o, x
} BlankOX;

void LibInitialize(int,int,int,BOOLEAN);
BOOLEAN NinaRow(void*,void*,int,int);
BOOLEAN statelessNinaRow(void*,void*,int);
BOOLEAN AmountOfWhat();
void printBoard(BlankOX*);
void Test();

BOOLEAN mymemcmp(void*,void*,int);

#endif