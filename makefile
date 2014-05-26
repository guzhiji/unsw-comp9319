PSEARCH_FILES=bwtsearch.c bwtsearch.h bwttext.c bwttext.h chartable.c chartable.h common.h main.c occtable.c occtable.h plset.c plset.h strbuf.c strbuf.h

all:
	gcc -o psearch *.h *.c

clean:
	rm psearch

psearch: ${PSEARCH_FILES}
	gcc -o psearch ${PSEARCH_FILES}


