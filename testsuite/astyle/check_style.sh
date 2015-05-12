#!/bin/bash -e

cd ../..

code=0

command -v astyle >/dev/null 2>&1 || { echo "astyle not found"; exit 1; }

for file in $(find **/*.c)
do
	if [[ $file = "bandb/sqlite3.c" ]]
	then
		continue
	fi

	if [[ $file = "src/lex.yy.c" ]]
	then
		continue
	fi

	if [[ $file = "src/y.tab.c" ]]
	then
		continue
	fi

	if [[ $file = "tools/convertilines.c" ]]
	then
		continue
	fi

	if [[ $file = "tools/convertklines.c" ]]
	then
		continue
	fi

	if [[ $file = "tools/mkpasswd.c" ]]
	then
		continue
	fi

	if [[ $file = "tools/viconf.c" ]]
	then
		continue
	fi

	if [[ $file = "unsupported/make_override_immune.c" ]]
	then
		continue
	fi

	if [[ $file = "unsupported/m_clearchan.c" ]]
	then
		continue
	fi

	if [[ $file = "unsupported/sno_channeljoin.c" ]]
	then
		continue
	fi

	if [[ $(astyle --style=linux --mode=c -n $file 2>&1 | cut -f1 -d' ') != "Unchanged" ]]
	then
		echo "$file is not at coding standards."
		code=1
	fi
done

exit $code
