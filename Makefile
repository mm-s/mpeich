IDIR=/usr/local/mpeich

all:  pub

mpeich: main.cpp
	g++ -g -std=c++14 main.cpp -o mpeich

graph.dot: mpeich data
	./mpeich dot > graph.dot


graph.png: graph.dot
	dot -Tpng graph.dot -o graph.png	

leafs: mpeich
	./mpeich leafs > leafs
	
pub: graph.png header.html
	cp graph.png htdocs/
	cp red_bl.gif htdocs/
	cp header.html htdocs/index.html
	./mpeich branches >> htdocs/index.html
	echo "</html>" >> htdocs/index.html
	md5sum htdocs/* > pub
clean:
	rm -f mpeich
	rm -f graph.png
	rm -f leafs
	rm -f htdocs/*
	rm -f pub

install: pub
	mkdir -p ${IDIR}
	cp htdocs ${IDIR} -R

