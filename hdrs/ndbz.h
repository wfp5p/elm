
/* $Id: ndbz.h,v 1.2 1995/09/29 17:40:52 wfp5p Exp $ */

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.2 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 * 			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: ndbz.h,v $
 * Revision 1.2  1995/09/29  17:40:52  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:30  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/**  define file for ndbz for mail system.  **/

/*
 * Stdio buffer for .pag reads.  Buffering more than about 16 does not help
 * significantly at the densities we try to maintain, and the much larger
 * buffers that most stdios default to are much more expensive to fill.
 * With small buffers, stdio is performance-competitive with raw read(),
 * and it's much more portable.
 */
#ifndef NPAGBUF
#define	NPAGBUF	16
#endif

/*
 * Stdio buffer for base-file reads.
 */
#ifndef SHISTBUF
#define	SHISTBUF	512
#endif

/* for dbz and ndbz */
typedef struct {
	char *dptr;
	size_t dsize;
} datum;

/*
 * ANSI C says an offset into a file is a long, not an off_t, for some
 * reason.  This actually does simplify life a bit, but it's still nice
 * to have a distinctive name for it.  Beware, this is just for readability,
 * don't try to change this.
 */

/*
 * Big kludge: this is set up as 32-bit rather than a long so that ndbz db's 
 * will work across NFS on 64 bit machines as well as 32 bit machines.
 */

#define	of_t	int32
#define	SOF	(sizeof(of_t))

/*
 * We read configuration info from the .dir file into this structure,
 * so we can avoid wired-in assumptions for an existing database.
 *
 * Among the info is a record of recent peak usages, so that a new table
 * size can be chosen intelligently when rebuilding.  10 is a good
 * number of usages to keep, since news displays marked fluctuations
 * in volume on a 7-day cycle.
 */
struct dbzconfig {
	int olddbz;		/* .dir file empty but .pag not? */
	of_t tsize;		/* table size */
#	ifndef NMEMORY
#	define	NMEMORY	10	/* # days of use info to remember */
#	endif
#	define	NUSEDS	(1+NMEMORY)
	of_t used[NUSEDS];	/* entries used today, yesterday, ... */
	int valuesize;		/* size of table values, == SOF */
	int bytemap[SOF];	/* byte-order map */
	char casemap;		/* case-mapping algorithm (see cipoint()) */
	char fieldsep;		/* field separator in base file, if any */
	of_t tagenb;		/* unshifted tag-enable bit */
	of_t tagmask;		/* unshifted tag mask */
	int tagshift;		/* shift count for tagmask and tagenb */
};

/*
 * Data structure for recording info about searches.
 */
struct searcher {
	of_t place;		/* current location in file */
	int tabno;		/* which table we're in */
	int run;		/* how long we'll stay in this table */
	long hash;		/* the key's hash code (for optimization) */
	of_t tag;		/* tag we are looking for */
	int seen;		/* have we examined current location? */
	int aborted;		/* has i/o error aborted search? */
};
typedef struct dbz {
	FILE *dbz_basef;		/* descriptor for base file */
	char *dbz_basefname;		/* name for not-yet-opened base file */
	FILE *dbz_dirf;			/* descriptor for .dir file */
	int dbz_dirronly;		/* dirf open read-only? */
	FILE *dbz_pagf;			/* descriptor for .pag file */
	of_t dbz_pagpos;		/* posn in pagf; only search may set != -1 */
	int dbz_pagronly;		/* pagf open read-only? */
	of_t *dbz_corepag;		/* incore version of .pag file, if any */
	FILE *dbz_bufpagf;		/* well-buffered pagf, for incore rewrite */
	of_t dbz_tagbits;		/* pre-shifted tag mask */
	of_t dbz_taghere;		/* pre-shifted tag-enable bit */
	of_t dbz_tagboth;		/* tagbits|taghere */
	struct dbzconfig dbz_conf;
	int dbz_incore;
	of_t dbz_pagbuf[NPAGBUF];
	char dbz_basebuf[SHISTBUF];
	struct searcher dbz_srch;
	struct searcher *dbz_prevp;	/* &srch or FRESH */
	int dbz_mybmap[SOF];		/* my byte order (see mybytemap()) */
	int dbz_bytesame;		/* is database order same as mine? */
	int dbz_debug;			/* controlled by dbzdebug() */
	int dbz_written;		/* has a store() been done? */
	} DBZ;

/* FOO - should these declarations be ANSIfied? */

/* standard dbz functions */
extern DBZ *dbz_open(char *name, int mode, int flags);
extern datum dbz_fetch(DBZ *db, datum key);
extern int dbz_store(DBZ *db, datum key, datum data);
extern int dbz_close(DBZ *db);

/* new stuff for dbz */
extern DBZ *dbz_fresh(char *name, long size, int fs, int32_t tagmask);
extern DBZ *dbz_again(char *name, char *oldname);
extern int dbz_sync(DBZ *db);
extern int dbz_incore(int value);
extern int dbz_cancel(DBZ *db);

/*
 * In principle we could handle unlimited-length keys by operating a chunk
 * at a time, but it's not worth it in practice.  Setting a nice large
 * bound on them simplifies the code and doesn't hurt anything.
 */
#define DBZMAXKEY	255
