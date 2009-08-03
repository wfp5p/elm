case $CONFIG in
'')	for dir in . .. ../.. ../../.. ; do
		if test -f $dir/config.sh ; then
			. $dir/config.sh
			break
		fi
	done
	case $CONFIG in
	'') (echo "Can't find config.sh."; exit 1) ;;
	esac
	;;
esac
case "$0" in
*/*)	cd `echo "$0" | sed -e 's!/[^/]*$!!' -e "s!^$source_dir!$build_dir!"` ;;
esac

echo "Extracting utils/printmail (with variable substitutions)"
echo $startsh > printmail
cat >>printmail <<'!NO!SUBS!'
# printmail: part of the Elm mail system
# @(#)$Id: printmail.SH,v 1.2 1995/06/30 14:56:31 wfp5p Exp $
#		Copyright (c) 1988-1992 USENET Community Trust
# 		Copyright (c) 1986,1987 Dave Taylor
 
if [ "$1" = "-p" ]; then
  flags="-p";shift
fi
 
if [ "$1" != "" ]; then
  if [ ! -r $1 ]; then
    echo printmail: cannot open folder $1 1>&2
    exit 1
  else
    flags="$flags -f $1"
  fi
fi
 
# and now invoke readmsg
exec readmsg $flags \*
!NO!SUBS!
