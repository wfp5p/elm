#
# @(#)$Id: mailrc.awk,v 1.1.1.1 1995/04/19 20:38:40 wfp5p Exp $
#		Copyright (c) 1988-1992 USENET Community Trust
# 		Copyright (c) 1986,1987 Dave Taylor
# Bug reports, patches, comments, suggestions should be sent to:
#
#	Syd Weinstein, Elm Coordinator - elm@DSI.COM
#					 dsinc!elm
#
# $Log: mailrc.awk,v $
# Revision 1.1.1.1  1995/04/19  20:38:40  wfp5p
# Initial import of elm 2.4 PL0 as base for elm 2.5.
#
# Revision 5.1  1992/10/04  00:46:45  syd
# Initial checkin as of 2.4 Release at PL0
#
# 
#


BEGIN { 
	print "# MSG alias_text file, from a .mailrc file..." 
	print ""
      }

next_line == 1 { 

	next_line = 0;
        group = ""
	for (i = 1; i <= NF; i++) {
	  if (i == NF && $i == "\\") sep = ""
	  else                       sep = ", "
	
	  if ($i == "\\") {
	    group = sprintf("%s,", group)
	    next_line = 1;
	  }
	  else if (length(group) > 0)
	    group = sprintf("%s%s%s", group, sep, $i);
	  else
	    group = $i;
	  }
	  print "\t" group

	}

$1 ~ /[Aa]lias|[Gg]roup/ { 

	if ( NF == 3)
	  print $2 " = user alias = " $3;
	else {
	  group = ""
	  for (i = 3; i <= NF; i++) {
	    if (i == NF && $i == "\\") sep = ""
	    else        sep = ", "
	
	    if ($i == "\\") {
 	      group = sprintf("%s,", group)
 	      next_line = 1;
	    }
	    else if (length(group) > 0) 
 	      group = sprintf("%s%s%s", group, sep, $i);
	    else
 	      group = $i;
	    }
	    print $2 " = group alias = " group;
	  }
 	}
