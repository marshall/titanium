/* fts2 has a design flaw which can lead to database corruption (see
** below).  It is recommended not to use it any longer, instead use
** fts3 (or higher).  If you believe that your use of fts2 is safe,
** add -DSQLITE_ENABLE_BROKEN_FTS2=1 to your CFLAGS.
*/
#if (!defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS2)) \
        && !defined(SQLITE_ENABLE_BROKEN_FTS2)
#error fts2 has a design flaw and has been deprecated.
#endif
/* The flaw is that fts2 uses the content table's unaliased rowid as
** the unique docid.  fts2 embeds the rowid in the index it builds,
** and expects the rowid to not change.  The SQLite VACUUM operation
** will renumber such rowids, thereby breaking fts2.  If you are using
** fts2 in a system which has disabled VACUUM, then you can continue
** to use it safely.  Note that PRAGMA auto_vacuum does NOT disable
** VACUUM, though systems using auto_vacuum are unlikely to invoke
** VACUUM.
**
** Unlike fts1, which is safe across VACUUM if you never delete
** documents, fts2 has a second exposure to this flaw, in the segments
** table.  So fts2 should be considered unsafe across VACUUM in all
** cases.
*/

/*
** 2006 Oct 10
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This is an SQLite module implementing full-text search.
*/

/* TODO(shess): I am importing fts2 to capture a bunch of fixes, but
** there were a couple pieces which came along for the ride which rely
** on later SQLite core code.  After long and rigorous debate, I've
** decided to list the set of things affected here, and handle them
** with an #ifdef.  When SQLite core is imported, these should be
** backed out.  My goal is to have the code from sqlite_vendor and
** sqlite_google be cleanly diffable.
**
** SQLite core's version of fts3/2 use a hashtable to store a mapping
** from tokenizer names to implementations.  This hashtable is scoped
** to the module, which requires a new API function
** sqlite3_create_module_v2() in order to handle deleting the
** hashtable when it goes out of scope.
**
** SQLite core adds the custom function fts2_tokenizer() to be used
** for defining new tokenizers.  The second parameter is a vtable
** pointer encoded as a blob.  Obviously this cannot be exposed to
** Gears callers.  It could be suppressed in the authorizer, but for
** now I think it's more reasonable to disable it entirely.
**
** The above two paragraphs lead to reverting to tokenizer-selection
** code which is similar to how it was done before this capability was
** added to the core.
**
** Additionally, the core adds sqlite3_context_db_handle().  The code
** in optimizeFunc() actually already has a handle on the database, so
** this can be worked around for now.
**
** SQLite core added a capability to rename virtual tables.
*/
#define GEARS_FTS2_CHANGES 1

/*
** The code in this file is only compiled if:
**
**     * The FTS2 module is being built as an extension
**       (in which case SQLITE_CORE is not defined), or
**
**     * The FTS2 module is being built into the core of
**       SQLite (in which case SQLITE_ENABLE_FTS2 is defined).
*/

/* TODO(shess) Consider exporting this comment to an HTML file or the
** wiki.
*/
/* The full-text index is stored in a series of b+tree (-like)
** structures called segments which map terms to doclists.  The
** structures are like b+trees in layout, but are constructed from the
** bottom up in optimal fashion and are not updatable.  Since trees
** are built from the bottom up, things will be described from the
** bottom up.
**
**
**** Varints ****
** The basic unit of encoding is a variable-length integer called a
** varint.  We encode variable-length integers in little-endian order
** using seven bits * per byte as follows:
**
** KEY:
**         A = 0xxxxxxx    7 bits of data and one flag bit
**         B = 1xxxxxxx    7 bits of data and one flag bit
**
**  7 bits - A
** 14 bits - BA
** 21 bits - BBA
** and so on.
**
** This is identical to how sqlite encodes varints (see util.c).
**
**
**** Document lists ****
** A doclist (document list) holds a docid-sorted list of hits for a
** given term.  Doclists hold docids, and can optionally associate
** token positions and offsets with docids.
**
** A DL_POSITIONS_OFFSETS doclist is stored like this:
**
** array {
**   varint docid;
**   array {                (position list for column 0)
**     varint position;     (delta from previous position plus POS_BASE)
**     varint startOffset;  (delta from previous startOffset)
**     varint endOffset;    (delta from startOffset)
**   }
**   array {
**     varint POS_COLUMN;   (marks start of position list for new column)
**     varint column;       (index of new column)
**     array {
**       varint position;   (delta from previous position plus POS_BASE)
**       varint startOffset;(delta from previous startOffset)
**       varint endOffset;  (delta from startOffset)
**     }
**   }
**   varint POS_END;        (marks end of positions for this document.
** }
**
** Here, array { X } means zero or more occurrences of X, adjacent in
** memory.  A "position" is an index of a token in the token stream
** generated by the tokenizer, while an "offset" is a byte offset,
** both based at 0.  Note that POS_END and POS_COLUMN occur in the
** same logical place as the position element, and act as sentinals
** ending a position list array.
**
** A DL_POSITIONS doclist omits the startOffset and endOffset
** information.  A DL_DOCIDS doclist omits both the position and
** offset information, becoming an array of varint-encoded docids.
**
** On-disk data is stored as type DL_DEFAULT, so we don't serialize
** the type.  Due to how deletion is implemented in the segmentation
** system, on-disk doclists MUST store at least positions.
**
**
**** Segment leaf nodes ****
** Segment leaf nodes store terms and doclists, ordered by term.  Leaf
** nodes are written using LeafWriter, and read using LeafReader (to
** iterate through a single leaf node's data) and LeavesReader (to
** iterate through a segment's entire leaf layer).  Leaf nodes have
** the format:
**
** varint iHeight;             (height from leaf level, always 0)
** varint nTerm;               (length of first term)
** char pTerm[nTerm];          (content of first term)
** varint nDoclist;            (length of term's associated doclist)
** char pDoclist[nDoclist];    (content of doclist)
** array {
**                             (further terms are delta-encoded)
**   varint nPrefix;           (length of prefix shared with previous term)
**   varint nSuffix;           (length of unshared suffix)
**   char pTermSuffix[nSuffix];(unshared suffix of next term)
**   varint nDoclist;          (length of term's associated doclist)
**   char pDoclist[nDoclist];  (content of doclist)
** }
**
** Here, array { X } means zero or more occurrences of X, adjacent in
** memory.
**
** Leaf nodes are broken into blocks which are stored contiguously in
** the %_segments table in sorted order.  This means that when the end
** of a node is reached, the next term is in the node with the next
** greater node id.
**
** New data is spilled to a new leaf node when the current node
** exceeds LEAF_MAX bytes (default 2048).  New data which itself is
** larger than STANDALONE_MIN (default 1024) is placed in a standalone
** node (a leaf node with a single term and doclist).  The goal of
** these settings is to pack together groups of small doclists while
** making it efficient to directly access large doclists.  The
** assumption is that large doclists represent terms which are more
** likely to be query targets.
**
** TODO(shess) It may be useful for blocking decisions to be more
** dynamic.  For instance, it may make more sense to have a 2.5k leaf
** node rather than splitting into 2k and .5k nodes.  My intuition is
** that this might extend through 2x or 4x the pagesize.
**
**
**** Segment interior nodes ****
** Segment interior nodes store blockids for subtree nodes and terms
** to describe what data is stored by the each subtree.  Interior
** nodes are written using InteriorWriter, and read using
** InteriorReader.  InteriorWriters are created as needed when
** SegmentWriter creates new leaf nodes, or when an interior node
** itself grows too big and must be split.  The format of interior
** nodes:
**
** varint iHeight;           (height from leaf level, always >0)
** varint iBlockid;          (block id of node's leftmost subtree)
** optional {
**   varint nTerm;           (length of first term)
**   char pTerm[nTerm];      (content of first term)
**   array {
**                                (further terms are delta-encoded)
**     varint nPrefix;            (length of shared prefix with previous term)
**     varint nSuffix;            (length of unshared suffix)
**     char pTermSuffix[nSuffix]; (unshared suffix of next term)
**   }
** }
**
** Here, optional { X } means an optional element, while array { X }
** means zero or more occurrences of X, adjacent in memory.
**
** An interior node encodes n terms separating n+1 subtrees.  The
** subtree blocks are contiguous, so only the first subtree's blockid
** is encoded.  The subtree at iBlockid will contain all terms less
** than the first term encoded (or all terms if no term is encoded).
** Otherwise, for terms greater than or equal to pTerm[i] but less
** than pTerm[i+1], the subtree for that term will be rooted at
** iBlockid+i.  Interior nodes only store enough term data to
** distinguish adjacent children (if the rightmost term of the left
** child is "something", and the leftmost term of the right child is
** "wicked", only "w" is stored).
**
** New data is spilled to a new interior node at the same height when
** the current node exceeds INTERIOR_MAX bytes (default 2048).
** INTERIOR_MIN_TERMS (default 7) keeps large terms from monopolizing
** interior nodes and making the tree too skinny.  The interior nodes
** at a given height are naturally tracked by interior nodes at
** height+1, and so on.
**
**
**** Segment directory ****
** The segment directory in table %_segdir stores meta-information for
** merging and deleting segments, and also the root node of the
** segment's tree.
**
** The root node is the top node of the segment's tree after encoding
** the entire segment, restricted to ROOT_MAX bytes (default 1024).
** This could be either a leaf node or an interior node.  If the top
** node requires more than ROOT_MAX bytes, it is flushed to %_segments
** and a new root interior node is generated (which should always fit
** within ROOT_MAX because it only needs space for 2 varints, the
** height and the blockid of the previous root).
**
** The meta-information in the segment directory is:
**   level               - segment level (see below)
**   idx                 - index within level
**                       - (level,idx uniquely identify a segment)
**   start_block         - first leaf node
**   leaves_end_block    - last leaf node
**   end_block           - last block (including interior nodes)
**   root                - contents of root node
**
** If the root node is a leaf node, then start_block,
** leaves_end_block, and end_block are all 0.
**
**
**** Segment merging ****
** To amortize update costs, segments are groups into levels and
** merged in matches.  Each increase in level represents exponentially
** more documents.
**
** New documents (actually, document updates) are tokenized and
** written individually (using LeafWriter) to a level 0 segment, with
** incrementing idx.  When idx reaches MERGE_COUNT (default 16), all
** level 0 segments are merged into a single level 1 segment.  Level 1
** is populated like level 0, and eventually MERGE_COUNT level 1
** segments are merged to a single level 2 segment (representing
** MERGE_COUNT^2 updates), and so on.
**
** A segment merge traverses all segments at a given level in
** parallel, performing a straightforward sorted merge.  Since segment
** leaf nodes are written in to the %_segments table in order, this
** merge traverses the underlying sqlite disk structures efficiently.
** After the merge, all segment blocks from the merged level are
** deleted.
**
** MERGE_COUNT controls how often we merge segments.  16 seems to be
** somewhat of a sweet spot for insertion performance.  32 and 64 show
** very similar performance numbers to 16 on insertion, though they're
** a tiny bit slower (perhaps due to more overhead in merge-time
** sorting).  8 is about 20% slower than 16, 4 about 50% slower than
** 16, 2 about 66% slower than 16.
**
** At query time, high MERGE_COUNT increases the number of segments
** which need to be scanned and merged.  For instance, with 100k docs
** inserted:
**
**    MERGE_COUNT   segments
**       16           25
**        8           12
**        4           10
**        2            6
**
** This appears to have only a moderate impact on queries for very
** frequent terms (which are somewhat dominated by segment merge
** costs), and infrequent and non-existent terms still seem to be fast
** even with many segments.
**
** TODO(shess) That said, it would be nice to have a better query-side
** argument for MERGE_COUNT of 16.  Also, it is possible/likely that
** optimizations to things like doclist merging will swing the sweet
** spot around.
**
**
**
**** Handling of deletions and updates ****
** Since we're using a segmented structure, with no docid-oriented
** index into the term index, we clearly cannot simply update the term
** index when a document is deleted or updated.  For deletions, we
** write an empty doclist (varint(docid) varint(POS_END)), for updates
** we simply write the new doclist.  Segment merges overwrite older
** data for a particular docid with newer data, so deletes or updates
** will eventually overtake the earlier data and knock it out.  The
** query logic likewise merges doclists so that newer data knocks out
** older data.
**
** TODO(shess) Provide a VACUUM type operation to clear out all
** deletions and duplications.  This would basically be a forced merge
** into a single segment.
*/

#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS2)

#if defined(SQLITE_ENABLE_FTS2) && !defined(SQLITE_CORE)
# define SQLITE_CORE 1
#endif

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "fts2.h"
#include "fts2_hash.h"
#include "fts2_tokenizer.h"
#include "sqlite3.h"
#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1


/* TODO(shess) MAN, this thing needs some refactoring.  At minimum, it
** would be nice to order the file better, perhaps something along the
** lines of:
**
**  - utility functions
**  - table setup functions
**  - table update functions
**  - table query functions
**
** Put the query functions last because they're likely to reference
** typedefs or functions from the table update section.
*/

#if 0
# define TRACE(A)  printf A; fflush(stdout)
#else
# define TRACE(A)
#endif

/* It is not safe to call isspace(), tolower(), or isalnum() on
** hi-bit-set characters.  This is the same solution used in the
** tokenizer.
*/
/* TODO(shess) The snippet-generation code should be using the
** tokenizer-generated tokens rather than doing its own local
** tokenization.
*/
/* TODO(shess) Is __isascii() a portable version of (c&0x80)==0? */
static int safe_isspace(char c){
  return (c&0x80)==0 ? isspace(c) : 0;
}
static int safe_tolower(char c){
  return (c&0x80)==0 ? tolower(c) : c;
}
static int safe_isalnum(char c){
  return (c&0x80)==0 ? isalnum(c) : 0;
}

typedef enum DocListType {
  DL_DOCIDS,              /* docids only */
  DL_POSITIONS,           /* docids + positions */
  DL_POSITIONS_OFFSETS    /* docids + positions + offsets */
} DocListType;

/*
** By default, only positions and not offsets are stored in the doclists.
** To change this so that offsets are stored too, compile with
**
**          -DDL_DEFAULT=DL_POSITIONS_OFFSETS
**
** If DL_DEFAULT is set to DL_DOCIDS, your table can only be inserted
** into (no deletes or updates).
*/
#ifndef DL_DEFAULT
# define DL_DEFAULT DL_POSITIONS
#endif

enum {
  POS_END = 0,        /* end of this position list */
  POS_COLUMN,         /* followed by new column number */
  POS_BASE
};

/* MERGE_COUNT controls how often we merge segments (see comment at
** top of file).
*/
#define MERGE_COUNT 16

/* utility functions */

/* CLEAR() and SCRAMBLE() abstract memset() on a pointer to a single
** record to prevent errors of the form:
**
** my_function(SomeType *b){
**   memset(b, '\0', sizeof(b));  // sizeof(b)!=sizeof(*b)
** }
*/
/* TODO(shess) Obvious candidates for a header file. */
#define CLEAR(b) memset(b, '\0', sizeof(*(b)))

#ifndef NDEBUG
#  define SCRAMBLE(b) memset(b, 0x55, sizeof(*(b)))
#else
#  define SCRAMBLE(b)
#endif

/* We may need up to VARINT_MAX bytes to store an encoded 64-bit integer. */
#define VARINT_MAX 10

/* Write a 64-bit variable-length integer to memory starting at p[0].
 * The length of data written will be between 1 and VARINT_MAX bytes.
 * The number of bytes written is returned. */
