
/*
 * The "sndparts" routines manage chunks of outgoing messages.
 * In the simplest case, there will be a single part that corresponds
 * to the outbound mail message.  These routines provide support to
 * add attachments and inclusions, as well as complex parts such
 * as the MIME "multipart/mixed" type.
 */

/*
 * RFC-1521 (sec 4.5.3) says 7bit/8bit parts cannot exceed this length
 * Actually, the max is 1000, but we've accounted for CRNL termination here.
 */
#define RFC821_MAXLEN	998

/* generate maximally random/minimally ugly multipart separator */
#define MIME_MAKE_BOUNDARY(buf, type, seq) \
	    sprintf(buf, "%%--%s-boundary-%d.%d.%lu--%%", \
			(type), (seq), getpid(), \
			(unsigned long)time((time_t *)NULL));

/* commands that handle various content encodings */
# define MIME_ENCODE_CMD_QUOTED(buf, infile, outfile) \
	    sprintf((buf), "mmencode -q %s >%s", (infile), (outfile))
# define MIME_ENCODE_CMD_BASE64(buf, infile, outfile) \
	    sprintf((buf), "mmencode -b %s >%s", (infile), (outfile))
# define MIME_ENCODE_CMD_UUENCODE(buf, infile, outfile) \
	    sprintf((buf), "uuencode %s <%s >%s", \
	    basename(infile), (infile), (outfile))	/* SIDE EFFECTS */

/* command that provides information on the content of a part */
# define MIME_FILE_CMD(buf, infile, outfile) \
	    sprintf((buf), "file %s >%s", (infile), (outfile))


/*
 * A (SEND_BODYPART) describes a chunk of data being sent.  In the
 * simplest case (a plain old mail message) there is just a single
 * part representing the message.  Additional parts will be used
 * as attachments and inclusions are created.
 */

/* part_type settings for a (SEND_BODYPART) */
#define BP_IS_DUMMY	0	/* not a real part			*/
#define BP_IS_MSSGTEXT	1	/* text/plain subject to [...] interp	*/
#define BP_IS_MIMEPART	2	/* a regular MIME body part		*/
#define BP_IS_MULTIPART	3	/* head of a multipart			*/

/* content_header[] selectors */
#define BP_CONT_TYPE		0	/* Content-Type			*/
#define BP_CONT_ENCODING	1	/* Content-Transfer-Encoding	*/
#define BP_CONT_DESCRIPTION	2	/* Content-Description		*/
#define BP_CONT_DISPOSITION	3	/* Content-Disposition		*/
#define BP_CONT_MD5		4	/* Content-MD5			*/
#define BP_NUM_CONT_HEADERS	5

/* the header names associated with the above */
extern char *Mime_header_names[BP_NUM_CONT_HEADERS];

#ifdef ANSI_C
struct send_multipart;
#endif

/*
 * The (SEND_BODYPART) describes a chunk of the message body.
 */
typedef struct send_bodypart {

	/*
	 * The "part_type" indicate what this part contains.  The remaining
	 * members of the structure are interpreted according to the type.
	 *
	 * BP_IS_DUMMY - Not a real part.  This is used in the attachments
	 * menu to create labels.  The only member of the structure that
	 * is valid for one of these is "fname", which contains a label.
	 * Never ever try to output one of these!
	 *
	 * BP_IS_MIMEPART - This part is a single entity, such as a
	 * file attachment.  The MIME header information and the content
	 * statistics members will be valid.  The multipart information,
	 * however, does not contain meaningful information.  Except
	 * for the main message (which will be a BP_IS_MSSGTEXT), every
	 * chunk of data is represented by one of these.
	 *
	 * BP_IS_MULTIPART - This part does not contain any data, but
	 * instead represents a chunk of parts, a "multipart/mixed".  In
	 * this case, the multipart members are valid, and will lead
	 * to the parts that comprise this multipart.  The MIME header
	 * and content statistics members should be ignored.
	 * ("multipart/mixed" is the only form of multipart supported at
	 * this time.)
	 *
	 * BP_IS_MSSGTEXT - This part represents the message entered by
	 * the user.  The MIME header and content statistics members
	 * describe the message text.  The multipart information will
	 * indicate whether the user has provided any attachments or
	 * inclusions.  If the multipart list is empty, this message
	 * is sent as a simple "text/plain".  If there is multipart
	 * information, then the message is a "multipart/mixed", with
	 * this text appearing with one (or more) subparts.
	 */
	int part_type;

	/*
	 * The (SEND_MULTIPART) structure is just a list of these
	 * (SEND_BODYPART) pieces.  It is possible to have a body part
	 * appear in multiple lists.  (For instance, it could appear
	 * in the list of attachments for a BP_IS_MSSGTEXT part, as well
	 * as a separate list that tracks the attachments that were
	 * created by the attachments menu.)  The "link_count" indicates
	 * how many (SEND_MULTIPART) lists reference this part.  When
	 * the link count reaches zero, this part no longer is in use
	 * and may be destroyed.
	 */
	int link_count;

	/*
	 * The file that contains the data for this part.
	 * BP_IS_DUMMY - a text label.
	 * BP_IS_MIMEPART - filename containing part data.
	 * BP_IS_MULTIPART - undefined value, do not use.
	 * BP_IS_MSSGTEXT - filename containing message text.
	 */
	char *fname;

	/*
	 * Procedures that emit the part header and body.
	 * BP_IS_DUMMY - undefined values, do not use.
	 * BP_IS_MIMEPART - procedures to emit part header and body.
	 * BP_IS_MULTIPART - procedures to emit the multipart hdr and body.
	 * BP_IS_MSSGTEXT - procedures to emit message (and attachments and
	 *	inclusions) header and body.
	 */
	int (*emit_hdr_proc) P_((FILE *, struct send_bodypart *));
	int (*emit_body_proc) P_((FILE *, struct send_bodypart *));

	/*
	 * Values of the MIME "Content-" headers.
	 * BP_IS_DUMMY - undefined values, do not use.
	 * BP_IS_MIMEPART - valid header values.
	 * BP_IS_MULTIPART - undefined values, do not use.
	 * BP_IS_MSSGTEXT - header values associated with the message text.
	 *	If the message will be transmitted as "text/plain" then
	 *	these appear in the main message headers.  If the message
	 *	is "multipart/mixed" then these appear in the part
	 *	that contains the message text.
	 */
	char *content_header[BP_NUM_CONT_HEADERS];

	/*
	 * Statistics on the data.
	 * BP_IS_DUMMY - undefined values, do not use.
	 * BP_IS_MIMEPART - valid statistics on the part data.
	 * BP_IS_MULTIPART - undefined values, do not use.
	 * BP_IS_MSSGTEXT - valid statistics on the message text.
	 */
	long total_chars;	/* size of the part			*/
	long ascii_chars;	/* chars 0x20 thru 0x7F plus whitespace	*/
	long binlo_chars;	/* non-space chars < 0x20 (incl 0x0!)	*/
	long binhi_chars;	/* chars >= 0x80			*/
	int max_linelen;	/* length of longest line, excluding NL	*/

	/*
	 * Multipart information.
	 * BP_IS_DUMMY - undefined values, do not use.
	 * BP_IS_MIMEPART - undefined values, do not use.
	 * BP_IS_MULTIPART - lists the parts that comprise this multipart.
	 * BP_IS_MSSGTEXT - the attachments and inclusions for the
	 *	message.  If the list is empty then there are none
	 *	and the message will be sent as "text/plain".
	 */
	struct send_multipart *subparts;
	char *boundary;		/* multipart boundary seperator		*/

} SEND_BODYPART;


