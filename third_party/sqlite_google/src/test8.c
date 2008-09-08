/*
** 2006 June 10
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Code for testing the virtual table interfaces.  This code
** is not included in the SQLite library.  It is used for automated
** testing of the SQLite library.
**
** $Id: test8.c,v 1.46 2007/04/18 17:04:01 danielk1977 Exp $
*/
#include "sqliteInt.h"
#include "tcl.h"
#include "os.h"
#include <stdlib.h>
#include <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

typedef struct echo_vtab echo_vtab;
typedef struct echo_cursor echo_cursor;

/*
** The test module defined in this file uses four global Tcl variables to
** commicate with test-scripts:
**
**     $::echo_module
**     $::echo_module_sync_fail
**     $::echo_module_begin_fail
**     $::echo_module_cost
**
** The variable ::echo_module is a list. Each time one of the following
** methods is called, one or more elements are appended to the list.
** This is used for automated testing of virtual table modules.
**
** The ::echo_module_sync_fail variable is set by test scripts and read
** by code in this file. If it is set to the name of a real table in the
** the database, then all xSync operations on echo virtual tables that
** use the named table as a backing store will fail.
*/

/* 
** An echo virtual-table object.
**
** echo.vtab.aIndex is an array of booleans. The nth entry is true if 
** the nth column of the real table is the left-most column of an index
** (implicit or otherwise). In other words, if SQLite can optimize
** a query like "SELECT * FROM real_table WHERE col = ?".
**
** Member variable aCol[] contains copies of the column names of the real
** table.
*/
struct echo_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;     /* Tcl interpreter containing debug variables */
  sqlite3 *db;            /* Database connection */

  char *zTableName;       /* Name of the real table */
  char *zLogName;         /* Name of the log table */
  int nCol;               /* Number of columns in the real table */
  int *aIndex;            /* Array of size nCol. True if column has an index */
  char **aCol;            /* Array of size nCol. Column names */
};

/* An echo cursor object */
struct echo_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
};