static int putVarint(char *p, sqlite_int64 v){
  unsigned char *q = (unsigned char *) p;
  sqlite_uint64 vu = v;
  do{
    *q++ = (unsigned char) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }while( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  assert( q - (unsigned char *)p <= VARINT_MAX );
  return (int) (q - (unsigned char *)p);
}

/* Read a 64-bit variable-length integer from memory starting at p[0].
 * Return the number of bytes read, or 0 on error.
 * The value is stored in *v. */
static int getVarint(const char *p, sqlite_int64 *v){
  const unsigned char *q = (const unsigned char *) p;
  sqlite_uint64 x = 0, y = 1;
  while( (*q & 0x80) == 0x80 ){
    x += y * (*q++ & 0x7f);
    y <<= 7;
    if( q - (unsigned char *)p >= VARINT_MAX ){  /* bad data */
      assert( 0 );
      return 0;
    }
  }
  x += y * (*q++);
  *v = (sqlite_int64) x;
  return (int) (q - (unsigned char *)p);
}

static int getVarint32(const char *p, int *pi){
 sqlite_int64 i;
 int ret = getVarint(p, &i);
 *pi = (int) i;
 assert( *pi==i );
 return ret;
}

/*******************************************************************/
/* DataBuffer is used to collect data into a buffer in piecemeal
** fashion.  It implements the usual distinction between amount of
** data currently stored (nData) and buffer capacity (nCapacity).
**
** dataBufferInit - create a buffer with given initial capacity.
** dataBufferReset - forget buffer's data, retaining capacity.
** dataBufferDestroy - free buffer's data.
** dataBufferSwap - swap contents of two buffers.
** dataBufferExpand - expand capacity without adding data.
** dataBufferAppend - append data.
** dataBufferAppend2 - append two pieces of data at once.
** dataBufferReplace - replace buffer's data.
*/
typedef struct DataBuffer {
  char *pData;          /* Pointer to malloc'ed buffer. */
  int nCapacity;        /* Size of pData buffer. */
  int nData;            /* End of data loaded into pData. */
} DataBuffer;

static void dataBufferInit(DataBuffer *pBuffer, int nCapacity){
  assert( nCapacity>=0 );
  pBuffer->nData = 0;
  pBuffer->nCapacity = nCapacity;
  pBuffer->pData = nCapacity==0 ? NULL : sqlite3_malloc(nCapacity);
}
static void dataBufferReset(DataBuffer *pBuffer){
  pBuffer->nData = 0;
}
static void dataBufferDestroy(DataBuffer *pBuffer){
  if( pBuffer->pData!=NULL ) sqlite3_free(pBuffer->pData);
  SCRAMBLE(pBuffer);
}
static void dataBufferSwap(DataBuffer *pBuffer1, DataBuffer *pBuffer2){
  DataBuffer tmp = *pBuffer1;
  *pBuffer1 = *pBuffer2;
  *pBuffer2 = tmp;
}
static void dataBufferExpand(DataBuffer *pBuffer, int nAddCapacity){
  assert( nAddCapacity>0 );
  /* TODO(shess) Consider expanding more aggressively.  Note that the
  ** underlying malloc implementation may take care of such things for
  ** us already.
  */
  if( pBuffer->nData+nAddCapacity>pBuffer->nCapacity ){
    pBuffer->nCapacity = pBuffer->nData+nAddCapacity;
    pBuffer->pData = sqlite3_realloc(pBuffer->pData, pBuffer->nCapacity);
  }
}
static void dataBufferAppend(DataBuffer *pBuffer,
                             const char *pSource, int nSource){
  assert( nSource>0 && pSource!=NULL );
  dataBufferExpand(pBuffer, nSource);
  memcpy(pBuffer->pData+pBuffer->nData, pSource, nSource);
  pBuffer->nData += nSource;
}
static void dataBufferAppend2(DataBuffer *pBuffer,
                              const char *pSource1, int nSource1,
                              const char *pSource2, int nSource2){
  assert( nSource1>0 && pSource1!=NULL );
  assert( nSource2>0 && pSource2!=NULL );
  dataBufferExpand(pBuffer, nSource1+nSource2);
  memcpy(pBuffer->pData+pBuffer->nData, pSource1, nSource1);
  memcpy(pBuffer->pData+pBuffer->nData+nSource1, pSource2, nSource2);
  pBuffer->nData += nSource1+nSource2;
}
static void dataBufferReplace(DataBuffer *pBuffer,
                              const char *pSource, int nSource){
  dataBufferReset(pBuffer);
  dataBufferAppend(pBuffer, pSource, nSource);
}

/* StringBuffer is a null-terminated version of DataBuffer. */
typedef struct StringBuffer {
  DataBuffer b;            /* Includes null terminator. */
} StringBuffer;

static void initStringBuffer(StringBuffer *sb){
  dataBufferInit(&sb->b, 100);
  dataBufferReplace(&sb->b, "", 1);
}
static int stringBufferLength(StringBuffer *sb){
  return sb->b.nData-1;
}
static char *stringBufferData(StringBuffer *sb){
  return sb->b.pData;
}
static void stringBufferDestroy(StringBuffer *sb){
  dataBufferDestroy(&sb->b);
}

static void nappend(StringBuffer *sb, const char *zFrom, int nFrom){
  assert( sb->b.nData>0 );
  if( nFrom>0 ){
    sb->b.nData--;
    dataBufferAppend2(&sb->b, zFrom, nFrom, "", 1);
  }
}
static void append(StringBuffer *sb, const char *zFrom){
  nappend(sb, zFrom, strlen(zFrom));
}

/* Append a list of strings separated by commas. */
static void appendList(StringBuffer *sb, int nString, char **azString){
  int i;
  for(i=0; i<nString; ++i){
    if( i>0 ) append(sb, ", ");
    append(sb, azString[i]);
  }
}

static int endsInWhiteSpace(StringBuffer *p){
  return stringBufferLength(p)>0 &&
    safe_isspace(stringBufferData(p)[stringBufferLength(p)-1]);
}

/* If the StringBuffer ends in something other than white space, add a
** single space character to the end.
*/
static void appendWhiteSpace(StringBuffer *p){
  if( stringBufferLength(p)==0 ) return;
  if( !endsInWhiteSpace(p) ) append(p, " ");
}

/* Remove white space from the end of the StringBuffer */
static void trimWhiteSpace(StringBuffer *p){
  while( endsInWhiteSpace(p) ){
    p->b.pData[--p->b.nData-1] = '\0';
  }
}

/*******************************************************************/
/* DLReader is used to read document elements from a doclist.  The
** current docid is cached, so dlrDocid() is fast.  DLReader does not
** own the doclist buffer.
**
** dlrAtEnd - true if there's no more data to read.
** dlrDocid - docid of current document.
** dlrDocData - doclist data for current document (including docid).
** dlrDocDataBytes - length of same.
** dlrAllDataBytes - length of all remaining data.
** dlrPosData - position data for current document.
** dlrPosDataLen - length of pos data for current document (incl POS_END).
** dlrStep - step to current document.
** dlrInit - initial for doclist of given type against given data.
** dlrDestroy - clean up.
**
** Expected usage is something like:
**
**   DLReader reader;
**   dlrInit(&reader, pData, nData);
**   while( !dlrAtEnd(&reader) ){
**     // calls to dlrDocid() and kin.
**     dlrStep(&reader);
**   }
**   dlrDestroy(&reader);
*/
typedef struct DLReader {
  DocListType iType;
  const char *pData;
  int nData;

  sqlite_int64 iDocid;
  int nElement;
} DLReader;

static int dlrAtEnd(DLReader *pReader){
  assert( pReader->nData>=0 );
  return pReader->nData==0;
}
static sqlite_int64 dlrDocid(DLReader *pReader){
  assert( !dlrAtEnd(pReader) );
  return pReader->iDocid;
}
static const char *dlrDocData(DLReader *pReader){
  assert( !dlrAtEnd(pReader) );
  return pReader->pData;
}
static int dlrDocDataBytes(DLReader *pReader){
  assert( !dlrAtEnd(pReader) );
  return pReader->nElement;
}
static int dlrAllDataBytes(DLReader *pReader){
  assert( !dlrAtEnd(pReader) );
  return pReader->nData;
}
/* TODO(shess) Consider adding a field to track iDocid varint length
** to make these two functions faster.  This might matter (a tiny bit)
** for queries.
*/
static const char *dlrPosData(DLReader *pReader){
  sqlite_int64 iDummy;
  int n = getVarint(pReader->pData, &iDummy);
  assert( !dlrAtEnd(pReader) );
  return pReader->pData+n;
}
static int dlrPosDataLen(DLReader *pReader){
  sqlite_int64 iDummy;
  int n = getVarint(pReader->pData, &iDummy);
  assert( !dlrAtEnd(pReader) );
  return pReader->nElement-n;
}
static void dlrStep(DLReader *pReader){
  assert( !dlrAtEnd(pReader) );

  /* Skip past current doclist element. */
  assert( pReader->nElement<=pReader->nData );
  pReader->pData += pReader->nElement;
  pReader->nData -= pReader->nElement;

  /* If there is more data, read the next doclist element. */
  if( pReader->nData!=0 ){
    sqlite_int64 iDocidDelta;
    int iDummy, n = getVarint(pReader->pData, &iDocidDelta);
    pReader->iDocid += iDocidDelta;
    if( pReader->iType>=DL_POSITIONS ){
      assert( n<pReader->nData );
      while( 1 ){
        n += getVarint32(pReader->pData+n, &iDummy);
        assert( n<=pReader->nData );
        if( iDummy==POS_END ) break;
        if( iDummy==POS_COLUMN ){
          n += getVarint32(pReader->pData+n, &iDummy);
          assert( n<pReader->nData );
        }else if( pReader->iType==DL_POSITIONS_OFFSETS ){
          n += getVarint32(pReader->pData+n, &iDummy);
          n += getVarint32(pReader->pData+n, &iDummy);
          assert( n<pReader->nData );
        }
      }
    }
    pReader->nElement = n;
    assert( pReader->nElement<=pReader->nData );
  }
}
static void dlrInit(DLReader *pReader, DocListType iType,
                    const char *pData, int nData){
  assert( pData!=NULL && nData!=0 );
  pReader->iType = iType;
  pReader->pData = pData;
  pReader->nData = nData;
  pReader->nElement = 0;
  pReader->iDocid = 0;

  /* Load the first element's data.  There must be a first element. */
  dlrStep(pReader);
}
static void dlrDestroy(DLReader *pReader){
  SCRAMBLE(pReader);
}

#ifndef NDEBUG
/* Verify that the doclist can be validly decoded.  Also returns the
** last docid found because it is convenient in other assertions for
** DLWriter.
*/
static void docListValidate(DocListType iType, const char *pData, int nData,
                            sqlite_int64 *pLastDocid){
  sqlite_int64 iPrevDocid = 0;
  assert( nData>0 );
  assert( pData!=0 );
  assert( pData+nData>pData );
  while( nData!=0 ){
    sqlite_int64 iDocidDelta;
    int n = getVarint(pData, &iDocidDelta);
    iPrevDocid += iDocidDelta;
    if( iType>DL_DOCIDS ){
      int iDummy;
      while( 1 ){
        n += getVarint32(pData+n, &iDummy);
        if( iDummy==POS_END ) break;
        if( iDummy==POS_COLUMN ){
          n += getVarint32(pData+n, &iDummy);
        }else if( iType>DL_POSITIONS ){
          n += getVarint32(pData+n, &iDummy);
          n += getVarint32(pData+n, &iDummy);
        }
        assert( n<=nData );
      }
    }
    assert( n<=nData );
    pData += n;
    nData -= n;
  }
  if( pLastDocid ) *pLastDocid = iPrevDocid;
}
#define ASSERT_VALID_DOCLIST(i, p, n, o) docListValidate(i, p, n, o)
#else
#define ASSERT_VALID_DOCLIST(i, p, n, o) assert( 1 )
#endif

/*******************************************************************/
/* DLWriter is used to write doclist data to a DataBuffer.  DLWriter
** always appends to the buffer and does not own it.
**
** dlwInit - initialize to write a given type doclistto a buffer.
** dlwDestroy - clear the writer's memory.  Does not free buffer.
** dlwAppend - append raw doclist data to buffer.
** dlwCopy - copy next doclist from reader to writer.
** dlwAdd - construct doclist element and append to buffer.
**    Only apply dlwAdd() to DL_DOCIDS doclists (else use PLWriter).
*/
typedef struct DLWriter {
  DocListType iType;
  DataBuffer *b;
  sqlite_int64 iPrevDocid;
#ifndef NDEBUG
  int has_iPrevDocid;
#endif
} DLWriter;

static void dlwInit(DLWriter *pWriter, DocListType iType, DataBuffer *b){
  pWriter->b = b;
  pWriter->iType = iType;
  pWriter->iPrevDocid = 0;
#ifndef NDEBUG
  pWriter->has_iPrevDocid = 0;
#endif
}
static void dlwDestroy(DLWriter *pWriter){
  SCRAMBLE(pWriter);
}
/* iFirstDocid is the first docid in the doclist in pData.  It is
** needed because pData may point within a larger doclist, in which
** case the first item would be delta-encoded.
**
** iLastDocid is the final docid in the doclist in pData.  It is
** needed to create the new iPrevDocid for future delta-encoding.  The
** code could decode the passed doclist to recreate iLastDocid, but
** the only current user (docListMerge) already has decoded this
** information.
*/
/* TODO(shess) This has become just a helper for docListMerge.
** Consider a refactor to make this cleaner.
*/
static void dlwAppend(DLWriter *pWriter,
                      const char *pData, int nData,
                      sqlite_int64 iFirstDocid, sqlite_int64 iLastDocid){
  sqlite_int64 iDocid = 0;
  char c[VARINT_MAX];
  int nFirstOld, nFirstNew;     /* Old and new varint len of first docid. */
#ifndef NDEBUG
  sqlite_int64 iLastDocidDelta;
#endif

  /* Recode the initial docid as delta from iPrevDocid. */
  nFirstOld = getVarint(pData, &iDocid);
  assert( nFirstOld<nData || (nFirstOld==nData && pWriter->iType==DL_DOCIDS) );
  nFirstNew = putVarint(c, iFirstDocid-pWriter->iPrevDocid);

  /* Verify that the incoming doclist is valid AND that it ends with
  ** the expected docid.  This is essential because we'll trust this
  ** docid in future delta-encoding.
  */
  ASSERT_VALID_DOCLIST(pWriter->iType, pData, nData, &iLastDocidDelta);
  assert( iLastDocid==iFirstDocid-iDocid+iLastDocidDelta );

  /* Append recoded initial docid and everything else.  Rest of docids
  ** should have been delta-encoded from previous initial docid.
  */
  if( nFirstOld<nData ){
    dataBufferAppend2(pWriter->b, c, nFirstNew,
                      pData+nFirstOld, nData-nFirstOld);
  }else{
    dataBufferAppend(pWriter->b, c, nFirstNew);
  }
  pWriter->iPrevDocid = iLastDocid;
}
static void dlwCopy(DLWriter *pWriter, DLReader *pReader){
  dlwAppend(pWriter, dlrDocData(pReader), dlrDocDataBytes(pReader),
            dlrDocid(pReader), dlrDocid(pReader));
}
static void dlwAdd(DLWriter *pWriter, sqlite_int64 iDocid){
  char c[VARINT_MAX];
  int n = putVarint(c, iDocid-pWriter->iPrevDocid);

  /* Docids must ascend. */
  assert( !pWriter->has_iPrevDocid || iDocid>pWriter->iPrevDocid );
  assert( pWriter->iType==DL_DOCIDS );

  dataBufferAppend(pWriter->b, c, n);
  pWriter->iPrevDocid = iDocid;
#ifndef NDEBUG
  pWriter->has_iPrevDocid = 1;
#endif
}

/*******************************************************************/
/* PLReader is used to read data from a document's position list.  As
** the caller steps through the list, data is cached so that varints
** only need to be decoded once.
**
** plrInit, plrDestroy - create/destroy a reader.
** plrColumn, plrPosition, plrStartOffset, plrEndOffset - accessors
** plrAtEnd - at end of stream, only call plrDestroy once true.
** plrStep - step to the next element.
*/
typedef struct PLReader {
  /* These refer to the next position's data.  nData will reach 0 when
  ** reading the last position, so plrStep() signals EOF by setting
  ** pData to NULL.
  */
  const char *pData;
  int nData;

  DocListType iType;
  int iColumn;         /* the last column read */
  int iPosition;       /* the last position read */
  int iStartOffset;    /* the last start offset read */
  int iEndOffset;      /* the last end offset read */
} PLReader;

static int plrAtEnd(PLReader *pReader){
  return pReader->pData==NULL;
}
static int plrColumn(PLReader *pReader){
  assert( !plrAtEnd(pReader) );
  return pReader->iColumn;
}
static int plrPosition(PLReader *pReader){
  assert( !plrAtEnd(pReader) );
  return pReader->iPosition;
}
static int plrStartOffset(PLReader *pReader){
  assert( !plrAtEnd(pReader) );
  return pReader->iStartOffset;
}
static int plrEndOffset(PLReader *pReader){
  assert( !plrAtEnd(pReader) );
  return pReader->iEndOffset;
}
static void plrStep(PLReader *pReader){
  int i, n;

  assert( !plrAtEnd(pReader) );

  if( pReader->nData==0 ){
    pReader->pData = NULL;
    return;
  }

  n = getVarint32(pReader->pData, &i);
  if( i==POS_COLUMN ){
    n += getVarint32(pReader->pData+n, &pReader->iColumn);
    pReader->iPosition = 0;
    pReader->iStartOffset = 0;
    n += getVarint32(pReader->pData+n, &i);
  }
  /* Should never see adjacent column changes. */
  assert( i!=POS_COLUMN );

  if( i==POS_END ){
    pReader->nData = 0;
    pReader->pData = NULL;
    return;
  }

  pReader->iPosition += i-POS_BASE;
  if( pReader->iType==DL_POSITIONS_OFFSETS ){
    n += getVarint32(pReader->pData+n, &i);
    pReader->iStartOffset += i;
    n += getVarint32(pReader->pData+n, &i);
    pReader->iEndOffset = pReader->iStartOffset+i;
  }
  assert( n<=pReader->nData );
  pReader->pData += n;
  pReader->nData -= n;
}

static void plrInit(PLReader *pReader, DLReader *pDLReader){
  pReader->pData = dlrPosData(pDLReader);
  pReader->nData = dlrPosDataLen(pDLReader);
  pReader->iType = pDLReader->iType;
  pReader->iColumn = 0;
  pReader->iPosition = 0;
  pReader->iStartOffset = 0;
  pReader->iEndOffset = 0;
  plrStep(pReader);
}
static void plrDestroy(PLReader *pReader){
  SCRAMBLE(pReader);
}

/*******************************************************************/
/* PLWriter is used in constructing a document's position list.  As a
** convenience, if iType is DL_DOCIDS, PLWriter becomes a no-op.
** PLWriter writes to the associated DLWriter's buffer.
**
** plwInit - init for writing a document's poslist.
** plwDestroy - clear a writer.
** plwAdd - append position and offset information.
** plwCopy - copy next position's data from reader to writer.
** plwTerminate - add any necessary doclist terminator.
**
** Calling plwAdd() after plwTerminate() may result in a corrupt
** doclist.
*/
/* TODO(shess) Until we've written the second item, we can cache the
** first item's information.  Then we'd have three states:
**
** - initialized with docid, no positions.
** - docid and one position.
** - docid and multiple positions.
**
** Only the last state needs to actually write to dlw->b, which would
** be an improvement in the DLCollector case.
*/
typedef struct PLWriter {
  DLWriter *dlw;

  int iColumn;    /* the last column written */
  int iPos;       /* the last position written */
  int iOffset;    /* the last start offset written */
} PLWriter;

/* TODO(shess) In the case where the parent is reading these values
** from a PLReader, we could optimize to a copy if that PLReader has
** the same type as pWriter.
*/
static void plwAdd(PLWriter *pWriter, int iColumn, int iPos,
                   int iStartOffset, int iEndOffset){
  /* Worst-case space for POS_COLUMN, iColumn, iPosDelta,
  ** iStartOffsetDelta, and iEndOffsetDelta.
  */
  char c[5*VARINT_MAX];
  int n = 0;

  /* Ban plwAdd() after plwTerminate(). */
  assert( pWriter->iPos!=-1 );

  if( pWriter->dlw->iType==DL_DOCIDS ) return;

  if( iColumn!=pWriter->iColumn ){
    n += putVarint(c+n, POS_COLUMN);
    n += putVarint(c+n, iColumn);
    pWriter->iColumn = iColumn;
    pWriter->iPos = 0;
    pWriter->iOffset = 0;
  }
  assert( iPos>=pWriter->iPos );
  n += putVarint(c+n, POS_BASE+(iPos-pWriter->iPos));
  pWriter->iPos = iPos;
  if( pWriter->dlw->iType==DL_POSITIONS_OFFSETS ){
    assert( iStartOffset>=pWriter->iOffset );
    n += putVarint(c+n, iStartOffset-pWriter->iOffset);
    pWriter->iOffset = iStartOffset;
    assert( iEndOffset>=iStartOffset );
    n += putVarint(c+n, iEndOffset-iStartOffset);
  }
  dataBufferAppend(pWriter->dlw->b, c, n);
}
static void plwCopy(PLWriter *pWriter, PLReader *pReader){
  plwAdd(pWriter, plrColumn(pReader), plrPosition(pReader),
         plrStartOffset(pReader), plrEndOffset(pReader));
}
static void plwInit(PLWriter *pWriter, DLWriter *dlw, sqlite_int64 iDocid){
  char c[VARINT_MAX];
  int n;

  pWriter->dlw = dlw;

  /* Docids must ascend. */
  assert( !pWriter->dlw->has_iPrevDocid || iDocid>pWriter->dlw->iPrevDocid );
  n = putVarint(c, iDocid-pWriter->dlw->iPrevDocid);
  dataBufferAppend(pWriter->dlw->b, c, n);
  pWriter->dlw->iPrevDocid = iDocid;
#ifndef NDEBUG
  pWriter->dlw->has_iPrevDocid = 1;
#endif

  pWriter->iColumn = 0;
  pWriter->iPos = 0;
  pWriter->iOffset = 0;
}
/* TODO(shess) Should plwDestroy() also terminate the doclist?  But
** then plwDestroy() would no longer be just a destructor, it would
** also be doing work, which isn't consistent with the overall idiom.
** Another option would be for plwAdd() to always append any necessary
** terminator, so that the output is always correct.  But that would
** add incremental work to the common case with the only benefit being
** API elegance.  Punt for now.
*/
static void plwTerminate(PLWriter *pWriter){
  if( pWriter->dlw->iType>DL_DOCIDS ){
    char c[VARINT_MAX];
    int n = putVarint(c, POS_END);
    dataBufferAppend(pWriter->dlw->b, c, n);
  }
#ifndef NDEBUG
  /* Mark as terminated for assert in plwAdd(). */
  pWriter->iPos = -1;
#endif
}
static void plwDestroy(PLWriter *pWriter){
  SCRAMBLE(pWriter);
}

/*******************************************************************/
/* DLCollector wraps PLWriter and DLWriter to provide a
** dynamically-allocated doclist area to use during tokenization.
**
** dlcNew - malloc up and initialize a collector.
** dlcDelete - destroy a collector and all contained items.
** dlcAddPos - append position and offset information.
** dlcAddDoclist - add the collected doclist to the given buffer.
** dlcNext - terminate the current document and open another.
*/
typedef struct DLCollector {
  DataBuffer b;
  DLWriter dlw;
  PLWriter plw;
} DLCollector;

/* TODO(shess) This could also be done by calling plwTerminate() and
** dataBufferAppend().  I tried that, expecting nominal performance
** differences, but it seemed to pretty reliably be worth 1% to code
** it this way.  I suspect it is the incremental malloc overhead (some
** percentage of the plwTerminate() calls will cause a realloc), so
** this might be worth revisiting if the DataBuffer implementation
** changes.
*/
static void dlcAddDoclist(DLCollector *pCollector, DataBuffer *b){
  if( pCollector->dlw.iType>DL_DOCIDS ){
    char c[VARINT_MAX];
    int n = putVarint(c, POS_END);
    dataBufferAppend2(b, pCollector->b.pData, pCollector->b.nData, c, n);
  }else{
    dataBufferAppend(b, pCollector->b.pData, pCollector->b.nData);
  }
}
static void dlcNext(DLCollector *pCollector, sqlite_int64 iDocid){
  plwTerminate(&pCollector->plw);
  plwDestroy(&pCollector->plw);
  plwInit(&pCollector->plw, &pCollector->dlw, iDocid);
}
static void dlcAddPos(DLCollector *pCollector, int iColumn, int iPos,
                      int iStartOffset, int iEndOffset){
  plwAdd(&pCollector->plw, iColumn, iPos, iStartOffset, iEndOffset);
}

static DLCollector *dlcNew(sqlite_int64 iDocid, DocListType iType){
  DLCollector *pCollector = sqlite3_malloc(sizeof(DLCollector));
  dataBufferInit(&pCollector->b, 0);
  dlwInit(&pCollector->dlw, iType, &pCollector->b);
  plwInit(&pCollector->plw, &pCollector->dlw, iDocid);
  return pCollector;
}
static void dlcDelete(DLCollector *pCollector){
  plwDestroy(&pCollector->plw);
  dlwDestroy(&pCollector->dlw);
  dataBufferDestroy(&pCollector->b);
  SCRAMBLE(pCollector);
  sqlite3_free(pCollector);
}


/* Copy the doclist data of iType in pData/nData into *out, trimming
** unnecessary data as we go.  Only columns matching iColumn are
** copied, all columns copied if iColumn is -1.  Elements with no
** matching columns are dropped.  The output is an iOutType doclist.
*/
/* NOTE(shess) This code is only valid after all doclists are merged.
** If this is run before merges, then doclist items which represent
** deletion will be trimmed, and will thus not effect a deletion
** during the merge.
*/
static void docListTrim(DocListType iType, const char *pData, int nData,
                        int iColumn, DocListType iOutType, DataBuffer *out){
  DLReader dlReader;
  DLWriter dlWriter;

  assert( iOutType<=iType );

  dlrInit(&dlReader, iType, pData, nData);
  dlwInit(&dlWriter, iOutType, out);

  while( !dlrAtEnd(&dlReader) ){
    PLReader plReader;
    PLWriter plWriter;
    int match = 0;

    plrInit(&plReader, &dlReader);

    while( !plrAtEnd(&plReader) ){
      if( iColumn==-1 || plrColumn(&plReader)==iColumn ){
        if( !match ){
          plwInit(&plWriter, &dlWriter, dlrDocid(&dlReader));
          match = 1;
        }
        plwAdd(&plWriter, plrColumn(&plReader), plrPosition(&plReader),
               plrStartOffset(&plReader), plrEndOffset(&plReader));
      }
      plrStep(&plReader);
    }
    if( match ){
      plwTerminate(&plWriter);
      plwDestroy(&plWriter);
    }

    plrDestroy(&plReader);
    dlrStep(&dlReader);
  }
  dlwDestroy(&dlWriter);
  dlrDestroy(&dlReader);
}

/* Used by docListMerge() to keep doclists in the ascending order by
** docid, then ascending order by age (so the newest comes first).
*/
typedef struct OrderedDLReader {
  DLReader *pReader;

  /* TODO(shess) If we assume that docListMerge pReaders is ordered by
  ** age (which we do), then we could use pReader comparisons to break
  ** ties.
  */
  int idx;
} OrderedDLReader;

/* Order eof to end, then by docid asc, idx desc. */
static int orderedDLReaderCmp(OrderedDLReader *r1, OrderedDLReader *r2){
  if( dlrAtEnd(r1->pReader) ){
    if( dlrAtEnd(r2->pReader) ) return 0;  /* Both atEnd(). */
    return 1;                              /* Only r1 atEnd(). */
  }
  if( dlrAtEnd(r2->pReader) ) return -1;   /* Only r2 atEnd(). */

  if( dlrDocid(r1->pReader)<dlrDocid(r2->pReader) ) return -1;
  if( dlrDocid(r1->pReader)>dlrDocid(r2->pReader) ) return 1;

  /* Descending on idx. */
  return r2->idx-r1->idx;
}

/* Bubble p[0] to appropriate place in p[1..n-1].  Assumes that
** p[1..n-1] is already sorted.
*/
/* TODO(shess) Is this frequent enough to warrant a binary search?
** Before implementing that, instrument the code to check.  In most
** current usage, I expect that p[0] will be less than p[1] a very
** high proportion of the time.
*/
static void orderedDLReaderReorder(OrderedDLReader *p, int n){
  while( n>1 && orderedDLReaderCmp(p, p+1)>0 ){
    OrderedDLReader tmp = p[0];
    p[0] = p[1];
    p[1] = tmp;
    n--;
    p++;
  }
}

/* Given an array of doclist readers, merge their doclist elements
** into out in sorted order (by docid), dropping elements from older
** readers when there is a duplicate docid.  pReaders is assumed to be
** ordered by age, oldest first.
*/
/* TODO(shess) nReaders must be <= MERGE_COUNT.  This should probably
** be fixed.
*/
static void docListMerge(DataBuffer *out,
                         DLReader *pReaders, int nReaders){
  OrderedDLReader readers[MERGE_COUNT];
  DLWriter writer;
  int i, n;
  const char *pStart = 0;
  int nStart = 0;
  sqlite_int64 iFirstDocid = 0, iLastDocid = 0;

  assert( nReaders>0 );
  if( nReaders==1 ){
    dataBufferAppend(out, dlrDocData(pReaders), dlrAllDataBytes(pReaders));
    return;
  }

  assert( nReaders<=MERGE_COUNT );
  n = 0;
  for(i=0; i<nReaders; i++){
    assert( pReaders[i].iType==pReaders[0].iType );
    readers[i].pReader = pReaders+i;
    readers[i].idx = i;
    n += dlrAllDataBytes(&pReaders[i]);
  }
  /* Conservatively size output to sum of inputs.  Output should end
  ** up strictly smaller than input.
  */
  dataBufferExpand(out, n);

  /* Get the readers into sorted order. */
  while( i-->0 ){
    orderedDLReaderReorder(readers+i, nReaders-i);
  }

  dlwInit(&writer, pReaders[0].iType, out);
  while( !dlrAtEnd(readers[0].pReader) ){
    sqlite_int64 iDocid = dlrDocid(readers[0].pReader);

    /* If this is a continuation of the current buffer to copy, extend
    ** that buffer.  memcpy() seems to be more efficient if it has a
    ** lots of data to copy.
    */
    if( dlrDocData(readers[0].pReader)==pStart+nStart ){
      nStart += dlrDocDataBytes(readers[0].pReader);
    }else{
      if( pStart!=0 ){
        dlwAppend(&writer, pStart, nStart, iFirstDocid, iLastDocid);
      }
      pStart = dlrDocData(readers[0].pReader);
      nStart = dlrDocDataBytes(readers[0].pReader);
      iFirstDocid = iDocid;
    }
    iLastDocid = iDocid;
    dlrStep(readers[0].pReader);

    /* Drop all of the older elements with the same docid. */
    for(i=1; i<nReaders &&
             !dlrAtEnd(readers[i].pReader) &&
             dlrDocid(readers[i].pReader)==iDocid; i++){
      dlrStep(readers[i].pReader);
    }

    /* Get the readers back into order. */
    while( i-->0 ){
      orderedDLReaderReorder(readers+i, nReaders-i);
    }
  }

  /* Copy over any remaining elements. */
  if( nStart>0 ) dlwAppend(&writer, pStart, nStart, iFirstDocid, iLastDocid);
  dlwDestroy(&writer);
}

/* Helper function for posListUnion().  Compares the current position
** between left and right, returning as standard C idiom of <0 if
** left<right, >0 if left>right, and 0 if left==right.  "End" always
** compares greater.
*/
static int posListCmp(PLReader *pLeft, PLReader *pRight){
  assert( pLeft->iType==pRight->iType );
  if( pLeft->iType==DL_DOCIDS ) return 0;

  if( plrAtEnd(pLeft) ) return plrAtEnd(pRight) ? 0 : 1;
  if( plrAtEnd(pRight) ) return -1;

  if( plrColumn(pLeft)<plrColumn(pRight) ) return -1;
  if( plrColumn(pLeft)>plrColumn(pRight) ) return 1;

  if( plrPosition(pLeft)<plrPosition(pRight) ) return -1;
  if( plrPosition(pLeft)>plrPosition(pRight) ) return 1;
  if( pLeft->iType==DL_POSITIONS ) return 0;

  if( plrStartOffset(pLeft)<plrStartOffset(pRight) ) return -1;
  if( plrStartOffset(pLeft)>plrStartOffset(pRight) ) return 1;

  if( plrEndOffset(pLeft)<plrEndOffset(pRight) ) return -1;
  if( plrEndOffset(pLeft)>plrEndOffset(pRight) ) return 1;

  return 0;
}

/* Write the union of position lists in pLeft and pRight to pOut.
** "Union" in this case meaning "All unique position tuples".  Should
** work with any doclist type, though both inputs and the output
** should be the same type.
*/
static void posListUnion(DLReader *pLeft, DLReader *pRight, DLWriter *pOut){
  PLReader left, right;
  PLWriter writer;

  assert( dlrDocid(pLeft)==dlrDocid(pRight) );
  assert( pLeft->iType==pRight->iType );
  assert( pLeft->iType==pOut->iType );

  plrInit(&left, pLeft);
  plrInit(&right, pRight);
  plwInit(&writer, pOut, dlrDocid(pLeft));

  while( !plrAtEnd(&left) || !plrAtEnd(&right) ){
    int c = posListCmp(&left, &right);
    if( c<0 ){
      plwCopy(&writer, &left);
      plrStep(&left);
    }else if( c>0 ){
      plwCopy(&writer, &right);
      plrStep(&right);
    }else{
      plwCopy(&writer, &left);
      plrStep(&left);
      plrStep(&right);
    }
  }

  plwTerminate(&writer);
  plwDestroy(&writer);
  plrDestroy(&left);
  plrDestroy(&right);
}

/* Write the union of doclists in pLeft and pRight to pOut.  For
** docids in common between the inputs, the union of the position
** lists is written.  Inputs and outputs are always type DL_DEFAULT.
*/
static void docListUnion(
  const char *pLeft, int nLeft,
  const char *pRight, int nRight,
  DataBuffer *pOut      /* Write the combined doclist here */
){
  DLReader left, right;
  DLWriter writer;

  if( nLeft==0 ){
    if( nRight!=0) dataBufferAppend(pOut, pRight, nRight);
    return;
  }
  if( nRight==0 ){
    dataBufferAppend(pOut, pLeft, nLeft);
    return;
  }

  dlrInit(&left, DL_DEFAULT, pLeft, nLeft);
  dlrInit(&right, DL_DEFAULT, pRight, nRight);
  dlwInit(&writer, DL_DEFAULT, pOut);

  while( !dlrAtEnd(&left) || !dlrAtEnd(&right) ){
    if( dlrAtEnd(&right) ){
      dlwCopy(&writer, &left);
      dlrStep(&left);
    }else if( dlrAtEnd(&left) ){
      dlwCopy(&writer, &right);
      dlrStep(&right);
    }else if( dlrDocid(&left)<dlrDocid(&right) ){
      dlwCopy(&writer, &left);
      dlrStep(&left);
    }else if( dlrDocid(&left)>dlrDocid(&right) ){
      dlwCopy(&writer, &right);
      dlrStep(&right);
    }else{
      posListUnion(&left, &right, &writer);
      dlrStep(&left);
      dlrStep(&right);
    }
  }

  dlrDestroy(&left);
  dlrDestroy(&right);
  dlwDestroy(&writer);
}

/* pLeft and pRight are DLReaders positioned to the same docid.
**
** If there are no instances in pLeft or pRight where the position
** of pLeft is one less than the position of pRight, then this
** routine adds nothing to pOut.
**
** If there are one or more instances where positions from pLeft
** are exactly one less than positions from pRight, then add a new
** document record to pOut.  If pOut wants to hold positions, then
** include the positions from pRight that are one more than a
** position in pLeft.  In other words:  pRight.iPos==pLeft.iPos+1.
*/
static void posListPhraseMerge(DLReader *pLeft, DLReader *pRight,
                               DLWriter *pOut){
  PLReader left, right;
  PLWriter writer;
  int match = 0;

  assert( dlrDocid(pLeft)==dlrDocid(pRight) );
  assert( pOut->iType!=DL_POSITIONS_OFFSETS );

  plrInit(&left, pLeft);
  plrInit(&right, pRight);

  while( !plrAtEnd(&left) && !plrAtEnd(&right) ){
    if( plrColumn(&left)<plrColumn(&right) ){
      plrStep(&left);
    }else if( plrColumn(&left)>plrColumn(&right) ){
      plrStep(&right);
    }else if( plrPosition(&left)+1<plrPosition(&right) ){
      plrStep(&left);
    }else if( plrPosition(&left)+1>plrPosition(&right) ){
      plrStep(&right);
    }else{
      if( !match ){
        plwInit(&writer, pOut, dlrDocid(pLeft));
        match = 1;
      }
      plwAdd(&writer, plrColumn(&right), plrPosition(&right), 0, 0);
      plrStep(&left);
      plrStep(&right);
    }
  }

  if( match ){
    plwTerminate(&writer);
    plwDestroy(&writer);
  }

  plrDestroy(&left);
  plrDestroy(&right);
}

/* We have two doclists with positions:  pLeft and pRight.
** Write the phrase intersection of these two doclists into pOut.
**
** A phrase intersection means that two documents only match
** if pLeft.iPos+1==pRight.iPos.
**
** iType controls the type of data written to pOut.  If iType is
** DL_POSITIONS, the positions are those from pRight.
*/
static void docListPhraseMerge(
  const char *pLeft, int nLeft,
  const char *pRight, int nRight,
  DocListType iType,
  DataBuffer *pOut      /* Write the combined doclist here */
){
  DLReader left, right;
  DLWriter writer;

  if( nLeft==0 || nRight==0 ) return;

  assert( iType!=DL_POSITIONS_OFFSETS );

  dlrInit(&left, DL_POSITIONS, pLeft, nLeft);
  dlrInit(&right, DL_POSITIONS, pRight, nRight);
  dlwInit(&writer, iType, pOut);

  while( !dlrAtEnd(&left) && !dlrAtEnd(&right) ){
    if( dlrDocid(&left)<dlrDocid(&right) ){
      dlrStep(&left);
    }else if( dlrDocid(&right)<dlrDocid(&left) ){
      dlrStep(&right);
    }else{
      posListPhraseMerge(&left, &right, &writer);
      dlrStep(&left);
      dlrStep(&right);
    }
  }

  dlrDestroy(&left);
  dlrDestroy(&right);
  dlwDestroy(&writer);
}

/* We have two DL_DOCIDS doclists:  pLeft and pRight.
** Write the intersection of these two doclists into pOut as a
** DL_DOCIDS doclist.
*/
static void docListAndMerge(
  const char *pLeft, int nLeft,
  const char *pRight, int nRight,
  DataBuffer *pOut      /* Write the combined doclist here */
){
  DLReader left, right;
  DLWriter writer;

  if( nLeft==0 || nRight==0 ) return;

  dlrInit(&left, DL_DOCIDS, pLeft, nLeft);
  dlrInit(&right, DL_DOCIDS, pRight, nRight);
  dlwInit(&writer, DL_DOCIDS, pOut);

  while( !dlrAtEnd(&left) && !dlrAtEnd(&right) ){
    if( dlrDocid(&left)<dlrDocid(&right) ){
      dlrStep(&left);
    }else if( dlrDocid(&right)<dlrDocid(&left) ){
      dlrStep(&right);
    }else{
      dlwAdd(&writer, dlrDocid(&left));
      dlrStep(&left);
      dlrStep(&right);
    }
  }

  dlrDestroy(&left);
  dlrDestroy(&right);
  dlwDestroy(&writer);
}

/* We have two DL_DOCIDS doclists:  pLeft and pRight.
** Write the union of these two doclists into pOut as a
** DL_DOCIDS doclist.
*/
static void docListOrMerge(
  const char *pLeft, int nLeft,
  const char *pRight, int nRight,
  DataBuffer *pOut      /* Write the combined doclist here */
){
  DLReader left, right;
  DLWriter writer;

  if( nLeft==0 ){
    if( nRight!=0 ) dataBufferAppend(pOut, pRight, nRight);
    return;
  }
  if( nRight==0 ){
    dataBufferAppend(pOut, pLeft, nLeft);
    return;
  }

  dlrInit(&left, DL_DOCIDS, pLeft, nLeft);
  dlrInit(&right, DL_DOCIDS, pRight, nRight);
  dlwInit(&writer, DL_DOCIDS, pOut);

  while( !dlrAtEnd(&left) || !dlrAtEnd(&right) ){
    if( dlrAtEnd(&right) ){
      dlwAdd(&writer, dlrDocid(&left));
      dlrStep(&left);
    }else if( dlrAtEnd(&left) ){
      dlwAdd(&writer, dlrDocid(&right));
      dlrStep(&right);
    }else if( dlrDocid(&left)<dlrDocid(&right) ){
      dlwAdd(&writer, dlrDocid(&left));
      dlrStep(&left);
    }else if( dlrDocid(&right)<dlrDocid(&left) ){
      dlwAdd(&writer, dlrDocid(&right));
      dlrStep(&right);
    }else{
      dlwAdd(&writer, dlrDocid(&left));
      dlrStep(&left);
      dlrStep(&right);
    }
  }

  dlrDestroy(&left);
  dlrDestroy(&right);
  dlwDestroy(&writer);
}

/* We have two DL_DOCIDS doclists:  pLeft and pRight.
** Write into pOut as DL_DOCIDS doclist containing all documents that
** occur in pLeft but not in pRight.
*/
static void docListExceptMerge(
  const char *pLeft, int nLeft,
  const char *pRight, int nRight,
  DataBuffer *pOut      /* Write the combined doclist here */
){
  DLReader left, right;
  DLWriter writer;

  if( nLeft==0 ) return;
  if( nRight==0 ){
    dataBufferAppend(pOut, pLeft, nLeft);
    return;
  }

  dlrInit(&left, DL_DOCIDS, pLeft, nLeft);
  dlrInit(&right, DL_DOCIDS, pRight, nRight);
  dlwInit(&writer, DL_DOCIDS, pOut);

  while( !dlrAtEnd(&left) ){
    while( !dlrAtEnd(&right) && dlrDocid(&right)<dlrDocid(&left) ){
      dlrStep(&right);
    }
    if( dlrAtEnd(&right) || dlrDocid(&left)<dlrDocid(&right) ){
      dlwAdd(&writer, dlrDocid(&left));
    }
    dlrStep(&left);
  }

  dlrDestroy(&left);
  dlrDestroy(&right);
  dlwDestroy(&writer);
}

static char *string_dup_n(const char *s, int n){
  char *str = sqlite3_malloc(n + 1);
  memcpy(str, s, n);
  str[n] = '\0';
  return str;
}

/* Duplicate a string; the caller must free() the returned string.
 * (We don't use strdup() since it is not part of the standard C library and
 * may not be available everywhere.) */
static char *string_dup(const char *s){
  return string_dup_n(s, strlen(s));
}

/* Format a string, replacing each occurrence of the % character with
 * zDb.zName.  This may be more convenient than sqlite_mprintf()
 * when one string is used repeatedly in a format string.
 * The caller must free() the returned string. */
static char *string_format(const char *zFormat,
                           const char *zDb, const char *zName){
  const char *p;
  size_t len = 0;
  size_t nDb = strlen(zDb);
  size_t nName = strlen(zName);
  size_t nFullTableName = nDb+1+nName;
  char *result;
  char *r;

  /* first compute length needed */
  for(p = zFormat ; *p ; ++p){
    len += (*p=='%' ? nFullTableName : 1);
  }
  len += 1;  /* for null terminator */

  r = result = sqlite3_malloc(len);
  for(p = zFormat; *p; ++p){
    if( *p=='%' ){
      memcpy(r, zDb, nDb);
      r += nDb;
      *r++ = '.';
      memcpy(r, zName, nName);
      r += nName;
    } else {
      *r++ = *p;
    }
  }
  *r++ = '\0';
  assert( r == result + len );
  return result;
}

static int sql_exec(sqlite3 *db, const char *zDb, const char *zName,
                    const char *zFormat){
  char *zCommand = string_format(zFormat, zDb, zName);
  int rc;
  TRACE(("FTS2 sql: %s\n", zCommand));
  rc = sqlite3_exec(db, zCommand, NULL, 0, NULL);
  sqlite3_free(zCommand);
  return rc;
}

static int sql_prepare(sqlite3 *db, const char *zDb, const char *zName,
                       sqlite3_stmt **ppStmt, const char *zFormat){
  char *zCommand = string_format(zFormat, zDb, zName);
  int rc;
  TRACE(("FTS2 prepare: %s\n", zCommand));
  rc = sqlite3_prepare_v2(db, zCommand, -1, ppStmt, NULL);
  sqlite3_free(zCommand);
  return rc;
}

/* end utility functions */

/* Forward reference */
typedef struct fulltext_vtab fulltext_vtab;

/* A single term in a query is represented by an instances of
** the following structure.
*/
typedef struct QueryTerm {
  short int nPhrase; /* How many following terms are part of the same phrase */
  short int iPhrase; /* This is the i-th term of a phrase. */
  short int iColumn; /* Column of the index that must match this term */
  signed char isOr;  /* this term is preceded by "OR" */
  signed char isNot; /* this term is preceded by "-" */
  signed char isPrefix; /* this term is followed by "*" */
  char *pTerm;       /* text of the term.  '\000' terminated.  malloced */
  int nTerm;         /* Number of bytes in pTerm[] */
} QueryTerm;


/* A query string is parsed into a Query structure.
 *
 * We could, in theory, allow query strings to be complicated
 * nested expressions with precedence determined by parentheses.
 * But none of the major search engines do this.  (Perhaps the
 * feeling is that an parenthesized expression is two complex of
 * an idea for the average user to grasp.)  Taking our lead from
 * the major search engines, we will allow queries to be a list
 * of terms (with an implied AND operator) or phrases in double-quotes,
 * with a single optional "-" before each non-phrase term to designate
 * negation and an optional OR connector.
 *
 * OR binds more tightly than the implied AND, which is what the
 * major search engines seem to do.  So, for example:
 * 
 *    [one two OR three]     ==>    one AND (two OR three)
 *    [one OR two three]     ==>    (one OR two) AND three
 *
 * A "-" before a term matches all entries that lack that term.
 * The "-" must occur immediately before the term with in intervening
 * space.  This is how the search engines do it.
 *
 * A NOT term cannot be the right-hand operand of an OR.  If this
 * occurs in the query string, the NOT is ignored:
 *
 *    [one OR -two]          ==>    one OR two
 *
 */
typedef struct Query {
  fulltext_vtab *pFts;  /* The full text index */
  int nTerms;           /* Number of terms in the query */
  QueryTerm *pTerms;    /* Array of terms.  Space obtained from malloc() */
  int nextIsOr;         /* Set the isOr flag on the next inserted term */
  int nextColumn;       /* Next word parsed must be in this column */
  int dfltColumn;       /* The default column */
} Query;


/*
** An instance of the following structure keeps track of generated
** matching-word offset information and snippets.
*/
typedef struct Snippet {
  int nMatch;     /* Total number of matches */
  int nAlloc;     /* Space allocated for aMatch[] */
  struct snippetMatch { /* One entry for each matching term */
    char snStatus;       /* Status flag for use while constructing snippets */
    short int iCol;      /* The column that contains the match */
    short int iTerm;     /* The index in Query.pTerms[] of the matching term */
    short int nByte;     /* Number of bytes in the term */
    int iStart;          /* The offset to the first character of the term */
  } *aMatch;      /* Points to space obtained from malloc */
  char *zOffset;  /* Text rendering of aMatch[] */
  int nOffset;    /* strlen(zOffset) */
  char *zSnippet; /* Snippet text */
  int nSnippet;   /* strlen(zSnippet) */
} Snippet;


typedef enum QueryType {
  QUERY_GENERIC,   /* table scan */
  QUERY_ROWID,     /* lookup by rowid */
  QUERY_FULLTEXT   /* QUERY_FULLTEXT + [i] is a full-text search for column i*/
} QueryType;

typedef enum fulltext_statement {
  CONTENT_INSERT_STMT,
  CONTENT_SELECT_STMT,
  CONTENT_UPDATE_STMT,
  CONTENT_DELETE_STMT,
  CONTENT_EXISTS_STMT,

  BLOCK_INSERT_STMT,
  BLOCK_SELECT_STMT,
  BLOCK_DELETE_STMT,
  BLOCK_DELETE_ALL_STMT,

  SEGDIR_MAX_INDEX_STMT,
  SEGDIR_SET_STMT,
  SEGDIR_SELECT_LEVEL_STMT,
  SEGDIR_SPAN_STMT,
  SEGDIR_DELETE_STMT,
  SEGDIR_SELECT_SEGMENT_STMT,
  SEGDIR_SELECT_ALL_STMT,
  SEGDIR_DELETE_ALL_STMT,
  SEGDIR_COUNT_STMT,

  MAX_STMT                     /* Always at end! */
} fulltext_statement;

/* These must exactly match the enum above. */
/* TODO(shess): Is there some risk that a statement will be used in two
** cursors at once, e.g.  if a query joins a virtual table to itself?
** If so perhaps we should move some of these to the cursor object.
*/
static const char *const fulltext_zStatement[MAX_STMT] = {
  /* CONTENT_INSERT */ NULL,  /* generated in contentInsertStatement() */
  /* CONTENT_SELECT */ "select * from %_content where rowid = ?",
  /* CONTENT_UPDATE */ NULL,  /* generated in contentUpdateStatement() */
  /* CONTENT_DELETE */ "delete from %_content where rowid = ?",
  /* CONTENT_EXISTS */ "select rowid from %_content limit 1",

  /* BLOCK_INSERT */ "insert into %_segments values (?)",
  /* BLOCK_SELECT */ "select block from %_segments where rowid = ?",
  /* BLOCK_DELETE */ "delete from %_segments where rowid between ? and ?",
  /* BLOCK_DELETE_ALL */ "delete from %_segments",

  /* SEGDIR_MAX_INDEX */ "select max(idx) from %_segdir where level = ?",
  /* SEGDIR_SET */ "insert into %_segdir values (?, ?, ?, ?, ?, ?)",
  /* SEGDIR_SELECT_LEVEL */
  "select start_block, leaves_end_block, root from %_segdir "
  " where level = ? order by idx",
  /* SEGDIR_SPAN */
  "select min(start_block), max(end_block) from %_segdir "
  " where level = ? and start_block <> 0",
  /* SEGDIR_DELETE */ "delete from %_segdir where level = ?",

  /* NOTE(shess): The first three results of the following two
  ** statements must match.
  */
  /* SEGDIR_SELECT_SEGMENT */
  "select start_block, leaves_end_block, root from %_segdir "
  " where level = ? and idx = ?",
  /* SEGDIR_SELECT_ALL */
  "select start_block, leaves_end_block, root from %_segdir "
  " order by level desc, idx asc",
  /* SEGDIR_DELETE_ALL */ "delete from %_segdir",
  /* SEGDIR_COUNT */ "select count(*), ifnull(max(level),0) from %_segdir",
};

/*
** A connection to a fulltext index is an instance of the following
** structure.  The xCreate and xConnect methods create an instance
** of this structure and xDestroy and xDisconnect free that instance.
** All other methods receive a pointer to the structure as one of their
** arguments.
*/
struct fulltext_vtab {
  sqlite3_vtab base;               /* Base class used by SQLite core */
  sqlite3 *db;                     /* The database connection */
  const char *zDb;                 /* logical database name */
  const char *zName;               /* virtual table name */
  int nColumn;                     /* number of columns in virtual table */
  char **azColumn;                 /* column names.  malloced */
  char **azContentColumn;          /* column names in content table; malloced */
  sqlite3_tokenizer *pTokenizer;   /* tokenizer for inserts and queries */

  /* Precompiled statements which we keep as long as the table is
  ** open.
  */
  sqlite3_stmt *pFulltextStatements[MAX_STMT];

  /* Precompiled statements used for segment merges.  We run a
  ** separate select across the leaf level of each tree being merged.
  */
  sqlite3_stmt *pLeafSelectStmts[MERGE_COUNT];
  /* The statement used to prepare pLeafSelectStmts. */
#define LEAF_SELECT \
  "select block from %_segments where rowid between ? and ? order by rowid"

  /* These buffer pending index updates during transactions.
  ** nPendingData estimates the memory size of the pending data.  It
  ** doesn't include the hash-bucket overhead, nor any malloc
  ** overhead.  When nPendingData exceeds kPendingThreshold, the
  ** buffer is flushed even before the transaction closes.
  ** pendingTerms stores the data, and is only valid when nPendingData
  ** is >=0 (nPendingData<0 means pendingTerms has not been
  ** initialized).  iPrevDocid is the last docid written, used to make
  ** certain we're inserting in sorted order.
  */
  int nPendingData;
#define kPendingThreshold (1*1024*1024)
  sqlite_int64 iPrevDocid;
  fts2Hash pendingTerms;
};

/*
** When the core wants to do a query, it create a cursor using a
** call to xOpen.  This structure is an instance of a cursor.  It
** is destroyed by xClose.
*/
typedef struct fulltext_cursor {
  sqlite3_vtab_cursor base;        /* Base class used by SQLite core */
  QueryType iCursorType;           /* Copy of sqlite3_index_info.idxNum */
  sqlite3_stmt *pStmt;             /* Prepared statement in use by the cursor */
  int eof;                         /* True if at End Of Results */
  Query q;                         /* Parsed query string */
  Snippet snippet;                 /* Cached snippet for the current row */
  int iColumn;                     /* Column being searched */
  DataBuffer result;               /* Doclist results from fulltextQuery */
  DLReader reader;                 /* Result reader if result not empty */
} fulltext_cursor;

static struct fulltext_vtab *cursor_vtab(fulltext_cursor *c){
  return (fulltext_vtab *) c->base.pVtab;
}

static const sqlite3_module fts2Module;   /* forward declaration */

/* Return a dynamically generated statement of the form
 *   insert into %_content (rowid, ...) values (?, ...)
 */
static const char *contentInsertStatement(fulltext_vtab *v){
  StringBuffer sb;
  int i;

  initStringBuffer(&sb);
  append(&sb, "insert into %_content (rowid, ");
  appendList(&sb, v->nColumn, v->azContentColumn);
  append(&sb, ") values (?");
  for(i=0; i<v->nColumn; ++i)
    append(&sb, ", ?");
  append(&sb, ")");
  return stringBufferData(&sb);
}

/* Return a dynamically generated statement of the form
 *   update %_content set [col_0] = ?, [col_1] = ?, ...
 *                    where rowid = ?
 */
static const char *contentUpdateStatement(fulltext_vtab *v){
  StringBuffer sb;
  int i;

  initStringBuffer(&sb);
  append(&sb, "update %_content set ");
  for(i=0; i<v->nColumn; ++i) {
    if( i>0 ){
      append(&sb, ", ");
    }
    append(&sb, v->azContentColumn[i]);
    append(&sb, " = ?");
  }
  append(&sb, " where rowid = ?");
  return stringBufferData(&sb);
}

/* Puts a freshly-prepared statement determined by iStmt in *ppStmt.
** If the indicated statement has never been prepared, it is prepared
** and cached, otherwise the cached version is reset.
*/
static int sql_get_statement(fulltext_vtab *v, fulltext_statement iStmt,
                             sqlite3_stmt **ppStmt){
  assert( iStmt<MAX_STMT );
  if( v->pFulltextStatements[iStmt]==NULL ){
    const char *zStmt;
    int rc;
    switch( iStmt ){
      case CONTENT_INSERT_STMT:
        zStmt = contentInsertStatement(v); break;
      case CONTENT_UPDATE_STMT:
        zStmt = contentUpdateStatement(v); break;
      default:
        zStmt = fulltext_zStatement[iStmt];
    }
    rc = sql_prepare(v->db, v->zDb, v->zName, &v->pFulltextStatements[iStmt],
                         zStmt);
    if( zStmt != fulltext_zStatement[iStmt]) sqlite3_free((void *) zStmt);
    if( rc!=SQLITE_OK ) return rc;
  } else {
    int rc = sqlite3_reset(v->pFulltextStatements[iStmt]);
    if( rc!=SQLITE_OK ) return rc;
  }

  *ppStmt = v->pFulltextStatements[iStmt];
  return SQLITE_OK;
}

/* Like sqlite3_step(), but convert SQLITE_DONE to SQLITE_OK and
** SQLITE_ROW to SQLITE_ERROR.  Useful for statements like UPDATE,
** where we expect no results.
*/
static int sql_single_step(sqlite3_stmt *s){
  int rc = sqlite3_step(s);
  return (rc==SQLITE_DONE) ? SQLITE_OK : rc;
}

/* Like sql_get_statement(), but for special replicated LEAF_SELECT
** statements.  idx -1 is a special case for an uncached version of
** the statement (used in the optimize implementation).
*/
/* TODO(shess) Write version for generic statements and then share
** that between the cached-statement functions.
*/
static int sql_get_leaf_statement(fulltext_vtab *v, int idx,
                                  sqlite3_stmt **ppStmt){
  assert( idx>=-1 && idx<MERGE_COUNT );
  if( idx==-1 ){
    return sql_prepare(v->db, v->zDb, v->zName, ppStmt, LEAF_SELECT);
  }else if( v->pLeafSelectStmts[idx]==NULL ){
    int rc = sql_prepare(v->db, v->zDb, v->zName, &v->pLeafSelectStmts[idx],
                         LEAF_SELECT);
    if( rc!=SQLITE_OK ) return rc;
  }else{
    int rc = sqlite3_reset(v->pLeafSelectStmts[idx]);
    if( rc!=SQLITE_OK ) return rc;
  }

  *ppStmt = v->pLeafSelectStmts[idx];
  return SQLITE_OK;
}

/* insert into %_content (rowid, ...) values ([rowid], [pValues]) */
static int content_insert(fulltext_vtab *v, sqlite3_value *rowid,
                          sqlite3_value **pValues){
  sqlite3_stmt *s;
  int i;
  int rc = sql_get_statement(v, CONTENT_INSERT_STMT, &s);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_bind_value(s, 1, rowid);
  if( rc!=SQLITE_OK ) return rc;

  for(i=0; i<v->nColumn; ++i){
    rc = sqlite3_bind_value(s, 2+i, pValues[i]);
    if( rc!=SQLITE_OK ) return rc;
  }

  return sql_single_step(s);
}

/* update %_content set col0 = pValues[0], col1 = pValues[1], ...
 *                  where rowid = [iRowid] */
static int content_update(fulltext_vtab *v, sqlite3_value **pValues,
                          sqlite_int64 iRowid){
  sqlite3_stmt *s;
  int i;
  int rc = sql_get_statement(v, CONTENT_UPDATE_STMT, &s);
  if( rc!=SQLITE_OK ) return rc;

  for(i=0; i<v->nColumn; ++i){
    rc = sqlite3_bind_value(s, 1+i, pValues[i]);
    if( rc!=SQLITE_OK ) return rc;
  }

  rc = sqlite3_bind_int64(s, 1+v->nColumn, iRowid);
  if( rc!=SQLITE_OK ) return rc;

  return sql_single_step(s);
}

static void freeStringArray(int nString, const char **pString){
  int i;

  for (i=0 ; i < nString ; ++i) {
    if( pString[i]!=NULL ) sqlite3_free((void *) pString[i]);
  }
  sqlite3_free((void *) pString);
}

/* select * from %_content where rowid = [iRow]
 * The caller must delete the returned array and all strings in it.
 * null fields will be NULL in the returned array.
 *
 * TODO: Perhaps we should return pointer/length strings here for consistency
 * with other code which uses pointer/length. */
static int content_select(fulltext_vtab *v, sqlite_int64 iRow,
                          const char ***pValues){
  sqlite3_stmt *s;
  const char **values;
  int i;
  int rc;

  *pValues = NULL;

  rc = sql_get_statement(v, CONTENT_SELECT_STMT, &s);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_bind_int64(s, 1, iRow);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_step(s);
  if( rc!=SQLITE_ROW ) return rc;

  values = (const char **) sqlite3_malloc(v->nColumn * sizeof(const char *));
  for(i=0; i<v->nColumn; ++i){
    if( sqlite3_column_type(s, i)==SQLITE_NULL ){
      values[i] = NULL;
    }else{
      values[i] = string_dup((char*)sqlite3_column_text(s, i));
    }
  }

  /* We expect only one row.  We must execute another sqlite3_step()
   * to complete the iteration; otherwise the table will remain locked. */
  rc = sqlite3_step(s);
  if( rc==SQLITE_DONE ){
    *pValues = values;
    return SQLITE_OK;
  }

  freeStringArray(v->nColumn, values);
  return rc;
}

/* delete from %_content where rowid = [iRow ] */
static int content_delete(fulltext_vtab *v, sqlite_int64 iRow){
  sqlite3_stmt *s;
  int rc = sql_get_statement(v, CONTENT_DELETE_STMT, &s);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_bind_int64(s, 1, iRow);
  if( rc!=SQLITE_OK ) return rc;

  return sql_single_step(s);
}

/* Returns SQLITE_ROW if any rows exist in %_content, SQLITE_DONE if
** no rows exist, and any error in case of failure.
*/
static int content_exists(fulltext_vtab *v){
  sqlite3_stmt *s;
  int rc = sql_get_statement(v, CONTENT_EXISTS_STMT, &s);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_step(s);
  if( rc!=SQLITE_ROW ) return rc;

  /* We expect only one row.  We must execute another sqlite3_step()
   * to complete the iteration; otherwise the table will remain locked. */
  rc = sqlite3_step(s);
  if( rc==SQLITE_DONE ) return SQLITE_ROW;
  if( rc==SQLITE_ROW ) return SQLITE_ERROR;
  return rc;
}

/* insert into %_segments values ([pData])
**   returns assigned rowid in *piBlockid
*/
static int block_insert(fulltext_vtab *v, const char *pData, int nData,
                        sqlite_int64 *piBlockid){
  sqlite3_stmt *s;
  int rc = sql_get_statement(v, BLOCK_INSERT_STMT, &s);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_bind_blob(s, 1, pData, nData, SQLITE_STATIC);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_step(s);
  if( rc==SQLITE_ROW ) return SQLITE_ERROR;
  if( rc!=SQLITE_DONE ) return rc;

  *piBlockid = sqlite3_last_insert_rowid(v->db);
  return SQLITE_OK;
}

/* delete from %_segments
**   where rowid between [iStartBlockid] and [iEndBlockid]
**
** Deletes the range of blocks, inclusive, used to delete the blocks
** which form a segment.
*/
static int block_delete(fulltext_vtab *v,
                        sqlite_int64 iStartBlockid, sqlite_int64 iEndBlockid){
  sqlite3_stmt *s;
  int rc = sql_get_statement(v, BLOCK_DELETE_STMT, &s);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_bind_int64(s, 1, iStartBlockid);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_bind_int64(s, 2, iEndBlockid);
  if( rc!=SQLITE_OK ) return rc;

  return sql_single_step(s);
}

/* Returns SQLITE_ROW with *pidx set to the maximum segment idx found
** at iLevel.  Returns SQLITE_DONE if there are no segments at
** iLevel.  Otherwise returns an error.
*/
static int segdir_max_index(fulltext_vtab *v, int iLevel, int *pidx){
  sqlite3_stmt *s;
  int rc = sql_get_statement(v, SEGDIR_MAX_INDEX_STMT, &s);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_bind_int(s, 1, iLevel);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_step(s);
  /* Should always get at least one row due to how max() works. */
  if( rc==SQLITE_DONE ) return SQLITE_DONE;
  if( rc!=SQLITE_ROW ) return rc;

  /* NULL means that there were no inputs to max(). */
  if( SQLITE_NULL==sqlite3_column_type(s, 0) ){
    rc = sqlite3_step(s);
    if( rc==SQLITE_ROW ) return SQLITE_ERROR;
    return rc;
  }

  *pidx = sqlite3_column_int(s, 0);

  /* We expect only one row.  We must execute another sqlite3_step()
   * to complete the iteration; otherwise the table will remain locked. */
  rc = sqlite3_step(s);
  if( rc==SQLITE_ROW ) return SQLITE_ERROR;
  if( rc!=SQLITE_DONE ) return rc;
  return SQLITE_ROW;
}

/* insert into %_segdir values (
**   [iLevel], [idx],
**   [iStartBlockid], [iLeavesEndBlockid], [iEndBlockid],
**   [pRootData]
** )
*/
static int segdir_set(fulltext_vtab *v, int iLevel, int idx,
                      sqlite_int64 iStartBlockid,
                      sqlite_int64 iLeavesEndBlockid,
                      sqlite_int64 iEndBlockid,
                      const char *pRootData, int nRootData){
  sqlite3_stmt *s;
  int rc = sql_get_statement(v, SEGDIR_SET_STMT, &s);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_bind_int(s, 1, iLevel);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_bind_int(s, 2, idx);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_bind_int64(s, 3, iStartBlockid);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_bind_int64(s, 4, iLeavesEndBlockid);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_bind_int64(s, 5, iEndBlockid);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_bind_blob(s, 6, pRootData, nRootData, SQLITE_STATIC);
  if( rc!=SQLITE_OK ) return rc;

  return sql_single_step(s);
}

/* Queries %_segdir for the block span of the segments in level
** iLevel.  Returns SQLITE_DONE if there are no blocks for iLevel,
** SQLITE_ROW if there are blocks, else an error.
*/
static int segdir_span(fulltext_vtab *v, int iLevel,
                       sqlite_int64 *piStartBlockid,
                       sqlite_int64 *piEndBlockid){
  sqlite3_stmt *s;
  int rc = sql_get_statement(v, SEGDIR_SPAN_STMT, &s);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_bind_int(s, 1, iLevel);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_step(s);
  if( rc==SQLITE_DONE ) return SQLITE_DONE;  /* Should never happen */
  if( rc!=SQLITE_ROW ) return rc;

  /* This happens if all segments at this level are entirely inline. */
  if( SQLITE_NULL==sqlite3_column_type(s, 0) ){
    /* We expect only one row.  We must execute another sqlite3_step()
     * to complete the iteration; otherwise the table will remain locked. */
    int rc2 = sqlite3_step(s);
    if( rc2==SQLITE_ROW ) return SQLITE_ERROR;
    return rc2;
  }

  *piStartBlockid = sqlite3_column_int64(s, 0);
  *piEndBlockid = sqlite3_column_int64(s, 1);

  /* We expect only one row.  We must execute another sqlite3_step()
   * to complete the iteration; otherwise the table will remain locked. */
  rc = sqlite3_step(s);
  if( rc==SQLITE_ROW ) return SQLITE_ERROR;
  if( rc!=SQLITE_DONE ) return rc;
  return SQLITE_ROW;
}

/* Delete the segment blocks and segment directory records for all
** segments at iLevel.
*/
static int segdir_delete(fulltext_vtab *v, int iLevel){
  sqlite3_stmt *s;
  sqlite_int64 iStartBlockid, iEndBlockid;
  int rc = segdir_span(v, iLevel, &iStartBlockid, &iEndBlockid);
  if( rc!=SQLITE_ROW && rc!=SQLITE_DONE ) return rc;

  if( rc==SQLITE_ROW ){
    rc = block_delete(v, iStartBlockid, iEndBlockid);
    if( rc!=SQLITE_OK ) return rc;
  }

  /* Delete the segment directory itself. */
  rc = sql_get_statement(v, SEGDIR_DELETE_STMT, &s);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_bind_int64(s, 1, iLevel);
  if( rc!=SQLITE_OK ) return rc;

  return sql_single_step(s);
}

/* Delete entire fts index, SQLITE_OK on success, relevant error on
** failure.
*/
static int segdir_delete_all(fulltext_vtab *v){
  sqlite3_stmt *s;
  int rc = sql_get_statement(v, SEGDIR_DELETE_ALL_STMT, &s);
  if( rc!=SQLITE_OK ) return rc;

  rc = sql_single_step(s);
  if( rc!=SQLITE_OK ) return rc;

  rc = sql_get_statement(v, BLOCK_DELETE_ALL_STMT, &s);
  if( rc!=SQLITE_OK ) return rc;

  return sql_single_step(s);
}

/* Returns SQLITE_OK with *pnSegments set to the number of entries in
** %_segdir and *piMaxLevel set to the highest level which has a
** segment.  Otherwise returns the SQLite error which caused failure.
*/
static int segdir_count(fulltext_vtab *v, int *pnSegments, int *piMaxLevel){
  sqlite3_stmt *s;
  int rc = sql_get_statement(v, SEGDIR_COUNT_STMT, &s);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_step(s);
  /* TODO(shess): This case should not be possible?  Should stronger
  ** measures be taken if it happens?
  */
  if( rc==SQLITE_DONE ){
    *pnSegments = 0;
    *piMaxLevel = 0;
    return SQLITE_OK;
  }
  if( rc!=SQLITE_ROW ) return rc;

  *pnSegments = sqlite3_column_int(s, 0);
  *piMaxLevel = sqlite3_column_int(s, 1);

  /* We expect only one row.  We must execute another sqlite3_step()
   * to complete the iteration; otherwise the table will remain locked. */
  rc = sqlite3_step(s);
  if( rc==SQLITE_DONE ) return SQLITE_OK;
  if( rc==SQLITE_ROW ) return SQLITE_ERROR;
  return rc;
}

/* TODO(shess) clearPendingTerms() is far down the file because
** writeZeroSegment() is far down the file because LeafWriter is far
** down the file.  Consider refactoring the code to move the non-vtab
** code above the vtab code so that we don't need this forward
** reference.
*/
static int clearPendingTerms(fulltext_vtab *v);

/*
** Free the memory used to contain a fulltext_vtab structure.
*/
static void fulltext_vtab_destroy(fulltext_vtab *v){
  int iStmt, i;

  TRACE(("FTS2 Destroy %p\n", v));
  for( iStmt=0; iStmt<MAX_STMT; iStmt++ ){
    if( v->pFulltextStatements[iStmt]!=NULL ){
      sqlite3_finalize(v->pFulltextStatements[iStmt]);
      v->pFulltextStatements[iStmt] = NULL;
    }
  }

  for( i=0; i<MERGE_COUNT; i++ ){
    if( v->pLeafSelectStmts[i]!=NULL ){
      sqlite3_finalize(v->pLeafSelectStmts[i]);
      v->pLeafSelectStmts[i] = NULL;
    }
  }

  if( v->pTokenizer!=NULL ){
    v->pTokenizer->pModule->xDestroy(v->pTokenizer);
    v->pTokenizer = NULL;
  }

  clearPendingTerms(v);

  sqlite3_free(v->azColumn);
  for(i = 0; i < v->nColumn; ++i) {
    sqlite3_free(v->azContentColumn[i]);
  }
  sqlite3_free(v->azContentColumn);
  sqlite3_free(v);
}

/*
** Token types for parsing the arguments to xConnect or xCreate.
*/
#define TOKEN_EOF         0    /* End of file */
#define TOKEN_SPACE       1    /* Any kind of whitespace */
#define TOKEN_ID          2    /* An identifier */
#define TOKEN_STRING      3    /* A string literal */
#define TOKEN_PUNCT       4    /* A single punctuation character */

/*
** If X is a character that can be used in an identifier then
** IdChar(X) will be true.  Otherwise it is false.
**
** For ASCII, any character with the high-order bit set is
** allowed in an identifier.  For 7-bit characters, 
** sqlite3IsIdChar[X] must be 1.
**
** Ticket #1066.  the SQL standard does not allow '$' in the
** middle of identfiers.  But many SQL implementations do. 
** SQLite will allow '$' in identifiers for compatibility.
** But the feature is undocumented.
*/
static const char isIdChar[] = {
/* x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF */
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 2x */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,  /* 3x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 4x */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,  /* 5x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 6x */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,  /* 7x */
};
#define IdChar(C)  (((c=C)&0x80)!=0 || (c>0x1f && isIdChar[c-0x20]))


/*
** Return the length of the token that begins at z[0]. 
** Store the token type in *tokenType before returning.
*/
static int getToken(const char *z, int *tokenType){
  int i, c;
  switch( *z ){
    case 0: {
      *tokenType = TOKEN_EOF;
      return 0;
    }
    case ' ': case '\t': case '\n': case '\f': case '\r': {
      for(i=1; safe_isspace(z[i]); i++){}
      *tokenType = TOKEN_SPACE;
      return i;
    }
    case '`':
    case '\'':
    case '"': {
      int delim = z[0];
      for(i=1; (c=z[i])!=0; i++){
        if( c==delim ){
          if( z[i+1]==delim ){
            i++;
          }else{
            break;
          }
        }
      }
      *tokenType = TOKEN_STRING;
      return i + (c!=0);
    }
    case '[': {
      for(i=1, c=z[0]; c!=']' && (c=z[i])!=0; i++){}
      *tokenType = TOKEN_ID;
      return i;
    }
    default: {
      if( !IdChar(*z) ){
        break;
      }
      for(i=1; IdChar(z[i]); i++){}
      *tokenType = TOKEN_ID;
      return i;
    }
  }
  *tokenType = TOKEN_PUNCT;
  return 1;
}

/*
** A token extracted from a string is an instance of the following
** structure.
*/
typedef struct Token {
  const char *z;       /* Pointer to token text.  Not '\000' terminated */
  short int n;         /* Length of the token text in bytes. */
} Token;

/*
** Given a input string (which is really one of the argv[] parameters
** passed into xConnect or xCreate) split the string up into tokens.
** Return an array of pointers to '\000' terminated strings, one string
** for each non-whitespace token.
**
** The returned array is terminated by a single NULL pointer.
**
** Space to hold the returned array is obtained from a single
** malloc and should be freed by passing the return value to free().
** The individual strings within the token list are all a part of
** the single memory allocation and will all be freed at once.
*/
static char **tokenizeString(const char *z, int *pnToken){
  int nToken = 0;
  Token *aToken = sqlite3_malloc( strlen(z) * sizeof(aToken[0]) );
  int n = 1;
  int e, i;
  int totalSize = 0;
  char **azToken;
  char *zCopy;
  while( n>0 ){
    n = getToken(z, &e);
    if( e!=TOKEN_SPACE ){
      aToken[nToken].z = z;
      aToken[nToken].n = n;
      nToken++;
      totalSize += n+1;
    }
    z += n;
  }
  azToken = (char**)sqlite3_malloc( nToken*sizeof(char*) + totalSize );
  zCopy = (char*)&azToken[nToken];
  nToken--;
  for(i=0; i<nToken; i++){
    azToken[i] = zCopy;
    n = aToken[i].n;
    memcpy(zCopy, aToken[i].z, n);
    zCopy[n] = 0;
    zCopy += n+1;
  }
  azToken[nToken] = 0;
  sqlite3_free(aToken);
  *pnToken = nToken;
  return azToken;
}

/*
** Convert an SQL-style quoted string into a normal string by removing
** the quote characters.  The conversion is done in-place.  If the
** input does not begin with a quote character, then this routine
** is a no-op.
**
** Examples:
**
**     "abc"   becomes   abc
**     'xyz'   becomes   xyz
**     [pqr]   becomes   pqr
**     `mno`   becomes   mno
*/
static void dequoteString(char *z){
  int quote;
  int i, j;
  if( z==0 ) return;
  quote = z[0];
  switch( quote ){
    case '\'':  break;
    case '"':   break;
    case '`':   break;                /* For MySQL compatibility */
    case '[':   quote = ']';  break;  /* For MS SqlServer compatibility */
    default:    return;
  }
  for(i=1, j=0; z[i]; i++){
    if( z[i]==quote ){
      if( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }else{
        z[j++] = 0;
        break;
      }
    }else{
      z[j++] = z[i];
    }
  }
}

/*
** The input azIn is a NULL-terminated list of tokens.  Remove the first
** token and all punctuation tokens.  Remove the quotes from
** around string literal tokens.
**
** Example:
**
**     input:      tokenize chinese ( 'simplifed' , 'mixed' )
**     output:     chinese simplifed mixed
**
** Another example:
**
**     input:      delimiters ( '[' , ']' , '...' )
**     output:     [ ] ...
*/
static void tokenListToIdList(char **azIn){
  int i, j;
  if( azIn ){
    for(i=0, j=-1; azIn[i]; i++){
      if( safe_isalnum(azIn[i][0]) || azIn[i][1] ){
        dequoteString(azIn[i]);
        if( j>=0 ){
          azIn[j] = azIn[i];
        }
        j++;
      }
    }
    azIn[j] = 0;
  }
}


/*
** Find the first alphanumeric token in the string zIn.  Null-terminate
** this token.  Remove any quotation marks.  And return a pointer to
** the result.
*/
static char *firstToken(char *zIn, char **pzTail){
  int n, ttype;
  while(1){
    n = getToken(zIn, &ttype);
    if( ttype==TOKEN_SPACE ){
      zIn += n;
    }else if( ttype==TOKEN_EOF ){
      *pzTail = zIn;
      return 0;
    }else{
      zIn[n] = 0;
      *pzTail = &zIn[1];
      dequoteString(zIn);
      return zIn;
    }
  }
  /*NOTREACHED*/
}

/* Return true if...
**
**   *  s begins with the string t, ignoring case
**   *  s is longer than t
**   *  The first character of s beyond t is not a alphanumeric
** 
** Ignore leading space in *s.
**
** To put it another way, return true if the first token of
** s[] is t[].
*/
static int startsWith(const char *s, const char *t){
  while( safe_isspace(*s) ){ s++; }
  while( *t ){
    if( safe_tolower(*s++)!=safe_tolower(*t++) ) return 0;
  }
  return *s!='_' && !safe_isalnum(*s);
}

/*
** An instance of this structure defines the "spec" of a
** full text index.  This structure is populated by parseSpec
** and use by fulltextConnect and fulltextCreate.
*/
typedef struct TableSpec {
  const char *zDb;         /* Logical database name */
  const char *zName;       /* Name of the full-text index */
  int nColumn;             /* Number of columns to be indexed */
  char **azColumn;         /* Original names of columns to be indexed */
  char **azContentColumn;  /* Column names for %_content */
  char **azTokenizer;      /* Name of tokenizer and its arguments */
} TableSpec;

/*
** Reclaim all of the memory used by a TableSpec
*/
static void clearTableSpec(TableSpec *p) {
  sqlite3_free(p->azColumn);
  sqlite3_free(p->azContentColumn);
  sqlite3_free(p->azTokenizer);
}

/* Parse a CREATE VIRTUAL TABLE statement, which looks like this:
 *
 * CREATE VIRTUAL TABLE email
 *        USING fts2(subject, body, tokenize mytokenizer(myarg))
 *
 * We return parsed information in a TableSpec structure.
 * 
 */
static int parseSpec(TableSpec *pSpec, int argc, const char *const*argv,
                     char**pzErr){
  int i, n;
  char *z, *zDummy;
  char **azArg;
  const char *zTokenizer = 0;    /* argv[] entry describing the tokenizer */

  assert( argc>=3 );
  /* Current interface:
  ** argv[0] - module name
  ** argv[1] - database name
  ** argv[2] - table name
  ** argv[3..] - columns, optionally followed by tokenizer specification
  **             and snippet delimiters specification.
  */

  /* Make a copy of the complete argv[][] array in a single allocation.
  ** The argv[][] array is read-only and transient.  We can write to the
  ** copy in order to modify things and the copy is persistent.
  */
  CLEAR(pSpec);
  for(i=n=0; i<argc; i++){
    n += strlen(argv[i]) + 1;
  }
  azArg = sqlite3_malloc( sizeof(char*)*argc + n );
  if( azArg==0 ){
    return SQLITE_NOMEM;
  }
  z = (char*)&azArg[argc];
  for(i=0; i<argc; i++){
    azArg[i] = z;
    strcpy(z, argv[i]);
    z += strlen(z)+1;
  }

  /* Identify the column names and the tokenizer and delimiter arguments
  ** in the argv[][] array.
  */
  pSpec->zDb = azArg[1];
  pSpec->zName = azArg[2];
  pSpec->nColumn = 0;
  pSpec->azColumn = azArg;
  zTokenizer = "tokenize simple";
  for(i=3; i<argc; ++i){
    if( startsWith(azArg[i],"tokenize") ){
      zTokenizer = azArg[i];
    }else{
      z = azArg[pSpec->nColumn] = firstToken(azArg[i], &zDummy);
      pSpec->nColumn++;
    }
  }
  if( pSpec->nColumn==0 ){
    azArg[0] = "content";
    pSpec->nColumn = 1;
  }

  /*
  ** Construct the list of content column names.
  **
  ** Each content column name will be of the form cNNAAAA
  ** where NN is the column number and AAAA is the sanitized
  ** column name.  "sanitized" means that special characters are
  ** converted to "_".  The cNN prefix guarantees that all column
  ** names are unique.
  **
  ** The AAAA suffix is not strictly necessary.  It is included
  ** for the convenience of people who might examine the generated
  ** %_content table and wonder what the columns are used for.
  */
  pSpec->azContentColumn = sqlite3_malloc( pSpec->nColumn * sizeof(char *) );
  if( pSpec->azContentColumn==0 ){
    clearTableSpec(pSpec);
    return SQLITE_NOMEM;
  }
  for(i=0; i<pSpec->nColumn; i++){
    char *p;
    pSpec->azContentColumn[i] = sqlite3_mprintf("c%d%s", i, azArg[i]);
    for (p = pSpec->azContentColumn[i]; *p ; ++p) {
      if( !safe_isalnum(*p) ) *p = '_';
    }
  }

  /*
  ** Parse the tokenizer specification string.
  */
  pSpec->azTokenizer = tokenizeString(zTokenizer, &n);
  tokenListToIdList(pSpec->azTokenizer);

  return SQLITE_OK;
}

/*
** Generate a CREATE TABLE statement that describes the schema of
** the virtual table.  Return a pointer to this schema string.
**
** Space is obtained from sqlite3_mprintf() and should be freed
** using sqlite3_free().
*/
static char *fulltextSchema(
  int nColumn,                  /* Number of columns */
  const char *const* azColumn,  /* List of columns */
  const char *zTableName        /* Name of the table */
){
  int i;
  char *zSchema, *zNext;
  const char *zSep = "(";
  zSchema = sqlite3_mprintf("CREATE TABLE x");
  for(i=0; i<nColumn; i++){
    zNext = sqlite3_mprintf("%s%s%Q", zSchema, zSep, azColumn[i]);
    sqlite3_free(zSchema);
    zSchema = zNext;
    zSep = ",";
  }
  zNext = sqlite3_mprintf("%s,%Q)", zSchema, zTableName);
  sqlite3_free(zSchema);
  return zNext;
}

#if GEARS_FTS2_CHANGES
/*
** The fts2 built-in tokenizers - "simple" and "porter" - are implemented
** in files fts2_tokenizer1.c and fts2_porter.c respectively. The following
** two forward declarations are for functions declared in these files
** used to retrieve the respective implementations.
**
** Calling sqlite3Fts2SimpleTokenizerModule() sets the value pointed
** to by the argument to point a the "simple" tokenizer implementation.
** Function ...PorterTokenizerModule() sets *pModule to point to the
** porter tokenizer/stemmer implementation.
*/
void sqlite3Fts2SimpleTokenizerModule(sqlite3_tokenizer_module const**ppModule);
void sqlite3Fts2PorterTokenizerModule(sqlite3_tokenizer_module const**ppModule);
#endif

/*
** Build a new sqlite3_vtab structure that will describe the
** fulltext index defined by spec.
*/
static int constructVtab(
  sqlite3 *db,              /* The SQLite database connection */
  fts2Hash *pHash,          /* Hash table containing tokenizers */
  TableSpec *spec,          /* Parsed spec information from parseSpec() */
  sqlite3_vtab **ppVTab,    /* Write the resulting vtab structure here */
  char **pzErr              /* Write any error message here */
){
  int rc;
  int n;
  fulltext_vtab *v = 0;
  const sqlite3_tokenizer_module *m = NULL;
  char *schema;

  char const *zTok;         /* Name of tokenizer to use for this fts table */
  int nTok;                 /* Length of zTok, including nul terminator */

  v = (fulltext_vtab *) sqlite3_malloc(sizeof(fulltext_vtab));
  if( v==0 ) return SQLITE_NOMEM;
  CLEAR(v);
  /* sqlite will initialize v->base */
  v->db = db;
  v->zDb = spec->zDb;       /* Freed when azColumn is freed */
  v->zName = spec->zName;   /* Freed when azColumn is freed */
  v->nColumn = spec->nColumn;
  v->azContentColumn = spec->azContentColumn;
  spec->azContentColumn = 0;
  v->azColumn = spec->azColumn;
  spec->azColumn = 0;

  if( spec->azTokenizer==0 ){
    return SQLITE_NOMEM;
  }

  zTok = spec->azTokenizer[0]; 
  if( !zTok ){
    zTok = "simple";
  }
  nTok = strlen(zTok)+1;

#if GEARS_FTS2_CHANGES
  if( 0==strcmp(zTok, "simple") ){
    sqlite3Fts2SimpleTokenizerModule(&m);
  }else if( 0==strcmp(zTok, "porter") ){
    sqlite3Fts2PorterTokenizerModule(&m);
  }else{
    *pzErr = sqlite3_mprintf("unknown tokenizer: %s", spec->azTokenizer[0]);
    rc = SQLITE_ERROR;
    goto err;
  }
#else
  m = (sqlite3_tokenizer_module *)sqlite3Fts2HashFind(pHash, zTok, nTok);
  if( !m ){
    *pzErr = sqlite3_mprintf("unknown tokenizer: %s", spec->azTokenizer[0]);
    rc = SQLITE_ERROR;
    goto err;
  }
#endif

  for(n=0; spec->azTokenizer[n]; n++){}
  if( n ){
    rc = m->xCreate(n-1, (const char*const*)&spec->azTokenizer[1],
                    &v->pTokenizer);
  }else{
    rc = m->xCreate(0, 0, &v->pTokenizer);
  }
  if( rc!=SQLITE_OK ) goto err;
  v->pTokenizer->pModule = m;

  /* TODO: verify the existence of backing tables foo_content, foo_term */

  schema = fulltextSchema(v->nColumn, (const char*const*)v->azColumn,
                          spec->zName);
  rc = sqlite3_declare_vtab(db, schema);
  sqlite3_free(schema);
  if( rc!=SQLITE_OK ) goto err;

  memset(v->pFulltextStatements, 0, sizeof(v->pFulltextStatements));

  /* Indicate that the buffer is not live. */
  v->nPendingData = -1;

  *ppVTab = &v->base;
  TRACE(("FTS2 Connect %p\n", v));

  return rc;

err:
  fulltext_vtab_destroy(v);
  return rc;
}

static int fulltextConnect(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVTab,
  char **pzErr
){
  TableSpec spec;
  int rc = parseSpec(&spec, argc, argv, pzErr);
  if( rc!=SQLITE_OK ) return rc;

  rc = constructVtab(db, (fts2Hash *)pAux, &spec, ppVTab, pzErr);
  clearTableSpec(&spec);
  return rc;
}

/* The %_content table holds the text of each document, with
** the rowid used as the docid.
*/
/* TODO(shess) This comment needs elaboration to match the updated
** code.  Work it into the top-of-file comment at that time.
*/
static int fulltextCreate(sqlite3 *db, void *pAux,
                          int argc, const char * const *argv,
                          sqlite3_vtab **ppVTab, char **pzErr){
  int rc;
  TableSpec spec;
  StringBuffer schema;
  TRACE(("FTS2 Create\n"));

  rc = parseSpec(&spec, argc, argv, pzErr);
  if( rc!=SQLITE_OK ) return rc;

  initStringBuffer(&schema);
  append(&schema, "CREATE TABLE %_content(");
  appendList(&schema, spec.nColumn, spec.azContentColumn);
  append(&schema, ")");
  rc = sql_exec(db, spec.zDb, spec.zName, stringBufferData(&schema));
  stringBufferDestroy(&schema);
  if( rc!=SQLITE_OK ) goto out;

  rc = sql_exec(db, spec.zDb, spec.zName,
                "create table %_segments(block blob);");
  if( rc!=SQLITE_OK ) goto out;

  rc = sql_exec(db, spec.zDb, spec.zName,
                "create table %_segdir("
                "  level integer,"
                "  idx integer,"
                "  start_block integer,"
                "  leaves_end_block integer,"
                "  end_block integer,"
                "  root blob,"
                "  primary key(level, idx)"
                ");");
  if( rc!=SQLITE_OK ) goto out;

  rc = constructVtab(db, (fts2Hash *)pAux, &spec, ppVTab, pzErr);

out:
  clearTableSpec(&spec);
  return rc;
}

/* Decide how to handle an SQL query. */
static int fulltextBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info *pInfo){
  int i;
  TRACE(("FTS2 BestIndex\n"));

  for(i=0; i<pInfo->nConstraint; ++i){
    const struct sqlite3_index_constraint *pConstraint;
    pConstraint = &pInfo->aConstraint[i];
    if( pConstraint->usable ) {
      if( pConstraint->iColumn==-1 &&
          pConstraint->op==SQLITE_INDEX_CONSTRAINT_EQ ){
        pInfo->idxNum = QUERY_ROWID;      /* lookup by rowid */
        TRACE(("FTS2 QUERY_ROWID\n"));
      } else if( pConstraint->iColumn>=0 &&
                 pConstraint->op==SQLITE_INDEX_CONSTRAINT_MATCH ){
        /* full-text search */
        pInfo->idxNum = QUERY_FULLTEXT + pConstraint->iColumn;
        TRACE(("FTS2 QUERY_FULLTEXT %d\n", pConstraint->iColumn));
      } else continue;

      pInfo->aConstraintUsage[i].argvIndex = 1;
      pInfo->aConstraintUsage[i].omit = 1;

      /* An arbitrary value for now.
       * TODO: Perhaps rowid matches should be considered cheaper than
       * full-text searches. */
      pInfo->estimatedCost = 1.0;   

      return SQLITE_OK;
    }
  }
  pInfo->idxNum = QUERY_GENERIC;
  return SQLITE_OK;
}

