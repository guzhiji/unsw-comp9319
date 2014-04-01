
#include "lzw.h"

lzw_string * lzw_string_init() {

    lzw_string * s = (lzw_string *) malloc(sizeof(lzw_string));
    //s->head = NULL;
    //s->tail = NULL;
    s->data = (char *) malloc(sizeof(char) * _LZW_STRING_INC);
    s->data[0] = '\0';
    s->max_length = _LZW_STRING_INC;
    s->length = 0;
    s->hash_code = 0;
    return s;

}

int lzw_string_hash(lzw_string * s) {

    int h = 0;
/*
    lzw_char * cc = s->head;
    while (cc != NULL) {
        h = 31 * h + cc->data;
        cc = cc->next;
    }
*/
    int l = 0;
    char * c = s->data;
    while (l++ < s->length) {
        // h = 31 * h + (int)*c;
        h = 7 * h + (int)*c;
        //h ^= (int)*c;
        c++;
    }

    return h;

}

int lzw_string_hashnew(lzw_string * s, int c) {

    // return lzw_string_hash(s) * 31 + c;
    // return s->hash_code * 31 + c;
    return s->hash_code * 7 + c;
    //return s->hash_code ^ c;

}

/**
 * append a char to a string with a pre-determined hash code
 *
 * @param lzw_string * s    string to which the character is appended
 * @param int c             the new character
 * @param int h             the pre-determined hash code
 */
void _lzw_string_append(lzw_string * s, int c, int h) {

    if (s->length + 1 == s->max_length) {
        char * t = s->data;
        s->max_length += _LZW_STRING_INC;
        s->data = (char *) malloc(sizeof(char) * s->max_length);
        strncpy(s->data, t, s->length);
        free(t);
    }
    s->data[s->length] = (char) c;
    s->data[++s->length] = '\0';
    s->hash_code = h;

/*
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
*/
}

int lzw_string_append(lzw_string * s, int c) {

    int h = lzw_string_hashnew(s, c); // hash before append
    _lzw_string_append(s, c, h); // append using a known hash

    return h;

}

int lzw_string_equals(lzw_string * s1, lzw_string * s2) {

    if (s1->hash_code != s2->hash_code)
        return 0;

    if (s1->length != s2->length)
        return 0;

    return 0 == strncmp(s1->data, s2->data, s1->length);

/*
    lzw_char * c1 = s1->head;
    lzw_char * c2 = s2->head;
    while (c1 != NULL && c2 != NULL) {
        if (c1->data != c2->data) return 0;
        c1 = c1->next;
        c2 = c2->next;
    }
    if (c1 == NULL && c2 == NULL) return 1;
    return 0;
*/
}

void lzw_string_print(lzw_string * s, FILE * fp) {
/*
    lzw_char * cc = s->head;
    while (cc != NULL) {
        fputc(cc->data, fp);
        //fprintf(fp, "%d", cc->data);
        cc = cc->next;
    }
*/
    fputs(s->data, fp);
}

void lzw_string_free(lzw_string * s) {

    // free chars
/*
    lzw_char * t;
    lzw_char * cc = s->head;
    while (cc != NULL) {
        t = cc;
        cc = cc->next;
        free(t);
    }
*/
    free(s->data);
    // free string itself
    free(s);

}

/**
 * custom function for hashing a LZW string
 *
 * @param lzw_string *      pointer to a string key
 * @return unsigned int     hash code of the string
 */
unsigned int hashtable_hash_lzwstring(void * k) {
    return ((lzw_string *) k)->hash_code;
}

/**
 * custom function for comparing two LZW strings
 *
 * @param lzw_string *      pointer to a string key
 * @param lzw_string *      pointer to a string key
 * @return int              1 for equal; 0 for inequal
 */
int hashtable_comp_lzwstring(void * k1, void * k2) {
    return lzw_string_equals((lzw_string *) k1, (lzw_string *) k2);
}

/**
 * custom function for releasing a LZW string key and an int value
 *
 * @param lzw_string *      pointer to a string key
 * @param unsigned int *    pointer to an unsigned integer as data
 */
void hashtable_free_lzwstringint(void * key, void * data) {
    lzw_string_free(key);
    free(data);
}

/**
 * custom function for releasing a key-value pair of LZW strings
 *
 * @param lzw_string *      pointer to a string key
 * @param lzw_string *      pointer to a string as data
 */
void hashtable_free_lzwstring(void * key, void * data) {

    // key and data may point to the same
    // block of memory
    if (key != data)
        lzw_string_free(data);
    lzw_string_free(key);

}

void lzw_compress(FILE * fin, FILE * fout, unsigned short w) {

    int c, h;
    int * cp;
    unsigned int next_code = 256;
    unsigned int prev_code = 0;
    lzw_string * buf = lzw_string_init();
    hashtable * ht = hashtable_init(5021);

    hashtable_setcompfunc(ht, hashtable_comp_lzwstring);
    hashtable_sethashfunc(ht, hashtable_hash_lzwstring);
    hashtable_setfreefunc(ht, hashtable_free_lzwstringint);

    while ((c = fgetc(fin)) != EOF) {

        h = lzw_string_append(buf, c);
        cp = (int *) hashtable_get(ht, &h);
        if (cp != NULL) { // known

            prev_code = *cp;

        } else { // the newer string is unknown

            // output its known string before the newer
            if (prev_code)
                putsymbol(fout, prev_code, w);

            // save the newer unknown string
            if (buf->length > 1) {
                // make sure it's a string, not a single char
                unsigned int * code = (unsigned int *) malloc(sizeof(unsigned int));
                *code = next_code++;
                hashtable_put(ht, buf, code);
                // buf is stored and its resource is managed by hashtable

                // reset buf to the newest char
                buf = lzw_string_init();
                lzw_string_append(buf, c);
            } // for buf->length == 1, the newest char is already in buf

            prev_code = c;
        }

    }

    // flush
    if (prev_code)
        putsymbol(fout, EOF, w);

    hashtable_free(ht);
    lzw_string_free(buf);

}

int lzw_csize(FILE * fp, unsigned short w) {

    int c;
    int s = 0; // size in bits
    hashtable * ht = hashtable_init(5021);
    lzw_string * buf = lzw_string_init();
    lzw_string * hts;

    // setup hashtable custom functions
    hashtable_setcompfunc(ht, hashtable_comp_lzwstring);
    hashtable_sethashfunc(ht, hashtable_hash_lzwstring);
    hashtable_setfreefunc(ht, hashtable_free_lzwstring);

    while ((c = fgetc(fp)) != EOF) {

        lzw_string_append(buf, c);
        hts = hashtable_get(ht, buf);
        // hashtable has been upgraded and there is no need to call equals() here
        if (hts == NULL /* || !lzw_string_equals(buf, hts) */) { // unknown

            if (buf->length > 1) {

                // buf + c is not a single char
                // save the hashed new string
                hashtable_put(ht, buf, buf);

            }

            if (buf->length > 0) {

                // reset the string
                // release buf when it's not stored in hashtable
                if (buf->length == 1)
                    lzw_string_free(buf);
                buf = lzw_string_init();

            }
            // otherwise
            // buf->length == 0
            // there is no need to reset the string/buf

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
            lzw_string_append(buf, c); // since buf's been cleared

        }

    }

    lzw_string_free(buf);
    hashtable_free(ht);

    // bits to bytes
    if (s % 8 > 0)
        return s / 8 + 1;
    return s / 8;

}