/*
** Retrieve the column names for the table named zTab via database
** connection db. SQLITE_OK is returned on success, or an sqlite error
** code otherwise.
**
** If successful, the number of columns is written to *pnCol. *paCol is
** set to point at sqliteMalloc()'d space containing the array of
** nCol column names. The caller is responsible for calling sqliteFree
** on *paCol.
*/
static int getColumnNames(
  sqlite3 *db, 
  const char *zTab,
  char ***paCol, 
  int *pnCol
){
  char **aCol = 0;
  char *zSql;
  sqlite3_stmt *pStmt = 0;
  int rc = SQLITE_OK;
  int nCol = 0;

  /* Prepare the statement "SELECT * FROM <tbl>". The column names
  ** of the result set of the compiled SELECT will be the same as
  ** the column names of table <tbl>.
  */
  zSql = sqlite3MPrintf("SELECT * FROM %Q", zTab);
  if( !zSql ){
    rc = SQLITE_NOMEM;
    goto out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqliteFree(zSql);

  if( rc==SQLITE_OK ){
    int ii;
    int nBytes;
    char *zSpace;
    nCol = sqlite3_column_count(pStmt);

    /* Figure out how much space to allocate for the array of column names 
    ** (including space for the strings themselves). Then allocate it.
    */
    nBytes = sizeof(char *) * nCol;
    for(ii=0; ii<nCol; ii++){
      nBytes += (strlen(sqlite3_column_name(pStmt, ii)) + 1);
    }
    aCol = (char **)sqliteMalloc(nBytes);
    if( !aCol ){
      rc = SQLITE_NOMEM;
      goto out;
    }

    /* Copy the column names into the allocated space and set up the
    ** pointers in the aCol[] array.
    */
    zSpace = (char *)(&aCol[nCol]);
    for(ii=0; ii<nCol; ii++){
      aCol[ii] = zSpace;
      zSpace += sprintf(zSpace, "%s", sqlite3_column_name(pStmt, ii));
      zSpace++;
    }
    assert( (zSpace-nBytes)==(char *)aCol );
  }

  *paCol = aCol;
  *pnCol = nCol;

out:
  sqlite3_finalize(pStmt);
  return rc;
}

/*
** Parameter zTab is the name of a table in database db with nCol 
** columns. This function allocates an array of integers nCol in 
** size and populates it according to any implicit or explicit 
** indices on table zTab.
**
** If successful, SQLITE_OK is returned and *paIndex set to point 
** at the allocated array. Otherwise, an error code is returned.
**
** See comments associated with the member variable aIndex above 
** "struct echo_vtab" for details of the contents of the array.
*/
static int getIndexArray(
  sqlite3 *db,             /* Database connection */
  const char *zTab,        /* Name of table in database db */
  int nCol,
  int **paIndex
){
  sqlite3_stmt *pStmt = 0;
  int *aIndex = 0;
  int rc;
  char *zSql;

  /* Allocate space for the index array */
  aIndex = (int *)sqliteMalloc(sizeof(int) * nCol);
  if( !aIndex ){
    rc = SQLITE_NOMEM;
    goto get_index_array_out;
  }

  /* Compile an sqlite pragma to loop through all indices on table zTab */
  zSql = sqlite3MPrintf("PRAGMA index_list(%s)", zTab);
  if( !zSql ){
    rc = SQLITE_NOMEM;
    goto get_index_array_out;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqliteFree(zSql);

  /* For each index, figure out the left-most column and set the 
  ** corresponding entry in aIndex[] to 1.
  */
  while( pStmt && sqlite3_step(pStmt)==SQLITE_ROW ){
    const char *zIdx = (const char *)sqlite3_column_text(pStmt, 1);
    sqlite3_stmt *pStmt2 = 0;
    zSql = sqlite3MPrintf("PRAGMA index_info(%s)", zIdx);
    if( !zSql ){
      rc = SQLITE_NOMEM;
      goto get_index_array_out;
    }
    rc = sqlite3_prepare(db, zSql, -1, &pStmt2, 0);
    sqliteFree(zSql);
    if( pStmt2 && sqlite3_step(pStmt2)==SQLITE_ROW ){
      int cid = sqlite3_column_int(pStmt2, 1);
      assert( cid>=0 && cid<nCol );
      aIndex[cid] = 1;
    }
    if( pStmt2 ){
      rc = sqlite3_finalize(pStmt2);
    }
    if( rc!=SQLITE_OK ){
      goto get_index_array_out;
    }
  }


get_index_array_out:
  if( pStmt ){
    int rc2 = sqlite3_finalize(pStmt);
    if( rc==SQLITE_OK ){
      rc = rc2;
    }
  }
  if( rc!=SQLITE_OK ){
    sqliteFree(aIndex);
    aIndex = 0;
  }
  *paIndex = aIndex;
  return rc;
}

/*
** Global Tcl variable $echo_module is a list. This routine appends
** the string element zArg to that list in interpreter interp.
*/
static void appendToEchoModule(Tcl_Interp *interp, const char *zArg){
  int flags = (TCL_APPEND_VALUE | TCL_LIST_ELEMENT | TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp, "echo_module", (zArg?zArg:""), flags);
}

/*
** This function is called from within the echo-modules xCreate and
** xConnect methods. The argc and argv arguments are copies of those 
** passed to the calling method. This function is responsible for
** calling sqlite3_declare_vtab() to declare the schema of the virtual
** table being created or connected.
**
** If the constructor was passed just one argument, i.e.:
**
**   CREATE TABLE t1 AS echo(t2);
**
** Then t2 is assumed to be the name of a *real* database table. The
** schema of the virtual table is declared by passing a copy of the 
** CREATE TABLE statement for the real table to sqlite3_declare_vtab().
** Hence, the virtual table should have exactly the same column names and 
** types as the real table.
*/
static int echoDeclareVtab(
  echo_vtab *pVtab, 
  sqlite3 *db, 
  int argc, 
  const char *const*argv
){
  int rc = SQLITE_OK;

  if( argc>=4 ){
    sqlite3_stmt *pStmt = 0;
    sqlite3_prepare(db, 
        "SELECT sql FROM sqlite_master WHERE type = 'table' AND name = ?",
        -1, &pStmt, 0);
    sqlite3_bind_text(pStmt, 1, argv[3], -1, 0);
    if( sqlite3_step(pStmt)==SQLITE_ROW ){
      int rc2;
      const char *zCreateTable = (const char *)sqlite3_column_text(pStmt, 0);
      rc = sqlite3_declare_vtab(db, zCreateTable);
      rc2 = sqlite3_finalize(pStmt);
      if( rc==SQLITE_OK ){
        rc = rc2;
      }
    } else {
      rc = sqlite3_finalize(pStmt);
      if( rc==SQLITE_OK ){ 
        rc = SQLITE_ERROR;
      }
    }

    if( rc==SQLITE_OK ){
      rc = getColumnNames(db, argv[3], &pVtab->aCol, &pVtab->nCol);
    }
    if( rc==SQLITE_OK ){
      rc = getIndexArray(db, argv[3], pVtab->nCol, &pVtab->aIndex);
    }
  }

  return rc;
}

/*
** This function frees all runtime structures associated with the virtual
** table pVtab.
*/
static int echoDestructor(sqlite3_vtab *pVtab){
  echo_vtab *p = (echo_vtab*)pVtab;
  sqliteFree(p->aIndex);
  sqliteFree(p->aCol);
  sqliteFree(p->zTableName);
  sqliteFree(p->zLogName);
  sqliteFree(p);
  return 0;
}

/*
** This function is called to do the work of the xConnect() method -
** to allocate the required in-memory structures for a newly connected
** virtual table.
*/
static int echoConstructor(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  int i;
  echo_vtab *pVtab;

  /* Allocate the sqlite3_vtab/echo_vtab structure itself */
  pVtab = sqliteMalloc( sizeof(*pVtab) );
  if( !pVtab ){
    return SQLITE_NOMEM;
  }
  pVtab->interp = (Tcl_Interp *)pAux;
  pVtab->db = db;

  /* Allocate echo_vtab.zTableName */
  pVtab->zTableName = sqlite3MPrintf("%s", argv[3]);
  if( !pVtab->zTableName ){
    echoDestructor((sqlite3_vtab *)pVtab);
    return SQLITE_NOMEM;
  }

  /* Log the arguments to this function to Tcl var ::echo_module */
  for(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, argv[i]);
  }

  /* Invoke sqlite3_declare_vtab and set up other members of the echo_vtab
  ** structure. If an error occurs, delete the sqlite3_vtab structure and
  ** return an error code.
  */
  if( echoDeclareVtab(pVtab, db, argc, argv) ){
    echoDestructor((sqlite3_vtab *)pVtab);
    return SQLITE_ERROR;
  }

  /* Success. Set *ppVtab and return */
  *ppVtab = &pVtab->base;
  return SQLITE_OK;
}

/* 
** Echo virtual table module xCreate method.
*/
static int echoCreate(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  int rc = SQLITE_OK;
  appendToEchoModule((Tcl_Interp *)(pAux), "xCreate");
  rc = echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);

  /* If there were two arguments passed to the module at the SQL level 
  ** (i.e. "CREATE VIRTUAL TABLE tbl USING echo(arg1, arg2)"), then 
  ** the second argument is used as a table name. Attempt to create
  ** such a table with a single column, "logmsg". This table will
  ** be used to log calls to the xUpdate method. It will be deleted
  ** when the virtual table is DROPed.
  **
  ** Note: The main point of this is to test that we can drop tables
  ** from within an xDestroy method call.
  */
  if( rc==SQLITE_OK && argc==5 ){
    char *zSql;
    echo_vtab *pVtab = *(echo_vtab **)ppVtab;
    pVtab->zLogName = sqlite3MPrintf("%s", argv[4]);
    zSql = sqlite3MPrintf("CREATE TABLE %Q(logmsg)", pVtab->zLogName);
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqliteFree(zSql);
  }

  return rc;
}

