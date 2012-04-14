/*
** 2006 July 10
**
** The author disclaims copyright to this source code.
**
*************************************************************************
** Defines the interface to tokenizers used by fulltext-search.  There
** are three basic components:
**
** sqlite3_tokenizer_module is a singleton defining the tokenizer
** interface functions.  This is essentially the class structure for
** tokenizers.
**
** sqlite3_tokenizer is used to define a particular tokenizer, perhaps
** including customization information defined at creation time.
**
** sqlite3_tokenizer_cursor is generated by a tokenizer to generate
** tokens from a particular input.
*/
#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

/* TODO(shess) Only used for SQLITE_OK and SQLITE_DONE at this time.
** If tokenizers are to be allowed to call sqlite3_*() functions, then
** we will need a way to register the API consistently.
*/
#include "sqlite3.h"

/*
** Structures used by the tokenizer interface.
*/
typedef struct sqlite3_tokenizer sqlite3_tokenizer;
typedef struct sqlite3_tokenizer_cursor sqlite3_tokenizer_cursor;
typedef struct sqlite3_tokenizer_module sqlite3_tokenizer_module;

struct sqlite3_tokenizer_module {
  int iVersion;                  /* currently 0 */

  /*
  ** Create and destroy a tokenizer.  argc/argv are passed down from
  ** the fulltext virtual table creation to allow customization.
  */
  int (*xCreate)(int argc, const char **argv,
                 sqlite3_tokenizer **ppTokenizer);
  int (*xDestroy)(sqlite3_tokenizer *pTokenizer);

  /*
  ** Tokenize a particular input.  Call xOpen() to prepare to
  ** tokenize, xNext() repeatedly until it returns SQLITE_DONE, then
  ** xClose() to free any internal state.  The pInput passed to
  ** xOpen() must exist until the cursor is closed.  The ppToken
  ** result from xNext() is only valid until the next call to xNext()
  ** or until xClose() is called.
  */
  /* TODO(shess) current implementation requires pInput to be
  ** nul-terminated.  This should either be fixed, or pInput/nBytes
  ** should be converted to zInput.
  */
  int (*xOpen)(sqlite3_tokenizer *pTokenizer,
               const char *pInput, int nBytes,
               sqlite3_tokenizer_cursor **ppCursor);
  int (*xClose)(sqlite3_tokenizer_cursor *pCursor);
  int (*xNext)(sqlite3_tokenizer_cursor *pCursor,
               const char **ppToken, int *pnBytes,
               int *piStartOffset, int *piEndOffset, int *piPosition);
};

struct sqlite3_tokenizer {
  sqlite3_tokenizer_module *pModule;  /* The module for this tokenizer */
  /* Tokenizer implementations will typically add additional fields */
};

struct sqlite3_tokenizer_cursor {
  sqlite3_tokenizer *pTokenizer;       /* Tokenizer for this cursor. */
  /* Tokenizer implementations will typically add additional fields */
};

/*
** Get the module for a tokenizer which generates tokens based on a
** set of non-token characters.  The default is to break tokens at any
** non-alnum character, though the set of delimiters can also be
** specified by the first argv argument to xCreate().
*/
/* TODO(shess) This doesn't belong here.  Need some sort of
** registration process.
*/
void get_simple_tokenizer_module(sqlite3_tokenizer_module **ppModule);

#endif /* _TOKENIZER_H_ */