static int fulltextDisconnect(sqlite3_vtab *pVTab){
  TRACE(("FTS2 Disconnect %p\n", pVTab));
  fulltext_vtab_destroy((fulltext_vtab *)pVTab);
  return SQLITE_OK;
}

static int fulltextDestroy(sqlite3_vtab *pVTab){
  fulltext_vtab *v = (fulltext_vtab *)pVTab;
  int rc;

  TRACE(("FTS2 Destroy %p\n", pVTab));
  rc = sql_exec(v->db, v->zDb, v->zName,
                "drop table if exists %_content;"
                "drop table if exists %_segments;"
                "drop table if exists %_segdir;"
                );
  if( rc!=SQLITE_OK ) return rc;

  fulltext_vtab_destroy((fulltext_vtab *)pVTab);
  return SQLITE_OK;
}

static int fulltextOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fulltext_cursor *c;

  c = (fulltext_cursor *) sqlite3_malloc(sizeof(fulltext_cursor));
  if( c ){
    memset(c, 0, sizeof(fulltext_cursor));
    /* sqlite will initialize c->base */
    *ppCursor = &c->base;
    TRACE(("FTS2 Open %p: %p\n", pVTab, c));
    return SQLITE_OK;
  }else{
    return SQLITE_NOMEM;
  }
}


