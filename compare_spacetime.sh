awk '/^[^#]/{print 500*($4+$5)*$1/16,$1,FILENAME}' *.point | sort -rg
