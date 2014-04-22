
#ifndef _CHARGROUP_H_
#define _CHARGROUP_H_

#define CHARGROUP_NUM_THRESHOLD 196608

typedef struct {
    unsigned int start;
    unsigned int size;
} chargroup;

typedef struct {
    unsigned char c;
    unsigned long start;
    unsigned int length;
    unsigned long next;
} chargroup_listinfo;

typedef struct _chargroup_list chargroup_list;
struct _chargroup_list {
    chargroup_listinfo * info;
    exarray * groups; // array of chargroups
    chargroup_list * next;
    //character * cp;
    unsigned char c;
};


typedef struct {
    unsigned int char_num; // distinct char number
    character char_table[256]; // sorted by freq after loaded
    character * char_hash[256]; // ref to char_table elements

    unsigned long chargroup_num; // up to CHARGROUP_NUM_THRESHOLD
    unsigned int chargroup_list_num; // number of lists loaded
    chargroup_list * chargroup_list_sorted[256]; // by char freq, only use top chargroup_list_num
    chargroup_list * chargroup_list_hash[256];
    // FILE * fp;
} bwttext;

unsigned int chargroup_list_size(chargroup_list * l);
unsigned int chargroup_list_free(chargroup_list * l);
chargroup_list * chargroup_list_read(FILE * fp, unsigned char c);
void chargroup_list_write(chargroup_list * cgl, FILE * fp);
chargroup_list * chargroup_list_get(bwttext * t, FILE * fp, unsigned char c);
void chargroup_list_add(bwttext * t, chargroup_list * l);

#endif