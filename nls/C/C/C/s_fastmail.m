$set 18 #Fastmail
$ #NoDebug
1	%s: cannot use "-d" (not compiled with DEBUG)\n
$ #Usage
2	Usage: %s [args ...] filename address ...\n\
\n\
\t"filename" may be "-" to send stdin\n\
\t"args ..." can be:\n\
\n\
\t-b bcc-list\n\
\t-c cc-list\n\
\t-C comments\n\
\t-f from-name\n\
\t-F from-addr\n\
\t-i msg-id\n\
\t-r reply-to\n\
\t-R references\n\
\t-s subject\n\
\t-d  (to enable debugging)\n\
\n
$ #CannotOpenInput
3	%s: cannot open input file "%s" [%s]\n
$ #TooManyToRecip
4	%s: too many "To" recipients\n
$ #TooManyRecip
5	%s: too many recipients specified\n
$ #CannotCreateTemp
6	%s: cannot create temp file %s [%s]\n
$ #ErrorSending
7	%s: error sending message [%s return status %d]\n
