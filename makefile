
BASE_FILES=bwtsearch.c bwtsearch.h bwttext.c bwttext.h chartable.c chartable.h common.h occtable.c occtable.h plset.c plset.h strbuf.c strbuf.h
BWTSEARCH_FILES=${BASE_FILES} main_bwtsearch.c
PSEARCH_FILES=${BASE_FILES} main_psearch.c
DEBUG_FILES=${BASE_FILES} main_debug.c

all:
	gcc -o psearch *.h *.c

clean:
	rm psearch bwtsearch bwt_debug

bwtsearch: ${BWTSEARCH_FILES}
	gcc -o bwtsearch ${BWTSEARCH_FILES}

psearch: ${PSEARCH_FILES}
	gcc -o psearch ${PSEARCH_FILES}

debug: ${DEBUG_FILES}
	gcc -o bwt_debug ${DEBUG_FILES}

