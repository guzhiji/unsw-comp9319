
#include "lzw.h"

// lzw_string: linked-list-based string

/**
 * initialize a string
 */
lzw_string * lzw_string_init() {

    lzw_string * s = (lzw_string *) malloc(sizeof(lzw_string));
    s->head = NULL;
    s->tail = NULL;
    s->length = 0;
    s->hash_code = 0;
    return s;

}

/**
 * hash a string
 * the algorithm is based on Java's String hashCode() implementation
 */
int lzw_string_hash(lzw_string * s) {

    int h = 0;

    lzw_char * cc = s->head;
    while (cc != NULL) {
        h = 31 * h + cc->data;
        cc = cc->next;
    }

    return h;

}

/**
 * re-hash a string and a potential appending char
 */
int lzw_string_hashnew(lzw_string * s, int c) {

    // return lzw_string_hash(s) * 31 + c;
    return s->hash_code * 31 + c;

}

/**
 * append a char to a string with a pre-determined hash code
 */
void _lzw_string_append(lzw_string * s, int c, int h) {

    // create a char node
    lzw_char * nc = (lzw_char *) malloc(sizeof(lzw_char));
    nc->data = c;
    nc->next = NULL;

    // append the char node
    if (s->tail == NULL) {
        s->head = s->tail = nc;
    } else {
        s->tail->next = nc;
        s->tail = nc;
    }

    // update other info
    s->length++;
    s->hash_code = h;

}

/**
 * append a char to a string
 */
int lzw_string_append(lzw_string * s, int c) {

    int h = lzw_string_hashnew(s, c);
    _lzw_string_append(s, c, h);

    return h;

}

void lzw_string_print(lzw_string * s, FILE * fp) {

    lzw_char * cc = s->head;
    while (cc != NULL) {
        fputc(cc->data, fp);
        cc = cc->next;
    }

}

/**
 * release memory for a string
 */
void lzw_string_free(lzw_string * s) {

    // free chars
    lzw_char * t;
    lzw_char * cc = s->head;
    while (cc != NULL) {
        t = cc;
        cc = cc->next;
        free(t);
    }

    // free string itself
    free(s);

}

void lzw_compress(FILE * fin, FILE * fout, unsigned short w) {

    int c;
    int next_code = 256;
    int prev_code = 0;
    lzw_string * buf = lzw_string_init();
    hashtable * ht = hashtable_init();

    while ((c = fgetc(fin)) != EOF) {

        // test if the new string is known
        int h = lzw_string_hashnew(buf, c);
        int * cp = hashtable_get(ht, h);
        if (cp != NULL) { // known

            prev_code = *cp;
            _lzw_string_append(buf, c, h); // since h is already known

        } else { // unknown

            if (buf->length) {
                // buf is not empty
                // which means that buf + c is a string
                // not a single char

                int code = next_code++;
                // save the hashed new string with the next code
                hashtable_put(ht, h, &code);

                // reset the string
                lzw_string_free(buf);
                buf = lzw_string_init();

            }

            if (prev_code) {
                // for the first char,
                // there not previous one

                // output code for the old string or char
                // TODO: in symbol.c
                fputc(prev_code, fout);

            }

            // use the first char value as its code
            prev_code = c;
            lzw_string_append(buf, c);
            // since buf is cleared,
            // h is no longer valid

        }

    }

    hashtable_free(ht);
    lzw_string_free(buf);

}

int lzw_csize(FILE * fp, unsigned short w) {

    int c;
    int s = 0; // size in bits
    lzw_string * buf = lzw_string_init();
    hashtable * ht = hashtable_init();

    while ((c = fgetc(fp)) != EOF) {

        // test if the new string is known
        int h = lzw_string_hashnew(buf, c);
        if (hashtable_get(ht, h) == NULL) { // unknown

            if (buf->length) {
                // buf + c is not a single char

                // save the hashed new string
                hashtable_put(ht, h, &c);

                // reset the string
                lzw_string_free(buf);
                buf = lzw_string_init();
            }

            // instead of outputing a symbol
            // s counts bits
            // NOTE:
            // when the first char comes,
            // nothing is in buf to be added here
            // but, for the last char, EOF pre-maturely 
            // stops the loop before adding the last code
            // in all, just add w here though it doesn't 
            // logically make sense
            s += w;
            lzw_string_append(buf, c);
            // since buf is cleared, 
            // h is no longer valid

        } else {
            // append the new char to string
            _lzw_string_append(buf, c, h);
        }

    }

    hashtable_free(ht);
    lzw_string_free(buf);

    if (s % 8 > 0)
        return s / 8 + 1;
    return s / 8;

}