/* 
** Echo virtual table module xConnect method.
*/
static int echoConnect(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  appendToEchoModule((Tcl_Interp *)(pAux), "xConnect");
  return echoConstructor(db, pAux, argc, argv, ppVtab, pzErr);
}

/* 
** Echo virtual table module xDisconnect method.
*/
static int echoDisconnect(sqlite3_vtab *pVtab){
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDisconnect");
  return echoDestructor(pVtab);
}

/* 
** Echo virtual table module xDestroy method.
*/
static int echoDestroy(sqlite3_vtab *pVtab){
  int rc = SQLITE_OK;
  echo_vtab *p = (echo_vtab *)pVtab;
  appendToEchoModule(((echo_vtab *)pVtab)->interp, "xDestroy");

  /* Drop the "log" table, if one exists (see echoCreate() for details) */
  if( p && p->zLogName ){
    char *zSql;
    zSql = sqlite3MPrintf("DROP TABLE %Q", p->zLogName);
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqliteFree(zSql);
  }

  if( rc==SQLITE_OK ){
    rc = echoDestructor(pVtab);
  }
  return rc;
}

/* 
** Echo virtual table module xOpen method.
*/
static int echoOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  echo_cursor *pCur;
  pCur = sqliteMalloc(sizeof(echo_cursor));
  *ppCursor = (sqlite3_vtab_cursor *)pCur;
  return (pCur ? SQLITE_OK : SQLITE_NOMEM);
}

