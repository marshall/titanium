/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Header file for the Virtual DataBase Engine (VDBE)
**
** This header defines the interface to the virtual database engine
** or VDBE.  The VDBE implements an abstract machine that runs a
** simple program to access and modify the underlying database.
**
** $Id: vdbe.h,v 1.110 2007/05/08 21:45:28 drh Exp $
*/
#ifndef _SQLITE_VDBE_H_
#define _SQLITE_VDBE_H_
#include <stdio.h>

/*
** A single VDBE is an opaque structure named "Vdbe".  Only routines
** in the source file sqliteVdbe.c are allowed to see the insides
** of this structure.
*/
typedef struct Vdbe Vdbe;

/*
** A single instruction of the virtual machine has an opcode
** and as many as three operands.  The instruction is recorded
** as an instance of the following structure:
*/
struct VdbeOp {
  u8 opcode;          /* What operation to perform */
  int p1;             /* First operand */
  int p2;             /* Second parameter (often the jump destination) */
  char *p3;           /* Third parameter */
  int p3type;         /* One of the P3_xxx constants defined below */
#ifdef VDBE_PROFILE
  int cnt;            /* Number of times this instruction was executed */
  long long cycles;   /* Total time spend executing this instruction */
#endif
};
typedef struct VdbeOp VdbeOp;

/*
** A smaller version of VdbeOp used for the VdbeAddOpList() function because
** it takes up less space.
*/
struct VdbeOpList {
  u8 opcode;          /* What operation to perform */
  signed char p1;     /* First operand */
  short int p2;       /* Second parameter (often the jump destination) */
  char *p3;           /* Third parameter */
};
typedef struct VdbeOpList VdbeOpList;

/*
** Allowed values of VdbeOp.p3type
*/
#define P3_NOTUSED    0   /* The P3 parameter is not used */
#define P3_DYNAMIC  (-1)  /* Pointer to a string obtained from sqliteMalloc() */
#define P3_STATIC   (-2)  /* Pointer to a static string */
#define P3_COLLSEQ  (-4)  /* P3 is a pointer to a CollSeq structure */
#define P3_FUNCDEF  (-5)  /* P3 is a pointer to a FuncDef structure */
#define P3_KEYINFO  (-6)  /* P3 is a pointer to a KeyInfo structure */
#define P3_VDBEFUNC (-7)  /* P3 is a pointer to a VdbeFunc structure */
#define P3_MEM      (-8)  /* P3 is a pointer to a Mem*    structure */
#define P3_TRANSIENT (-9) /* P3 is a pointer to a transient string */
#define P3_VTAB     (-10) /* P3 is a pointer to an sqlite3_vtab structure */
#define P3_MPRINTF  (-11) /* P3 is a string obtained from sqlite3_mprintf() */

/* When adding a P3 argument using P3_KEYINFO, a copy of the KeyInfo structure
** is made.  That copy is freed when the Vdbe is finalized.  But if the
** argument is P3_KEYINFO_HANDOFF, the passed in pointer is used.  It still
** gets freed when the Vdbe is finalized so it still should be obtained
** from a single sqliteMalloc().  But no copy is made and the calling
** function should *not* try to free the KeyInfo.
*/
#define P3_KEYINFO_HANDOFF (-9)

/*
** The Vdbe.aColName array contains 5n Mem structures, where n is the 
** number of columns of data returned by the statement.
*/
#define COLNAME_NAME     0
#define COLNAME_DECLTYPE 1
#define COLNAME_DATABASE 2
#define COLNAME_TABLE    3
#define COLNAME_COLUMN   4
#define COLNAME_N        5      /* Number of COLNAME_xxx symbols */

/*
** The following macro converts a relative address in the p2 field
** of a VdbeOp structure into a negative number so that 
** sqlite3VdbeAddOpList() knows that the address is relative.  Calling
** the macro again restores the address.
*/
#define ADDR(X)  (-1-(X))

/*
** The makefile scans the vdbe.c source file and creates the "opcodes.h"
** header file that defines a number for each opcode used by the VDBE.
*/
#include "opcodes.h"

/*
** Prototypes for the VDBE interface.  See comments on the implementation
** for a description of what each of these routines does.
*/
Vdbe *sqlite3VdbeCreate(sqlite3*);
int sqlite3VdbeAddOp(Vdbe*,int,int,int);
int sqlite3VdbeOp3(Vdbe*,int,int,int,const char *zP3,int);
int sqlite3VdbeAddOpList(Vdbe*, int nOp, VdbeOpList const *aOp);
void sqlite3VdbeChangeP1(Vdbe*, int addr, int P1);
void sqlite3VdbeChangeP2(Vdbe*, int addr, int P2);
void sqlite3VdbeJumpHere(Vdbe*, int addr);
void sqlite3VdbeChangeToNoop(Vdbe*, int addr, int N);
void sqlite3VdbeChangeP3(Vdbe*, int addr, const char *zP1, int N);
VdbeOp *sqlite3VdbeGetOp(Vdbe*, int);
int sqlite3VdbeMakeLabel(Vdbe*);
void sqlite3VdbeDelete(Vdbe*);
void sqlite3VdbeMakeReady(Vdbe*,int,int,int,int);
int sqlite3VdbeFinalize(Vdbe*);
void sqlite3VdbeResolveLabel(Vdbe*, int);
int sqlite3VdbeCurrentAddr(Vdbe*);
#ifdef SQLITE_DEBUG
  void sqlite3VdbeTrace(Vdbe*,FILE*);
#endif
void sqlite3VdbeResetStepResult(Vdbe*);
int sqlite3VdbeReset(Vdbe*);
void sqlite3VdbeSetNumCols(Vdbe*,int);
int sqlite3VdbeSetColName(Vdbe*, int, int, const char *, int);
void sqlite3VdbeCountChanges(Vdbe*);
sqlite3 *sqlite3VdbeDb(Vdbe*);
void sqlite3VdbeSetSql(Vdbe*, const char *z, int n);
const char *sqlite3VdbeGetSql(Vdbe*);
void sqlite3VdbeSwap(Vdbe*,Vdbe*);

#ifndef NDEBUG
  void sqlite3VdbeComment(Vdbe*, const char*, ...);
# define VdbeComment(X)  sqlite3VdbeComment X
#else
# define VdbeComment(X)
#endif

#endif
