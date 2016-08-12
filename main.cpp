#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <limits>

using namespace std;

struct edge;

struct vertex {
	vertex(int id_): id(id_) {
	}
	bool is_leaf() const { return e.empty(); }
	vector<edge*> e;
	int id;
};

struct edge {  //from, to
	edge(const vertex* f, const vertex* t): from(f), to(t) {
	}
	const vertex* from;
	const vertex* to;

	string key;
};


struct graph {

	map<int,vertex*> V;
	map<string,edge*> E;

	graph(const vector<string>& al) {  //v1 v2
		for (auto& l:al) {
			int v1=-1;
			int v2=-1;
			istringstream is(l);
			is >> v1;
			is >> v2;
			add(v1,v2);
		}
	}

	graph(const vector<pair<int,int>>& al) {  //v1 v2
		for (auto& l:al) {
			add(l.first,l.second);
		}
	}
	template<class P>
	void dot(const P& pred, ostream& os) const {
		os << "digraph {" << endl;
		for (auto v: V) {
			os << "task" << v.second->id << "[label=\"" << v.second->id << ": " << pred(v.second->id) << "\"";
			if (v.second->is_leaf()) {
				os << ",color=red";
			}
			os << "];" << endl;
		}
		for (auto e: E) {
			os << "task" << e.second->to->id << " -> task" << e.second->from->id << ";" << endl;
		}
		os << "}" << endl;
	}

	void dot(ostream& os) const {
		dot([](int i) -> string { return to_string(i); }, os);
	}	

	vertex* add(int iv) {
		auto i=V.find(iv);
		if (i==V.end()) {
			V.emplace(iv,new vertex(iv));
			i=V.find(iv);
		}
		return i->second;
	}

	void add(int iv1, int iv2) {
		if (iv1<0 || iv2<0) return;
		ostringstream os;
		os << iv1 << " " << iv2;
		auto v1=add(iv1);
		auto v2=add(iv2);
		edge* e=new edge(v1,v2);
		E.emplace(os.str(),e);
		v1->e.push_back(e);

	}

	~graph() {
		for (auto i: E) delete i.second;
		for (auto i: V) delete i.second;
	}

	struct visitor {
		enum algorithm {
			breath_first, depth_first
		
		};
		virtual void start(const vertex&){};
		virtual void visit(const edge&){};
		virtual void visit(const vertex&){};
		virtual void finished(){};
	};
	void breath_first(int iv, visitor& visitor_) const {
		auto v=V.find(iv);
		if (v==V.end()) return;
		breath_first(*v->second,visitor_);
	}

	void breath_first(const vertex& s, visitor& visitor_) const {
		visitor_.start(s);
		visitor_.visit(s);
		unordered_set<edge*> visited;
		queue<edge*> q;
		for (auto e:s.e) {
			q.push(e);
		}
		while(!q.empty()) {
			edge* e=q.front(); q.pop();
			if (visited.find(e)==visited.end()) {		
				for (auto ed:e->to->e) {
					q.push(ed);
				}
				visitor_.visit(*e);
				visitor_.visit(*e->to);
				visited.emplace(e);
			}
		}
		visitor_.finished();
	}

	void depth_first(int iv, visitor& visitor_) const {
		auto v=V.find(iv);
		if (v==V.end()) return;
		depth_first(*v->second,visitor_);
	}

	void depth_first(const vertex& s, visitor& visitor_) const {
		visitor_.start(s);
		visitor_.visit(s);
		unordered_set<edge*> visited;

		stack<edge*> q;
		for (auto e:s.e) {
			q.push(e);
		}
		while(!q.empty()) {
			edge* e=q.top(); q.pop();
			if (visited.find(e)==visited.end()) {		
				for (auto ed:e->to->e) {
					q.push(ed);
				}
				visitor_.visit(*e);
				visitor_.visit(*e->to);
				visited.emplace(e);
			}
		}
		visitor_.finished();
	}

	
};



struct vis:graph::visitor {
	void start(const vertex&) override {};
	void visit(const edge& e) override {
		cout << (e.from!=0?e.from->id:-1) << " -> " << e.to->id << endl;
	}
	void finished() override {};
};


