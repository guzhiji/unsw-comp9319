
BASE_FILES=bwtsearch.c bwtsearch.h bwttext.c bwttext.h chartable.c chartable.h common.h occtable.c occtable.h plset.c plset.h strbuf.c strbuf.h
BASE_ENCODER_FILES=bwtencoder.h bwtencoder.c
BWTSEARCH_FILES=${BASE_FILES} main_bwtsearch.c
BENCODE_FILES=${BASE_ENCODER_FILES} main_bencode.c
PSEARCH_FILES=${BASE_FILES} main_psearch.c
PENCODE_FILES=${BASE_ENCODER_FILES} main_pencode.c
DEBUG_FILES=${BASE_FILES} main_debug.c

all:
	gcc -o psearch *.h *.c

clean:
	rm psearch bwtsearch pencode bencode bwt_debug

bwtsearch: ${BWTSEARCH_FILES}
	gcc -o bwtsearch ${BWTSEARCH_FILES}

bencode: ${BENCODE_FILES}
	gcc -o bencode ${BENCODE_FILES}

psearch: ${PSEARCH_FILES}
	gcc -o psearch ${PSEARCH_FILES}

pencode: ${PENCODE_FILES}
	gcc -o pencode ${PENCODE_FILES}

debug: ${DEBUG_FILES}
	gcc -o bwt_debug ${DEBUG_FILES}

