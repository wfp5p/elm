
/*
 * Privatizing the message header buffers is a first step towards
 * making these dynamic strings...
 */
typedef struct send_header {

    char subject[SLEN];

    char to[VERY_LONG_STRING];
    char cc[VERY_LONG_STRING];
    char bcc[VERY_LONG_STRING];
    char reply_to[SLEN];

    char expanded_bcc[VERY_LONG_STRING];
    char expanded_cc[VERY_LONG_STRING];
    char expanded_to[VERY_LONG_STRING];
    char expanded_reply_to[LONG_STRING];

    char in_reply_to[SLEN];
    char precedence[SLEN];
    char priority[SLEN];
    char action[SLEN];
    char user_defined_header[SLEN];

} SEND_HEADER;

/* sndhdrs.c */
SEND_HEADER *sndhdr_new P_((void));
void sndhdr_destroy P_((SEND_HEADER *));
int sndhdr_output P_((FILE *, const SEND_HEADER *, int, int));
void generate_in_reply_to P_((SEND_HEADER *, int));

/* editmsg.c */
int edit_message P_((const char *, SEND_HEADER *, const char *));

/* hdrconfg.c */
void edit_headers P_((SEND_HEADER *));
int show_msg_headers P_((const SEND_HEADER *, const char *));
int edit_header_char P_((SEND_HEADER *, int));

/* savecopy.c */
int save_copy P_((const char *, const char *, const SEND_HEADER *, int));
int save_mssg P_((const char *, const char *, const SEND_HEADER *, int));
int name_copy_file P_((char *));