template<typename T, typename data>
struct best_path:graph::visitor {
	unordered_map<const edge*,T> values;



	unordered_map<const vertex*,data> vd; //vertex data
	const graph& g;
	const vertex* f{0};
	const vertex* t{0};
	struct result:vector<const vertex*> {
		void dump(ostream& os) const {
			for (auto i:*this) os << i->id << endl;
		}
	};
	result r;
	best_path(const graph&g_, const vector<string>& al_values): g(g_) { //adjacency list + values
		for (auto& l:al_values) {
			int v1=-1;
			int v2=-1;
			istringstream is(l);
			is >> v1;
			is >> v2;
			ostringstream os;
			os << v1 << " " << v2;
			auto ie=g.E.find(os.str());
			if (ie==g.E.end()) {
				cerr << "edge not found " << os.str() << endl;
				continue;
			}
			string value;
			getline(is,value);
			T t(value);
			values.emplace(ie->second,move(t));
		}
	}
	~best_path() {
	}

	result compute(int from, int to,algorithm a) {
		auto ifrom=g.V.find(from);
		if (ifrom==g.V.end()) {
			cerr << from << " node not found" << endl;
			return r;
		}
		else f=ifrom->second;

		auto ito=g.V.find(to);
		if (ito==g.V.end()) {
			cerr << to << " node not found" << endl;
			return r;
		}
		else t=ito->second;
		switch(a) {
			case breath_first:
				g.breath_first(*f,*this);
				break;
			case depth_first:
				g.depth_first(*f,*this);
				break;
			default: cerr << "unknown algorithm" << endl;
		}
		return r;
	}

	data& relax(const edge& e) {
		auto& ed=values.find(&e)->second;
		auto ivdf=vd.find(e.from);
		if (ivdf==vd.end()) {
			vd.emplace(e.from,data());
			ivdf=vd.find(e.from);
		}
		auto ivdt=vd.find(e.to);
		if (ivdt==vd.end()) {
			vd.emplace(e.to,data());
			ivdt=vd.find(e.to);
		}
		data& df=ivdf->second;
		data& dt=ivdt->second;
		auto sum=ed+df;	
		if (sum<dt) {
			dt=sum;
			dt.from=&e;
		}
		return dt;
	}


	void start(const vertex& s) override {
		r.clear();
		vd.clear();
		vd.emplace(&s,data(0));
	};
	
	void visit(const edge& e) override {
		relax(e);
/*
		if (e->to==t) { //he llegado a f
			if (best!=0) {
				if (d<*best) {
					best=&d;
				}
			}
			else {
				best=&d;
			}
		}
*/
	}

	void finished() override {
		auto icur=vd.find(t);
		if (icur==vd.end()) return; //no hay conexion con el nodo buscado
		data*cur=&icur->second;
		vector<const vertex*> rp;
		while(cur->from!=0) {
			rp.push_back(cur->get_dst_vertex());
			const vertex* src=cur->get_src_vertex();
			if (src==0) break;
			cur=&vd.find(src)->second;
		}		
		//reverse it
		r.reserve(rp.size()+1);
		for (auto i=rp.rbegin(); i!=rp.rend(); ++i) {
			r.push_back(*i);
		}
		r.push_back(f);
	}


};


struct data {
	data():score(numeric_limits<int>::max()) {}
	data(int v):score(v) {}
	const vertex* get_dst_vertex() const { return from->to; }
	const vertex* get_src_vertex() const { return from->from; }
	int score;
	const edge* from{0};
	bool operator <(const data& other) const { return score<other.score; }
};

template<typename T>
struct scalar {
	T value{0};
	scalar(string& s) {
		istringstream is(s);
		is>>value;
	}
	data operator+(const data& d) const {
		data r;
		r.score=d.score+value;
		return move(r);
	}	
};


