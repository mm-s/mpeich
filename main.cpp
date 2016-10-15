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
#include <mstd/graph>

using namespace std;
using namespace mstd;

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
	if (!is.good()) return;
	int ln=0;
	while(!is.eof()) {
		string line;
		getline(is,line);
		//cerr << "line " << ++ln << endl;
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

int main(int argc, char** argv) {
	string filename="data";
	string cmd;
	for (int i=1; i<argc; ++i) {
		string item=argv[i];
		if (item=="-f") {
			++i;
			if (i>=argc) break;
			filename=argv[i];
			continue;
		}
		cmd=argv[i];
		ifstream is(filename);
		load(is);
		break;
	}

	auto f=[&](int id)-> string { 
		ostringstream os;
		os << tasks.find(id)->second.name;
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
			os << t.name;
			os << "<BR /><FONT POINT-SIZE=\"10\">(" << t.get_workers() << ")</FONT>";
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
	else {
		cout << "mpeich [-f <datafile>] <command>:" << endl;
		cout << "generates dependency information on the tasks defined in the data file" << endl;
		cout << "commands:" << endl;
		cout << " dot        generates dot file" << endl;
		cout << " leafs      print blocking tasks" << endl;
		cout << " branches   print all task branches" << endl;
		return 1;
	}

	return 0;
}