/* 
** Echo virtual table module xClose method.
*/
static int echoClose(sqlite3_vtab_cursor *cur){
  int rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  sqlite3_stmt *pStmt = pCur->pStmt;
  pCur->pStmt = 0;
  sqliteFree(pCur);
  rc = sqlite3_finalize(pStmt);
  return rc;
}

/*
** Return non-zero if the cursor does not currently point to a valid record
** (i.e if the scan has finished), or zero otherwise.
*/
static int echoEof(sqlite3_vtab_cursor *cur){
  return (((echo_cursor *)cur)->pStmt ? 0 : 1);
}

/* 
** Echo virtual table module xNext method.
*/
static int echoNext(sqlite3_vtab_cursor *cur){
  int rc;
  echo_cursor *pCur = (echo_cursor *)cur;
  rc = sqlite3_step(pCur->pStmt);

  if( rc==SQLITE_ROW ){
    rc = SQLITE_OK;
  }else{
    rc = sqlite3_finalize(pCur->pStmt);
    pCur->pStmt = 0;
  }

  return rc;
}

/* 
** Echo virtual table module xColumn method.
*/
static int echoColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, int i){
  int iCol = i + 1;
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;
  if( !pStmt ){
    sqlite3_result_null(ctx);
  }else{
    assert( sqlite3_data_count(pStmt)>iCol );
    sqlite3_result_value(ctx, sqlite3_column_value(pStmt, iCol));
  }
  return SQLITE_OK;
}

/* 
** Echo virtual table module xRowid method.
*/
static int echoRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  sqlite3_stmt *pStmt = ((echo_cursor *)cur)->pStmt;
  *pRowid = sqlite3_column_int64(pStmt, 0);
  return SQLITE_OK;
}

/*
** Compute a simple hash of the null terminated string zString.
**
** This module uses only sqlite3_index_info.idxStr, not 
** sqlite3_index_info.idxNum. So to test idxNum, when idxStr is set
** in echoBestIndex(), idxNum is set to the corresponding hash value.
** In echoFilter(), code assert()s that the supplied idxNum value is
** indeed the hash of the supplied idxStr.
*/
static int hashString(const char *zString){
  int val = 0;
  int ii;
  for(ii=0; zString[ii]; ii++){
    val = (val << 3) + (int)zString[ii];
  }
  return val;
}

/* 
** Echo virtual table module xFilter method.
*/
static int echoFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  int idxNum, const char *idxStr,
  int argc, sqlite3_value **argv
){
  int rc;
  int i;

  echo_cursor *pCur = (echo_cursor *)pVtabCursor;
  echo_vtab *pVtab = (echo_vtab *)pVtabCursor->pVtab;
  sqlite3 *db = pVtab->db;

  /* Check that idxNum matches idxStr */
  assert( idxNum==hashString(idxStr) );

  /* Log arguments to the ::echo_module Tcl variable */
  appendToEchoModule(pVtab->interp, "xFilter");
  appendToEchoModule(pVtab->interp, idxStr);
  for(i=0; i<argc; i++){
    appendToEchoModule(pVtab->interp, (const char*)sqlite3_value_text(argv[i]));
  }

  sqlite3_finalize(pCur->pStmt);
  pCur->pStmt = 0;

  /* Prepare the SQL statement created by echoBestIndex and bind the
  ** runtime parameters passed to this function to it.
  */
  rc = sqlite3_prepare(db, idxStr, -1, &pCur->pStmt, 0);
  assert( pCur->pStmt || rc!=SQLITE_OK );
  for(i=0; rc==SQLITE_OK && i<argc; i++){
    sqlite3_bind_value(pCur->pStmt, i+1, argv[i]);
  }

  /* If everything was successful, advance to the first row of the scan */
  if( rc==SQLITE_OK ){
    rc = echoNext(pVtabCursor);
  }

  return rc;
}