void graph_example() {
	vector<string> al;
	al.push_back("1 2");
	al.push_back("1 3");
	al.push_back("1 4");
	al.push_back("2 5");
	al.push_back("2 6");
	al.push_back("3 7");
	al.push_back("4 8");
	al.push_back("4 9");
	al.push_back("5 10");
	al.push_back("5 11");
	al.push_back("6 12");
	al.push_back("7 13");
	al.push_back("8 14");
	al.push_back("14 8");
	al.push_back("9 2");
	al.push_back("6 2");
	al.push_back("12 3");
	
	graph g(al);
	g.dot(cout);

	vis vis_;
	cout << "breath first traversal" << endl;
	g.breath_first(1,vis_);
	cout << "depth first traversal" << endl;
	g.depth_first(1,vis_);

	cout << "best path" << endl;
	vector<string> distances;
	distances.push_back("1 2 1");
	distances.push_back("1 3 1");
	distances.push_back("1 4 1");
	distances.push_back("2 5 1");
	distances.push_back("2 6 1");
	distances.push_back("3 7 1");
	distances.push_back("4 8 1");
	distances.push_back("4 9 1");
	distances.push_back("5 10 1");
	distances.push_back("5 11 1");
	distances.push_back("6 12 1");
	distances.push_back("7 13 1");
	distances.push_back("8 14 1");
	distances.push_back("14 8 1");
	distances.push_back("9 2 1");
	distances.push_back("6 2 1");
	distances.push_back("12 3 1");

	typedef best_path<scalar<int>,data> pathfinder;
	pathfinder bp(g,distances);
	{
	cout << "best path from 1 to 11 )" << endl;
	auto r=bp.compute(1,11,pathfinder::breath_first);
	r.dump(cout);
	}
	{
	cout << "best path from 14 to 2 )" << endl;
	auto r=bp.compute(14,2,pathfinder::breath_first);
	r.dump(cout);
	}
	{
	cout << "best path from 1 to 2 )" << endl;
	auto r=bp.compute(1,2,pathfinder::depth_first);
	r.dump(cout);
	}


}



///////////////////////////////////// montepeich

struct task {
	task(string n): name(n), duration(0), id(++next_id) {
	}
	int id;
	string name;
	int duration;
	int cost{0};
	string text;
	static int next_id;
};

int task::next_id=0;

#include <unordered_map>

unordered_map<int,task> tasks;
vector<pair<int,int>> al; //adjacency list


int regtask(string name) {
	task t(name);
	tasks.emplace(t.id,t);
	return t.id;
}
void dep(int from, int to) {
	al.emplace_back(from,to);
}
void cost(int id, int v) {
	tasks.find(id)->second.cost=v;
}
void dur(int id, int v) {
	tasks.find(id)->second.duration=v;
}
void text(int id, string v) {
	tasks.find(id)->second.text=v;
}
#include <functional>
#include <unordered_set>
struct leafs:graph::visitor {
	std::function <string (int)> _t;
	leafs(std::function <string (int)> t):_t(t) {}
	void visit(const vertex&v) override {
		if (!v.is_leaf()) return;
		if (_uniq.find(v.id)!=_uniq.end()) return;
		_uniq.emplace(v.id);
		cout << v.id << " " << _t(v.id) << endl;
	}
	unordered_set<int> _uniq;
};

