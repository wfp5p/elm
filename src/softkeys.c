

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
 * $Log: softkeys.c,v $
 * Revision 1.3  1996/03/14  17:29:57  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:42:33  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:39  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 *******************************************************************************
 */

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"

#define f_key1	1
#define f_key2	2
#define f_key3	3
#define f_key4	4
#define f_key5	5
#define f_key6	6
#define f_key7	7
#define f_key8	8


static void define_key(int key, char *display, char *send)
{
	fprintf(stderr, "%c&f%dk%dd%dL%s%s", ESCAPE, key,
		strlen(display), strlen(send), display, send);
}

static void clear_key(int key)
{
       define_key(key, "                ", "");
}


int define_softkeys(int sel)
{
	static int prev_selection = -1;
	int ret_selection;

	if (!hp_softkeys)
	  return prev_selection;

	switch (sel) {

	case SOFTKEYS_MAIN:
	  define_key(f_key1, catgets(elm_msg_cat, ElmSet, ElmKeyF1,
		" Display   Msg"),   "\r");
	  define_key(f_key2, catgets(elm_msg_cat, ElmSet, ElmKeyF2,
		"  Mail     Msg"),   "m");
	  define_key(f_key3, catgets(elm_msg_cat, ElmSet, ElmKeyF3,
		"  Reply  to Msg"),  "r");
	  if (user_level == 0) {
	    define_key(f_key4, catgets(elm_msg_cat, ElmSet, ElmKey0F4,
		"  Save     Msg"),   "s");
	    define_key(f_key5, catgets(elm_msg_cat, ElmSet, ElmKey0F5
		," Delete    Msg"),   "d");
	    define_key(f_key6, catgets(elm_msg_cat, ElmSet, ElmKey0F6,
		"Undelete   Msg"),   "u");
   	  } else {
	    define_key(f_key4, catgets(elm_msg_cat, ElmSet, ElmKey1F4,
		" Change  Folder"), "c");
	    define_key(f_key5, catgets(elm_msg_cat, ElmSet, ElmKey1F5,
		"  Save     Msg"),   "s");
	    define_key(f_key6, catgets(elm_msg_cat, ElmSet, ElmKey1F6,
		" Delete/Undelete"), "^");
	  }
	  define_key(f_key7, catgets(elm_msg_cat, ElmSet, ElmKeyF7,
		" Print     Msg"),   "p");
	  define_key(f_key8, catgets(elm_msg_cat, ElmSet, ElmKeyF8,
		"  Quit     ELM"),   "q");

	  break;

	case SOFTKEYS_ALIAS:
	  define_key(f_key1, catgets(elm_msg_cat, ElmSet, ElmKeyAF1,
		" Alias  Current"),  "a");
	  define_key(f_key2, catgets(elm_msg_cat, ElmSet, ElmKeyAF2,
		" Check  Person"),   "p");
	  define_key(f_key3, catgets(elm_msg_cat, ElmSet, ElmKeyAF3,
		" Check  System"),   "s");
	  define_key(f_key4, catgets(elm_msg_cat, ElmSet, ElmKeyAF4,
		" Make    Alias"),   "m");
	  clear_key(f_key5);
	  clear_key(f_key6);
	  clear_key(f_key7);
	  define_key(f_key8, catgets(elm_msg_cat, ElmSet, ElmKeyAF8,
		" Return  to ELM"),  "r");

	case SOFTKEYS_YESNO:
	  define_key(f_key1, catgets(elm_msg_cat, ElmSet, ElmKeyYF1,
		"  Yes"),  "y");
	  clear_key(f_key2);
	  clear_key(f_key3);
	  clear_key(f_key4);
	  clear_key(f_key5);
	  clear_key(f_key6);
	  clear_key(f_key7);
	  define_key(f_key8, catgets(elm_msg_cat, ElmSet, ElmKeyYF8,
		"   No"),  "n");

	case SOFTKEYS_READ:
	  define_key(f_key1, catgets(elm_msg_cat, ElmSet, ElmKeyRF1,
		"  Next    Page  "), " ");
	  clear_key(f_key2);
	  define_key(f_key3, catgets(elm_msg_cat, ElmSet, ElmKeyRF3,
		"  Next    Msg   "), "j");
	  define_key(f_key4, catgets(elm_msg_cat, ElmSet, ElmKeyRF4,
		"  Prev    Msg   "), "k");
	  define_key(f_key5, catgets(elm_msg_cat, ElmSet, ElmKeyRF5,
		"  Reply  to Msg "), "r");
	  define_key(f_key6, catgets(elm_msg_cat, ElmSet, ElmKeyRF6,
		" Delete   Msg   "), "d");
	  define_key(f_key7, catgets(elm_msg_cat, ElmSet, ElmKeyRF7,
		"  Send    Msg   "), "m");
	  define_key(f_key8, catgets(elm_msg_cat, ElmSet, ElmKeyRF8,
		" Return  to ELM "), "q");

	case SOFTKEYS_CHANGE:
	  define_key(f_key1, catgets(elm_msg_cat, ElmSet, ElmKeyCF1,
		"  Mail  Directry"), "=/");
	  define_key(f_key2, catgets(elm_msg_cat, ElmSet, ElmKeyCF2,
		"  Home  Directry"), "~/");
	  clear_key(f_key3);
	  define_key(f_key4, catgets(elm_msg_cat, ElmSet, ElmKeyCF4,
		"Incoming Mailbox"), "!\r");
	  define_key(f_key5, catgets(elm_msg_cat, ElmSet, ElmKeyCF5,
		"\"Received\" Folder"), ">\r");
	  define_key(f_key6, catgets(elm_msg_cat, ElmSet, ElmKeyCF6,
		"\"Sent\"   Folder "), "<\r");
	  clear_key(f_key7);
	  define_key(f_key8, catgets(elm_msg_cat, ElmSet, ElmKeyCF8,
		" Cancel"), "\n");

	}

	softkeys_on();
	ret_selection = prev_selection;
	prev_selection = sel;
	return ret_selection;
}

void softkeys_on(void)
{
	/* enable (esc&s1A) turn on softkeys (esc&jB) and turn on MENU
	   and USER/SYSTEM options. */

	if (hp_softkeys) {
	  fprintf(stderr, "%c&s1A%c&jB%c&jR", ESCAPE, ESCAPE, ESCAPE);
	  fflush(stderr);
	}
}

void softkeys_off(void)
{
	/* turn off softkeys (esc&j@) */

	if (hp_softkeys) {
	  fprintf(stderr, "%c&s0A%c&j@", ESCAPE, ESCAPE);
	  fflush(stderr);
	}
}