/*
** A helper function used by echoUpdate() and echoBestIndex() for
** manipulating strings in concert with the sqlite3_mprintf() function.
**
** Parameter pzStr points to a pointer to a string allocated with
** sqlite3_mprintf. The second parameter, zAppend, points to another
** string. The two strings are concatenated together and *pzStr
** set to point at the result. The initial buffer pointed to by *pzStr
** is deallocated via sqlite3_free().
**
** If the third argument, doFree, is true, then sqlite3_free() is
** also called to free the buffer pointed to by zAppend.
*/
static void string_concat(char **pzStr, char *zAppend, int doFree){
  char *zIn = *pzStr;
  if( zIn ){
    char *zTemp = zIn;
    zIn = sqlite3_mprintf("%s%s", zIn, zAppend);
    sqlite3_free(zTemp);
  }else{
    zIn = sqlite3_mprintf("%s", zAppend);
  }
  *pzStr = zIn;
  if( doFree ){
    sqlite3_free(zAppend);
  }
}

/*
** The echo module implements the subset of query constraints and sort
** orders that may take advantage of SQLite indices on the underlying
** real table. For example, if the real table is declared as:
**
**     CREATE TABLE real(a, b, c);
**     CREATE INDEX real_index ON real(b);
**
** then the echo module handles WHERE or ORDER BY clauses that refer
** to the column "b", but not "a" or "c". If a multi-column index is
** present, only it's left most column is considered. 
**
** This xBestIndex method encodes the proposed search strategy as
** an SQL query on the real table underlying the virtual echo module 
** table and stores the query in sqlite3_index_info.idxStr. The SQL
** statement is of the form:
**
**   SELECT rowid, * FROM <real-table> ?<where-clause>? ?<order-by-clause>?
**
** where the <where-clause> and <order-by-clause> are determined
** by the contents of the structure pointed to by the pIdxInfo argument.
*/
static int echoBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  int ii;
  char *zQuery = 0;
  char *zNew;
  int nArg = 0;
  const char *zSep = "WHERE";
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3_stmt *pStmt = 0;
  Tcl_Interp *interp = pVtab->interp;

  int nRow;
  int useIdx = 0;
  int rc = SQLITE_OK;
  int useCost = 0;
  double cost;

  /* Determine the number of rows in the table and store this value in local
  ** variable nRow. The 'estimated-cost' of the scan will be the number of
  ** rows in the table for a linear scan, or the log (base 2) of the 
  ** number of rows if the proposed scan uses an index.  
  */
  if( Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY) ){
    cost = atof(Tcl_GetVar(interp, "echo_module_cost", TCL_GLOBAL_ONLY));
    useCost = 1;
  } else {
    zQuery = sqlite3_mprintf("SELECT count(*) FROM %Q", pVtab->zTableName);
    rc = sqlite3_prepare(pVtab->db, zQuery, -1, &pStmt, 0);
    sqlite3_free(zQuery);
    if( rc!=SQLITE_OK ){
      return rc;
    }
    sqlite3_step(pStmt);
    nRow = sqlite3_column_int(pStmt, 0);
    rc = sqlite3_finalize(pStmt);
    if( rc!=SQLITE_OK ){
      return rc;
    }
  }

  zQuery = sqlite3_mprintf("SELECT rowid, * FROM %Q", pVtab->zTableName);
  for(ii=0; ii<pIdxInfo->nConstraint; ii++){
    const struct sqlite3_index_constraint *pConstraint;
    struct sqlite3_index_constraint_usage *pUsage;
    int iCol;

    pConstraint = &pIdxInfo->aConstraint[ii];
    pUsage = &pIdxInfo->aConstraintUsage[ii];

    iCol = pConstraint->iColumn;
    if( pVtab->aIndex[iCol] ){
      char *zCol = pVtab->aCol[iCol];
      char *zOp = 0;
      useIdx = 1;
      if( iCol<0 ){
        zCol = "rowid";
      }
      switch( pConstraint->op ){
        case SQLITE_INDEX_CONSTRAINT_EQ:
          zOp = "="; break;
        case SQLITE_INDEX_CONSTRAINT_LT:
          zOp = "<"; break;
        case SQLITE_INDEX_CONSTRAINT_GT:
          zOp = ">"; break;
        case SQLITE_INDEX_CONSTRAINT_LE:
          zOp = "<="; break;
        case SQLITE_INDEX_CONSTRAINT_GE:
          zOp = ">="; break;
        case SQLITE_INDEX_CONSTRAINT_MATCH:
          zOp = "LIKE"; break;
      }
      if( zOp[0]=='L' ){
        zNew = sqlite3_mprintf(" %s %s LIKE (SELECT '%%'||?||'%%')", 
                               zSep, zCol);
      } else {
        zNew = sqlite3_mprintf(" %s %s %s ?", zSep, zCol, zOp);
      }
      string_concat(&zQuery, zNew, 1);

      zSep = "AND";
      pUsage->argvIndex = ++nArg;
      pUsage->omit = 1;
    }
  }

  /* If there is only one term in the ORDER BY clause, and it is
  ** on a column that this virtual table has an index for, then consume 
  ** the ORDER BY clause.
  */
  if( pIdxInfo->nOrderBy==1 && pVtab->aIndex[pIdxInfo->aOrderBy->iColumn] ){
    int iCol = pIdxInfo->aOrderBy->iColumn;
    char *zCol = pVtab->aCol[iCol];
    char *zDir = pIdxInfo->aOrderBy->desc?"DESC":"ASC";
    if( iCol<0 ){
      zCol = "rowid";
    }
    zNew = sqlite3_mprintf(" ORDER BY %s %s", zCol, zDir);
    string_concat(&zQuery, zNew, 1);
    pIdxInfo->orderByConsumed = 1;
  }

  appendToEchoModule(pVtab->interp, "xBestIndex");;
  appendToEchoModule(pVtab->interp, zQuery);

  pIdxInfo->idxNum = hashString(zQuery);
  pIdxInfo->idxStr = zQuery;
  pIdxInfo->needToFreeIdxStr = 1;
  if (useCost) {
    pIdxInfo->estimatedCost = cost;
  } else if( useIdx ){
    /* Approximation of log2(nRow). */
    for( ii=0; ii<(sizeof(int)*8); ii++ ){
      if( nRow & (1<<ii) ){
        pIdxInfo->estimatedCost = (double)ii;
      }
    }
  } else {
    pIdxInfo->estimatedCost = (double)nRow;
  }
  return rc;
}

