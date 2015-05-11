

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.3 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: aliasdb.c,v $
 * Revision 1.3  1995/09/29  17:41:03  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/05/30  19:20:47  wfp5p
 * Changes for Cray from Al Anderson <arander@msc.edu>
 *
 * Revision 1.1.1.1  1995/04/19  20:38:31  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** Alias interface with dbz routines.

	This code is shared with newalias and elm so that
  it is easier to do updates while in elm.  The routines in
  this file are interface routines between elm alias code,
  newalias, and listalias and the dbm routines.

**/

#include "elm_defs.h"
#include "ndbz.h"

#ifdef BSD
#  include <sys/file.h>
#endif

/* byte-ordering stuff */
#define	MAPIN(o)	((db->dbz_bytesame) ? (of_t) (o) : bytemap((of_t)(o), db->dbz_conf.bytemap, db->dbz_mybmap))
#define	MAPOUT(o)	((db->dbz_bytesame) ? (of_t) (o) : bytemap((of_t)(o), db->dbz_mybmap, db->dbz_conf.bytemap))

/* transformed result */
static int32_t bytemap(int32_t ino, int *map1, int *map2)
{
	union oc {
		of_t o;
		char c[SOF];
	};
	union oc in;
	union oc out;
	unsigned int i;

	in.o = ino;
	for (i = 0; i < SOF; i++)
		out.c[map2[i]] = in.c[map1[i]];
	return(out.o);
}

int read_one_alias(DBZ *db, struct alias_disk_rec *adr)
{
/*
 *	Read an alias (name, address, etc.) from the data file
 */

	FILE *data_file = db->dbz_basef;

	if (data_file == NULL)
	    return(0);	/* no alias file, but hash exists, error condition */

	if (fread((char *) adr, sizeof(struct alias_disk_rec), 1, data_file)
	    <= 0)
		return(0);

	adr->status = (int32) MAPIN(adr->status);
	adr->alias = (int32) MAPIN(adr->alias);
	adr->last_name = (int32) MAPIN(adr->last_name);
	adr->name = (int32) MAPIN(adr->name);
	adr->comment = (int32) MAPIN(adr->comment);
	adr->address = (int32) MAPIN(adr->address);
	adr->type = (int32) MAPIN(adr->type);
	adr->length = (int32) MAPIN(adr->length);

	return(1);
}


/*
 * Retrieve an alias record and information from a database.
 *
 * If "alias" is non-NULL, it is the name of the alias to fetch (searching
 * is case insensitive).  If the alias is found, we return a pointer to
 * dynamically allocated memory that holds an alias record *plus* the
 * text data for that record.  If the lookup fails or an error occurs,
 * then a NULL is returned.
 *
 * If "alias" is NULL then the next alias in the database is retrieved.
 * This may be used to scan through the database.  Again, a pointer to
 * dynamically allocated memory is returned.  When the end of file is
 * reached, a NULL is returned.
 */
struct alias_rec *fetch_alias(DBZ *db, char *alias)
{
	datum key, val;
	struct alias_disk_rec adrec;
	long pos;
	struct alias_rec *ar;
	char *buf, *s;

	/*
	 * If an alias is specifed then locate it.
	 */
	if (alias != NULL) {

	    /*
	     * Fetch location of this alias.
	     */
	    key.dptr = safe_strdup(alias);
	    for (s = key.dptr ; *s != '\0' ; ++s) {
		    if (isascii(*s) && isupper(*s))
			    *s = tolower(*s);
	    }
	    key.dsize = strlen(alias);
	    val = dbz_fetch(db, key);
	    (void) free((malloc_t)key.dptr);

	    /*
	     * Make sure the alias was found.
	     */
	    if (val.dptr == NULL)
		    return (struct alias_rec *)NULL;

	    /*
	     * Sanity check - return value should be a seek offset.
	     */
	    if (val.dsize != sizeof(size_t))
		    return (struct alias_rec *)NULL;

	    /*
	     * Move to the position of the selected alias record.
	     */
	    pos = *((long *)(val.dptr)) - sizeof(struct alias_disk_rec);
	    if (fseek(db->dbz_basef, pos, SEEK_SET) != 0)
		    return (struct alias_rec *)NULL;

	}

	/*
	 * Each alias in the data file is stored as a (struct alias_rec)
	 * followed by text information for that alias record.  The
	 * size of the following text information is specified by `length',
	 * and the value of the other members of the structure are actually
	 * offsets into that buffer space.  So, to load in an alias we
	 * need to:  (1) read the alias record, (2) see how long the data
	 * buffer is and pull it in, and (3) fixup the pointers in the
	 * alias record so they point into the data buffer.
	 */

	/*
	 * We are now positioned at the alias record we want.  Pull it in.
	 */
	if (!read_one_alias(db, &adrec))
		return (struct alias_rec *)NULL;

	/*
	 * Allocate space to hold the alias record and data content.
	 */

	ar = (struct alias_rec *)
		safe_malloc(sizeof(struct alias_rec) + (size_t)adrec.length);

	buf = (char *)ar + sizeof(struct alias_rec);

        /*
	 * Fixup pointers in the alias record.
	 */
	ar->status = (int)adrec.status;
#if defined(CRAY) ||  defined(__alpha__)
        ar->alias = (size_t) adrec.alias + buf;
        ar->last_name = (size_t) adrec.last_name + buf;
        ar->name = (size_t) adrec.name + buf;
        ar->comment = (size_t) adrec.comment + buf;
        ar->address = (size_t) adrec.address + buf;
#else
	ar->alias = (char *) ((size_t) adrec.alias + (size_t) buf);
	ar->last_name = (char *) ((size_t) adrec.last_name + (size_t) buf);
	ar->name = (char *) ((size_t) adrec.name + (size_t) buf);
	ar->comment = (char *) ((size_t) adrec.comment + (size_t) buf);
	ar->address = (char *) ((size_t) adrec.address + (size_t) buf);
#endif
	ar->type = (int)adrec.type;
	ar->length = (size_t)adrec.length;

	/*
	 * Read in the data content
	 */
	if (fread(buf, ar->length, 1, db->dbz_basef) != 1)
		return (struct alias_rec *)NULL;

	return ar;
}


/*
 * Return a pointer to the next address in this list, and update the pointer
 * to the list.  Addresses are seperated by whitespace and/or commas.
 * Return NULL when list finished.  This routine scribbles on the list.
 */
char *next_addr_in_list(char **aptr)
{
	char *front, *back;

	/*
	 * Locate the first letter of the address.
	 */
	front = *aptr;
	while (*front == ',' || isspace(*front))
		++front;
	if (*front == '\0')
		return (char *) NULL;

	/*
	 * Locate the end of the address.
	 */
	back = front;
	while (*back != '\0' && *back != ',' && !isspace(*back))
		++back;
	if (*back != '\0')
		*back++ = '\0';

	*aptr = back;
	return front;
}