/*
 * A multipart list is constructed with (SEND_MULTIPART).  For a list
 * with N parts, there will be N+1 (SEND_MULTIPART) entries.  One serves
 * as the list head, and the remainder encapsulate a (SEND_BODYPART)
 * that contains the data for the corresponding part.
 */
typedef struct send_multipart {

    /*
     * A part may be keyed with an ID number.  This is optional, and is
     * required only if a lookup-by-ID will be performed.  This is
     * used, for instance, by the [include] feature.  When initially
     * scanning the user's message, the included parts are keyed by
     * fseek position in the file.  This means that when the final
     * output is produced, the part can be located easily without
     * reparsing the input.  On the other hand, the [attach] parts
     * are not keyed, since no lookup is performed (they merely are
     * dumped out when the message text is complete).
     */
    long id;

    /*
     * The "part" is the (SEND_BODYPART) that contains the data for
     * this part.  The list head will have a NULL value here.
     */
    struct send_bodypart *part;

    /*
     * Forward and back pointers for double-linked list.
     */
    struct send_multipart *prev, *next;

} SEND_MULTIPART;


/* operations that may be performed on the head element of a (SEND_MULTIPART) */
#define MULTIPART_HEAD(mp)	((mp)->next)
#define MULTIPART_TAIL(mp)	((mp)->prev)
#define MULTIPART_IS_EMPTY(mp)	((mp)->next == (mp))

/* operations that may be performed on any element of a (SEND_MULTIPART) */
#define MULTIPART_PART(mp)	((mp)->part)
#define MULTIPART_ID(mp)	((mp)->id)

/* ID used for attachment parts */
#define MP_ID_ATTACHMENT	(-1L)


/* sndpart_lib.c */

int encoding_is_reasonable P_((const char *));

SEND_BODYPART *bodypart_new P_((const char *, int));
void bodypart_destroy P_((SEND_BODYPART *));
void bodypart_set_content P_((SEND_BODYPART *, int, const char *));
const char *bodypart_get_content P_((SEND_BODYPART *, int));
void bodypart_guess_content P_((SEND_BODYPART *, int));

#define bodypart_get_filename(part)	((part)->fname)

SEND_MULTIPART *multipart_new P_((SEND_BODYPART *, long));
void multipart_destroy P_((SEND_MULTIPART *));
SEND_MULTIPART *multipart_insertpart P_((SEND_MULTIPART *, SEND_MULTIPART *, SEND_BODYPART *, long));
SEND_MULTIPART *multipart_appendpart P_((SEND_MULTIPART *, SEND_MULTIPART *, SEND_BODYPART *, long));
SEND_BODYPART *multipart_deletepart P_((SEND_MULTIPART *, SEND_MULTIPART *));
SEND_MULTIPART *multipart_next P_((SEND_MULTIPART *, SEND_MULTIPART *));
SEND_MULTIPART *multipart_find P_((SEND_MULTIPART *, long));

#ifdef NDEBUG
# define bodypart_integrity_check(part)
# define multipart_integrity_check(part)
#else
  void bodypart_integrity_check P_((const SEND_BODYPART *));
  void multipart_integrity_check P_((const SEND_MULTIPART *));
#endif


/* sndpart_io.c */

SEND_BODYPART *newpart_mssgtext P_((const char *));
SEND_BODYPART *newpart_mimepart P_((const char *));

int emitpart P_((FILE *, SEND_BODYPART *));
int emitpart_mssghdr P_((FILE *, SEND_BODYPART *));
int emitpart_mssgbody P_((FILE *, SEND_BODYPART *));

