awk '/^[^#]/{print 1000*$4*$1/16,FILENAME}' *.point | sort -rg