void mp(string cmd) {
	int id=regtask("inquilino dentro");
	int fc=regtask("firma contrato");
	dep(id,fc);
	int pc=regtask("preparar contrato");
	dep(fc,pc);

	int vi=regtask("visita");
	dep(fc,vi);
	int pr=regtask("poder ruben");
	dep(vi,pr);
	int in=regtask("ir notario");
	dep(pr,in);

	int ime=regtask("investigar mercado");

	int depe=regtask("decidir precio");

	int riemv=regtask("ruben ir EMV");
	dep(depe,riemv);
	dep(depe,ime);
	int rea=regtask("redactar anuncio");
	dep(rea,depe);
	int safo=regtask("sacar fotos");
	int publ=regtask("publicar");
	dep(publ,rea);
	dep(publ,safo);
	dep(vi,publ);

	int ciha=regtask("cita con hacienda");
	dep(depe,ciha);
	dep(pc,ciha);
	dep(pc,riemv);

	int ca=regtask("casa acondicionada");
	dep(safo,ca);

	int ga=regtask("garaje acondicionado");
	dep(ca,ga);
	int ma=regtask("mover astra");
	dep(ga,ma);
	int bpa=regtask("buscar parking astra");
	dep(ma,bpa);

	int cl=regtask("casa limpia");
	dep(ca,cl);

	int cv=regtask("casa vacia");
	dep(ca,cv);

	int ice=regtask("Inspeccion calif energetica");
	dep(ice,cv);
	dep(fc,ice);

	int tra=regtask("transporte");
	int trapl=regtask("transportar basura a punto limpio");
	int tratr=regtask("transportar a trasteros");
	int tratrj=regtask("transportar trastero Judith");
	int tratrn=regtask("transportar trastero Noe");
	int tratra=regtask("transportar trastero Alquilado");

	int pretrn=regtask("preparar trastero Noe");

	dep(tratrn,pretrn);
	dep(tra,trapl);
	dep(tra,tratr);
	dep(tratr,tratra);
	dep(tratrn,tratrj);
	dep(tratra,tratrn);
	int cotra=regtask("contratar trastero");
	dep(tratra,cotra);
	int llasadi=regtask("saber disponibilidad");
	dep(cotra,llasadi);
	dep(vi,ca);

	dep(cv,tra);
	int cajas=regtask("hacer cajas");
	dep(tratrj,cajas);

	int tro=regtask("trocear");
	dep(ca,cv);
	dep(trapl,tro);
	int dm=regtask("desmontar");
	int dmc=regtask("desmontar cama");
	int dme=regtask("desmontar estanterias");
	dep(dm,dmc);
	dep(dm,dme);
	dep(cv,dm);

	int fi=regtask("arreglos");
	dep(ca,fi);
	
	int vadorm=regtask("vaciar dormitorio");
	int pidorm=regtask("pintar dormitorio");
	dep(cv,vadorm);
	dep(pidorm,vadorm);
	int pica=regtask("pintar casa");
	dep(pica,pidorm);
	dep(fi,pica);

	int copi=regtask("comprar pintura");
	dep(pidorm,copi);
	dep(pica,copi);
	cost(copi,100);
	text(copi,"25 Kg x 3 ");

	int hu=regtask("humedades");
	dep(fi,hu);

	int go=regtask("goteras");
	dep(fi,go);
	int pt=regtask("puerta terraza");
	dep(fi,pt);
	int ap=regtask("agujero pasillo");
	dep(fi,ap);
	int lu=regtask("luces");
	dep(fi,lu);
	int ne=regtask("nevera");
	dep(fi,ne);
	int tico=regtask("tiradores cocina");
	dep(fi,tico);
	int civa=regtask("cisterna vater");
	dep(fi,civa);
	int kipa=regtask("comprar kits papel higienico");
	dep(fi,kipa);
	int pq=regtask("parquet");
	dep(fi,pq);
	dep(pq,pica);

	int cotico=regtask("comprar tiradores cocina");
	dep(tico,cotico);

	int ipt=regtask("investigar puerta terraza");
	int cppt=regtask("comprar piezas puerta terraza");
	dep(pt,cppt);
	dep(cppt,ipt);

	int cpcva=regtask("comprar pieza cisterna vater");
	dep(civa,cpcva);

	int potine=regtask("poner tiradores nevera");
	int pocane=regtask("poner cajones nevera");
	int cotine=regtask("comprar tiradores nevera");
	int cocane=regtask("comprar cajones nevera");
	dep(potine,cotine);
	dep(pocane,cocane);
	dep(ne,potine);
	dep(ne,pocane);


	int lupa=regtask("luces pasillo");
	int luha=regtask("luces hall");
	int luco=regtask("luces cocina");
	int luba=regtask("luces baños");
	dep(lu,lupa);
	dep(lu,luha);
	dep(lu,luco);
	dep(lu,luba);

	int colu=regtask("comprar luces");
	dep(lupa,colu);
	dep(luha,colu);
	dep(luco,colu);
	dep(luba,colu);

	auto f=[&](int id)-> string { 
		ostringstream os;
//		os << "\"";
		os << tasks.find(id)->second.name;
//		os << "\"";
		return os.str();
		};

	graph g(al);
	if (cmd=="dot") {
		g.dot(f, cout);
	}
	else if (cmd=="leafs") {
		leafs visitor(f);
		g.breath_first(id,visitor);
	}
}


int main(int argc, char** argv) {
	string cmd=argv[1];
	
	mp(cmd);
	return 0;
}