/* Free all of the dynamically allocated memory held by *q
*/
static void queryClear(Query *q){
  int i;
  for(i = 0; i < q->nTerms; ++i){
    sqlite3_free(q->pTerms[i].pTerm);
  }
  sqlite3_free(q->pTerms);
  CLEAR(q);
}

/* Free all of the dynamically allocated memory held by the
** Snippet
*/
static void snippetClear(Snippet *p){
  sqlite3_free(p->aMatch);
  sqlite3_free(p->zOffset);
  sqlite3_free(p->zSnippet);
  CLEAR(p);
}
/*
** Append a single entry to the p->aMatch[] log.
*/
static void snippetAppendMatch(
  Snippet *p,               /* Append the entry to this snippet */
  int iCol, int iTerm,      /* The column and query term */
  int iStart, int nByte     /* Offset and size of the match */
){
  int i;
  struct snippetMatch *pMatch;
  if( p->nMatch+1>=p->nAlloc ){
    p->nAlloc = p->nAlloc*2 + 10;
    p->aMatch = sqlite3_realloc(p->aMatch, p->nAlloc*sizeof(p->aMatch[0]) );
    if( p->aMatch==0 ){
      p->nMatch = 0;
      p->nAlloc = 0;
      return;
    }
  }
  i = p->nMatch++;
  pMatch = &p->aMatch[i];
  pMatch->iCol = iCol;
  pMatch->iTerm = iTerm;
  pMatch->iStart = iStart;
  pMatch->nByte = nByte;
}

/*
** Sizing information for the circular buffer used in snippetOffsetsOfColumn()
*/
#define FTS2_ROTOR_SZ   (32)
#define FTS2_ROTOR_MASK (FTS2_ROTOR_SZ-1)

/*
** Add entries to pSnippet->aMatch[] for every match that occurs against
** document zDoc[0..nDoc-1] which is stored in column iColumn.
*/
static void snippetOffsetsOfColumn(
  Query *pQuery,
  Snippet *pSnippet,
  int iColumn,
  const char *zDoc,
  int nDoc
){
  const sqlite3_tokenizer_module *pTModule;  /* The tokenizer module */
  sqlite3_tokenizer *pTokenizer;             /* The specific tokenizer */
  sqlite3_tokenizer_cursor *pTCursor;        /* Tokenizer cursor */
  fulltext_vtab *pVtab;                /* The full text index */
  int nColumn;                         /* Number of columns in the index */
  const QueryTerm *aTerm;              /* Query string terms */
  int nTerm;                           /* Number of query string terms */  
  int i, j;                            /* Loop counters */
  int rc;                              /* Return code */
  unsigned int match, prevMatch;       /* Phrase search bitmasks */
  const char *zToken;                  /* Next token from the tokenizer */
  int nToken;                          /* Size of zToken */
  int iBegin, iEnd, iPos;              /* Offsets of beginning and end */

  /* The following variables keep a circular buffer of the last
  ** few tokens */
  unsigned int iRotor = 0;             /* Index of current token */
  int iRotorBegin[FTS2_ROTOR_SZ];      /* Beginning offset of token */
  int iRotorLen[FTS2_ROTOR_SZ];        /* Length of token */

  pVtab = pQuery->pFts;
  nColumn = pVtab->nColumn;
  pTokenizer = pVtab->pTokenizer;
  pTModule = pTokenizer->pModule;
  rc = pTModule->xOpen(pTokenizer, zDoc, nDoc, &pTCursor);
  if( rc ) return;
  pTCursor->pTokenizer = pTokenizer;
  aTerm = pQuery->pTerms;
  nTerm = pQuery->nTerms;
  if( nTerm>=FTS2_ROTOR_SZ ){
    nTerm = FTS2_ROTOR_SZ - 1;
  }
  prevMatch = 0;
  while(1){
    rc = pTModule->xNext(pTCursor, &zToken, &nToken, &iBegin, &iEnd, &iPos);
    if( rc ) break;
    iRotorBegin[iRotor&FTS2_ROTOR_MASK] = iBegin;
    iRotorLen[iRotor&FTS2_ROTOR_MASK] = iEnd-iBegin;
    match = 0;
    for(i=0; i<nTerm; i++){
      int iCol;
      iCol = aTerm[i].iColumn;
      if( iCol>=0 && iCol<nColumn && iCol!=iColumn ) continue;
      if( aTerm[i].nTerm>nToken ) continue;
      if( !aTerm[i].isPrefix && aTerm[i].nTerm<nToken ) continue;
      assert( aTerm[i].nTerm<=nToken );
      if( memcmp(aTerm[i].pTerm, zToken, aTerm[i].nTerm) ) continue;
      if( aTerm[i].iPhrase>1 && (prevMatch & (1<<i))==0 ) continue;
      match |= 1<<i;
      if( i==nTerm-1 || aTerm[i+1].iPhrase==1 ){
        for(j=aTerm[i].iPhrase-1; j>=0; j--){
          int k = (iRotor-j) & FTS2_ROTOR_MASK;
          snippetAppendMatch(pSnippet, iColumn, i-j,
                iRotorBegin[k], iRotorLen[k]);
        }
      }
    }
    prevMatch = match<<1;
    iRotor++;
  }
  pTModule->xClose(pTCursor);  
}


/*
** Compute all offsets for the current row of the query.  
** If the offsets have already been computed, this routine is a no-op.
*/
static void snippetAllOffsets(fulltext_cursor *p){
  int nColumn;
  int iColumn, i;
  int iFirst, iLast;
  fulltext_vtab *pFts;

  if( p->snippet.nMatch ) return;
  if( p->q.nTerms==0 ) return;
  pFts = p->q.pFts;
  nColumn = pFts->nColumn;
  iColumn = (p->iCursorType - QUERY_FULLTEXT);
  if( iColumn<0 || iColumn>=nColumn ){
    iFirst = 0;
    iLast = nColumn-1;
  }else{
    iFirst = iColumn;
    iLast = iColumn;
  }
  for(i=iFirst; i<=iLast; i++){
    const char *zDoc;
    int nDoc;
    zDoc = (const char*)sqlite3_column_text(p->pStmt, i+1);
    nDoc = sqlite3_column_bytes(p->pStmt, i+1);
    snippetOffsetsOfColumn(&p->q, &p->snippet, i, zDoc, nDoc);
  }
}

/*
** Convert the information in the aMatch[] array of the snippet
** into the string zOffset[0..nOffset-1].
*/
static void snippetOffsetText(Snippet *p){
  int i;
  int cnt = 0;
  StringBuffer sb;
  char zBuf[200];
  if( p->zOffset ) return;
  initStringBuffer(&sb);
  for(i=0; i<p->nMatch; i++){
    struct snippetMatch *pMatch = &p->aMatch[i];
    zBuf[0] = ' ';
    sqlite3_snprintf(sizeof(zBuf)-1, &zBuf[cnt>0], "%d %d %d %d",
        pMatch->iCol, pMatch->iTerm, pMatch->iStart, pMatch->nByte);
    append(&sb, zBuf);
    cnt++;
  }
  p->zOffset = stringBufferData(&sb);
  p->nOffset = stringBufferLength(&sb);
}

/*
** zDoc[0..nDoc-1] is phrase of text.  aMatch[0..nMatch-1] are a set
** of matching words some of which might be in zDoc.  zDoc is column
** number iCol.
**
** iBreak is suggested spot in zDoc where we could begin or end an
** excerpt.  Return a value similar to iBreak but possibly adjusted
** to be a little left or right so that the break point is better.
*/
static int wordBoundary(
  int iBreak,                   /* The suggested break point */
  const char *zDoc,             /* Document text */
  int nDoc,                     /* Number of bytes in zDoc[] */
  struct snippetMatch *aMatch,  /* Matching words */
  int nMatch,                   /* Number of entries in aMatch[] */
  int iCol                      /* The column number for zDoc[] */
){
  int i;
  if( iBreak<=10 ){
    return 0;
  }
  if( iBreak>=nDoc-10 ){
    return nDoc;
  }
  for(i=0; i<nMatch && aMatch[i].iCol<iCol; i++){}
  while( i<nMatch && aMatch[i].iStart+aMatch[i].nByte<iBreak ){ i++; }
  if( i<nMatch ){
    if( aMatch[i].iStart<iBreak+10 ){
      return aMatch[i].iStart;
    }
    if( i>0 && aMatch[i-1].iStart+aMatch[i-1].nByte>=iBreak ){
      return aMatch[i-1].iStart;
    }
  }
  for(i=1; i<=10; i++){
    if( safe_isspace(zDoc[iBreak-i]) ){
      return iBreak - i + 1;
    }
    if( safe_isspace(zDoc[iBreak+i]) ){
      return iBreak + i + 1;
    }
  }
  return iBreak;
}



/*
** Allowed values for Snippet.aMatch[].snStatus
*/
#define SNIPPET_IGNORE  0   /* It is ok to omit this match from the snippet */
#define SNIPPET_DESIRED 1   /* We want to include this match in the snippet */

/*
** Generate the text of a snippet.
*/
static void snippetText(
  fulltext_cursor *pCursor,   /* The cursor we need the snippet for */
  const char *zStartMark,     /* Markup to appear before each match */
  const char *zEndMark,       /* Markup to appear after each match */
  const char *zEllipsis       /* Ellipsis mark */
){
  int i, j;
  struct snippetMatch *aMatch;
  int nMatch;
  int nDesired;
  StringBuffer sb;
  int tailCol;
  int tailOffset;
  int iCol;
  int nDoc;
  const char *zDoc;
  int iStart, iEnd;
  int tailEllipsis = 0;
  int iMatch;
  

  sqlite3_free(pCursor->snippet.zSnippet);
  pCursor->snippet.zSnippet = 0;
  aMatch = pCursor->snippet.aMatch;
  nMatch = pCursor->snippet.nMatch;
  initStringBuffer(&sb);

  for(i=0; i<nMatch; i++){
    aMatch[i].snStatus = SNIPPET_IGNORE;
  }
  nDesired = 0;
  for(i=0; i<pCursor->q.nTerms; i++){
    for(j=0; j<nMatch; j++){
      if( aMatch[j].iTerm==i ){
        aMatch[j].snStatus = SNIPPET_DESIRED;
        nDesired++;
        break;
      }
    }
  }

  iMatch = 0;
  tailCol = -1;
  tailOffset = 0;
  for(i=0; i<nMatch && nDesired>0; i++){
    if( aMatch[i].snStatus!=SNIPPET_DESIRED ) continue;
    nDesired--;
    iCol = aMatch[i].iCol;
    zDoc = (const char*)sqlite3_column_text(pCursor->pStmt, iCol+1);
    nDoc = sqlite3_column_bytes(pCursor->pStmt, iCol+1);
    iStart = aMatch[i].iStart - 40;
    iStart = wordBoundary(iStart, zDoc, nDoc, aMatch, nMatch, iCol);
    if( iStart<=10 ){
      iStart = 0;
    }
    if( iCol==tailCol && iStart<=tailOffset+20 ){
      iStart = tailOffset;
    }
    if( (iCol!=tailCol && tailCol>=0) || iStart!=tailOffset ){
      trimWhiteSpace(&sb);
      appendWhiteSpace(&sb);
      append(&sb, zEllipsis);
      appendWhiteSpace(&sb);
    }
    iEnd = aMatch[i].iStart + aMatch[i].nByte + 40;
    iEnd = wordBoundary(iEnd, zDoc, nDoc, aMatch, nMatch, iCol);
    if( iEnd>=nDoc-10 ){
      iEnd = nDoc;
      tailEllipsis = 0;
    }else{
      tailEllipsis = 1;
    }
    while( iMatch<nMatch && aMatch[iMatch].iCol<iCol ){ iMatch++; }
    while( iStart<iEnd ){
      while( iMatch<nMatch && aMatch[iMatch].iStart<iStart
             && aMatch[iMatch].iCol<=iCol ){
        iMatch++;
      }
      if( iMatch<nMatch && aMatch[iMatch].iStart<iEnd
             && aMatch[iMatch].iCol==iCol ){
        nappend(&sb, &zDoc[iStart], aMatch[iMatch].iStart - iStart);
        iStart = aMatch[iMatch].iStart;
        append(&sb, zStartMark);
        nappend(&sb, &zDoc[iStart], aMatch[iMatch].nByte);
        append(&sb, zEndMark);
        iStart += aMatch[iMatch].nByte;
        for(j=iMatch+1; j<nMatch; j++){
          if( aMatch[j].iTerm==aMatch[iMatch].iTerm
              && aMatch[j].snStatus==SNIPPET_DESIRED ){
            nDesired--;
            aMatch[j].snStatus = SNIPPET_IGNORE;
          }
        }
      }else{
        nappend(&sb, &zDoc[iStart], iEnd - iStart);
        iStart = iEnd;
      }
    }
    tailCol = iCol;
    tailOffset = iEnd;
  }
  trimWhiteSpace(&sb);
  if( tailEllipsis ){
    appendWhiteSpace(&sb);
    append(&sb, zEllipsis);
  }
  pCursor->snippet.zSnippet = stringBufferData(&sb);
  pCursor->snippet.nSnippet = stringBufferLength(&sb);
}


/*
** Close the cursor.  For additional information see the documentation
** on the xClose method of the virtual table interface.
*/
static int fulltextClose(sqlite3_vtab_cursor *pCursor){
  fulltext_cursor *c = (fulltext_cursor *) pCursor;
  TRACE(("FTS2 Close %p\n", c));
  sqlite3_finalize(c->pStmt);
  queryClear(&c->q);
  snippetClear(&c->snippet);
  if( c->result.nData!=0 ) dlrDestroy(&c->reader);
  dataBufferDestroy(&c->result);
  sqlite3_free(c);
  return SQLITE_OK;
}

static int fulltextNext(sqlite3_vtab_cursor *pCursor){
  fulltext_cursor *c = (fulltext_cursor *) pCursor;
  int rc;

  TRACE(("FTS2 Next %p\n", pCursor));
  snippetClear(&c->snippet);
  if( c->iCursorType < QUERY_FULLTEXT ){
    /* TODO(shess) Handle SQLITE_SCHEMA AND SQLITE_BUSY. */
    rc = sqlite3_step(c->pStmt);
    switch( rc ){
      case SQLITE_ROW:
        c->eof = 0;
        return SQLITE_OK;
      case SQLITE_DONE:
        c->eof = 1;
        return SQLITE_OK;
      default:
        c->eof = 1;
        return rc;
    }
  } else {  /* full-text query */
    rc = sqlite3_reset(c->pStmt);
    if( rc!=SQLITE_OK ) return rc;

    if( c->result.nData==0 || dlrAtEnd(&c->reader) ){
      c->eof = 1;
      return SQLITE_OK;
    }
    rc = sqlite3_bind_int64(c->pStmt, 1, dlrDocid(&c->reader));
    dlrStep(&c->reader);
    if( rc!=SQLITE_OK ) return rc;
    /* TODO(shess) Handle SQLITE_SCHEMA AND SQLITE_BUSY. */
    rc = sqlite3_step(c->pStmt);
    if( rc==SQLITE_ROW ){   /* the case we expect */
      c->eof = 0;
      return SQLITE_OK;
    }
    /* an error occurred; abort */
    return rc==SQLITE_DONE ? SQLITE_ERROR : rc;
  }
}


/* TODO(shess) If we pushed LeafReader to the top of the file, or to
** another file, term_select() could be pushed above
** docListOfTerm().
*/
static int termSelect(fulltext_vtab *v, int iColumn,
                      const char *pTerm, int nTerm, int isPrefix,
                      DocListType iType, DataBuffer *out);

/* Return a DocList corresponding to the query term *pTerm.  If *pTerm
** is the first term of a phrase query, go ahead and evaluate the phrase
** query and return the doclist for the entire phrase query.
**
** The resulting DL_DOCIDS doclist is stored in pResult, which is
** overwritten.
*/
static int docListOfTerm(
  fulltext_vtab *v,   /* The full text index */
  int iColumn,        /* column to restrict to.  No restriction if >=nColumn */
  QueryTerm *pQTerm,  /* Term we are looking for, or 1st term of a phrase */
  DataBuffer *pResult /* Write the result here */
){
  DataBuffer left, right, new;
  int i, rc;

  /* No phrase search if no position info. */
  assert( pQTerm->nPhrase==0 || DL_DEFAULT!=DL_DOCIDS );

  /* This code should never be called with buffered updates. */
  assert( v->nPendingData<0 );

  dataBufferInit(&left, 0);
  rc = termSelect(v, iColumn, pQTerm->pTerm, pQTerm->nTerm, pQTerm->isPrefix,
                  0<pQTerm->nPhrase ? DL_POSITIONS : DL_DOCIDS, &left);
  if( rc ) return rc;
  for(i=1; i<=pQTerm->nPhrase && left.nData>0; i++){
    dataBufferInit(&right, 0);
    rc = termSelect(v, iColumn, pQTerm[i].pTerm, pQTerm[i].nTerm,
                    pQTerm[i].isPrefix, DL_POSITIONS, &right);
    if( rc ){
      dataBufferDestroy(&left);
      return rc;
    }
    dataBufferInit(&new, 0);
    docListPhraseMerge(left.pData, left.nData, right.pData, right.nData,
                       i<pQTerm->nPhrase ? DL_POSITIONS : DL_DOCIDS, &new);
    dataBufferDestroy(&left);
    dataBufferDestroy(&right);
    left = new;
  }
  *pResult = left;
  return SQLITE_OK;
}

