IDIR=/usr/local/mpeich
file=examples/home_cleaning
outdir=htdocs

all:  pub

datalog:
	git log --no-decorate -n3 ${file} | grep -v "^Author" | grep -v "^commit" | sed "s/^Date: *\(.*\)/\1/" | grep -v '^$$' > datalog


mpeich: main.cpp
	g++ -g -O0 -std=c++14 main.cpp -o mpeich

graph.dot: mpeich ${file}
	./mpeich -f ${file} dot > graph.dot


graph.png: graph.dot
	dot -Tpng graph.dot -o graph.png	

leafs: mpeich
	./mpeich -f ${file} leafs > leafs
	
pub: graph.png header.html datalog
	mkdir -p ${outdir}
	cp graph.png ${outdir}/
	cp red_bl.gif ${outdir}/
	cp header.html ${outdir}/index.html
	echo '<pre class="datalog">' >> ${outdir}/index.html
	cat datalog >> ${outdir}/index.html
	echo "</pre>" >> ${outdir}/index.html
	./mpeich -f ${file} branches >> ${outdir}/index.html
	echo "</html>" >> ${outdir}/index.html
	
	md5sum ${outdir}/* > pub
clean:
	rm -f mpeich
	rm -f graph.png
	rm -f leafs
	rm -f ${outdir}/*
	rm -f pub
	rm datalog
	rm graph.dot

install: pub
	mkdir -p ${IDIR}
	cp ${outdir} ${IDIR} -R