/*
** The xUpdate method for echo module virtual tables.
** 
**    apData[0]  apData[1]  apData[2..]
**
**    INTEGER                              DELETE            
**
**    INTEGER    NULL       (nCol args)    UPDATE (do not set rowid)
**    INTEGER    INTEGER    (nCol args)    UPDATE (with SET rowid = <arg1>)
**
**    NULL       NULL       (nCol args)    INSERT INTO (automatic rowid value)
**    NULL       INTEGER    (nCol args)    INSERT (incl. rowid value)
**
*/
int echoUpdate(
  sqlite3_vtab *tab, 
  int nData, 
  sqlite3_value **apData, 
  sqlite_int64 *pRowid
){
  echo_vtab *pVtab = (echo_vtab *)tab;
  sqlite3 *db = pVtab->db;
  int rc = SQLITE_OK;

  sqlite3_stmt *pStmt;
  char *z = 0;               /* SQL statement to execute */
  int bindArgZero = 0;       /* True to bind apData[0] to sql var no. nData */
  int bindArgOne = 0;        /* True to bind apData[1] to sql var no. 1 */
  int i;                     /* Counter variable used by for loops */

  assert( nData==pVtab->nCol+2 || nData==1 );

  /* If apData[0] is an integer and nData>1 then do an UPDATE */
  if( nData>1 && sqlite3_value_type(apData[0])==SQLITE_INTEGER ){
    char *zSep = " SET";
    z = sqlite3_mprintf("UPDATE %Q", pVtab->zTableName);

    bindArgOne = (apData[1] && sqlite3_value_type(apData[1])==SQLITE_INTEGER);
    bindArgZero = 1;

    if( bindArgOne ){
       string_concat(&z, " SET rowid=?1 ", 0);
       zSep = ",";
    }
    for(i=2; i<nData; i++){
      if( apData[i]==0 ) continue;
      string_concat(&z, sqlite3_mprintf(
          "%s %Q=?%d", zSep, pVtab->aCol[i-2], i), 1);
      zSep = ",";
    }
    string_concat(&z, sqlite3_mprintf(" WHERE rowid=?%d", nData), 0);
  }

  /* If apData[0] is an integer and nData==1 then do a DELETE */
  else if( nData==1 && sqlite3_value_type(apData[0])==SQLITE_INTEGER ){
    z = sqlite3_mprintf("DELETE FROM %Q WHERE rowid = ?1", pVtab->zTableName);
    bindArgZero = 1;
  }

  /* If the first argument is NULL and there are more than two args, INSERT */
  else if( nData>2 && sqlite3_value_type(apData[0])==SQLITE_NULL ){
    int ii;
    char *zInsert = 0;
    char *zValues = 0;
  
    zInsert = sqlite3_mprintf("INSERT INTO %Q (", pVtab->zTableName);
    if( sqlite3_value_type(apData[1])==SQLITE_INTEGER ){
      bindArgOne = 1;
      zValues = sqlite3_mprintf("?");
      string_concat(&zInsert, "rowid", 0);
    }

    assert((pVtab->nCol+2)==nData);
    for(ii=2; ii<nData; ii++){
      string_concat(&zInsert, 
          sqlite3_mprintf("%s%Q", zValues?", ":"", pVtab->aCol[ii-2]), 1);
      string_concat(&zValues, 
          sqlite3_mprintf("%s?%d", zValues?", ":"", ii), 1);
    }

    string_concat(&z, zInsert, 1);
    string_concat(&z, ") VALUES(", 0);
    string_concat(&z, zValues, 1);
    string_concat(&z, ")", 0);
  }

  /* Anything else is an error */
  else{
    assert(0);
    return SQLITE_ERROR;
  }

  rc = sqlite3_prepare(db, z, -1, &pStmt, 0);
  assert( rc!=SQLITE_OK || pStmt );
  sqlite3_free(z);
  if( rc==SQLITE_OK ) {
    if( bindArgZero ){
      sqlite3_bind_value(pStmt, nData, apData[0]);
    }
    if( bindArgOne ){
      sqlite3_bind_value(pStmt, 1, apData[1]);
    }
    for(i=2; i<nData; i++){
      if( apData[i] ) sqlite3_bind_value(pStmt, i, apData[i]);
    }
    sqlite3_step(pStmt);
    rc = sqlite3_finalize(pStmt);
  }

  if( pRowid && rc==SQLITE_OK ){
    *pRowid = sqlite3_last_insert_rowid(db);
  }

  return rc;
}