/* Add a new term pTerm[0..nTerm-1] to the query *q.
*/
static void queryAdd(Query *q, const char *pTerm, int nTerm){
  QueryTerm *t;
  ++q->nTerms;
  q->pTerms = sqlite3_realloc(q->pTerms, q->nTerms * sizeof(q->pTerms[0]));
  if( q->pTerms==0 ){
    q->nTerms = 0;
    return;
  }
  t = &q->pTerms[q->nTerms - 1];
  CLEAR(t);
  t->pTerm = sqlite3_malloc(nTerm+1);
  memcpy(t->pTerm, pTerm, nTerm);
  t->pTerm[nTerm] = 0;
  t->nTerm = nTerm;
  t->isOr = q->nextIsOr;
  t->isPrefix = 0;
  q->nextIsOr = 0;
  t->iColumn = q->nextColumn;
  q->nextColumn = q->dfltColumn;
}

/*
** Check to see if the string zToken[0...nToken-1] matches any
** column name in the virtual table.   If it does,
** return the zero-indexed column number.  If not, return -1.
*/
static int checkColumnSpecifier(
  fulltext_vtab *pVtab,    /* The virtual table */
  const char *zToken,      /* Text of the token */
  int nToken               /* Number of characters in the token */
){
  int i;
  for(i=0; i<pVtab->nColumn; i++){
    if( memcmp(pVtab->azColumn[i], zToken, nToken)==0
        && pVtab->azColumn[i][nToken]==0 ){
      return i;
    }
  }
  return -1;
}

/*
** Parse the text at pSegment[0..nSegment-1].  Add additional terms
** to the query being assemblied in pQuery.
**
** inPhrase is true if pSegment[0..nSegement-1] is contained within
** double-quotes.  If inPhrase is true, then the first term
** is marked with the number of terms in the phrase less one and
** OR and "-" syntax is ignored.  If inPhrase is false, then every
** term found is marked with nPhrase=0 and OR and "-" syntax is significant.
*/
static int tokenizeSegment(
  sqlite3_tokenizer *pTokenizer,          /* The tokenizer to use */
  const char *pSegment, int nSegment,     /* Query expression being parsed */
  int inPhrase,                           /* True if within "..." */
  Query *pQuery                           /* Append results here */
){
  const sqlite3_tokenizer_module *pModule = pTokenizer->pModule;
  sqlite3_tokenizer_cursor *pCursor;
  int firstIndex = pQuery->nTerms;
  int iCol;
  int nTerm = 1;
  
  int rc = pModule->xOpen(pTokenizer, pSegment, nSegment, &pCursor);
  if( rc!=SQLITE_OK ) return rc;
  pCursor->pTokenizer = pTokenizer;

  while( 1 ){
    const char *pToken;
    int nToken, iBegin, iEnd, iPos;

    rc = pModule->xNext(pCursor,
                        &pToken, &nToken,
                        &iBegin, &iEnd, &iPos);
    if( rc!=SQLITE_OK ) break;
    if( !inPhrase &&
        pSegment[iEnd]==':' &&
         (iCol = checkColumnSpecifier(pQuery->pFts, pToken, nToken))>=0 ){
      pQuery->nextColumn = iCol;
      continue;
    }
    if( !inPhrase && pQuery->nTerms>0 && nToken==2
         && pSegment[iBegin]=='O' && pSegment[iBegin+1]=='R' ){
      pQuery->nextIsOr = 1;
      continue;
    }
    queryAdd(pQuery, pToken, nToken);
    if( !inPhrase && iBegin>0 && pSegment[iBegin-1]=='-' ){
      pQuery->pTerms[pQuery->nTerms-1].isNot = 1;
    }
    if( iEnd<nSegment && pSegment[iEnd]=='*' ){
      pQuery->pTerms[pQuery->nTerms-1].isPrefix = 1;
    }
    pQuery->pTerms[pQuery->nTerms-1].iPhrase = nTerm;
    if( inPhrase ){
      nTerm++;
    }
  }

  if( inPhrase && pQuery->nTerms>firstIndex ){
    pQuery->pTerms[firstIndex].nPhrase = pQuery->nTerms - firstIndex - 1;
  }

  return pModule->xClose(pCursor);
}

/* Parse a query string, yielding a Query object pQuery.
**
** The calling function will need to queryClear() to clean up
** the dynamically allocated memory held by pQuery.
*/
static int parseQuery(
  fulltext_vtab *v,        /* The fulltext index */
  const char *zInput,      /* Input text of the query string */
  int nInput,              /* Size of the input text */
  int dfltColumn,          /* Default column of the index to match against */
  Query *pQuery            /* Write the parse results here. */
){
  int iInput, inPhrase = 0;

  if( zInput==0 ) nInput = 0;
  if( nInput<0 ) nInput = strlen(zInput);
  pQuery->nTerms = 0;
  pQuery->pTerms = NULL;
  pQuery->nextIsOr = 0;
  pQuery->nextColumn = dfltColumn;
  pQuery->dfltColumn = dfltColumn;
  pQuery->pFts = v;

  for(iInput=0; iInput<nInput; ++iInput){
    int i;
    for(i=iInput; i<nInput && zInput[i]!='"'; ++i){}
    if( i>iInput ){
      tokenizeSegment(v->pTokenizer, zInput+iInput, i-iInput, inPhrase,
                       pQuery);
    }
    iInput = i;
    if( i<nInput ){
      assert( zInput[i]=='"' );
      inPhrase = !inPhrase;
    }
  }

  if( inPhrase ){
    /* unmatched quote */
    queryClear(pQuery);
    return SQLITE_ERROR;
  }
  return SQLITE_OK;
}

/* TODO(shess) Refactor the code to remove this forward decl. */
static int flushPendingTerms(fulltext_vtab *v);

/* Perform a full-text query using the search expression in
** zInput[0..nInput-1].  Return a list of matching documents
** in pResult.
**
** Queries must match column iColumn.  Or if iColumn>=nColumn
** they are allowed to match against any column.
*/
static int fulltextQuery(
  fulltext_vtab *v,      /* The full text index */
  int iColumn,           /* Match against this column by default */
  const char *zInput,    /* The query string */
  int nInput,            /* Number of bytes in zInput[] */
  DataBuffer *pResult,   /* Write the result doclist here */
  Query *pQuery          /* Put parsed query string here */
){
  int i, iNext, rc;
  DataBuffer left, right, or, new;
  int nNot = 0;
  QueryTerm *aTerm;

  /* TODO(shess) Instead of flushing pendingTerms, we could query for
  ** the relevant term and merge the doclist into what we receive from
  ** the database.  Wait and see if this is a common issue, first.
  **
  ** A good reason not to flush is to not generate update-related
  ** error codes from here.
  */

  /* Flush any buffered updates before executing the query. */
  rc = flushPendingTerms(v);
  if( rc!=SQLITE_OK ) return rc;

  /* TODO(shess) I think that the queryClear() calls below are not
  ** necessary, because fulltextClose() already clears the query.
  */
  rc = parseQuery(v, zInput, nInput, iColumn, pQuery);
  if( rc!=SQLITE_OK ) return rc;

  /* Empty or NULL queries return no results. */
  if( pQuery->nTerms==0 ){
    dataBufferInit(pResult, 0);
    return SQLITE_OK;
  }

  /* Merge AND terms. */
  /* TODO(shess) I think we can early-exit if( i>nNot && left.nData==0 ). */
  aTerm = pQuery->pTerms;
  for(i = 0; i<pQuery->nTerms; i=iNext){
    if( aTerm[i].isNot ){
      /* Handle all NOT terms in a separate pass */
      nNot++;
      iNext = i + aTerm[i].nPhrase+1;
      continue;
    }
    iNext = i + aTerm[i].nPhrase + 1;
    rc = docListOfTerm(v, aTerm[i].iColumn, &aTerm[i], &right);
    if( rc ){
      if( i!=nNot ) dataBufferDestroy(&left);
      queryClear(pQuery);
      return rc;
    }
    while( iNext<pQuery->nTerms && aTerm[iNext].isOr ){
      rc = docListOfTerm(v, aTerm[iNext].iColumn, &aTerm[iNext], &or);
      iNext += aTerm[iNext].nPhrase + 1;
      if( rc ){
        if( i!=nNot ) dataBufferDestroy(&left);
        dataBufferDestroy(&right);
        queryClear(pQuery);
        return rc;
      }
      dataBufferInit(&new, 0);
      docListOrMerge(right.pData, right.nData, or.pData, or.nData, &new);
      dataBufferDestroy(&right);
      dataBufferDestroy(&or);
      right = new;
    }
    if( i==nNot ){           /* first term processed. */
      left = right;
    }else{
      dataBufferInit(&new, 0);
      docListAndMerge(left.pData, left.nData, right.pData, right.nData, &new);
      dataBufferDestroy(&right);
      dataBufferDestroy(&left);
      left = new;
    }
  }

  if( nNot==pQuery->nTerms ){
    /* We do not yet know how to handle a query of only NOT terms */
    return SQLITE_ERROR;
  }

  /* Do the EXCEPT terms */
  for(i=0; i<pQuery->nTerms;  i += aTerm[i].nPhrase + 1){
    if( !aTerm[i].isNot ) continue;
    rc = docListOfTerm(v, aTerm[i].iColumn, &aTerm[i], &right);
    if( rc ){
      queryClear(pQuery);
      dataBufferDestroy(&left);
      return rc;
    }
    dataBufferInit(&new, 0);
    docListExceptMerge(left.pData, left.nData, right.pData, right.nData, &new);
    dataBufferDestroy(&right);
    dataBufferDestroy(&left);
    left = new;
  }

  *pResult = left;
  return rc;
}

/*
** This is the xFilter interface for the virtual table.  See
** the virtual table xFilter method documentation for additional
** information.
**
** If idxNum==QUERY_GENERIC then do a full table scan against
** the %_content table.
**
** If idxNum==QUERY_ROWID then do a rowid lookup for a single entry
** in the %_content table.
**
** If idxNum>=QUERY_FULLTEXT then use the full text index.  The
** column on the left-hand side of the MATCH operator is column
** number idxNum-QUERY_FULLTEXT, 0 indexed.  argv[0] is the right-hand
** side of the MATCH operator.
*/
/* TODO(shess) Upgrade the cursor initialization and destruction to
** account for fulltextFilter() being called multiple times on the
** same cursor.  The current solution is very fragile.  Apply fix to
** fts2 as appropriate.
*/
static int fulltextFilter(
  sqlite3_vtab_cursor *pCursor,     /* The cursor used for this query */
  int idxNum, const char *idxStr,   /* Which indexing scheme to use */
  int argc, sqlite3_value **argv    /* Arguments for the indexing scheme */
){
  fulltext_cursor *c = (fulltext_cursor *) pCursor;
  fulltext_vtab *v = cursor_vtab(c);
  int rc;

  TRACE(("FTS2 Filter %p\n",pCursor));

  /* If the cursor has a statement that was not prepared according to
  ** idxNum, clear it.  I believe all calls to fulltextFilter with a
  ** given cursor will have the same idxNum , but in this case it's
  ** easy to be safe.
  */
  if( c->pStmt && c->iCursorType!=idxNum ){
    sqlite3_finalize(c->pStmt);
    c->pStmt = NULL;
  }

  /* Get a fresh statement appropriate to idxNum. */
  /* TODO(shess): Add a prepared-statement cache in the vt structure.
  ** The cache must handle multiple open cursors.  Easier to cache the
  ** statement variants at the vt to reduce malloc/realloc/free here.
  ** Or we could have a StringBuffer variant which allowed stack
  ** construction for small values.
  */
  if( !c->pStmt ){
    char *zSql = sqlite3_mprintf("select rowid, * from %%_content %s",
                                 idxNum==QUERY_GENERIC ? "" : "where rowid=?");
    rc = sql_prepare(v->db, v->zDb, v->zName, &c->pStmt, zSql);
    sqlite3_free(zSql);
    if( rc!=SQLITE_OK ) return rc;
    c->iCursorType = idxNum;
  }else{
    sqlite3_reset(c->pStmt);
    assert( c->iCursorType==idxNum );
  }

  switch( idxNum ){
    case QUERY_GENERIC:
      break;

    case QUERY_ROWID:
      rc = sqlite3_bind_int64(c->pStmt, 1, sqlite3_value_int64(argv[0]));
      if( rc!=SQLITE_OK ) return rc;
      break;

    default:   /* full-text search */
    {
      const char *zQuery = (const char *)sqlite3_value_text(argv[0]);
      assert( idxNum<=QUERY_FULLTEXT+v->nColumn);
      assert( argc==1 );
      queryClear(&c->q);
      if( c->result.nData!=0 ){
        /* This case happens if the same cursor is used repeatedly. */
        dlrDestroy(&c->reader);
        dataBufferReset(&c->result);
      }else{
        dataBufferInit(&c->result, 0);
      }
      rc = fulltextQuery(v, idxNum-QUERY_FULLTEXT, zQuery, -1, &c->result, &c->q);
      if( rc!=SQLITE_OK ) return rc;
      if( c->result.nData!=0 ){
        dlrInit(&c->reader, DL_DOCIDS, c->result.pData, c->result.nData);
      }
      break;
    }
  }

  return fulltextNext(pCursor);
}

/* This is the xEof method of the virtual table.  The SQLite core
** calls this routine to find out if it has reached the end of
** a query's results set.
*/
static int fulltextEof(sqlite3_vtab_cursor *pCursor){
  fulltext_cursor *c = (fulltext_cursor *) pCursor;
  return c->eof;
}

/* This is the xColumn method of the virtual table.  The SQLite
** core calls this method during a query when it needs the value
** of a column from the virtual table.  This method needs to use
** one of the sqlite3_result_*() routines to store the requested
** value back in the pContext.
*/
static int fulltextColumn(sqlite3_vtab_cursor *pCursor,
                          sqlite3_context *pContext, int idxCol){
  fulltext_cursor *c = (fulltext_cursor *) pCursor;
  fulltext_vtab *v = cursor_vtab(c);

  if( idxCol<v->nColumn ){
    sqlite3_value *pVal = sqlite3_column_value(c->pStmt, idxCol+1);
    sqlite3_result_value(pContext, pVal);
  }else if( idxCol==v->nColumn ){
    /* The extra column whose name is the same as the table.
    ** Return a blob which is a pointer to the cursor
    */
    sqlite3_result_blob(pContext, &c, sizeof(c), SQLITE_TRANSIENT);
  }
  return SQLITE_OK;
}

/* This is the xRowid method.  The SQLite core calls this routine to
** retrive the rowid for the current row of the result set.  The
** rowid should be written to *pRowid.
*/
static int fulltextRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  fulltext_cursor *c = (fulltext_cursor *) pCursor;

  *pRowid = sqlite3_column_int64(c->pStmt, 0);
  return SQLITE_OK;
}

/* Add all terms in [zText] to pendingTerms table.  If [iColumn] > 0,
** we also store positions and offsets in the hash table using that
** column number.
*/
static int buildTerms(fulltext_vtab *v, sqlite_int64 iDocid,
                      const char *zText, int iColumn){
  sqlite3_tokenizer *pTokenizer = v->pTokenizer;
  sqlite3_tokenizer_cursor *pCursor;
  const char *pToken;
  int nTokenBytes;
  int iStartOffset, iEndOffset, iPosition;
  int rc;

  rc = pTokenizer->pModule->xOpen(pTokenizer, zText, -1, &pCursor);
  if( rc!=SQLITE_OK ) return rc;

  pCursor->pTokenizer = pTokenizer;
  while( SQLITE_OK==(rc=pTokenizer->pModule->xNext(pCursor,
                                                   &pToken, &nTokenBytes,
                                                   &iStartOffset, &iEndOffset,
                                                   &iPosition)) ){
    DLCollector *p;
    int nData;                   /* Size of doclist before our update. */

    /* Positions can't be negative; we use -1 as a terminator
     * internally.  Token can't be NULL or empty. */
    if( iPosition<0 || pToken == NULL || nTokenBytes == 0 ){
      rc = SQLITE_ERROR;
      break;
    }

    p = fts2HashFind(&v->pendingTerms, pToken, nTokenBytes);
    if( p==NULL ){
      nData = 0;
      p = dlcNew(iDocid, DL_DEFAULT);
      fts2HashInsert(&v->pendingTerms, pToken, nTokenBytes, p);

      /* Overhead for our hash table entry, the key, and the value. */
      v->nPendingData += sizeof(struct fts2HashElem)+sizeof(*p)+nTokenBytes;
    }else{
      nData = p->b.nData;
      if( p->dlw.iPrevDocid!=iDocid ) dlcNext(p, iDocid);
    }
    if( iColumn>=0 ){
      dlcAddPos(p, iColumn, iPosition, iStartOffset, iEndOffset);
    }

    /* Accumulate data added by dlcNew or dlcNext, and dlcAddPos. */
    v->nPendingData += p->b.nData-nData;
  }

  /* TODO(shess) Check return?  Should this be able to cause errors at
  ** this point?  Actually, same question about sqlite3_finalize(),
  ** though one could argue that failure there means that the data is
  ** not durable.  *ponder*
  */
  pTokenizer->pModule->xClose(pCursor);
  if( SQLITE_DONE == rc ) return SQLITE_OK;
  return rc;
}

/* Add doclists for all terms in [pValues] to pendingTerms table. */
static int insertTerms(fulltext_vtab *v, sqlite_int64 iRowid,
                       sqlite3_value **pValues){
  int i;
  for(i = 0; i < v->nColumn ; ++i){
    char *zText = (char*)sqlite3_value_text(pValues[i]);
    int rc = buildTerms(v, iRowid, zText, i);
    if( rc!=SQLITE_OK ) return rc;
  }
  return SQLITE_OK;
}

/* Add empty doclists for all terms in the given row's content to
** pendingTerms.
*/
static int deleteTerms(fulltext_vtab *v, sqlite_int64 iRowid){
  const char **pValues;
  int i, rc;

  /* TODO(shess) Should we allow such tables at all? */
  if( DL_DEFAULT==DL_DOCIDS ) return SQLITE_ERROR;

  rc = content_select(v, iRowid, &pValues);
  if( rc!=SQLITE_OK ) return rc;

  for(i = 0 ; i < v->nColumn; ++i) {
    rc = buildTerms(v, iRowid, pValues[i], -1);
    if( rc!=SQLITE_OK ) break;
  }

  freeStringArray(v->nColumn, pValues);
  return SQLITE_OK;
}

/* TODO(shess) Refactor the code to remove this forward decl. */
static int initPendingTerms(fulltext_vtab *v, sqlite_int64 iDocid);

/* Insert a row into the %_content table; set *piRowid to be the ID of the
** new row.  Add doclists for terms to pendingTerms.
*/
static int index_insert(fulltext_vtab *v, sqlite3_value *pRequestRowid,
                        sqlite3_value **pValues, sqlite_int64 *piRowid){
  int rc;

  rc = content_insert(v, pRequestRowid, pValues);  /* execute an SQL INSERT */
  if( rc!=SQLITE_OK ) return rc;

  *piRowid = sqlite3_last_insert_rowid(v->db);
  rc = initPendingTerms(v, *piRowid);
  if( rc!=SQLITE_OK ) return rc;

  return insertTerms(v, *piRowid, pValues);
}

/* Delete a row from the %_content table; add empty doclists for terms
** to pendingTerms.
*/
static int index_delete(fulltext_vtab *v, sqlite_int64 iRow){
  int rc = initPendingTerms(v, iRow);
  if( rc!=SQLITE_OK ) return rc;

  rc = deleteTerms(v, iRow);
  if( rc!=SQLITE_OK ) return rc;

  return content_delete(v, iRow);  /* execute an SQL DELETE */
}

/* Update a row in the %_content table; add delete doclists to
** pendingTerms for old terms not in the new data, add insert doclists
** to pendingTerms for terms in the new data.
*/
static int index_update(fulltext_vtab *v, sqlite_int64 iRow,
                        sqlite3_value **pValues){
  int rc = initPendingTerms(v, iRow);
  if( rc!=SQLITE_OK ) return rc;

  /* Generate an empty doclist for each term that previously appeared in this
   * row. */
  rc = deleteTerms(v, iRow);
  if( rc!=SQLITE_OK ) return rc;

  rc = content_update(v, pValues, iRow);  /* execute an SQL UPDATE */
  if( rc!=SQLITE_OK ) return rc;

  /* Now add positions for terms which appear in the updated row. */
  return insertTerms(v, iRow, pValues);
}

/*******************************************************************/
/* InteriorWriter is used to collect terms and block references into
** interior nodes in %_segments.  See commentary at top of file for
** format.
*/

/* How large interior nodes can grow. */
#define INTERIOR_MAX 2048

/* Minimum number of terms per interior node (except the root). This
** prevents large terms from making the tree too skinny - must be >0
** so that the tree always makes progress.  Note that the min tree
** fanout will be INTERIOR_MIN_TERMS+1.
*/
#define INTERIOR_MIN_TERMS 7
#if INTERIOR_MIN_TERMS<1
# error INTERIOR_MIN_TERMS must be greater than 0.
#endif

/* ROOT_MAX controls how much data is stored inline in the segment
** directory.
*/
/* TODO(shess) Push ROOT_MAX down to whoever is writing things.  It's
** only here so that interiorWriterRootInfo() and leafWriterRootInfo()
** can both see it, but if the caller passed it in, we wouldn't even
** need a define.
*/
#define ROOT_MAX 1024
#if ROOT_MAX<VARINT_MAX*2
# error ROOT_MAX must have enough space for a header.
#endif

/* InteriorBlock stores a linked-list of interior blocks while a lower
** layer is being constructed.
*/
typedef struct InteriorBlock {
  DataBuffer term;           /* Leftmost term in block's subtree. */
  DataBuffer data;           /* Accumulated data for the block. */
  struct InteriorBlock *next;
} InteriorBlock;

static InteriorBlock *interiorBlockNew(int iHeight, sqlite_int64 iChildBlock,
                                       const char *pTerm, int nTerm){
  InteriorBlock *block = sqlite3_malloc(sizeof(InteriorBlock));
  char c[VARINT_MAX+VARINT_MAX];
  int n;

  if( block ){
    memset(block, 0, sizeof(*block));
    dataBufferInit(&block->term, 0);
    dataBufferReplace(&block->term, pTerm, nTerm);

    n = putVarint(c, iHeight);
    n += putVarint(c+n, iChildBlock);
    dataBufferInit(&block->data, INTERIOR_MAX);
    dataBufferReplace(&block->data, c, n);
  }
  return block;
}

#ifndef NDEBUG
/* Verify that the data is readable as an interior node. */
static void interiorBlockValidate(InteriorBlock *pBlock){
  const char *pData = pBlock->data.pData;
  int nData = pBlock->data.nData;
  int n, iDummy;
  sqlite_int64 iBlockid;

  assert( nData>0 );
  assert( pData!=0 );
  assert( pData+nData>pData );

  /* Must lead with height of node as a varint(n), n>0 */
  n = getVarint32(pData, &iDummy);
  assert( n>0 );
  assert( iDummy>0 );
  assert( n<nData );
  pData += n;
  nData -= n;

  /* Must contain iBlockid. */
  n = getVarint(pData, &iBlockid);
  assert( n>0 );
  assert( n<=nData );
  pData += n;
  nData -= n;

  /* Zero or more terms of positive length */
  if( nData!=0 ){
    /* First term is not delta-encoded. */
    n = getVarint32(pData, &iDummy);
    assert( n>0 );
    assert( iDummy>0 );
    assert( n+iDummy>0);
    assert( n+iDummy<=nData );
    pData += n+iDummy;
    nData -= n+iDummy;

    /* Following terms delta-encoded. */
    while( nData!=0 ){
      /* Length of shared prefix. */
      n = getVarint32(pData, &iDummy);
      assert( n>0 );
      assert( iDummy>=0 );
      assert( n<nData );
      pData += n;
      nData -= n;

      /* Length and data of distinct suffix. */
      n = getVarint32(pData, &iDummy);
      assert( n>0 );
      assert( iDummy>0 );
      assert( n+iDummy>0);
      assert( n+iDummy<=nData );
      pData += n+iDummy;
      nData -= n+iDummy;
    }
  }
}
#define ASSERT_VALID_INTERIOR_BLOCK(x) interiorBlockValidate(x)
#else
#define ASSERT_VALID_INTERIOR_BLOCK(x) assert( 1 )
#endif

typedef struct InteriorWriter {
  int iHeight;                   /* from 0 at leaves. */
  InteriorBlock *first, *last;
  struct InteriorWriter *parentWriter;

  DataBuffer term;               /* Last term written to block "last". */
  sqlite_int64 iOpeningChildBlock; /* First child block in block "last". */
#ifndef NDEBUG
  sqlite_int64 iLastChildBlock;  /* for consistency checks. */
#endif
} InteriorWriter;

/* Initialize an interior node where pTerm[nTerm] marks the leftmost
** term in the tree.  iChildBlock is the leftmost child block at the
** next level down the tree.
*/
static void interiorWriterInit(int iHeight, const char *pTerm, int nTerm,
                               sqlite_int64 iChildBlock,
                               InteriorWriter *pWriter){
  InteriorBlock *block;
  assert( iHeight>0 );
  CLEAR(pWriter);

  pWriter->iHeight = iHeight;
  pWriter->iOpeningChildBlock = iChildBlock;
#ifndef NDEBUG
  pWriter->iLastChildBlock = iChildBlock;
#endif
  block = interiorBlockNew(iHeight, iChildBlock, pTerm, nTerm);
  pWriter->last = pWriter->first = block;
  ASSERT_VALID_INTERIOR_BLOCK(pWriter->last);
  dataBufferInit(&pWriter->term, 0);
}

/* Append the child node rooted at iChildBlock to the interior node,
** with pTerm[nTerm] as the leftmost term in iChildBlock's subtree.
*/
static void interiorWriterAppend(InteriorWriter *pWriter,
                                 const char *pTerm, int nTerm,
                                 sqlite_int64 iChildBlock){
  char c[VARINT_MAX+VARINT_MAX];
  int n, nPrefix = 0;

  ASSERT_VALID_INTERIOR_BLOCK(pWriter->last);

  /* The first term written into an interior node is actually
  ** associated with the second child added (the first child was added
  ** in interiorWriterInit, or in the if clause at the bottom of this
  ** function).  That term gets encoded straight up, with nPrefix left
  ** at 0.
  */
  if( pWriter->term.nData==0 ){
    n = putVarint(c, nTerm);
  }else{
    while( nPrefix<pWriter->term.nData &&
           pTerm[nPrefix]==pWriter->term.pData[nPrefix] ){
      nPrefix++;
    }

    n = putVarint(c, nPrefix);
    n += putVarint(c+n, nTerm-nPrefix);
  }

#ifndef NDEBUG
  pWriter->iLastChildBlock++;
#endif
  assert( pWriter->iLastChildBlock==iChildBlock );

  /* Overflow to a new block if the new term makes the current block
  ** too big, and the current block already has enough terms.
  */
  if( pWriter->last->data.nData+n+nTerm-nPrefix>INTERIOR_MAX &&
      iChildBlock-pWriter->iOpeningChildBlock>INTERIOR_MIN_TERMS ){
    pWriter->last->next = interiorBlockNew(pWriter->iHeight, iChildBlock,
                                           pTerm, nTerm);
    pWriter->last = pWriter->last->next;
    pWriter->iOpeningChildBlock = iChildBlock;
    dataBufferReset(&pWriter->term);
  }else{
    dataBufferAppend2(&pWriter->last->data, c, n,
                      pTerm+nPrefix, nTerm-nPrefix);
    dataBufferReplace(&pWriter->term, pTerm, nTerm);
  }
  ASSERT_VALID_INTERIOR_BLOCK(pWriter->last);
}

/* Free the space used by pWriter, including the linked-list of
** InteriorBlocks, and parentWriter, if present.
*/
static int interiorWriterDestroy(InteriorWriter *pWriter){
  InteriorBlock *block = pWriter->first;

  while( block!=NULL ){
    InteriorBlock *b = block;
    block = block->next;
    dataBufferDestroy(&b->term);
    dataBufferDestroy(&b->data);
    sqlite3_free(b);
  }
  if( pWriter->parentWriter!=NULL ){
    interiorWriterDestroy(pWriter->parentWriter);
    sqlite3_free(pWriter->parentWriter);
  }
  dataBufferDestroy(&pWriter->term);
  SCRAMBLE(pWriter);
  return SQLITE_OK;
}

/* If pWriter can fit entirely in ROOT_MAX, return it as the root info
** directly, leaving *piEndBlockid unchanged.  Otherwise, flush
** pWriter to %_segments, building a new layer of interior nodes, and
** recursively ask for their root into.
*/
static int interiorWriterRootInfo(fulltext_vtab *v, InteriorWriter *pWriter,
                                  char **ppRootInfo, int *pnRootInfo,
                                  sqlite_int64 *piEndBlockid){
  InteriorBlock *block = pWriter->first;
  sqlite_int64 iBlockid = 0;
  int rc;

  /* If we can fit the segment inline */
  if( block==pWriter->last && block->data.nData<ROOT_MAX ){
    *ppRootInfo = block->data.pData;
    *pnRootInfo = block->data.nData;
    return SQLITE_OK;
  }

  /* Flush the first block to %_segments, and create a new level of
  ** interior node.
  */
  ASSERT_VALID_INTERIOR_BLOCK(block);
  rc = block_insert(v, block->data.pData, block->data.nData, &iBlockid);
  if( rc!=SQLITE_OK ) return rc;
  *piEndBlockid = iBlockid;

  pWriter->parentWriter = sqlite3_malloc(sizeof(*pWriter->parentWriter));
  interiorWriterInit(pWriter->iHeight+1,
                     block->term.pData, block->term.nData,
                     iBlockid, pWriter->parentWriter);

  /* Flush additional blocks and append to the higher interior
  ** node.
  */
  for(block=block->next; block!=NULL; block=block->next){
    ASSERT_VALID_INTERIOR_BLOCK(block);
    rc = block_insert(v, block->data.pData, block->data.nData, &iBlockid);
    if( rc!=SQLITE_OK ) return rc;
    *piEndBlockid = iBlockid;

    interiorWriterAppend(pWriter->parentWriter,
                         block->term.pData, block->term.nData, iBlockid);
  }

  /* Parent node gets the chance to be the root. */
  return interiorWriterRootInfo(v, pWriter->parentWriter,
                                ppRootInfo, pnRootInfo, piEndBlockid);
}

/****************************************************************/
/* InteriorReader is used to read off the data from an interior node
** (see comment at top of file for the format).
*/
typedef struct InteriorReader {
  const char *pData;
  int nData;

  DataBuffer term;          /* previous term, for decoding term delta. */

  sqlite_int64 iBlockid;
} InteriorReader;

static void interiorReaderDestroy(InteriorReader *pReader){
  dataBufferDestroy(&pReader->term);
  SCRAMBLE(pReader);
}

