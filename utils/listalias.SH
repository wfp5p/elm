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

echo "Extracting utils/listalias (with variable substitutions)"
echo $startsh > listalias
cat >>listalias <<'!NO!SUBS!'
# listalias: part of the Elm mail system
# @(#)$Id: listalias.SH,v 1.2 1995/06/30 14:56:30 wfp5p Exp $
#		Copyright (c) 1988-1992 USENET Community Trust
# 		Copyright (c) 1986,1987 Dave Taylor

USAGE="usage: $0 [-u | -s] [optional-regular-expression]"

opts=

while : ; do
	case "$1" in
	-u|-s)	opts="$opts $1" ; shift ;;
	--)	shift ; break ;;
	-*)	echo "$USAGE" 1>&2 ; exit 1 ;;
	*)	break ;;
	esac
done

listit="elmalias -v $opts"
grepit=
sortit="| sort"

case "$#" in

0)	;;

1)	# BSD uses "-y" for case insensitive, SysV uses "-i".
	grepit="| grep \"$1\""
	for flag in -i -y ; do
		x=`echo HELLO | grep $flag hello 2>/dev/null`
		if test $? -eq 0 -a "X$x" = "XHELLO" ; then
			grepit="| grep $flag \"$1\""
			break
		fi
	done
	;;

*)	echo "$USAGE" 1>&2
	exit 1
	;;

esac

eval "$listit $grepit $sortit"
exit $?
!NO!SUBS!
