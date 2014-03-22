
#include "lzw.h"

// lzw_string: linked-list-based string

lzw_string * lzw_string_init() {

    lzw_string * s = (lzw_string *) malloc(sizeof(lzw_string));
    s->head = NULL;
    s->tail = NULL;
    s->length = 0;
    s->hash_code = 0;
    return s;

}

int lzw_string_hash(lzw_string * s) {

    int h = 0;

    lzw_char * cc = s->head;
    while (cc != NULL) {
        h = 31 * h + cc->data;
        cc = cc->next;
    }

    return h;

}

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

int lzw_string_append(lzw_string * s, int c) {

    int h = lzw_string_hashnew(s, c);
    _lzw_string_append(s, c, h);

    return h;

}

int lzw_string_equals(lzw_string * s1, lzw_string * s2) {
    lzw_char * c1 = s1->head;
    lzw_char * c2 = s2->head;
    while (c1 != NULL && c2 != NULL) {
        if (c1->data != c2->data) return 0;
        c1 = c1->next;
        c2 = c2->next;
    }
    if (c1 == NULL && c2 == NULL) return 1;
    return 0;
}

void lzw_string_print(lzw_string * s, FILE * fp) {

    lzw_char * cc = s->head;
    while (cc != NULL) {
        fputc(cc->data, fp);
        //fprintf(fp, "%d", cc->data);
        cc = cc->next;
    }

}

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

unsigned int hashtable_hash_lzwstring(void * k) {
    return ((lzw_string *) k)->hash_code;
}

int hashtable_comp_lzwstring(void * k1, void * k2) {
    return lzw_string_equals((lzw_string *) k1, (lzw_string *) k2);
}

void lzw_compress(FILE * fin, FILE * fout, unsigned short w) {

    int c;
    int next_code = 256;
    int prev_code = 0;
    lzw_string * buf = lzw_string_init();
    hashtable * ht = hashtable_init();

    hashtable_setcompfunc(hashtable_comp_lzwstring);
    hashtable_sethashfunc(hashtable_hash_lzwstring);

    while ((c = fgetc(fin)) != EOF) {

        // test if the new string is known
        int h = lzw_string_hashnew(buf, c);
        int * cp = hashtable_get(ht, &h);
        if (cp != NULL) { // known

            prev_code = *cp;
            _lzw_string_append(buf, c, h); // since h is already known

        } else { // unknown

            if (buf->length) {
                int code;

                // buf is not empty
                // which means that buf + c is a string
                // not a single char

                // for the first char,
                // there not previous one

                // output code for the old string or char
                // TODO: in symbol.c
                fputc(prev_code, fout);

                // save the hashed new string with the next code
                code = next_code++;
                hashtable_put(ht, &h, &code);

                // reset the string/buf
                // it's not necessary when buf->length==0
                lzw_string_free(buf);
                buf = lzw_string_init();

            }

            // use the first char value as its code
            prev_code = c;
            lzw_string_append(buf, c);
            // since buf is cleared,
            // h is no longer valid

        }

    }

    if (prev_code) {
        // TODO: the last code for buf
        fputc(prev_code, fout);
        // TODO: flush
    }

    hashtable_free(ht);
    lzw_string_free(buf);

}

int lzw_csize(FILE * fp, unsigned short w) {

    int c;
    int s = 0; // size in bits
    hashtable * ht = hashtable_init();
    lzw_string * buf = lzw_string_init();
    lzw_string * hts;

    // setup hashtable custom functions
    hashtable_setcompfunc(hashtable_comp_lzwstring);
    hashtable_sethashfunc(hashtable_hash_lzwstring);

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
