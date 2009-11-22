#!/bin/sh
# $Id: mk_debug_config_h.sh 1462 2007-12-18 20:49:56Z holger $

cat << EOF
#define EXPTOSTRING1(x) #x
#define EXPTOSTRING(x) EXPTOSTRING1(x)
EOF

grep -E '^(//)? *# *define' config.h | grep -v 'CONFIG_H'	\
| sed -r -e 's/^(\/\/)? *# *define +(.*)$/\2/'	\
| while read param rest; do
	echo "#ifdef $param"
	echo "	std::cout << \"\t$param \" << EXPTOSTRING($param) << \"\n\";"
	echo "#endif"
done

cat << EOF
#undef EXPTOSTRING
#undef EXPTOSTRING1
EOF
