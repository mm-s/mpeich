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
#include <fstream>
#include <functional>
#include <unordered_set>

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
	void dot(const P& pred, const function<bool(const vertex&)>& is_leaf, const function<bool(const vertex&)>& is_done, ostream& os) const {
		os << "digraph {" << endl;
		for (auto v: V) {
			os << "task" << v.second->id << "[label=<" << v.second->id << ": " << pred(v.second->id) << ">";
			if (is_done(*v.second)) {
				os << ",color=green";
			}
			else if (is_leaf(*v.second)) {
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
		dot(
		[](int i) -> string { return to_string(i); },
		[](const vertex&v) -> bool { return v.is_leaf(); },
		[](const vertex&) -> bool { return false; },
		os);
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
		void dump(const std::function <string (const vertex&)>& _t, ostream& os) const {
			for (auto i:*this) os << _t(*i) << endl;
		}
		void dump(ostream& os) const {
			for (auto i:*this) os << i->id << endl;
		}
		void reverse() {
			result r;
			r.reserve(size());
			for (auto i=rbegin(); i!=rend(); ++i) 
				r.push_back(*i);
			*this=r;
		}
	};
	result r;
	best_path(const graph&g_): g(g_) { //adjacency list + values
	}
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
		auto edi=values.find(&e);
		if (edi==values.end()) {
			values.emplace(&e,T());
			edi=values.find(&e);
		}
		auto& ed=edi->second;

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
		r.push_back(f);
		for (auto i=rp.rbegin(); i!=rp.rend(); ++i) {
			r.push_back(*i);
		}
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
	T value{1};
	scalar() { }
	scalar(const T&t): value(t) { }
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
	task(int id_, string n): name(n), duration(0), id(id_) {
		if (next_id<=id_) next_id=id_+1;
	}
	struct worker {
		worker(string name_, bool req): name(name_), required(req) {}
		string name;
		bool required;
	};
	vector<worker> workers;

	string get_workers() const {
		ostringstream os;
		for (auto& w: workers) {
			os << (w.required?"!":"") << w.name << " ";
		}
		return os.str();
	}


	void add_worker(string name, bool required) {
		workers.emplace_back(name,required);
	}
	int id;
	string name;
	int duration;
	int cost{0};
	vector<string> text;
	bool done{false};
	static int next_id;
};

int task::next_id=0;

#include <unordered_map>

unordered_map<int,task> tasks;
vector<pair<int,int>> al; //adjacency list

int goal=-1;

void load(istream& is) {
	while(!is.eof()) {
		string line;
		getline(is,line);
		if (line.empty()) continue;
		istringstream lis(line);
		string type;
		lis >> type;
		if (type=="task") {
			int id=0;
			lis >> id;
			if (goal==-1) goal=id;
			lis.ignore(1);
			string label;
			getline(lis,label);
			task t(id,label);
			tasks.emplace(id,t);
		}
		else if (type=="dep") {
			int to, from;
			string tkn;
			lis >> to;
			lis >> tkn;
			lis >> from;
			al.emplace_back(to,from);
		}
		else if (type=="cost") {
			int id;
			int v;
			lis >> id;
			lis >> v;
			tasks.find(id)->second.cost=v;
		}
		else if (type=="dur") {
			int id;
			int v;
			lis >> id;
			lis >> v;
			tasks.find(id)->second.duration=v;
		}
		else if (type=="text") {
			int id=0;
			lis >> id;
			string lin;
			getline(lis,lin);
			tasks.find(id)->second.text.push_back(lin);
		}
		else if (type=="done") {
			int id=0;
			lis >> id;
			tasks.find(id)->second.done=true;
		}
		else if (type=="who") {
			int id=0;
			lis >> id;
			string who;
			lis >> who;
			string code;
			lis >> code;
			tasks.find(id)->second.add_worker(who,code=="r");
		}
	}
}


int regtask(string name) {
	task t(name);
	tasks.emplace(t.id,t);
	cout << "task" << " " << t.id << " " << name << endl;
	return t.id;
}
void dep(int to, int from) {
	al.emplace_back(to,from);
	cout << "dep" << " " << to << " -> " << from << endl;
}
void cost(int id, int v) {
	tasks.find(id)->second.cost=v;
	cout << "cost" << " " << id << " " << v << endl;
}
void dur(int id, int v) {
	tasks.find(id)->second.duration=v;
	cout << "dur" << " " << id << " " << v << endl;
}
void text(int id, string v) {
	tasks.find(id)->second.text.push_back(v);
	cout << "text" << " " << id << " " << v << endl;
}
struct leafs:graph::visitor {
	function<bool (const vertex&)> _is_leaf;
	leafs(const function<bool (const vertex&v)>& f): _is_leaf(f) {
	}
	void start(const vertex&) override {
		_uniq.clear();
	}
	void visit(const vertex&v) override {
		if (!_is_leaf(v)) return;
		if (_uniq.find(v.id)!=_uniq.end()) return;
		_uniq.emplace(v.id);
	}
	unordered_set<int> _uniq;

	void dump(const std::function <string (int)>& _t, ostream& os) const {
		for (auto id: _uniq)
			cout << id << " " << _t(id) << endl;
	}
};

void update_level(int& lvl, const vertex& v) {
	int sz=0;
	for (auto e:v.e) {
		if (tasks.find(e->to->id)->second.done) continue;
		++sz;
	}
	if (sz==0) lvl=1;
	if (sz>=1 && lvl==1) lvl=2;
	if (sz>1 && lvl==2) lvl=3;


}

void mp(string cmd) {
	{
	ifstream is("data");
	load(is);
	}

	auto f=[&](int id)-> string { 
		ostringstream os;
//		os << "\"";
		os << tasks.find(id)->second.name;
//		os << "\"";
		return os.str();

		};

	auto is_leaf=[&](const vertex&v) -> bool {
		if (tasks.find(v.id)->second.done) return false; // esta fuera del grafo de undone
		if (v.is_leaf()) return true;
		for (auto& e:v.e) {
			if (!tasks.find(e->to->id)->second.done) return false;
		}
		return true;
	};

	graph g(al);
	if (cmd=="dot") {

		auto f=[&](int id)-> string { 
			ostringstream os;
			const auto& t=tasks.find(id)->second;
	//		os << "\"";
			os << t.name;
			os << "<BR /><FONT POINT-SIZE=\"10\">(" << t.get_workers() << ")</FONT>";
//			os << "\"";
			return os.str();

			};

		auto is_done=[&](const vertex&v) -> bool {
			return tasks.find(v.id)->second.done;
		};
		g.dot(f,is_leaf,is_done, cout);
	}
	else if (cmd=="leafs") {
		leafs visitor(is_leaf);
		g.breath_first(goal,visitor);
		visitor.dump(f,cout);
	}
	else if (cmd=="branches") {
		leafs visitor(is_leaf);
		g.breath_first(goal,visitor);
		
		for (auto lf:visitor._uniq) {
			int lvl=0;
			auto f=[&](const vertex& v)-> string { 
				ostringstream os;
				const auto& t=tasks.find(v.id)->second;
				update_level(lvl,v);
				os << "<div class=\"task" << lvl << "\">" << v.id << ": " << t.name;
				os << "</div>";
				return os.str();
			};




			cout << "<div class=\"branch\">" << endl;
			typedef best_path<scalar<int>,data> pathfinder;
			pathfinder bp(g);
			auto r=bp.compute(goal,lf,pathfinder::breath_first);
			r.reverse();
			r.dump(f,cout);
			cout << "</div>" << endl;
		}
		
	}

}


int main(int argc, char** argv) {
	if (argc!=2) {
		cout << "dot leafs branches" << endl;
		return 1;
	}
	string cmd=argv[1];
	
	mp(cmd);
	return 0;
}