/* TODO(shess) The assertions are great, but what if we're in NDEBUG
** and the blob is empty or otherwise contains suspect data?
*/
static void interiorReaderInit(const char *pData, int nData,
                               InteriorReader *pReader){
  int n, nTerm;

  /* Require at least the leading flag byte */
  assert( nData>0 );
  assert( pData[0]!='\0' );

  CLEAR(pReader);

  /* Decode the base blockid, and set the cursor to the first term. */
  n = getVarint(pData+1, &pReader->iBlockid);
  assert( 1+n<=nData );
  pReader->pData = pData+1+n;
  pReader->nData = nData-(1+n);

  /* A single-child interior node (such as when a leaf node was too
  ** large for the segment directory) won't have any terms.
  ** Otherwise, decode the first term.
  */
  if( pReader->nData==0 ){
    dataBufferInit(&pReader->term, 0);
  }else{
    n = getVarint32(pReader->pData, &nTerm);
    dataBufferInit(&pReader->term, nTerm);
    dataBufferReplace(&pReader->term, pReader->pData+n, nTerm);
    assert( n+nTerm<=pReader->nData );
    pReader->pData += n+nTerm;
    pReader->nData -= n+nTerm;
  }
}

static int interiorReaderAtEnd(InteriorReader *pReader){
  return pReader->term.nData==0;
}

static sqlite_int64 interiorReaderCurrentBlockid(InteriorReader *pReader){
  return pReader->iBlockid;
}

static int interiorReaderTermBytes(InteriorReader *pReader){
  assert( !interiorReaderAtEnd(pReader) );
  return pReader->term.nData;
}
static const char *interiorReaderTerm(InteriorReader *pReader){
  assert( !interiorReaderAtEnd(pReader) );
  return pReader->term.pData;
}

/* Step forward to the next term in the node. */
static void interiorReaderStep(InteriorReader *pReader){
  assert( !interiorReaderAtEnd(pReader) );

  /* If the last term has been read, signal eof, else construct the
  ** next term.
  */
  if( pReader->nData==0 ){
    dataBufferReset(&pReader->term);
  }else{
    int n, nPrefix, nSuffix;

    n = getVarint32(pReader->pData, &nPrefix);
    n += getVarint32(pReader->pData+n, &nSuffix);

    /* Truncate the current term and append suffix data. */
    pReader->term.nData = nPrefix;
    dataBufferAppend(&pReader->term, pReader->pData+n, nSuffix);

    assert( n+nSuffix<=pReader->nData );
    pReader->pData += n+nSuffix;
    pReader->nData -= n+nSuffix;
  }
  pReader->iBlockid++;
}

/* Compare the current term to pTerm[nTerm], returning strcmp-style
** results.  If isPrefix, equality means equal through nTerm bytes.
*/
static int interiorReaderTermCmp(InteriorReader *pReader,
                                 const char *pTerm, int nTerm, int isPrefix){
  const char *pReaderTerm = interiorReaderTerm(pReader);
  int nReaderTerm = interiorReaderTermBytes(pReader);
  int c, n = nReaderTerm<nTerm ? nReaderTerm : nTerm;

  if( n==0 ){
    if( nReaderTerm>0 ) return -1;
    if( nTerm>0 ) return 1;
    return 0;
  }

  c = memcmp(pReaderTerm, pTerm, n);
  if( c!=0 ) return c;
  if( isPrefix && n==nTerm ) return 0;
  return nReaderTerm - nTerm;
}

/****************************************************************/
/* LeafWriter is used to collect terms and associated doclist data
** into leaf blocks in %_segments (see top of file for format info).
** Expected usage is:
**
** LeafWriter writer;
** leafWriterInit(0, 0, &writer);
** while( sorted_terms_left_to_process ){
**   // data is doclist data for that term.
**   rc = leafWriterStep(v, &writer, pTerm, nTerm, pData, nData);
**   if( rc!=SQLITE_OK ) goto err;
** }
** rc = leafWriterFinalize(v, &writer);
**err:
** leafWriterDestroy(&writer);
** return rc;
**
** leafWriterStep() may write a collected leaf out to %_segments.
** leafWriterFinalize() finishes writing any buffered data and stores
** a root node in %_segdir.  leafWriterDestroy() frees all buffers and
** InteriorWriters allocated as part of writing this segment.
**
** TODO(shess) Document leafWriterStepMerge().
*/

/* Put terms with data this big in their own block. */
#define STANDALONE_MIN 1024

/* Keep leaf blocks below this size. */
#define LEAF_MAX 2048

typedef struct LeafWriter {
  int iLevel;
  int idx;
  sqlite_int64 iStartBlockid;     /* needed to create the root info */
  sqlite_int64 iEndBlockid;       /* when we're done writing. */

  DataBuffer term;                /* previous encoded term */
  DataBuffer data;                /* encoding buffer */

  /* bytes of first term in the current node which distinguishes that
  ** term from the last term of the previous node.
  */
  int nTermDistinct;

  InteriorWriter parentWriter;    /* if we overflow */
  int has_parent;
} LeafWriter;

static void leafWriterInit(int iLevel, int idx, LeafWriter *pWriter){
  CLEAR(pWriter);
  pWriter->iLevel = iLevel;
  pWriter->idx = idx;

  dataBufferInit(&pWriter->term, 32);

  /* Start out with a reasonably sized block, though it can grow. */
  dataBufferInit(&pWriter->data, LEAF_MAX);
}

#ifndef NDEBUG
/* Verify that the data is readable as a leaf node. */
static void leafNodeValidate(const char *pData, int nData){
  int n, iDummy;

  if( nData==0 ) return;
  assert( nData>0 );
  assert( pData!=0 );
  assert( pData+nData>pData );

  /* Must lead with a varint(0) */
  n = getVarint32(pData, &iDummy);
  assert( iDummy==0 );
  assert( n>0 );
  assert( n<nData );
  pData += n;
  nData -= n;

  /* Leading term length and data must fit in buffer. */
  n = getVarint32(pData, &iDummy);
  assert( n>0 );
  assert( iDummy>0 );
  assert( n+iDummy>0 );
  assert( n+iDummy<nData );
  pData += n+iDummy;
  nData -= n+iDummy;

  /* Leading term's doclist length and data must fit. */
  n = getVarint32(pData, &iDummy);
  assert( n>0 );
  assert( iDummy>0 );
  assert( n+iDummy>0 );
  assert( n+iDummy<=nData );
  ASSERT_VALID_DOCLIST(DL_DEFAULT, pData+n, iDummy, NULL);
  pData += n+iDummy;
  nData -= n+iDummy;

  /* Verify that trailing terms and doclists also are readable. */
  while( nData!=0 ){
    n = getVarint32(pData, &iDummy);
    assert( n>0 );
    assert( iDummy>=0 );
    assert( n<nData );
    pData += n;
    nData -= n;
    n = getVarint32(pData, &iDummy);
    assert( n>0 );
    assert( iDummy>0 );
    assert( n+iDummy>0 );
    assert( n+iDummy<nData );
    pData += n+iDummy;
    nData -= n+iDummy;

    n = getVarint32(pData, &iDummy);
    assert( n>0 );
    assert( iDummy>0 );
    assert( n+iDummy>0 );
    assert( n+iDummy<=nData );
    ASSERT_VALID_DOCLIST(DL_DEFAULT, pData+n, iDummy, NULL);
    pData += n+iDummy;
    nData -= n+iDummy;
  }
}
#define ASSERT_VALID_LEAF_NODE(p, n) leafNodeValidate(p, n)
#else
#define ASSERT_VALID_LEAF_NODE(p, n) assert( 1 )
#endif

/* Flush the current leaf node to %_segments, and adding the resulting
** blockid and the starting term to the interior node which will
** contain it.
*/
static int leafWriterInternalFlush(fulltext_vtab *v, LeafWriter *pWriter,
                                   int iData, int nData){
  sqlite_int64 iBlockid = 0;
  const char *pStartingTerm;
  int nStartingTerm, rc, n;

  /* Must have the leading varint(0) flag, plus at least some
  ** valid-looking data.
  */
  assert( nData>2 );
  assert( iData>=0 );
  assert( iData+nData<=pWriter->data.nData );
  ASSERT_VALID_LEAF_NODE(pWriter->data.pData+iData, nData);

  rc = block_insert(v, pWriter->data.pData+iData, nData, &iBlockid);
  if( rc!=SQLITE_OK ) return rc;
  assert( iBlockid!=0 );

  /* Reconstruct the first term in the leaf for purposes of building
  ** the interior node.
  */
  n = getVarint32(pWriter->data.pData+iData+1, &nStartingTerm);
  pStartingTerm = pWriter->data.pData+iData+1+n;
  assert( pWriter->data.nData>iData+1+n+nStartingTerm );
  assert( pWriter->nTermDistinct>0 );
  assert( pWriter->nTermDistinct<=nStartingTerm );
  nStartingTerm = pWriter->nTermDistinct;

  if( pWriter->has_parent ){
    interiorWriterAppend(&pWriter->parentWriter,
                         pStartingTerm, nStartingTerm, iBlockid);
  }else{
    interiorWriterInit(1, pStartingTerm, nStartingTerm, iBlockid,
                       &pWriter->parentWriter);
    pWriter->has_parent = 1;
  }

  /* Track the span of this segment's leaf nodes. */
  if( pWriter->iEndBlockid==0 ){
    pWriter->iEndBlockid = pWriter->iStartBlockid = iBlockid;
  }else{
    pWriter->iEndBlockid++;
    assert( iBlockid==pWriter->iEndBlockid );
  }

  return SQLITE_OK;
}
static int leafWriterFlush(fulltext_vtab *v, LeafWriter *pWriter){
  int rc = leafWriterInternalFlush(v, pWriter, 0, pWriter->data.nData);
  if( rc!=SQLITE_OK ) return rc;

  /* Re-initialize the output buffer. */
  dataBufferReset(&pWriter->data);

  return SQLITE_OK;
}

/* Fetch the root info for the segment.  If the entire leaf fits
** within ROOT_MAX, then it will be returned directly, otherwise it
** will be flushed and the root info will be returned from the
** interior node.  *piEndBlockid is set to the blockid of the last
** interior or leaf node written to disk (0 if none are written at
** all).
*/
static int leafWriterRootInfo(fulltext_vtab *v, LeafWriter *pWriter,
                              char **ppRootInfo, int *pnRootInfo,
                              sqlite_int64 *piEndBlockid){
  /* we can fit the segment entirely inline */
  if( !pWriter->has_parent && pWriter->data.nData<ROOT_MAX ){
    *ppRootInfo = pWriter->data.pData;
    *pnRootInfo = pWriter->data.nData;
    *piEndBlockid = 0;
    return SQLITE_OK;
  }

  /* Flush remaining leaf data. */
  if( pWriter->data.nData>0 ){
    int rc = leafWriterFlush(v, pWriter);
    if( rc!=SQLITE_OK ) return rc;
  }

  /* We must have flushed a leaf at some point. */
  assert( pWriter->has_parent );

  /* Tenatively set the end leaf blockid as the end blockid.  If the
  ** interior node can be returned inline, this will be the final
  ** blockid, otherwise it will be overwritten by
  ** interiorWriterRootInfo().
  */
  *piEndBlockid = pWriter->iEndBlockid;

  return interiorWriterRootInfo(v, &pWriter->parentWriter,
                                ppRootInfo, pnRootInfo, piEndBlockid);
}

/* Collect the rootInfo data and store it into the segment directory.
** This has the effect of flushing the segment's leaf data to
** %_segments, and also flushing any interior nodes to %_segments.
*/
static int leafWriterFinalize(fulltext_vtab *v, LeafWriter *pWriter){
  sqlite_int64 iEndBlockid;
  char *pRootInfo;
  int rc, nRootInfo;

  rc = leafWriterRootInfo(v, pWriter, &pRootInfo, &nRootInfo, &iEndBlockid);
  if( rc!=SQLITE_OK ) return rc;

  /* Don't bother storing an entirely empty segment. */
  if( iEndBlockid==0 && nRootInfo==0 ) return SQLITE_OK;

  return segdir_set(v, pWriter->iLevel, pWriter->idx,
                    pWriter->iStartBlockid, pWriter->iEndBlockid,
                    iEndBlockid, pRootInfo, nRootInfo);
}

static void leafWriterDestroy(LeafWriter *pWriter){
  if( pWriter->has_parent ) interiorWriterDestroy(&pWriter->parentWriter);
  dataBufferDestroy(&pWriter->term);
  dataBufferDestroy(&pWriter->data);
}

/* Encode a term into the leafWriter, delta-encoding as appropriate.
** Returns the length of the new term which distinguishes it from the
** previous term, which can be used to set nTermDistinct when a node
** boundary is crossed.
*/
static int leafWriterEncodeTerm(LeafWriter *pWriter,
                                const char *pTerm, int nTerm){
  char c[VARINT_MAX+VARINT_MAX];
  int n, nPrefix = 0;

  assert( nTerm>0 );
  while( nPrefix<pWriter->term.nData &&
         pTerm[nPrefix]==pWriter->term.pData[nPrefix] ){
    nPrefix++;
    /* Failing this implies that the terms weren't in order. */
    assert( nPrefix<nTerm );
  }

  if( pWriter->data.nData==0 ){
    /* Encode the node header and leading term as:
    **  varint(0)
    **  varint(nTerm)
    **  char pTerm[nTerm]
    */
    n = putVarint(c, '\0');
    n += putVarint(c+n, nTerm);
    dataBufferAppend2(&pWriter->data, c, n, pTerm, nTerm);
  }else{
    /* Delta-encode the term as:
    **  varint(nPrefix)
    **  varint(nSuffix)
    **  char pTermSuffix[nSuffix]
    */
    n = putVarint(c, nPrefix);
    n += putVarint(c+n, nTerm-nPrefix);
    dataBufferAppend2(&pWriter->data, c, n, pTerm+nPrefix, nTerm-nPrefix);
  }
  dataBufferReplace(&pWriter->term, pTerm, nTerm);

  return nPrefix+1;
}

/* Used to avoid a memmove when a large amount of doclist data is in
** the buffer.  This constructs a node and term header before
** iDoclistData and flushes the resulting complete node using
** leafWriterInternalFlush().
*/
static int leafWriterInlineFlush(fulltext_vtab *v, LeafWriter *pWriter,
                                 const char *pTerm, int nTerm,
                                 int iDoclistData){
  char c[VARINT_MAX+VARINT_MAX];
  int iData, n = putVarint(c, 0);
  n += putVarint(c+n, nTerm);

  /* There should always be room for the header.  Even if pTerm shared
  ** a substantial prefix with the previous term, the entire prefix
  ** could be constructed from earlier data in the doclist, so there
  ** should be room.
  */
  assert( iDoclistData>=n+nTerm );

  iData = iDoclistData-(n+nTerm);
  memcpy(pWriter->data.pData+iData, c, n);
  memcpy(pWriter->data.pData+iData+n, pTerm, nTerm);

  return leafWriterInternalFlush(v, pWriter, iData, pWriter->data.nData-iData);
}

/* Push pTerm[nTerm] along with the doclist data to the leaf layer of
** %_segments.
*/
static int leafWriterStepMerge(fulltext_vtab *v, LeafWriter *pWriter,
                               const char *pTerm, int nTerm,
                               DLReader *pReaders, int nReaders){
  char c[VARINT_MAX+VARINT_MAX];
  int iTermData = pWriter->data.nData, iDoclistData;
  int i, nData, n, nActualData, nActual, rc, nTermDistinct;

  ASSERT_VALID_LEAF_NODE(pWriter->data.pData, pWriter->data.nData);
  nTermDistinct = leafWriterEncodeTerm(pWriter, pTerm, nTerm);

  /* Remember nTermDistinct if opening a new node. */
  if( iTermData==0 ) pWriter->nTermDistinct = nTermDistinct;

  iDoclistData = pWriter->data.nData;

  /* Estimate the length of the merged doclist so we can leave space
  ** to encode it.
  */
  for(i=0, nData=0; i<nReaders; i++){
    nData += dlrAllDataBytes(&pReaders[i]);
  }
  n = putVarint(c, nData);
  dataBufferAppend(&pWriter->data, c, n);

  docListMerge(&pWriter->data, pReaders, nReaders);
  ASSERT_VALID_DOCLIST(DL_DEFAULT,
                       pWriter->data.pData+iDoclistData+n,
                       pWriter->data.nData-iDoclistData-n, NULL);

  /* The actual amount of doclist data at this point could be smaller
  ** than the length we encoded.  Additionally, the space required to
  ** encode this length could be smaller.  For small doclists, this is
  ** not a big deal, we can just use memmove() to adjust things.
  */
  nActualData = pWriter->data.nData-(iDoclistData+n);
  nActual = putVarint(c, nActualData);
  assert( nActualData<=nData );
  assert( nActual<=n );

  /* If the new doclist is big enough for force a standalone leaf
  ** node, we can immediately flush it inline without doing the
  ** memmove().
  */
  /* TODO(shess) This test matches leafWriterStep(), which does this
  ** test before it knows the cost to varint-encode the term and
  ** doclist lengths.  At some point, change to
  ** pWriter->data.nData-iTermData>STANDALONE_MIN.
  */
  if( nTerm+nActualData>STANDALONE_MIN ){
    /* Push leaf node from before this term. */
    if( iTermData>0 ){
      rc = leafWriterInternalFlush(v, pWriter, 0, iTermData);
      if( rc!=SQLITE_OK ) return rc;

      pWriter->nTermDistinct = nTermDistinct;
    }

    /* Fix the encoded doclist length. */
    iDoclistData += n - nActual;
    memcpy(pWriter->data.pData+iDoclistData, c, nActual);

    /* Push the standalone leaf node. */
    rc = leafWriterInlineFlush(v, pWriter, pTerm, nTerm, iDoclistData);
    if( rc!=SQLITE_OK ) return rc;

    /* Leave the node empty. */
    dataBufferReset(&pWriter->data);

    return rc;
  }

  /* At this point, we know that the doclist was small, so do the
  ** memmove if indicated.
  */
  if( nActual<n ){
    memmove(pWriter->data.pData+iDoclistData+nActual,
            pWriter->data.pData+iDoclistData+n,
            pWriter->data.nData-(iDoclistData+n));
    pWriter->data.nData -= n-nActual;
  }

  /* Replace written length with actual length. */
  memcpy(pWriter->data.pData+iDoclistData, c, nActual);

  /* If the node is too large, break things up. */
  /* TODO(shess) This test matches leafWriterStep(), which does this
  ** test before it knows the cost to varint-encode the term and
  ** doclist lengths.  At some point, change to
  ** pWriter->data.nData>LEAF_MAX.
  */
  if( iTermData+nTerm+nActualData>LEAF_MAX ){
    /* Flush out the leading data as a node */
    rc = leafWriterInternalFlush(v, pWriter, 0, iTermData);
    if( rc!=SQLITE_OK ) return rc;

    pWriter->nTermDistinct = nTermDistinct;

    /* Rebuild header using the current term */
    n = putVarint(pWriter->data.pData, 0);
    n += putVarint(pWriter->data.pData+n, nTerm);
    memcpy(pWriter->data.pData+n, pTerm, nTerm);
    n += nTerm;

    /* There should always be room, because the previous encoding
    ** included all data necessary to construct the term.
    */
    assert( n<iDoclistData );
    /* So long as STANDALONE_MIN is half or less of LEAF_MAX, the
    ** following memcpy() is safe (as opposed to needing a memmove).
    */
    assert( 2*STANDALONE_MIN<=LEAF_MAX );
    assert( n+pWriter->data.nData-iDoclistData<iDoclistData );
    memcpy(pWriter->data.pData+n,
           pWriter->data.pData+iDoclistData,
           pWriter->data.nData-iDoclistData);
    pWriter->data.nData -= iDoclistData-n;
  }
  ASSERT_VALID_LEAF_NODE(pWriter->data.pData, pWriter->data.nData);

  return SQLITE_OK;
}

/* Push pTerm[nTerm] along with the doclist data to the leaf layer of
** %_segments.
*/
/* TODO(shess) Revise writeZeroSegment() so that doclists are
** constructed directly in pWriter->data.
*/
static int leafWriterStep(fulltext_vtab *v, LeafWriter *pWriter,
                          const char *pTerm, int nTerm,
                          const char *pData, int nData){
  int rc;
  DLReader reader;

  dlrInit(&reader, DL_DEFAULT, pData, nData);
  rc = leafWriterStepMerge(v, pWriter, pTerm, nTerm, &reader, 1);
  dlrDestroy(&reader);

  return rc;
}


/****************************************************************/
/* LeafReader is used to iterate over an individual leaf node. */
typedef struct LeafReader {
  DataBuffer term;          /* copy of current term. */

  const char *pData;        /* data for current term. */
  int nData;
} LeafReader;

static void leafReaderDestroy(LeafReader *pReader){
  dataBufferDestroy(&pReader->term);
  SCRAMBLE(pReader);
}

static int leafReaderAtEnd(LeafReader *pReader){
  return pReader->nData<=0;
}

/* Access the current term. */
static int leafReaderTermBytes(LeafReader *pReader){
  return pReader->term.nData;
}
static const char *leafReaderTerm(LeafReader *pReader){
  assert( pReader->term.nData>0 );
  return pReader->term.pData;
}

/* Access the doclist data for the current term. */
static int leafReaderDataBytes(LeafReader *pReader){
  int nData;
  assert( pReader->term.nData>0 );
  getVarint32(pReader->pData, &nData);
  return nData;
}
static const char *leafReaderData(LeafReader *pReader){
  int n, nData;
  assert( pReader->term.nData>0 );
  n = getVarint32(pReader->pData, &nData);
  return pReader->pData+n;
}

static void leafReaderInit(const char *pData, int nData,
                           LeafReader *pReader){
  int nTerm, n;

  assert( nData>0 );
  assert( pData[0]=='\0' );

  CLEAR(pReader);

  /* Read the first term, skipping the header byte. */
  n = getVarint32(pData+1, &nTerm);
  dataBufferInit(&pReader->term, nTerm);
  dataBufferReplace(&pReader->term, pData+1+n, nTerm);

  /* Position after the first term. */
  assert( 1+n+nTerm<nData );
  pReader->pData = pData+1+n+nTerm;
  pReader->nData = nData-1-n-nTerm;
}

/* Step the reader forward to the next term. */
static void leafReaderStep(LeafReader *pReader){
  int n, nData, nPrefix, nSuffix;
  assert( !leafReaderAtEnd(pReader) );

  /* Skip previous entry's data block. */
  n = getVarint32(pReader->pData, &nData);
  assert( n+nData<=pReader->nData );
  pReader->pData += n+nData;
  pReader->nData -= n+nData;

  if( !leafReaderAtEnd(pReader) ){
    /* Construct the new term using a prefix from the old term plus a
    ** suffix from the leaf data.
    */
    n = getVarint32(pReader->pData, &nPrefix);
    n += getVarint32(pReader->pData+n, &nSuffix);
    assert( n+nSuffix<pReader->nData );
    pReader->term.nData = nPrefix;
    dataBufferAppend(&pReader->term, pReader->pData+n, nSuffix);

    pReader->pData += n+nSuffix;
    pReader->nData -= n+nSuffix;
  }
}

/* strcmp-style comparison of pReader's current term against pTerm.
** If isPrefix, equality means equal through nTerm bytes.
*/
static int leafReaderTermCmp(LeafReader *pReader,
                             const char *pTerm, int nTerm, int isPrefix){
  int c, n = pReader->term.nData<nTerm ? pReader->term.nData : nTerm;
  if( n==0 ){
    if( pReader->term.nData>0 ) return -1;
    if(nTerm>0 ) return 1;
    return 0;
  }

  c = memcmp(pReader->term.pData, pTerm, n);
  if( c!=0 ) return c;
  if( isPrefix && n==nTerm ) return 0;
  return pReader->term.nData - nTerm;
}


/****************************************************************/
/* LeavesReader wraps LeafReader to allow iterating over the entire
** leaf layer of the tree.
*/
typedef struct LeavesReader {
  int idx;                  /* Index within the segment. */

  sqlite3_stmt *pStmt;      /* Statement we're streaming leaves from. */
  int eof;                  /* we've seen SQLITE_DONE from pStmt. */

  LeafReader leafReader;    /* reader for the current leaf. */
  DataBuffer rootData;      /* root data for inline. */
} LeavesReader;

/* Access the current term. */
static int leavesReaderTermBytes(LeavesReader *pReader){
  assert( !pReader->eof );
  return leafReaderTermBytes(&pReader->leafReader);
}
static const char *leavesReaderTerm(LeavesReader *pReader){
  assert( !pReader->eof );
  return leafReaderTerm(&pReader->leafReader);
}

/* Access the doclist data for the current term. */
static int leavesReaderDataBytes(LeavesReader *pReader){
  assert( !pReader->eof );
  return leafReaderDataBytes(&pReader->leafReader);
}
static const char *leavesReaderData(LeavesReader *pReader){
  assert( !pReader->eof );
  return leafReaderData(&pReader->leafReader);
}

static int leavesReaderAtEnd(LeavesReader *pReader){
  return pReader->eof;
}

/* loadSegmentLeaves() may not read all the way to SQLITE_DONE, thus
** leaving the statement handle open, which locks the table.
*/
/* TODO(shess) This "solution" is not satisfactory.  Really, there
** should be check-in function for all statement handles which
** arranges to call sqlite3_reset().  This most likely will require
** modification to control flow all over the place, though, so for now
** just punt.
**
** Note the the current system assumes that segment merges will run to
** completion, which is why this particular probably hasn't arisen in
** this case.  Probably a brittle assumption.
*/
static int leavesReaderReset(LeavesReader *pReader){
  return sqlite3_reset(pReader->pStmt);
}

static void leavesReaderDestroy(LeavesReader *pReader){
  /* If idx is -1, that means we're using a non-cached statement
  ** handle in the optimize() case, so we need to release it.
  */
  if( pReader->pStmt!=NULL && pReader->idx==-1 ){
    sqlite3_finalize(pReader->pStmt);
  }
  leafReaderDestroy(&pReader->leafReader);
  dataBufferDestroy(&pReader->rootData);
  SCRAMBLE(pReader);
}

/* Initialize pReader with the given root data (if iStartBlockid==0
** the leaf data was entirely contained in the root), or from the
** stream of blocks between iStartBlockid and iEndBlockid, inclusive.
*/
static int leavesReaderInit(fulltext_vtab *v,
                            int idx,
                            sqlite_int64 iStartBlockid,
                            sqlite_int64 iEndBlockid,
                            const char *pRootData, int nRootData,
                            LeavesReader *pReader){
  CLEAR(pReader);
  pReader->idx = idx;

  dataBufferInit(&pReader->rootData, 0);
  if( iStartBlockid==0 ){
    /* Entire leaf level fit in root data. */
    dataBufferReplace(&pReader->rootData, pRootData, nRootData);
    leafReaderInit(pReader->rootData.pData, pReader->rootData.nData,
                   &pReader->leafReader);
  }else{
    sqlite3_stmt *s;
    int rc = sql_get_leaf_statement(v, idx, &s);
    if( rc!=SQLITE_OK ) return rc;

    rc = sqlite3_bind_int64(s, 1, iStartBlockid);
    if( rc!=SQLITE_OK ) return rc;

    rc = sqlite3_bind_int64(s, 2, iEndBlockid);
    if( rc!=SQLITE_OK ) return rc;

    rc = sqlite3_step(s);
    if( rc==SQLITE_DONE ){
      pReader->eof = 1;
      return SQLITE_OK;
    }
    if( rc!=SQLITE_ROW ) return rc;

    pReader->pStmt = s;
    leafReaderInit(sqlite3_column_blob(pReader->pStmt, 0),
                   sqlite3_column_bytes(pReader->pStmt, 0),
                   &pReader->leafReader);
  }
  return SQLITE_OK;
}

/* Step the current leaf forward to the next term.  If we reach the
** end of the current leaf, step forward to the next leaf block.
*/
static int leavesReaderStep(fulltext_vtab *v, LeavesReader *pReader){
  assert( !leavesReaderAtEnd(pReader) );
  leafReaderStep(&pReader->leafReader);

  if( leafReaderAtEnd(&pReader->leafReader) ){
    int rc;
    if( pReader->rootData.pData ){
      pReader->eof = 1;
      return SQLITE_OK;
    }
    rc = sqlite3_step(pReader->pStmt);
    if( rc!=SQLITE_ROW ){
      pReader->eof = 1;
      return rc==SQLITE_DONE ? SQLITE_OK : rc;
    }
    leafReaderDestroy(&pReader->leafReader);
    leafReaderInit(sqlite3_column_blob(pReader->pStmt, 0),
                   sqlite3_column_bytes(pReader->pStmt, 0),
                   &pReader->leafReader);
  }
  return SQLITE_OK;
}

/* Order LeavesReaders by their term, ignoring idx.  Readers at eof
** always sort to the end.
*/
static int leavesReaderTermCmp(LeavesReader *lr1, LeavesReader *lr2){
  if( leavesReaderAtEnd(lr1) ){
    if( leavesReaderAtEnd(lr2) ) return 0;
    return 1;
  }
  if( leavesReaderAtEnd(lr2) ) return -1;

  return leafReaderTermCmp(&lr1->leafReader,
                           leavesReaderTerm(lr2), leavesReaderTermBytes(lr2),
                           0);
}

/* Similar to leavesReaderTermCmp(), with additional ordering by idx
** so that older segments sort before newer segments.
*/
static int leavesReaderCmp(LeavesReader *lr1, LeavesReader *lr2){
  int c = leavesReaderTermCmp(lr1, lr2);
  if( c!=0 ) return c;
  return lr1->idx-lr2->idx;
}

/* Assume that pLr[1]..pLr[nLr] are sorted.  Bubble pLr[0] into its
** sorted position.
*/
static void leavesReaderReorder(LeavesReader *pLr, int nLr){
  while( nLr>1 && leavesReaderCmp(pLr, pLr+1)>0 ){
    LeavesReader tmp = pLr[0];
    pLr[0] = pLr[1];
    pLr[1] = tmp;
    nLr--;
    pLr++;
  }
}

/* Initializes pReaders with the segments from level iLevel, returning
** the number of segments in *piReaders.  Leaves pReaders in sorted
** order.
*/
static int leavesReadersInit(fulltext_vtab *v, int iLevel,
                             LeavesReader *pReaders, int *piReaders){
  sqlite3_stmt *s;
  int i, rc = sql_get_statement(v, SEGDIR_SELECT_LEVEL_STMT, &s);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_bind_int(s, 1, iLevel);
  if( rc!=SQLITE_OK ) return rc;

  i = 0;
  while( (rc = sqlite3_step(s))==SQLITE_ROW ){
    sqlite_int64 iStart = sqlite3_column_int64(s, 0);
    sqlite_int64 iEnd = sqlite3_column_int64(s, 1);
    const char *pRootData = sqlite3_column_blob(s, 2);
    int nRootData = sqlite3_column_bytes(s, 2);

    assert( i<MERGE_COUNT );
    rc = leavesReaderInit(v, i, iStart, iEnd, pRootData, nRootData,
                          &pReaders[i]);
    if( rc!=SQLITE_OK ) break;

    i++;
  }
  if( rc!=SQLITE_DONE ){
    while( i-->0 ){
      leavesReaderDestroy(&pReaders[i]);
    }
    return rc;
  }

  *piReaders = i;

  /* Leave our results sorted by term, then age. */
  while( i-- ){
    leavesReaderReorder(pReaders+i, *piReaders-i);
  }
  return SQLITE_OK;
}

