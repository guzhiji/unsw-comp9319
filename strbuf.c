
#include "strbuf.h"
#include <stdlib.h>
#include <stdio.h>

strbuf * strbuf_init() {
    strbuf * buf = (strbuf *) malloc(sizeof(strbuf));
    buf->tail = NULL;
    buf->head = NULL;
    return buf;
}

void strbuf_free(strbuf * buf) {
    strbuf_node * cur, * t;

    cur = buf->tail;
    while (cur != NULL) {
        t = cur;
        cur = cur->p;
        free(t);
    }
    free(buf);
}

void strbuf_putchar(strbuf * buf, unsigned char c) {

    if (buf->tail == NULL || buf->tail->l == STRBUF_LEN) {
        strbuf_node * bn = (strbuf_node *) malloc(sizeof(strbuf_node));
        bn->l = 1;
        bn->c[0] = c;
        bn->n = NULL;
        bn->p = buf->tail;
        buf->tail = bn;
        if (bn->p != NULL) bn->p->n = bn;
        if (buf->head == NULL) buf->head = bn;
    } else {
        buf->tail->c[buf->tail->l++] = c;
    }

}

void strbuf_dump_rev(strbuf * buf, FILE * fout) {
    int i;
    strbuf_node * bn = buf->tail;

    while (bn != NULL) {
        for (i = bn->l - 1; i > -1; i--)
            fputc(bn->c[i], fout);
        bn = bn->p;
    }

}

void strbuf_dump(strbuf * buf, FILE * fout) {
    int i;
    strbuf_node * bn = buf->head;

    while (bn != NULL) {
        for (i = 0; i < bn->l; i++)
            fputc(bn->c[i], fout);
        bn = bn->n;
    }

}

