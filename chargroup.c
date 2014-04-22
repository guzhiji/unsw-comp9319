
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

typedef _chargroup_list chargroup_list;
struct {
    chargroup_listinfo * info;
    chargroup * groups;
    chargroup_list * next;
} _chargroup_list;


typedef struct {
    unsigned int char_num; // distinct char number
    character char_table[256];
    character * char_hash[256]; // ref to char_table elements
    unsigned long chargroup_num; // up to CHARGROUP_NUM_THRESHOLD
    unsigned int chargroup_list_num; // number of lists loaded
    chargroup_list chargroup_list_table[256];
    chargroup_list * chargroup_list_hash[256];
    // FILE * fp;
} bwttext;

void chargroup_list_write(chargroup_list * cgl, FILE * fp) {
    chargroup_list * cur;

    cur = cgl;
    while (cur != NULL) {
        fwrite(cur->info, sizeof(chargroup_listinfo), 1, fp);
        fwrite(cur->groups, sizeof(chargroup), cur->info->length, fp);
        cur = cur->next;
    }
}

chargroup_list * chargroup_list_read(FILE * fp, unsigned char c) {

}

unsigned int chargroup_list_size(chargroup_list * l) {
}

unsigned int chargroup_list_free(chargroup_list * l) {
    // never release the head, it's in bwttext
    // return total number of groups
}

chargroup_list * chargroup_list_get(bwttext * t, FILE * fp, unsigned char c) {
    unsigned int groupsize;
    chargroup_list * l = t->chargroup_list_hash[(unsigned int) c];
    if (l == NULL || l->info == NULL) { // not loaded yet
        while (t->chargroup_num >= CHARGROUP_NUM_THRESHOLD) {
            // release the bottom one
            t->chargroup_num -= chargroup_list_free(&t->chargroup_list_table[--t->chargroup_list_num]);
        }
        // read the chargroup list for c from index file
        l = chargroup_list_read(fp, c);
        t->chargroup_num = chargroup_list_size(l);
    }
}

void chargroup_list_add(bwttext * t, chargroup_list * l) {

}

void chargroup_write(chargroup * cg, char c) {
    if (cg->c == c) {
        cg->data.size++;
    } else {
        // write cg->data
        cg->data.start = cg->data.size;
        cg->data.size = 1;
        cg->c = c;
    }
}

unsigned long occ(bwttext * t, unsigned char c, unsigned long pos) {

}