/* Merge doclists from pReaders[nReaders] into a single doclist, which
** is written to pWriter.  Assumes pReaders is ordered oldest to
** newest.
*/
/* TODO(shess) Consider putting this inline in segmentMerge(). */
static int leavesReadersMerge(fulltext_vtab *v,
                              LeavesReader *pReaders, int nReaders,
                              LeafWriter *pWriter){
  DLReader dlReaders[MERGE_COUNT];
  const char *pTerm = leavesReaderTerm(pReaders);
  int i, nTerm = leavesReaderTermBytes(pReaders);

  assert( nReaders<=MERGE_COUNT );

  for(i=0; i<nReaders; i++){
    dlrInit(&dlReaders[i], DL_DEFAULT,
            leavesReaderData(pReaders+i),
            leavesReaderDataBytes(pReaders+i));
  }

  return leafWriterStepMerge(v, pWriter, pTerm, nTerm, dlReaders, nReaders);
}

/* Forward ref due to mutual recursion with segdirNextIndex(). */
static int segmentMerge(fulltext_vtab *v, int iLevel);

/* Put the next available index at iLevel into *pidx.  If iLevel
** already has MERGE_COUNT segments, they are merged to a higher
** level to make room.
*/
static int segdirNextIndex(fulltext_vtab *v, int iLevel, int *pidx){
  int rc = segdir_max_index(v, iLevel, pidx);
  if( rc==SQLITE_DONE ){              /* No segments at iLevel. */
    *pidx = 0;
  }else if( rc==SQLITE_ROW ){
    if( *pidx==(MERGE_COUNT-1) ){
      rc = segmentMerge(v, iLevel);
      if( rc!=SQLITE_OK ) return rc;
      *pidx = 0;
    }else{
      (*pidx)++;
    }
  }else{
    return rc;
  }
  return SQLITE_OK;
}

/* Merge MERGE_COUNT segments at iLevel into a new segment at
** iLevel+1.  If iLevel+1 is already full of segments, those will be
** merged to make room.
*/
static int segmentMerge(fulltext_vtab *v, int iLevel){
  LeafWriter writer;
  LeavesReader lrs[MERGE_COUNT];
  int i, rc, idx = 0;

  /* Determine the next available segment index at the next level,
  ** merging as necessary.
  */
  rc = segdirNextIndex(v, iLevel+1, &idx);
  if( rc!=SQLITE_OK ) return rc;

  /* TODO(shess) This assumes that we'll always see exactly
  ** MERGE_COUNT segments to merge at a given level.  That will be
  ** broken if we allow the developer to request preemptive or
  ** deferred merging.
  */
  memset(&lrs, '\0', sizeof(lrs));
  rc = leavesReadersInit(v, iLevel, lrs, &i);
  if( rc!=SQLITE_OK ) return rc;
  assert( i==MERGE_COUNT );

  leafWriterInit(iLevel+1, idx, &writer);

  /* Since leavesReaderReorder() pushes readers at eof to the end,
  ** when the first reader is empty, all will be empty.
  */
  while( !leavesReaderAtEnd(lrs) ){
    /* Figure out how many readers share their next term. */
    for(i=1; i<MERGE_COUNT && !leavesReaderAtEnd(lrs+i); i++){
      if( 0!=leavesReaderTermCmp(lrs, lrs+i) ) break;
    }

    rc = leavesReadersMerge(v, lrs, i, &writer);
    if( rc!=SQLITE_OK ) goto err;

    /* Step forward those that were merged. */
    while( i-->0 ){
      rc = leavesReaderStep(v, lrs+i);
      if( rc!=SQLITE_OK ) goto err;

      /* Reorder by term, then by age. */
      leavesReaderReorder(lrs+i, MERGE_COUNT-i);
    }
  }

  for(i=0; i<MERGE_COUNT; i++){
    leavesReaderDestroy(&lrs[i]);
  }

  rc = leafWriterFinalize(v, &writer);
  leafWriterDestroy(&writer);
  if( rc!=SQLITE_OK ) return rc;

  /* Delete the merged segment data. */
  return segdir_delete(v, iLevel);

 err:
  for(i=0; i<MERGE_COUNT; i++){
    leavesReaderDestroy(&lrs[i]);
  }
  leafWriterDestroy(&writer);
  return rc;
}

/* Accumulate the union of *acc and *pData into *acc. */
static void docListAccumulateUnion(DataBuffer *acc,
                                   const char *pData, int nData) {
  DataBuffer tmp = *acc;
  dataBufferInit(acc, tmp.nData+nData);
  docListUnion(tmp.pData, tmp.nData, pData, nData, acc);
  dataBufferDestroy(&tmp);
}

/* TODO(shess) It might be interesting to explore different merge
** strategies, here.  For instance, since this is a sorted merge, we
** could easily merge many doclists in parallel.  With some
** comprehension of the storage format, we could merge all of the
** doclists within a leaf node directly from the leaf node's storage.
** It may be worthwhile to merge smaller doclists before larger
** doclists, since they can be traversed more quickly - but the
** results may have less overlap, making them more expensive in a
** different way.
*/

/* Scan pReader for pTerm/nTerm, and merge the term's doclist over
** *out (any doclists with duplicate docids overwrite those in *out).
** Internal function for loadSegmentLeaf().
*/
static int loadSegmentLeavesInt(fulltext_vtab *v, LeavesReader *pReader,
                                const char *pTerm, int nTerm, int isPrefix,
                                DataBuffer *out){
  /* doclist data is accumulated into pBuffers similar to how one does
  ** increment in binary arithmetic.  If index 0 is empty, the data is
  ** stored there.  If there is data there, it is merged and the
  ** results carried into position 1, with further merge-and-carry
  ** until an empty position is found.
  */
  DataBuffer *pBuffers = NULL;
  int nBuffers = 0, nMaxBuffers = 0, rc;

  assert( nTerm>0 );

  for(rc=SQLITE_OK; rc==SQLITE_OK && !leavesReaderAtEnd(pReader);
      rc=leavesReaderStep(v, pReader)){
    /* TODO(shess) Really want leavesReaderTermCmp(), but that name is
    ** already taken to compare the terms of two LeavesReaders.  Think
    ** on a better name.  [Meanwhile, break encapsulation rather than
    ** use a confusing name.]
    */
    int c = leafReaderTermCmp(&pReader->leafReader, pTerm, nTerm, isPrefix);
    if( c>0 ) break;      /* Past any possible matches. */
    if( c==0 ){
      const char *pData = leavesReaderData(pReader);
      int iBuffer, nData = leavesReaderDataBytes(pReader);

      /* Find the first empty buffer. */
      for(iBuffer=0; iBuffer<nBuffers; ++iBuffer){
        if( 0==pBuffers[iBuffer].nData ) break;
      }

      /* Out of buffers, add an empty one. */
      if( iBuffer==nBuffers ){
        if( nBuffers==nMaxBuffers ){
          DataBuffer *p;
          nMaxBuffers += 20;

          /* Manual realloc so we can handle NULL appropriately. */
          p = sqlite3_malloc(nMaxBuffers*sizeof(*pBuffers));
          if( p==NULL ){
            rc = SQLITE_NOMEM;
            break;
          }

          if( nBuffers>0 ){
            assert(pBuffers!=NULL);
            memcpy(p, pBuffers, nBuffers*sizeof(*pBuffers));
            sqlite3_free(pBuffers);
          }
          pBuffers = p;
        }
        dataBufferInit(&(pBuffers[nBuffers]), 0);
        nBuffers++;
      }

      /* At this point, must have an empty at iBuffer. */
      assert(iBuffer<nBuffers && pBuffers[iBuffer].nData==0);

      /* If empty was first buffer, no need for merge logic. */
      if( iBuffer==0 ){
        dataBufferReplace(&(pBuffers[0]), pData, nData);
      }else{
        /* pAcc is the empty buffer the merged data will end up in. */
        DataBuffer *pAcc = &(pBuffers[iBuffer]);
        DataBuffer *p = &(pBuffers[0]);

        /* Handle position 0 specially to avoid need to prime pAcc
        ** with pData/nData.
        */
        dataBufferSwap(p, pAcc);
        docListAccumulateUnion(pAcc, pData, nData);

        /* Accumulate remaining doclists into pAcc. */
        for(++p; p<pAcc; ++p){
          docListAccumulateUnion(pAcc, p->pData, p->nData);

          /* dataBufferReset() could allow a large doclist to blow up
          ** our memory requirements.
          */
          if( p->nCapacity<1024 ){
            dataBufferReset(p);
          }else{
            dataBufferDestroy(p);
            dataBufferInit(p, 0);
          }
        }
      }
    }
  }

  /* Union all the doclists together into *out. */
  /* TODO(shess) What if *out is big?  Sigh. */
  if( rc==SQLITE_OK && nBuffers>0 ){
    int iBuffer;
    for(iBuffer=0; iBuffer<nBuffers; ++iBuffer){
      if( pBuffers[iBuffer].nData>0 ){
        if( out->nData==0 ){
          dataBufferSwap(out, &(pBuffers[iBuffer]));
        }else{
          docListAccumulateUnion(out, pBuffers[iBuffer].pData,
                                 pBuffers[iBuffer].nData);
        }
      }
    }
  }

  while( nBuffers-- ){
    dataBufferDestroy(&(pBuffers[nBuffers]));
  }
  if( pBuffers!=NULL ) sqlite3_free(pBuffers);

  return rc;
}

/* Call loadSegmentLeavesInt() with pData/nData as input. */
static int loadSegmentLeaf(fulltext_vtab *v, const char *pData, int nData,
                           const char *pTerm, int nTerm, int isPrefix,
                           DataBuffer *out){
  LeavesReader reader;
  int rc;

  assert( nData>1 );
  assert( *pData=='\0' );
  rc = leavesReaderInit(v, 0, 0, 0, pData, nData, &reader);
  if( rc!=SQLITE_OK ) return rc;

  rc = loadSegmentLeavesInt(v, &reader, pTerm, nTerm, isPrefix, out);
  leavesReaderReset(&reader);
  leavesReaderDestroy(&reader);
  return rc;
}

/* Call loadSegmentLeavesInt() with the leaf nodes from iStartLeaf to
** iEndLeaf (inclusive) as input, and merge the resulting doclist into
** out.
*/
static int loadSegmentLeaves(fulltext_vtab *v,
                             sqlite_int64 iStartLeaf, sqlite_int64 iEndLeaf,
                             const char *pTerm, int nTerm, int isPrefix,
                             DataBuffer *out){
  int rc;
  LeavesReader reader;

  assert( iStartLeaf<=iEndLeaf );
  rc = leavesReaderInit(v, 0, iStartLeaf, iEndLeaf, NULL, 0, &reader);
  if( rc!=SQLITE_OK ) return rc;

  rc = loadSegmentLeavesInt(v, &reader, pTerm, nTerm, isPrefix, out);
  leavesReaderReset(&reader);
  leavesReaderDestroy(&reader);
  return rc;
}

/* Taking pData/nData as an interior node, find the sequence of child
** nodes which could include pTerm/nTerm/isPrefix.  Note that the
** interior node terms logically come between the blocks, so there is
** one more blockid than there are terms (that block contains terms >=
** the last interior-node term).
*/
/* TODO(shess) The calling code may already know that the end child is
** not worth calculating, because the end may be in a later sibling
** node.  Consider whether breaking symmetry is worthwhile.  I suspect
** it is not worthwhile.
*/
static void getChildrenContaining(const char *pData, int nData,
                                  const char *pTerm, int nTerm, int isPrefix,
                                  sqlite_int64 *piStartChild,
                                  sqlite_int64 *piEndChild){
  InteriorReader reader;

  assert( nData>1 );
  assert( *pData!='\0' );
  interiorReaderInit(pData, nData, &reader);

  /* Scan for the first child which could contain pTerm/nTerm. */
  while( !interiorReaderAtEnd(&reader) ){
    if( interiorReaderTermCmp(&reader, pTerm, nTerm, 0)>0 ) break;
    interiorReaderStep(&reader);
  }
  *piStartChild = interiorReaderCurrentBlockid(&reader);

  /* Keep scanning to find a term greater than our term, using prefix
  ** comparison if indicated.  If isPrefix is false, this will be the
  ** same blockid as the starting block.
  */
  while( !interiorReaderAtEnd(&reader) ){
    if( interiorReaderTermCmp(&reader, pTerm, nTerm, isPrefix)>0 ) break;
    interiorReaderStep(&reader);
  }
  *piEndChild = interiorReaderCurrentBlockid(&reader);

  interiorReaderDestroy(&reader);

  /* Children must ascend, and if !prefix, both must be the same. */
  assert( *piEndChild>=*piStartChild );
  assert( isPrefix || *piStartChild==*piEndChild );
}

/* Read block at iBlockid and pass it with other params to
** getChildrenContaining().
*/
static int loadAndGetChildrenContaining(
  fulltext_vtab *v,
  sqlite_int64 iBlockid,
  const char *pTerm, int nTerm, int isPrefix,
  sqlite_int64 *piStartChild, sqlite_int64 *piEndChild
){
  sqlite3_stmt *s = NULL;
  int rc;

  assert( iBlockid!=0 );
  assert( pTerm!=NULL );
  assert( nTerm!=0 );        /* TODO(shess) Why not allow this? */
  assert( piStartChild!=NULL );
  assert( piEndChild!=NULL );

  rc = sql_get_statement(v, BLOCK_SELECT_STMT, &s);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_bind_int64(s, 1, iBlockid);
  if( rc!=SQLITE_OK ) return rc;

  rc = sqlite3_step(s);
  if( rc==SQLITE_DONE ) return SQLITE_ERROR;
  if( rc!=SQLITE_ROW ) return rc;

  getChildrenContaining(sqlite3_column_blob(s, 0), sqlite3_column_bytes(s, 0),
                        pTerm, nTerm, isPrefix, piStartChild, piEndChild);

  /* We expect only one row.  We must execute another sqlite3_step()
   * to complete the iteration; otherwise the table will remain
   * locked. */
  rc = sqlite3_step(s);
  if( rc==SQLITE_ROW ) return SQLITE_ERROR;
  if( rc!=SQLITE_DONE ) return rc;

  return SQLITE_OK;
}

/* Traverse the tree represented by pData[nData] looking for
** pTerm[nTerm], placing its doclist into *out.  This is internal to
** loadSegment() to make error-handling cleaner.
*/
static int loadSegmentInt(fulltext_vtab *v, const char *pData, int nData,
                          sqlite_int64 iLeavesEnd,
                          const char *pTerm, int nTerm, int isPrefix,
                          DataBuffer *out){
  /* Special case where root is a leaf. */
  if( *pData=='\0' ){
    return loadSegmentLeaf(v, pData, nData, pTerm, nTerm, isPrefix, out);
  }else{
    int rc;
    sqlite_int64 iStartChild, iEndChild;

    /* Process pData as an interior node, then loop down the tree
    ** until we find the set of leaf nodes to scan for the term.
    */
    getChildrenContaining(pData, nData, pTerm, nTerm, isPrefix,
                          &iStartChild, &iEndChild);
    while( iStartChild>iLeavesEnd ){
      sqlite_int64 iNextStart, iNextEnd;
      rc = loadAndGetChildrenContaining(v, iStartChild, pTerm, nTerm, isPrefix,
                                        &iNextStart, &iNextEnd);
      if( rc!=SQLITE_OK ) return rc;

      /* If we've branched, follow the end branch, too. */
      if( iStartChild!=iEndChild ){
        sqlite_int64 iDummy;
        rc = loadAndGetChildrenContaining(v, iEndChild, pTerm, nTerm, isPrefix,
                                          &iDummy, &iNextEnd);
        if( rc!=SQLITE_OK ) return rc;
      }

      assert( iNextStart<=iNextEnd );
      iStartChild = iNextStart;
      iEndChild = iNextEnd;
    }
    assert( iStartChild<=iLeavesEnd );
    assert( iEndChild<=iLeavesEnd );

    /* Scan through the leaf segments for doclists. */
    return loadSegmentLeaves(v, iStartChild, iEndChild,
                             pTerm, nTerm, isPrefix, out);
  }
}

/* Call loadSegmentInt() to collect the doclist for pTerm/nTerm, then
** merge its doclist over *out (any duplicate doclists read from the
** segment rooted at pData will overwrite those in *out).
*/
/* TODO(shess) Consider changing this to determine the depth of the
** leaves using either the first characters of interior nodes (when
** ==1, we're one level above the leaves), or the first character of
** the root (which will describe the height of the tree directly).
** Either feels somewhat tricky to me.
*/
/* TODO(shess) The current merge is likely to be slow for large
** doclists (though it should process from newest/smallest to
** oldest/largest, so it may not be that bad).  It might be useful to
** modify things to allow for N-way merging.  This could either be
** within a segment, with pairwise merges across segments, or across
** all segments at once.
*/
static int loadSegment(fulltext_vtab *v, const char *pData, int nData,
                       sqlite_int64 iLeavesEnd,
                       const char *pTerm, int nTerm, int isPrefix,
                       DataBuffer *out){
  DataBuffer result;
  int rc;

  assert( nData>1 );

  /* This code should never be called with buffered updates. */
  assert( v->nPendingData<0 );

  dataBufferInit(&result, 0);
  rc = loadSegmentInt(v, pData, nData, iLeavesEnd,
                      pTerm, nTerm, isPrefix, &result);
  if( rc==SQLITE_OK && result.nData>0 ){
    if( out->nData==0 ){
      DataBuffer tmp = *out;
      *out = result;
      result = tmp;
    }else{
      DataBuffer merged;
      DLReader readers[2];

      dlrInit(&readers[0], DL_DEFAULT, out->pData, out->nData);
      dlrInit(&readers[1], DL_DEFAULT, result.pData, result.nData);
      dataBufferInit(&merged, out->nData+result.nData);
      docListMerge(&merged, readers, 2);
      dataBufferDestroy(out);
      *out = merged;
      dlrDestroy(&readers[0]);
      dlrDestroy(&readers[1]);
    }
  }
  dataBufferDestroy(&result);
  return rc;
}

/* Scan the database and merge together the posting lists for the term
** into *out.
*/
static int termSelect(fulltext_vtab *v, int iColumn,
                      const char *pTerm, int nTerm, int isPrefix,
                      DocListType iType, DataBuffer *out){
  DataBuffer doclist;
  sqlite3_stmt *s;
  int rc = sql_get_statement(v, SEGDIR_SELECT_ALL_STMT, &s);
  if( rc!=SQLITE_OK ) return rc;

  /* This code should never be called with buffered updates. */
  assert( v->nPendingData<0 );

  dataBufferInit(&doclist, 0);

  /* Traverse the segments from oldest to newest so that newer doclist
  ** elements for given docids overwrite older elements.
  */
  while( (rc = sqlite3_step(s))==SQLITE_ROW ){
    const char *pData = sqlite3_column_blob(s, 2);
    const int nData = sqlite3_column_bytes(s, 2);
    const sqlite_int64 iLeavesEnd = sqlite3_column_int64(s, 1);
    rc = loadSegment(v, pData, nData, iLeavesEnd, pTerm, nTerm, isPrefix,
                     &doclist);
    if( rc!=SQLITE_OK ) goto err;
  }
  if( rc==SQLITE_DONE ){
    if( doclist.nData!=0 ){
      /* TODO(shess) The old term_select_all() code applied the column
      ** restrict as we merged segments, leading to smaller buffers.
      ** This is probably worthwhile to bring back, once the new storage
      ** system is checked in.
      */
      if( iColumn==v->nColumn) iColumn = -1;
      docListTrim(DL_DEFAULT, doclist.pData, doclist.nData,
                  iColumn, iType, out);
    }
    rc = SQLITE_OK;
  }

 err:
  dataBufferDestroy(&doclist);
  return rc;
}

/****************************************************************/
/* Used to hold hashtable data for sorting. */
typedef struct TermData {
  const char *pTerm;
  int nTerm;
  DLCollector *pCollector;
} TermData;

/* Orders TermData elements in strcmp fashion ( <0 for less-than, 0
** for equal, >0 for greater-than).
*/
static int termDataCmp(const void *av, const void *bv){
  const TermData *a = (const TermData *)av;
  const TermData *b = (const TermData *)bv;
  int n = a->nTerm<b->nTerm ? a->nTerm : b->nTerm;
  int c = memcmp(a->pTerm, b->pTerm, n);
  if( c!=0 ) return c;
  return a->nTerm-b->nTerm;
}

/* Order pTerms data by term, then write a new level 0 segment using
** LeafWriter.
*/
static int writeZeroSegment(fulltext_vtab *v, fts2Hash *pTerms){
  fts2HashElem *e;
  int idx, rc, i, n;
  TermData *pData;
  LeafWriter writer;
  DataBuffer dl;

  /* Determine the next index at level 0, merging as necessary. */
  rc = segdirNextIndex(v, 0, &idx);
  if( rc!=SQLITE_OK ) return rc;

  n = fts2HashCount(pTerms);
  pData = sqlite3_malloc(n*sizeof(TermData));

  for(i = 0, e = fts2HashFirst(pTerms); e; i++, e = fts2HashNext(e)){
    assert( i<n );
    pData[i].pTerm = fts2HashKey(e);
    pData[i].nTerm = fts2HashKeysize(e);
    pData[i].pCollector = fts2HashData(e);
  }
  assert( i==n );

  /* TODO(shess) Should we allow user-defined collation sequences,
  ** here?  I think we only need that once we support prefix searches.
  */
  if( n>1 ) qsort(pData, n, sizeof(*pData), termDataCmp);

  /* TODO(shess) Refactor so that we can write directly to the segment
  ** DataBuffer, as happens for segment merges.
  */
  leafWriterInit(0, idx, &writer);
  dataBufferInit(&dl, 0);
  for(i=0; i<n; i++){
    dataBufferReset(&dl);
    dlcAddDoclist(pData[i].pCollector, &dl);
    rc = leafWriterStep(v, &writer,
                        pData[i].pTerm, pData[i].nTerm, dl.pData, dl.nData);
    if( rc!=SQLITE_OK ) goto err;
  }
  rc = leafWriterFinalize(v, &writer);

 err:
  dataBufferDestroy(&dl);
  sqlite3_free(pData);
  leafWriterDestroy(&writer);
  return rc;
}

/* If pendingTerms has data, free it. */
static int clearPendingTerms(fulltext_vtab *v){
  if( v->nPendingData>=0 ){
    fts2HashElem *e;
    for(e=fts2HashFirst(&v->pendingTerms); e; e=fts2HashNext(e)){
      dlcDelete(fts2HashData(e));
    }
    fts2HashClear(&v->pendingTerms);
    v->nPendingData = -1;
  }
  return SQLITE_OK;
}

/* If pendingTerms has data, flush it to a level-zero segment, and
** free it.
*/
static int flushPendingTerms(fulltext_vtab *v){
  if( v->nPendingData>=0 ){
    int rc = writeZeroSegment(v, &v->pendingTerms);
    if( rc==SQLITE_OK ) clearPendingTerms(v);
    return rc;
  }
  return SQLITE_OK;
}

/* If pendingTerms is "too big", or docid is out of order, flush it.
** Regardless, be certain that pendingTerms is initialized for use.
*/
static int initPendingTerms(fulltext_vtab *v, sqlite_int64 iDocid){
  /* TODO(shess) Explore whether partially flushing the buffer on
  ** forced-flush would provide better performance.  I suspect that if
  ** we ordered the doclists by size and flushed the largest until the
  ** buffer was half empty, that would let the less frequent terms
  ** generate longer doclists.
  */
  if( iDocid<=v->iPrevDocid || v->nPendingData>kPendingThreshold ){
    int rc = flushPendingTerms(v);
    if( rc!=SQLITE_OK ) return rc;
  }
  if( v->nPendingData<0 ){
    fts2HashInit(&v->pendingTerms, FTS2_HASH_STRING, 1);
    v->nPendingData = 0;
  }
  v->iPrevDocid = iDocid;
  return SQLITE_OK;
}

/* This function implements the xUpdate callback; it is the top-level entry
 * point for inserting, deleting or updating a row in a full-text table. */
static int fulltextUpdate(sqlite3_vtab *pVtab, int nArg, sqlite3_value **ppArg,
                   sqlite_int64 *pRowid){
  fulltext_vtab *v = (fulltext_vtab *) pVtab;
  int rc;

  TRACE(("FTS2 Update %p\n", pVtab));

  if( nArg<2 ){
    rc = index_delete(v, sqlite3_value_int64(ppArg[0]));
    if( rc==SQLITE_OK ){
      /* If we just deleted the last row in the table, clear out the
      ** index data.
      */
      rc = content_exists(v);
      if( rc==SQLITE_ROW ){
        rc = SQLITE_OK;
      }else if( rc==SQLITE_DONE ){
        /* Clear the pending terms so we don't flush a useless level-0
        ** segment when the transaction closes.
        */
        rc = clearPendingTerms(v);
        if( rc==SQLITE_OK ){
          rc = segdir_delete_all(v);
        }
      }
    }
  } else if( sqlite3_value_type(ppArg[0]) != SQLITE_NULL ){
    /* An update:
     * ppArg[0] = old rowid
     * ppArg[1] = new rowid
     * ppArg[2..2+v->nColumn-1] = values
     * ppArg[2+v->nColumn] = value for magic column (we ignore this)
     */
    sqlite_int64 rowid = sqlite3_value_int64(ppArg[0]);
    if( sqlite3_value_type(ppArg[1]) != SQLITE_INTEGER ||
      sqlite3_value_int64(ppArg[1]) != rowid ){
      rc = SQLITE_ERROR;  /* we don't allow changing the rowid */
    } else {
      assert( nArg==2+v->nColumn+1);
      rc = index_update(v, rowid, &ppArg[2]);
    }
  } else {
    /* An insert:
     * ppArg[1] = requested rowid
     * ppArg[2..2+v->nColumn-1] = values
     * ppArg[2+v->nColumn] = value for magic column (we ignore this)
     */
    assert( nArg==2+v->nColumn+1);
    rc = index_insert(v, ppArg[1], &ppArg[2], pRowid);
  }

  return rc;
}

static int fulltextSync(sqlite3_vtab *pVtab){
  TRACE(("FTS2 xSync()\n"));
  return flushPendingTerms((fulltext_vtab *)pVtab);
}

static int fulltextBegin(sqlite3_vtab *pVtab){
  fulltext_vtab *v = (fulltext_vtab *) pVtab;
  TRACE(("FTS2 xBegin()\n"));

  /* Any buffered updates should have been cleared by the previous
  ** transaction.
  */
  assert( v->nPendingData<0 );
  return clearPendingTerms(v);
}

static int fulltextCommit(sqlite3_vtab *pVtab){
  fulltext_vtab *v = (fulltext_vtab *) pVtab;
  TRACE(("FTS2 xCommit()\n"));

  /* Buffered updates should have been cleared by fulltextSync(). */
  assert( v->nPendingData<0 );
  return clearPendingTerms(v);
}

static int fulltextRollback(sqlite3_vtab *pVtab){
  TRACE(("FTS2 xRollback()\n"));
  return clearPendingTerms((fulltext_vtab *)pVtab);
}

/*
** Implementation of the snippet() function for FTS2
*/
static void snippetFunc(
  sqlite3_context *pContext,
  int argc,
  sqlite3_value **argv
){
  fulltext_cursor *pCursor;
  if( argc<1 ) return;
  if( sqlite3_value_type(argv[0])!=SQLITE_BLOB ||
      sqlite3_value_bytes(argv[0])!=sizeof(pCursor) ){
    sqlite3_result_error(pContext, "illegal first argument to html_snippet",-1);
  }else{
    const char *zStart = "<b>";
    const char *zEnd = "</b>";
    const char *zEllipsis = "<b>...</b>";
    memcpy(&pCursor, sqlite3_value_blob(argv[0]), sizeof(pCursor));
    if( argc>=2 ){
      zStart = (const char*)sqlite3_value_text(argv[1]);
      if( argc>=3 ){
        zEnd = (const char*)sqlite3_value_text(argv[2]);
        if( argc>=4 ){
          zEllipsis = (const char*)sqlite3_value_text(argv[3]);
        }
      }
    }
    snippetAllOffsets(pCursor);
    snippetText(pCursor, zStart, zEnd, zEllipsis);
    sqlite3_result_text(pContext, pCursor->snippet.zSnippet,
                        pCursor->snippet.nSnippet, SQLITE_STATIC);
  }
}

/*
** Implementation of the offsets() function for FTS2
*/
static void snippetOffsetsFunc(
  sqlite3_context *pContext,
  int argc,
  sqlite3_value **argv
){
  fulltext_cursor *pCursor;
  if( argc<1 ) return;
  if( sqlite3_value_type(argv[0])!=SQLITE_BLOB ||
      sqlite3_value_bytes(argv[0])!=sizeof(pCursor) ){
    sqlite3_result_error(pContext, "illegal first argument to offsets",-1);
  }else{
    memcpy(&pCursor, sqlite3_value_blob(argv[0]), sizeof(pCursor));
    snippetAllOffsets(pCursor);
    snippetOffsetText(&pCursor->snippet);
    sqlite3_result_text(pContext,
                        pCursor->snippet.zOffset, pCursor->snippet.nOffset,
                        SQLITE_STATIC);
  }
}

/* OptLeavesReader is nearly identical to LeavesReader, except that
** where LeavesReader is geared towards the merging of complete
** segment levels (with exactly MERGE_COUNT segments), OptLeavesReader
** is geared towards implementation of the optimize() function, and
** can merge all segments simultaneously.  This version may be
** somewhat less efficient than LeavesReader because it merges into an
** accumulator rather than doing an N-way merge, but since segment
** size grows exponentially (so segment count logrithmically) this is
** probably not an immediate problem.
*/
/* TODO(shess): Prove that assertion, or extend the merge code to
** merge tree fashion (like the prefix-searching code does).
*/
/* TODO(shess): OptLeavesReader and LeavesReader could probably be
** merged with little or no loss of performance for LeavesReader.  The
** merged code would need to handle >MERGE_COUNT segments, and would
** also need to be able to optionally optimize away deletes.
*/
typedef struct OptLeavesReader {
  /* Segment number, to order readers by age. */
  int segment;
  LeavesReader reader;
} OptLeavesReader;

static int optLeavesReaderAtEnd(OptLeavesReader *pReader){
  return leavesReaderAtEnd(&pReader->reader);
}
static int optLeavesReaderTermBytes(OptLeavesReader *pReader){
  return leavesReaderTermBytes(&pReader->reader);
}
static const char *optLeavesReaderData(OptLeavesReader *pReader){
  return leavesReaderData(&pReader->reader);
}
static int optLeavesReaderDataBytes(OptLeavesReader *pReader){
  return leavesReaderDataBytes(&pReader->reader);
}
static const char *optLeavesReaderTerm(OptLeavesReader *pReader){
  return leavesReaderTerm(&pReader->reader);
}
static int optLeavesReaderStep(fulltext_vtab *v, OptLeavesReader *pReader){
  return leavesReaderStep(v, &pReader->reader);
}
static int optLeavesReaderTermCmp(OptLeavesReader *lr1, OptLeavesReader *lr2){
  return leavesReaderTermCmp(&lr1->reader, &lr2->reader);
}
/* Order by term ascending, segment ascending (oldest to newest), with
** exhausted readers to the end.
*/
static int optLeavesReaderCmp(OptLeavesReader *lr1, OptLeavesReader *lr2){
  int c = optLeavesReaderTermCmp(lr1, lr2);
  if( c!=0 ) return c;
  return lr1->segment-lr2->segment;
}
/* Bubble pLr[0] to appropriate place in pLr[1..nLr-1].  Assumes that
** pLr[1..nLr-1] is already sorted.
*/
static void optLeavesReaderReorder(OptLeavesReader *pLr, int nLr){
  while( nLr>1 && optLeavesReaderCmp(pLr, pLr+1)>0 ){
    OptLeavesReader tmp = pLr[0];
    pLr[0] = pLr[1];
    pLr[1] = tmp;
    nLr--;
    pLr++;
  }
}

