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

echo "Extracting utils/checkalias (with variable substitutions)"
echo $startsh > checkalias
cat >>checkalias <<!DO!SUBS!
# checkalias: part of the Elm mail system
# @(#)$Id: checkalias.SH,v 1.3 1995/07/18 19:00:12 wfp5p Exp $
#		Copyright (c) 1988-1992 USENET Community Trust
# 		Copyright (c) 1986,1987 Dave Taylor
 

if [ \$# -eq 0 ] ; then
	echo "usage: \$0 alias ..." 1>&2
	exit 1
fi

elmalias -en \$@ | \\
	prlong -w 78 -1 "Expands to: " -l "	" -f ", "
exit \$?

!DO!SUBS!
