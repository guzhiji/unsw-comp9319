
#include "symbol.h"

int utf8_size(int h) {

    // 11110xxx
    // 1e
    h >>= 3;
    if (h == 30) return 4;
    // 1110xxxx
    // e
    h >>= 1;
    if (h == 14) return 3;
    // 110xxxxx
    // 6
    h >>= 1;
    if (h == 6) return 2;

    // 0xxxxxxx
    h >>= 2;
    if (h == 0) return 1;

    // error
    return 0;

}

/**
 * a symbol is not necessarily an UTF-8 character,
 * but it can represent an UTF-8 character.
 */
int _getsymbol(FILE * fp) {

    int c, l;

    c = fgetc(fp);
    l = utf8_size(c);

    // ASCII
    if (l == 1) return c;

    // multi-byte
    if (l > 1) {

        int i, t;

        for (i = 1; i < l; i++) {
            t = fgetc(fp);
            if (t == EOF) return EOF;
            c <<= 8;
            c += t;
        }

        return c;
    }

    // error
    return 0;

}

/**
 * n=7 - xXXXXXXX
 * n=6 - xxXXXXXX
 * ...
 * n=1 - xxxxxxxX
 */
int _extractbits(int part, int n) {

    int mask = 127 >> (7 - n);// 0111 1111
    return part & mask;

}

int utf8_char(FILE * fp) {

    int len;
    int head = fgetc(fp);//printf("\ndebug: head=%d\n", head);
    if (head == EOF) return EOF;

    len = utf8_size(head);

    // ASCII
    if (len == 1) return head;

    // multi-byte
    if (len > 1) {

        int i;
        int p;

        int s = _extractbits(head, 7 - len);

        for (i = 1; i < len; i++) {
            s <<= 6;
            p = fgetc(fp);
            if (p == EOF) return EOF;
            s += _extractbits(p, 6);
        }

        return s;

    }

    // error
    return 0;
}

/**
 * get a symbol of specified width (in bits).
 */
int getsymbol(FILE * fp, unsigned short w) {

    static unsigned short bits_read = 0;
    static int symbol_buffer = 0;

    int c;
    unsigned int s = symbol_buffer;

    while (bits_read < w) {
        c = fgetc(fp); // a char of 8 bits
        if (c == EOF) {
            // reach the end before reaching the required width
            // reset
            bits_read = 0;
            symbol_buffer = 0;

            if (s == 0) return EOF;
            return s;
        }
        // accumulate bits
        s = (s << 8) | c;
        bits_read += 8;
    }

    bits_read -= w;
    symbol_buffer = ((1 << bits_read) - 1) & c;

    return s >> bits_read;

}

void putsymbol(FILE * fp, int c, unsigned short w) {

    static int bits_remaining = 0;
    static int symbol_buffer = 0;
    if (c != EOF) {
        // accept a new symbol of width w
        symbol_buffer = (symbol_buffer << w) | c;
        bits_remaining += w;
    } else if (bits_remaining > 0) {
        // flush buffer, which was fewer than 8 bits
        bits_remaining = 8;
    }
    while (bits_remaining >= 8) {
        fputc(symbol_buffer, fp); // write a byte
        symbol_buffer >>= 8; // drop the byte
        bits_remaining -= 8;
        // continue when there're more bytes
    }

}
