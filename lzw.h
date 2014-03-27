
#ifndef _LZW_H_
#define _LZW_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hashtable.h"
#include "symbol.h"

#ifndef _LZW_STRING_INC
#define _LZW_STRING_INC 32
#endif

/**
 * DATA TYPE: LZW string character
 */
/*
typedef struct _lzw_char lzw_char;

struct _lzw_char {
    int data;
    lzw_char * next;
};
*/

/**
 * DATA TYPE: LZW string
 */
typedef struct _lzw_string lzw_string;

struct _lzw_string {
    //lzw_char * head;
    //lzw_char * tail;
    char * data;
    int length;
    int max_length;
    int hash_code;
};

/**
 * initialize a string
 */
lzw_string * lzw_string_init();

/**
 * hash a string.
 *
 * refers to Java's String hashCode() implementation
 * @see http://docs.oracle.com/javase/7/docs/api/java/lang/String.html#hashCode()
 * @see http://hg.openjdk.java.net/jdk7u/jdk7u6/jdk/file/8c2c5d63a17e/src/share/classes/java/lang/String.java
 */
int lzw_string_hash(lzw_string * s);

/**
 * re-hash a string and a potential appending char (before appended)
 *
 * @param lzw_string * s    string
 * @param int c             a new character
 */
int lzw_string_hashnew(lzw_string * s, int c);

/**
 * append a char to a string
 *
 * @param lzw_string * s    string
 * @param int c             a new character
 */
int lzw_string_append(lzw_string * s, int c);

/**
 * determine whether two strings equal
 */
int lzw_string_equals(lzw_string * s1, lzw_string * s2);

void lzw_string_print(lzw_string * s, FILE * fp);

/**
 * release memory for a string
 */
void lzw_string_free(lzw_string * s);

void lzw_compress(FILE * fin, FILE * fout, unsigned short w);

/**
 * calculate LZW compression size in bytes
 *
 * @param FILE * fp             file pointer
 * @param unsigned short w      width of LZW output code, in bits, typically 12
 */
int lzw_csize(FILE * fp, unsigned short w);

#endif