/* optimize() helper function.  Put the readers in order and iterate
** through them, merging doclists for matching terms into pWriter.
** Returns SQLITE_OK on success, or the SQLite error code which
** prevented success.
*/
static int optimizeInternal(fulltext_vtab *v,
                            OptLeavesReader *readers, int nReaders,
                            LeafWriter *pWriter){
  int i, rc = SQLITE_OK;
  DataBuffer doclist, merged, tmp;

  /* Order the readers. */
  i = nReaders;
  while( i-- > 0 ){
    optLeavesReaderReorder(&readers[i], nReaders-i);
  }

  dataBufferInit(&doclist, LEAF_MAX);
  dataBufferInit(&merged, LEAF_MAX);

  /* Exhausted readers bubble to the end, so when the first reader is
  ** at eof, all are at eof.
  */
  while( !optLeavesReaderAtEnd(&readers[0]) ){

    /* Figure out how many readers share the next term. */
    for(i=1; i<nReaders && !optLeavesReaderAtEnd(&readers[i]); i++){
      if( 0!=optLeavesReaderTermCmp(&readers[0], &readers[i]) ) break;
    }

    /* Special-case for no merge. */
    if( i==1 ){
      /* Trim deletions from the doclist. */
      dataBufferReset(&merged);
      docListTrim(DL_DEFAULT,
                  optLeavesReaderData(&readers[0]),
                  optLeavesReaderDataBytes(&readers[0]),
                  -1, DL_DEFAULT, &merged);
    }else{
      DLReader dlReaders[MERGE_COUNT];
      int iReader, nReaders;

      /* Prime the pipeline with the first reader's doclist.  After
      ** one pass index 0 will reference the accumulated doclist.
      */
      dlrInit(&dlReaders[0], DL_DEFAULT,
              optLeavesReaderData(&readers[0]),
              optLeavesReaderDataBytes(&readers[0]));
      iReader = 1;

      assert( iReader<i );  /* Must execute the loop at least once. */
      while( iReader<i ){
        /* Merge 16 inputs per pass. */
        for( nReaders=1; iReader<i && nReaders<MERGE_COUNT;
             iReader++, nReaders++ ){
          dlrInit(&dlReaders[nReaders], DL_DEFAULT,
                  optLeavesReaderData(&readers[iReader]),
                  optLeavesReaderDataBytes(&readers[iReader]));
        }

        /* Merge doclists and swap result into accumulator. */
        dataBufferReset(&merged);
        docListMerge(&merged, dlReaders, nReaders);
        tmp = merged;
        merged = doclist;
        doclist = tmp;

        while( nReaders-- > 0 ){
          dlrDestroy(&dlReaders[nReaders]);
        }

        /* Accumulated doclist to reader 0 for next pass. */
        dlrInit(&dlReaders[0], DL_DEFAULT, doclist.pData, doclist.nData);
      }

      /* Destroy reader that was left in the pipeline. */
      dlrDestroy(&dlReaders[0]);

      /* Trim deletions from the doclist. */
      dataBufferReset(&merged);
      docListTrim(DL_DEFAULT, doclist.pData, doclist.nData,
                  -1, DL_DEFAULT, &merged);
    }

    /* Only pass doclists with hits (skip if all hits deleted). */
    if( merged.nData>0 ){
      rc = leafWriterStep(v, pWriter,
                          optLeavesReaderTerm(&readers[0]),
                          optLeavesReaderTermBytes(&readers[0]),
                          merged.pData, merged.nData);
      if( rc!=SQLITE_OK ) goto err;
    }

    /* Step merged readers to next term and reorder. */
    while( i-- > 0 ){
      rc = optLeavesReaderStep(v, &readers[i]);
      if( rc!=SQLITE_OK ) goto err;

      optLeavesReaderReorder(&readers[i], nReaders-i);
    }
  }

 err:
  dataBufferDestroy(&doclist);
  dataBufferDestroy(&merged);
  return rc;
}

/* Implement optimize() function for FTS3.  optimize(t) merges all
** segments in the fts index into a single segment.  't' is the magic
** table-named column.
*/
static void optimizeFunc(sqlite3_context *pContext,
                         int argc, sqlite3_value **argv){
  fulltext_cursor *pCursor;
  if( argc>1 ){
    sqlite3_result_error(pContext, "excess arguments to optimize()",-1);
  }else if( sqlite3_value_type(argv[0])!=SQLITE_BLOB ||
            sqlite3_value_bytes(argv[0])!=sizeof(pCursor) ){
    sqlite3_result_error(pContext, "illegal first argument to optimize",-1);
  }else{
    fulltext_vtab *v;
    int i, rc, iMaxLevel;
    OptLeavesReader *readers;
    int nReaders;
    LeafWriter writer;
    sqlite3_stmt *s;

    memcpy(&pCursor, sqlite3_value_blob(argv[0]), sizeof(pCursor));
    v = cursor_vtab(pCursor);

    /* Flush any buffered updates before optimizing. */
    rc = flushPendingTerms(v);
    if( rc!=SQLITE_OK ) goto err;

    rc = segdir_count(v, &nReaders, &iMaxLevel);
    if( rc!=SQLITE_OK ) goto err;
    if( nReaders==0 || nReaders==1 ){
      sqlite3_result_text(pContext, "Index already optimal", -1,
                          SQLITE_STATIC);
      return;
    }

    rc = sql_get_statement(v, SEGDIR_SELECT_ALL_STMT, &s);
    if( rc!=SQLITE_OK ) goto err;

    readers = sqlite3_malloc(nReaders*sizeof(readers[0]));
    if( readers==NULL ) goto err;

    /* Note that there will already be a segment at this position
    ** until we call segdir_delete() on iMaxLevel.
    */
    leafWriterInit(iMaxLevel, 0, &writer);

    i = 0;
    while( (rc = sqlite3_step(s))==SQLITE_ROW ){
      sqlite_int64 iStart = sqlite3_column_int64(s, 0);
      sqlite_int64 iEnd = sqlite3_column_int64(s, 1);
      const char *pRootData = sqlite3_column_blob(s, 2);
      int nRootData = sqlite3_column_bytes(s, 2);

      assert( i<nReaders );
      rc = leavesReaderInit(v, -1, iStart, iEnd, pRootData, nRootData,
                            &readers[i].reader);
      if( rc!=SQLITE_OK ) break;

      readers[i].segment = i;
      i++;
    }

    /* If we managed to succesfully read them all, optimize them. */
    if( rc==SQLITE_DONE ){
      assert( i==nReaders );
      rc = optimizeInternal(v, readers, nReaders, &writer);
    }

    while( i-- > 0 ){
      leavesReaderDestroy(&readers[i].reader);
    }
    sqlite3_free(readers);

    /* If we've successfully gotten to here, delete the old segments
    ** and flush the interior structure of the new segment.
    */
    if( rc==SQLITE_OK ){
      for( i=0; i<=iMaxLevel; i++ ){
        rc = segdir_delete(v, i);
        if( rc!=SQLITE_OK ) break;
      }

      if( rc==SQLITE_OK ) rc = leafWriterFinalize(v, &writer);
    }

    leafWriterDestroy(&writer);

    if( rc!=SQLITE_OK ) goto err;

    sqlite3_result_text(pContext, "Index optimized", -1, SQLITE_STATIC);
    return;

    /* TODO(shess): Error-handling needs to be improved along the
    ** lines of the dump_ functions.
    */
 err:
    {
      char buf[512];
#if GEARS_FTS2_CHANGES
      sqlite3_snprintf(sizeof(buf), buf, "Error in optimize: %s",
                       sqlite3_errmsg(v->db));
#else
      sqlite3_snprintf(sizeof(buf), buf, "Error in optimize: %s",
                       sqlite3_errmsg(sqlite3_context_db_handle(pContext)));
#endif
      sqlite3_result_error(pContext, buf, -1);
    }
  }
}

#ifdef SQLITE_TEST
/* Generate an error of the form "<prefix>: <msg>".  If msg is NULL,
** pull the error from the context's db handle.
*/
static void generateError(sqlite3_context *pContext,
                          const char *prefix, const char *msg){
  char buf[512];
  if( msg==NULL ) msg = sqlite3_errmsg(sqlite3_context_db_handle(pContext));
  sqlite3_snprintf(sizeof(buf), buf, "%s: %s", prefix, msg);
  sqlite3_result_error(pContext, buf, -1);
}

/* Helper function to collect the set of terms in the segment into
** pTerms.  The segment is defined by the leaf nodes between
** iStartBlockid and iEndBlockid, inclusive, or by the contents of
** pRootData if iStartBlockid is 0 (in which case the entire segment
** fit in a leaf).
*/
static int collectSegmentTerms(fulltext_vtab *v, sqlite3_stmt *s,
                               fts2Hash *pTerms){
  const sqlite_int64 iStartBlockid = sqlite3_column_int64(s, 0);
  const sqlite_int64 iEndBlockid = sqlite3_column_int64(s, 1);
  const char *pRootData = sqlite3_column_blob(s, 2);
  const int nRootData = sqlite3_column_bytes(s, 2);
  LeavesReader reader;
  int rc = leavesReaderInit(v, 0, iStartBlockid, iEndBlockid,
                            pRootData, nRootData, &reader);
  if( rc!=SQLITE_OK ) return rc;

  while( rc==SQLITE_OK && !leavesReaderAtEnd(&reader) ){
    const char *pTerm = leavesReaderTerm(&reader);
    const int nTerm = leavesReaderTermBytes(&reader);
    void *oldValue = sqlite3Fts2HashFind(pTerms, pTerm, nTerm);
    void *newValue = (void *)((char *)oldValue+1);

    /* From the comment before sqlite3Fts2HashInsert in fts2_hash.c,
    ** the data value passed is returned in case of malloc failure.
    */
    if( newValue==sqlite3Fts2HashInsert(pTerms, pTerm, nTerm, newValue) ){
      rc = SQLITE_NOMEM;
    }else{
      rc = leavesReaderStep(v, &reader);
    }
  }

  leavesReaderDestroy(&reader);
  return rc;
}

/* Helper function to build the result string for dump_terms(). */
static int generateTermsResult(sqlite3_context *pContext, fts2Hash *pTerms){
  int iTerm, nTerms, nResultBytes, iByte;
  char *result;
  TermData *pData;
  fts2HashElem *e;

  /* Iterate pTerms to generate an array of terms in pData for
  ** sorting.
  */
  nTerms = fts2HashCount(pTerms);
  assert( nTerms>0 );
  pData = sqlite3_malloc(nTerms*sizeof(TermData));
  if( pData==NULL ) return SQLITE_NOMEM;

  nResultBytes = 0;
  for(iTerm = 0, e = fts2HashFirst(pTerms); e; iTerm++, e = fts2HashNext(e)){
    nResultBytes += fts2HashKeysize(e)+1;   /* Term plus trailing space */
    assert( iTerm<nTerms );
    pData[iTerm].pTerm = fts2HashKey(e);
    pData[iTerm].nTerm = fts2HashKeysize(e);
    pData[iTerm].pCollector = fts2HashData(e);  /* unused */
  }
  assert( iTerm==nTerms );

  assert( nResultBytes>0 );   /* nTerms>0, nResultsBytes must be, too. */
  result = sqlite3_malloc(nResultBytes);
  if( result==NULL ){
    sqlite3_free(pData);
    return SQLITE_NOMEM;
  }

  if( nTerms>1 ) qsort(pData, nTerms, sizeof(*pData), termDataCmp);

  /* Read the terms in order to build the result. */
  iByte = 0;
  for(iTerm=0; iTerm<nTerms; ++iTerm){
    memcpy(result+iByte, pData[iTerm].pTerm, pData[iTerm].nTerm);
    iByte += pData[iTerm].nTerm;
    result[iByte++] = ' ';
  }
  assert( iByte==nResultBytes );
  assert( result[nResultBytes-1]==' ' );
  result[nResultBytes-1] = '\0';

  /* Passes away ownership of result. */
  sqlite3_result_text(pContext, result, nResultBytes-1, sqlite3_free);
  sqlite3_free(pData);
  return SQLITE_OK;
}

/* Implements dump_terms() for use in inspecting the fts2 index from
** tests.  TEXT result containing the ordered list of terms joined by
** spaces.  dump_terms(t, level, idx) dumps the terms for the segment
** specified by level, idx (in %_segdir), while dump_terms(t) dumps
** all terms in the index.  In both cases t is the fts table's magic
** table-named column.
*/
static void dumpTermsFunc(
  sqlite3_context *pContext,
  int argc, sqlite3_value **argv
){
  fulltext_cursor *pCursor;
  if( argc!=3 && argc!=1 ){
    generateError(pContext, "dump_terms", "incorrect arguments");
  }else if( sqlite3_value_type(argv[0])!=SQLITE_BLOB ||
            sqlite3_value_bytes(argv[0])!=sizeof(pCursor) ){
    generateError(pContext, "dump_terms", "illegal first argument");
  }else{
    fulltext_vtab *v;
    fts2Hash terms;
    sqlite3_stmt *s = NULL;
    int rc;

    memcpy(&pCursor, sqlite3_value_blob(argv[0]), sizeof(pCursor));
    v = cursor_vtab(pCursor);

    /* If passed only the cursor column, get all segments.  Otherwise
    ** get the segment described by the following two arguments.
    */
    if( argc==1 ){
      rc = sql_get_statement(v, SEGDIR_SELECT_ALL_STMT, &s);
    }else{
      rc = sql_get_statement(v, SEGDIR_SELECT_SEGMENT_STMT, &s);
      if( rc==SQLITE_OK ){
        rc = sqlite3_bind_int(s, 1, sqlite3_value_int(argv[1]));
        if( rc==SQLITE_OK ){
          rc = sqlite3_bind_int(s, 2, sqlite3_value_int(argv[2]));
        }
      }
    }

    if( rc!=SQLITE_OK ){
      generateError(pContext, "dump_terms", NULL);
      return;
    }

    /* Collect the terms for each segment. */
    sqlite3Fts2HashInit(&terms, FTS2_HASH_STRING, 1);
    while( (rc = sqlite3_step(s))==SQLITE_ROW ){
      rc = collectSegmentTerms(v, s, &terms);
      if( rc!=SQLITE_OK ) break;
    }

    if( rc!=SQLITE_DONE ){
      sqlite3_reset(s);
      generateError(pContext, "dump_terms", NULL);
    }else{
      const int nTerms = fts2HashCount(&terms);
      if( nTerms>0 ){
        rc = generateTermsResult(pContext, &terms);
        if( rc==SQLITE_NOMEM ){
          generateError(pContext, "dump_terms", "out of memory");
        }else{
          assert( rc==SQLITE_OK );
        }
      }else if( argc==3 ){
        /* The specific segment asked for could not be found. */
        generateError(pContext, "dump_terms", "segment not found");
      }else{
        /* No segments found. */
        /* TODO(shess): It should be impossible to reach this.  This
        ** case can only happen for an empty table, in which case
        ** SQLite has no rows to call this function on.
        */
        sqlite3_result_null(pContext);
      }
    }
    sqlite3Fts2HashClear(&terms);
  }
}

/* Expand the DL_DEFAULT doclist in pData into a text result in
** pContext.
*/
static void createDoclistResult(sqlite3_context *pContext,
                                const char *pData, int nData){
  DataBuffer dump;
  DLReader dlReader;

  assert( pData!=NULL && nData>0 );

  dataBufferInit(&dump, 0);
  dlrInit(&dlReader, DL_DEFAULT, pData, nData);
  for( ; !dlrAtEnd(&dlReader); dlrStep(&dlReader) ){
    char buf[256];
    PLReader plReader;

    plrInit(&plReader, &dlReader);
    if( DL_DEFAULT==DL_DOCIDS || plrAtEnd(&plReader) ){
      sqlite3_snprintf(sizeof(buf), buf, "[%lld] ", dlrDocid(&dlReader));
      dataBufferAppend(&dump, buf, strlen(buf));
    }else{
      int iColumn = plrColumn(&plReader);

      sqlite3_snprintf(sizeof(buf), buf, "[%lld %d[",
                       dlrDocid(&dlReader), iColumn);
      dataBufferAppend(&dump, buf, strlen(buf));

      for( ; !plrAtEnd(&plReader); plrStep(&plReader) ){
        if( plrColumn(&plReader)!=iColumn ){
          iColumn = plrColumn(&plReader);
          sqlite3_snprintf(sizeof(buf), buf, "] %d[", iColumn);
          assert( dump.nData>0 );
          dump.nData--;                     /* Overwrite trailing space. */
          assert( dump.pData[dump.nData]==' ');
          dataBufferAppend(&dump, buf, strlen(buf));
        }
        if( DL_DEFAULT==DL_POSITIONS_OFFSETS ){
          sqlite3_snprintf(sizeof(buf), buf, "%d,%d,%d ",
                           plrPosition(&plReader),
                           plrStartOffset(&plReader), plrEndOffset(&plReader));
        }else if( DL_DEFAULT==DL_POSITIONS ){
          sqlite3_snprintf(sizeof(buf), buf, "%d ", plrPosition(&plReader));
        }else{
          assert( NULL=="Unhandled DL_DEFAULT value");
        }
        dataBufferAppend(&dump, buf, strlen(buf));
      }
      plrDestroy(&plReader);

      assert( dump.nData>0 );
      dump.nData--;                     /* Overwrite trailing space. */
      assert( dump.pData[dump.nData]==' ');
      dataBufferAppend(&dump, "]] ", 3);
    }
  }
  dlrDestroy(&dlReader);

  assert( dump.nData>0 );
  dump.nData--;                     /* Overwrite trailing space. */
  assert( dump.pData[dump.nData]==' ');
  dump.pData[dump.nData] = '\0';
  assert( dump.nData>0 );

  /* Passes ownership of dump's buffer to pContext. */
  sqlite3_result_text(pContext, dump.pData, dump.nData, sqlite3_free);
  dump.pData = NULL;
  dump.nData = dump.nCapacity = 0;
}

/* Implements dump_doclist() for use in inspecting the fts2 index from
** tests.  TEXT result containing a string representation of the
** doclist for the indicated term.  dump_doclist(t, term, level, idx)
** dumps the doclist for term from the segment specified by level, idx
** (in %_segdir), while dump_doclist(t, term) dumps the logical
** doclist for the term across all segments.  The per-segment doclist
** can contain deletions, while the full-index doclist will not
** (deletions are omitted).
**
** Result formats differ with the setting of DL_DEFAULTS.  Examples:
**
** DL_DOCIDS: [1] [3] [7]
** DL_POSITIONS: [1 0[0 4] 1[17]] [3 1[5]]
** DL_POSITIONS_OFFSETS: [1 0[0,0,3 4,23,26] 1[17,102,105]] [3 1[5,20,23]]
**
** In each case the number after the outer '[' is the docid.  In the
** latter two cases, the number before the inner '[' is the column
** associated with the values within.  For DL_POSITIONS the numbers
** within are the positions, for DL_POSITIONS_OFFSETS they are the
** position, the start offset, and the end offset.
*/
static void dumpDoclistFunc(
  sqlite3_context *pContext,
  int argc, sqlite3_value **argv
){
  fulltext_cursor *pCursor;
  if( argc!=2 && argc!=4 ){
    generateError(pContext, "dump_doclist", "incorrect arguments");
  }else if( sqlite3_value_type(argv[0])!=SQLITE_BLOB ||
            sqlite3_value_bytes(argv[0])!=sizeof(pCursor) ){
    generateError(pContext, "dump_doclist", "illegal first argument");
  }else if( sqlite3_value_text(argv[1])==NULL ||
            sqlite3_value_text(argv[1])[0]=='\0' ){
    generateError(pContext, "dump_doclist", "empty second argument");
  }else{
    const char *pTerm = (const char *)sqlite3_value_text(argv[1]);
    const int nTerm = strlen(pTerm);
    fulltext_vtab *v;
    int rc;
    DataBuffer doclist;

    memcpy(&pCursor, sqlite3_value_blob(argv[0]), sizeof(pCursor));
    v = cursor_vtab(pCursor);

    dataBufferInit(&doclist, 0);

    /* termSelect() yields the same logical doclist that queries are
    ** run against.
    */
    if( argc==2 ){
      rc = termSelect(v, v->nColumn, pTerm, nTerm, 0, DL_DEFAULT, &doclist);
    }else{
      sqlite3_stmt *s = NULL;

      /* Get our specific segment's information. */
      rc = sql_get_statement(v, SEGDIR_SELECT_SEGMENT_STMT, &s);
      if( rc==SQLITE_OK ){
        rc = sqlite3_bind_int(s, 1, sqlite3_value_int(argv[2]));
        if( rc==SQLITE_OK ){
          rc = sqlite3_bind_int(s, 2, sqlite3_value_int(argv[3]));
        }
      }

      if( rc==SQLITE_OK ){
        rc = sqlite3_step(s);

        if( rc==SQLITE_DONE ){
          dataBufferDestroy(&doclist);
          generateError(pContext, "dump_doclist", "segment not found");
          return;
        }

        /* Found a segment, load it into doclist. */
        if( rc==SQLITE_ROW ){
          const sqlite_int64 iLeavesEnd = sqlite3_column_int64(s, 1);
          const char *pData = sqlite3_column_blob(s, 2);
          const int nData = sqlite3_column_bytes(s, 2);

          /* loadSegment() is used by termSelect() to load each
          ** segment's data.
          */
          rc = loadSegment(v, pData, nData, iLeavesEnd, pTerm, nTerm, 0,
                           &doclist);
          if( rc==SQLITE_OK ){
            rc = sqlite3_step(s);

            /* Should not have more than one matching segment. */
            if( rc!=SQLITE_DONE ){
              sqlite3_reset(s);
              dataBufferDestroy(&doclist);
              generateError(pContext, "dump_doclist", "invalid segdir");
              return;
            }
            rc = SQLITE_OK;
          }
        }
      }

      sqlite3_reset(s);
    }

    if( rc==SQLITE_OK ){
      if( doclist.nData>0 ){
        createDoclistResult(pContext, doclist.pData, doclist.nData);
      }else{
        /* TODO(shess): This can happen if the term is not present, or
        ** if all instances of the term have been deleted and this is
        ** an all-index dump.  It may be interesting to distinguish
        ** these cases.
        */
        sqlite3_result_text(pContext, "", 0, SQLITE_STATIC);
      }
    }else if( rc==SQLITE_NOMEM ){
      /* Handle out-of-memory cases specially because if they are
      ** generated in fts2 code they may not be reflected in the db
      ** handle.
      */
      /* TODO(shess): Handle this more comprehensively.
      ** sqlite3ErrStr() has what I need, but is internal.
      */
      generateError(pContext, "dump_doclist", "out of memory");
    }else{
      generateError(pContext, "dump_doclist", NULL);
    }

    dataBufferDestroy(&doclist);
  }
}
#endif

/*
** This routine implements the xFindFunction method for the FTS2
** virtual table.
*/
static int fulltextFindFunction(
  sqlite3_vtab *pVtab,
  int nArg,
  const char *zName,
  void (**pxFunc)(sqlite3_context*,int,sqlite3_value**),
  void **ppArg
){
  if( strcmp(zName,"snippet")==0 ){
    *pxFunc = snippetFunc;
    return 1;
  }else if( strcmp(zName,"offsets")==0 ){
    *pxFunc = snippetOffsetsFunc;
    return 1;
  }else if( strcmp(zName,"optimize")==0 ){
    *pxFunc = optimizeFunc;
    return 1;
#ifdef SQLITE_TEST
    /* NOTE(shess): These functions are present only for testing
    ** purposes.  No particular effort is made to optimize their
    ** execution or how they build their results.
    */
  }else if( strcmp(zName,"dump_terms")==0 ){
    /* fprintf(stderr, "Found dump_terms\n"); */
    *pxFunc = dumpTermsFunc;
    return 1;
  }else if( strcmp(zName,"dump_doclist")==0 ){
    /* fprintf(stderr, "Found dump_doclist\n"); */
    *pxFunc = dumpDoclistFunc;
    return 1;
#endif
  }
  return 0;
}

#if GEARS_FTS2_CHANGES
  /* xRename not supported here, yet. */
#else
/*
** Rename an fts2 table.
*/
static int fulltextRename(
  sqlite3_vtab *pVtab,
  const char *zName
){
  fulltext_vtab *p = (fulltext_vtab *)pVtab;
  int rc = SQLITE_NOMEM;
  char *zSql = sqlite3_mprintf(
    "ALTER TABLE %Q.'%q_content'  RENAME TO '%q_content';"
    "ALTER TABLE %Q.'%q_segments' RENAME TO '%q_segments';"
    "ALTER TABLE %Q.'%q_segdir'   RENAME TO '%q_segdir';"
    , p->zDb, p->zName, zName 
    , p->zDb, p->zName, zName 
    , p->zDb, p->zName, zName
  );
  if( zSql ){
    rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }
  return rc;
}
#endif

static const sqlite3_module fts2Module = {
  /* iVersion      */ 0,
  /* xCreate       */ fulltextCreate,
  /* xConnect      */ fulltextConnect,
  /* xBestIndex    */ fulltextBestIndex,
  /* xDisconnect   */ fulltextDisconnect,
  /* xDestroy      */ fulltextDestroy,
  /* xOpen         */ fulltextOpen,
  /* xClose        */ fulltextClose,
  /* xFilter       */ fulltextFilter,
  /* xNext         */ fulltextNext,
  /* xEof          */ fulltextEof,
  /* xColumn       */ fulltextColumn,
  /* xRowid        */ fulltextRowid,
  /* xUpdate       */ fulltextUpdate,
  /* xBegin        */ fulltextBegin,
  /* xSync         */ fulltextSync,
  /* xCommit       */ fulltextCommit,
  /* xRollback     */ fulltextRollback,
  /* xFindFunction */ fulltextFindFunction,
#if GEARS_FTS2_CHANGES
  /* xRename not supported here, yet. */
#else
  /* xRename */       fulltextRename,
#endif
};

#if GEARS_FTS2_CHANGES
/* hashDestroy() not needed until we can call
** sqlite3_create_module_v2().
*/
#else
static void hashDestroy(void *p){
  fts2Hash *pHash = (fts2Hash *)p;
  sqlite3Fts2HashClear(pHash);
  sqlite3_free(pHash);
}
#endif

/*
** The fts2 built-in tokenizers - "simple" and "porter" - are implemented
** in files fts2_tokenizer1.c and fts2_porter.c respectively. The following
** two forward declarations are for functions declared in these files
** used to retrieve the respective implementations.
**
** Calling sqlite3Fts2SimpleTokenizerModule() sets the value pointed
** to by the argument to point a the "simple" tokenizer implementation.
** Function ...PorterTokenizerModule() sets *pModule to point to the
** porter tokenizer/stemmer implementation.
*/
void sqlite3Fts2SimpleTokenizerModule(sqlite3_tokenizer_module const**ppModule);
void sqlite3Fts2PorterTokenizerModule(sqlite3_tokenizer_module const**ppModule);
void sqlite3Fts2IcuTokenizerModule(sqlite3_tokenizer_module const**ppModule);

int sqlite3Fts2InitHashTable(sqlite3 *, fts2Hash *, const char *);

/*
** Initialise the fts2 extension. If this extension is built as part
** of the sqlite library, then this function is called directly by
** SQLite. If fts2 is built as a dynamically loadable extension, this
** function is called by the sqlite3_extension_init() entry point.
*/
int sqlite3Fts2Init(sqlite3 *db){
  int rc = SQLITE_OK;
#if GEARS_FTS2_CHANGES
  fts2Hash *pHash = 0;
  const sqlite3_tokenizer_module *pSimple = 0;
  const sqlite3_tokenizer_module *pPorter = 0;
  const sqlite3_tokenizer_module *pIcu = 0;

  sqlite3Fts2SimpleTokenizerModule(&pSimple);
  sqlite3Fts2PorterTokenizerModule(&pPorter);
#ifdef SQLITE_ENABLE_ICU
  sqlite3Fts2IcuTokenizerModule(&pIcu);
#endif

  /* Allocate and initialise the hash-table used to store tokenizers. */
  pHash = sqlite3_malloc(sizeof(fts2Hash));
  if( !pHash ){
    rc = SQLITE_NOMEM;
  }else{
    sqlite3Fts2HashInit(pHash, FTS2_HASH_STRING, 1);
  }

  /* Load the built-in tokenizers into the hash table */
  if( rc==SQLITE_OK ){
    if( sqlite3Fts2HashInsert(pHash, "simple", 7, (void *)pSimple)
     || sqlite3Fts2HashInsert(pHash, "porter", 7, (void *)pPorter) 
     || (pIcu && sqlite3Fts2HashInsert(pHash, "icu", 4, (void *)pIcu))
    ){
      rc = SQLITE_NOMEM;
    }
  }
#endif

  /* Create the virtual table wrapper around the hash-table and overload 
  ** the two scalar functions. If this is successful, register the
  ** module with sqlite.
  */
  if( SQLITE_OK==rc 
#if GEARS_FTS2_CHANGES
      /* Not for Gears. */
#else
   && SQLITE_OK==(rc = sqlite3Fts2InitHashTable(db, pHash, "fts2_tokenizer"))
#endif
   && SQLITE_OK==(rc = sqlite3_overload_function(db, "snippet", -1))
   && SQLITE_OK==(rc = sqlite3_overload_function(db, "offsets", -1))
   && SQLITE_OK==(rc = sqlite3_overload_function(db, "optimize", -1))
#ifdef SQLITE_TEST
   && SQLITE_OK==(rc = sqlite3_overload_function(db, "dump_terms", -1))
   && SQLITE_OK==(rc = sqlite3_overload_function(db, "dump_doclist", -1))
#endif
  ){
#if GEARS_FTS2_CHANGES
    return sqlite3_create_module(db, "fts2", &fts2Module, NULL);
#else
    return sqlite3_create_module_v2(
        db, "fts2", &fts2Module, (void *)pHash, hashDestroy
    );
#endif
  }

  /* An error has occured. Delete the hash table and return the error code. */
  assert( rc!=SQLITE_OK );
  if( pHash ){
    sqlite3Fts2HashClear(pHash);
    sqlite3_free(pHash);
  }
  return rc;
}

#if !SQLITE_CORE
int sqlite3_extension_init(
  sqlite3 *db, 
  char **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi)
  return sqlite3Fts2Init(db);
}
#endif

#endif /* !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS2) */
