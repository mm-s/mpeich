all:  pub

mpeich: main.cpp
	g++ -g -std=c++14 main.cpp -o mpeich

graph.dot: mpeich data
	./mpeich dot > graph.dot


graph.png: graph.dot
	dot -Tpng graph.dot -o graph.png	

leafs: mpeich
	./mpeich leafs > leafs
	
pub: graph.png
	cp graph.png htdocs/
	./mpeich branches > htdocs/branches
clean:
	rm -f mpeich
	rm -f graph.png
	rm -f leafs
	rm -f htdocs/branches
	rm -f htdocs/graph.png