/*
** xBegin, xSync, xCommit and xRollback callbacks for echo module
** virtual tables. Do nothing other than add the name of the callback
** to the $::echo_module Tcl variable.
*/
static int echoTransactionCall(sqlite3_vtab *tab, const char *zCall){
  char *z;
  echo_vtab *pVtab = (echo_vtab *)tab;
  z = sqlite3_mprintf("echo(%s)", pVtab->zTableName);
  appendToEchoModule(pVtab->interp, zCall);
  appendToEchoModule(pVtab->interp, z);
  sqlite3_free(z);
  return SQLITE_OK;
}
static int echoBegin(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  const char *zVal; 

  echoTransactionCall(tab, "xBegin");

  /* Check if the $::echo_module_begin_fail variable is defined. If it is,
  ** and it is set to the name of the real table underlying this virtual
  ** echo module table, then cause this xSync operation to fail.
  */
  zVal = Tcl_GetVar(interp, "echo_module_begin_fail", TCL_GLOBAL_ONLY);
  if( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
    return SQLITE_ERROR;
  }
  return SQLITE_OK;
}
static int echoSync(sqlite3_vtab *tab){
  echo_vtab *pVtab = (echo_vtab *)tab;
  Tcl_Interp *interp = pVtab->interp;
  const char *zVal; 

  echoTransactionCall(tab, "xSync");

  /* Check if the $::echo_module_sync_fail variable is defined. If it is,
  ** and it is set to the name of the real table underlying this virtual
  ** echo module table, then cause this xSync operation to fail.
  */
  zVal = Tcl_GetVar(interp, "echo_module_sync_fail", TCL_GLOBAL_ONLY);
  if( zVal && 0==strcmp(zVal, pVtab->zTableName) ){
    return -1;
  }
  return SQLITE_OK;
}
static int echoCommit(sqlite3_vtab *tab){
  return echoTransactionCall(tab, "xCommit");
}
static int echoRollback(sqlite3_vtab *tab){
  return echoTransactionCall(tab, "xRollback");
}

