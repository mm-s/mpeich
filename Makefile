all: graph.png leafs

mpeich: main.cpp
	g++ -std=c++14 main.cpp -o mpeich

graph.dot: mpeich
	./mpeich dot > graph.dot


graph.png: graph.dot
	dot -Tpng graph.dot -o graph.png	

leafs: mpeich
	./mpeich leafs > leafs
	

clean:
	rm -f mpeich
	rm -f graph.png
	rm -f leafs

