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

echo "Extracting utils/messages (with variable substitutions)"
echo $startsh > messages
cat >>messages <<'!NO!SUBS!'
# messages: part of the Elm mail system
# @(#)$Id: messages.SH,v 1.2 1995/06/30 14:56:31 wfp5p Exp $
#		Copyright (c) 1988-1992 USENET Community Trust
# 		Copyright (c) 1986,1987 Dave Taylor
 
if [ "$2" != "" ]; then
  echo Usage: messages \{folder-name\} 1>&2
  exit 1
fi
 
if [ "$1" = "" ]; then
!NO!SUBS!
echo "  fname=\${MAIL-$maildir/\$LOGNAME}" >> messages
cat >>messages <<'!NO!SUBS!'
  optional="in your incoming mailbox"
else
  optional="in folder $1"
  first=`expr "$1" : "\(.\).*"`
  if [ "$first" = "=" ]; then
    last=`expr "$1" : ".\(.*\)"`
    fname="`grep maildir $HOME/.elm/elmrc | awk -F= '{print $2}'| tr -d ' '`/$last"
  else
    fname=$1
  fi
fi

if [ -f "$fname" ]; then
  mcount=`egrep -c "^From " $fname`
else
  echo "Folder $1 does not exist."
  exit 0
fi
 
if [ "$mcount" -eq 1 ]; then
  echo There is 1 message $optional.
elif [ "$mcount" -eq 0 ]; then
  echo There are no messages $optional.
else
  echo There are $mcount messages $optional.
fi
 
exit $mcount
!NO!SUBS!