/*
** Implementation of "GLOB" function on the echo module.  Pass
** all arguments to the ::echo_glob_overload procedure of TCL
** and return the result of that procedure as a string.
*/
static void overloadedGlobFunction(
  sqlite3_context *pContext,
  int nArg,
  sqlite3_value **apArg
){
  Tcl_Interp *interp = sqlite3_user_data(pContext);
  Tcl_DString str;
  int i;
  int rc;
  Tcl_DStringInit(&str);
  Tcl_DStringAppendElement(&str, "::echo_glob_overload");
  for(i=0; i<nArg; i++){
    Tcl_DStringAppendElement(&str, (char*)sqlite3_value_text(apArg[i]));
  }
  rc = Tcl_Eval(interp, Tcl_DStringValue(&str));
  Tcl_DStringFree(&str);
  if( rc ){
    sqlite3_result_error(pContext, Tcl_GetStringResult(interp), -1);
  }else{
    sqlite3_result_text(pContext, Tcl_GetStringResult(interp),
                        -1, SQLITE_TRANSIENT);
  }
  Tcl_ResetResult(interp);
}

/*
** This is the xFindFunction implementation for the echo module.
** SQLite calls this routine when the first argument of a function
** is a column of an echo virtual table.  This routine can optionally
** override the implementation of that function.  It will choose to
** do so if the function is named "glob", and a TCL command named
** ::echo_glob_overload exists.
*/
static int echoFindFunction(
  sqlite3_vtab *vtab,
  int nArg,
  const char *zFuncName,
  void (**pxFunc)(sqlite3_context*,int,sqlite3_value**),
  void **ppArg
){
  echo_vtab *pVtab = (echo_vtab *)vtab;
  Tcl_Interp *interp = pVtab->interp;
  Tcl_CmdInfo info;
  if( strcmp(zFuncName,"glob")!=0 ){
    return 0;
  }
  if( Tcl_GetCommandInfo(interp, "::echo_glob_overload", &info)==0 ){
    return 0;
  }
  *pxFunc = overloadedGlobFunction;
  *ppArg = interp;
  return 1;
}

/*
** A virtual table module that merely "echos" the contents of another
** table (like an SQL VIEW).
*/
static sqlite3_module echoModule = {
  0,                         /* iVersion */
  echoCreate,
  echoConnect,
  echoBestIndex,
  echoDisconnect, 
  echoDestroy,
  echoOpen,                  /* xOpen - open a cursor */
  echoClose,                 /* xClose - close a cursor */
  echoFilter,                /* xFilter - configure scan constraints */
  echoNext,                  /* xNext - advance a cursor */
  echoEof,                   /* xEof */
  echoColumn,                /* xColumn - read data */
  echoRowid,                 /* xRowid - read data */
  echoUpdate,                /* xUpdate - write data */
  echoBegin,                 /* xBegin - begin transaction */
  echoSync,                  /* xSync - sync transaction */
  echoCommit,                /* xCommit - commit transaction */
  echoRollback,              /* xRollback - rollback transaction */
  echoFindFunction,          /* xFindFunction - function overloading */
};

/*
** Decode a pointer to an sqlite3 object.
*/
static int getDbPointer(Tcl_Interp *interp, const char *zA, sqlite3 **ppDb){
  *ppDb = (sqlite3*)sqlite3TextToPtr(zA);
  return TCL_OK;
}

/*
** Register the echo virtual table module.
*/
static int register_echo_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  int objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  if( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    return TCL_ERROR;
  }
  if( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) return TCL_ERROR;
  sqlite3_create_module(db, "echo", &echoModule, (void *)interp);
  return TCL_OK;
}

/*
** Tcl interface to sqlite3_declare_vtab, invoked as follows from Tcl:
**
** sqlite3_declare_vtab DB SQL
*/
static int declare_vtab(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  int objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  int rc;
  if( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB SQL");
    return TCL_ERROR;
  }
  if( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) return TCL_ERROR;
  rc = sqlite3_declare_vtab(db, Tcl_GetString(objv[2]));
  if( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (char *)sqlite3_errmsg(db), TCL_VOLATILE);
    return TCL_ERROR;
  }
  return TCL_OK;
}

#endif /* ifndef SQLITE_OMIT_VIRTUALTABLE */

/*
** Register commands with the TCL interpreter.
*/
int Sqlitetest8_Init(Tcl_Interp *interp){
  static struct {
     char *zName;
     Tcl_ObjCmdProc *xProc;
     void *clientData;
  } aObjCmd[] = {
#ifndef SQLITE_OMIT_VIRTUALTABLE
     { "register_echo_module",   register_echo_module, 0 },
     { "sqlite3_declare_vtab",   declare_vtab, 0 },
#endif
  };
  int i;
  for(i=0; i<sizeof(aObjCmd)/sizeof(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
  return TCL_OK;
}
