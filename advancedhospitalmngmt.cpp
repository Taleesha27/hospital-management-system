/*
Advanced Hospital Management System (C++)
Single-file prototype containing:
 - Trie-based autocomplete/search for patients & doctors
 - Graph utilities (Dijkstra) for schedule optimization example
 - LRU Cache for recent patient records
 - Segment Tree for bed/ICU availability queries/updates
 - Emergency Priority Queue (max-heap) for critical patients
*/

#include <bits/stdc++.h>
using namespace std;
using ll = long long;

// ----------------------- TRIE -----------------------
struct TrieNode {
    bool end = false;
    unordered_map<char, TrieNode*> nxt;
};

class Trie {
public:
    Trie() { 
        root = new TrieNode(); 
    }
    void insert(const string &s) {
        TrieNode* cur = root;
        for (char c: s) {
            if (!cur->nxt.count(c)) cur->nxt[c] = new TrieNode();
            cur = cur->nxt[c];
        }
        cur->end = true;
    }
    bool search(const string &s) {
        TrieNode* cur = root;
        for (char c: s) {
            if (!cur->nxt.count(c)) return false;
            cur = cur->nxt[c];
        }
        return cur->end;
    }
    vector<string> autocomplete(const string &prefix, int limit=10) {
        vector<string> res;
        TrieNode* cur = root;
        for (char c: prefix) {
            if (!cur->nxt.count(c)) return res;
            cur = cur->nxt[c];
        }
        string tmp = prefix;
        dfs(cur, tmp, res, limit);
        return res;
    }
private:
    TrieNode* root;
    void dfs(TrieNode* node, string &curStr, vector<string> &out, int limit) {
        if ((int)out.size() >= limit) return;
        if (node->end) out.push_back(curStr);
        for (auto &p: node->nxt) {
            curStr.push_back(p.first);
            dfs(p.second, curStr, out, limit);
            curStr.pop_back();
            if ((int)out.size() >= limit) return;
        }
    }
};

// ----------------------- LRU CACHE -----------------------
template<typename K, typename V>
class LRUCache {
public:
    LRUCache(size_t cap): capacity(cap) {}
    void put(const K &k, const V &v) {
        if (mp.count(k)) {
            auto it = mp[k];
            it->second = v;
            touch(it);
            return;
        }
        if (dq.size() == capacity) {
            auto last = dq.back(); 
            dq.pop_back(); 
            mp.erase(last.first);
        }
        dq.emplace_front(k, v);
        mp[k] = dq.begin();
    }
    bool get(const K &k, V &out) {
        if (!mp.count(k)) return false;
        auto it = mp[k];
        out = it->second;
        touch(it);
        return true;
    }
    void display() {
        cout << "LRU (most->least): ";
        for (auto &p: dq) cout << "["<<p.first<<":"<<p.second<<"] ";
        cout << "\n";
    }
private:
    list<pair<K,V>> dq;
    unordered_map<K, typename list<pair<K,V>>::iterator> mp;
    size_t capacity;
    void touch(typename list<pair<K,V>>::iterator it) {
        auto pr = *it;
        dq.erase(it);
        dq.emplace_front(pr);
        mp[pr.first] = dq.begin();
    }
};

// ----------------------- SEGMENT TREE (sum) -----------------------
class SegmentTree {
public:
    SegmentTree() {}
    SegmentTree(const vector<int> &arr) { 
        build(arr); 
    }
    void build(const vector<int> &arr) {
        n = arr.size();
        tree.assign(4*n, 0);
        buildRec(arr,1,0,n-1);
    }
    void update(int idx, int val) { 
        updateRec(1,0,n-1,idx,val); 
    }
    int query(int l, int r) { 
        return queryRec(1,0,n-1,l,r); 
    }
private:
    int n;
    vector<int> tree;
    void buildRec(const vector<int> &a, int node, int l, int r) {
        if (l==r) { 
            tree[node]=a[l]; 
            return; 
        }
        int mid=(l+r)/2;
        buildRec(a,node*2,l,mid);
        buildRec(a,node*2+1,mid+1,r);
        tree[node]=tree[node*2]+tree[node*2+1];
    }
    void updateRec(int node,int l,int r,int idx,int val){
        if (l==r){ 
            tree[node]=val; 
            return; 
        }
        int mid=(l+r)/2;
        if (idx<=mid) updateRec(node*2,l,mid,idx,val);
        else updateRec(node*2+1,mid+1,r,idx,val);
        tree[node]=tree[node*2]+tree[node*2+1];
    }
    int queryRec(int node,int l,int r,int ql,int qr){
        if (qr<l || ql>r) return 0;
        if (ql<=l && r<=qr) return tree[node];
        int mid=(l+r)/2;
        return queryRec(node*2,l,mid,ql,qr)+queryRec(node*2+1,mid+1,r,ql,qr);
    }
};

// ----------------------- GRAPH (Dijkstra) -----------------------
class Graph {
public:
    Graph(int n): n(n) { 
        adj.assign(n, {}); 
    }
    void addEdge(int u,int v,int w){ 
        adj[u].push_back({v,w}); 
    }
    vector<long long> dijkstra(int src){
        const long long INF = (1LL<<60);
        vector<long long> dist(n, INF);
        priority_queue<pair<long long,int>, vector<pair<long long,int>>, greater<pair<long long,int>>> pq;
        dist[src]=0; 
        pq.push({0,src});
        while(!pq.empty()){
            auto [d,u]=pq.top(); 
            pq.pop();
            if (d!=dist[u]) continue;
            for (auto &e: adj[u]){
                int v=e.first; 
                int w=e.second;
                if (dist[v] > dist[u]+w){ 
                    dist[v]=dist[u]+w; 
                    pq.push({dist[v],v}); 
                }
            }
        }
        return dist;
    }
private:
    int n;
    vector<vector<pair<int,int>>> adj;
};

// ----------------------- EMERGENCY QUEUE -----------------------
struct EmergencyPatient {
    string name;
    int severity;
    long long ts;
};
struct EMComp { 
    bool operator()(const EmergencyPatient &a, const EmergencyPatient &b) const {
        if (a.severity != b.severity) return a.severity < b.severity;
        return a.ts > b.ts;
    }
};

// ----------------------- SAMPLE DATA STRUCTS -----------------------
struct Patient {
    string id, name; 
    int age;
    string history;
};

// ----------------------- MAIN INTEGRATION -----------------------
int main(){

    Trie patientTrie, doctorTrie;
    LRUCache<string,string> lru(5);

    vector<int> beds(10,1);
    SegmentTree seg(beds);

    Graph g(6);
    g.addEdge(0,1,2); 
    g.addEdge(0,2,5); 
    g.addEdge(1,2,1);
    g.addEdge(1,3,4);
    g.addEdge(2,4,2); 
    g.addEdge(3,5,3); 
    g.addEdge(4,5,1);

    priority_queue<EmergencyPatient, vector<EmergencyPatient>, EMComp> emq;
    long long global_ts = 0;

    vector<Patient> patients = { {"P1001","Alice",30,"Diabetes"}, {"P1002","Bob",45,"Asthma"}, {"P1003","Ravi",60,"BP"} };
    vector<string> doctors = {"DrAmit","DrSana","DrMoyeon","DrRavi"};

    for (auto &p: patients) patientTrie.insert(p.name);
    for (auto &d: doctors) doctorTrie.insert(d);

    unordered_map<string, Patient> patientDB;
    for (auto &p: patients) patientDB[p.id] = p;

    while (true) {
        cout << "\n=== Advanced Hospital Management ===\n";
        cout << "1) Add patient\n2) Search (Trie autocomplete)\n3) View patient record (LRU cache)\n";
        cout << "4) Schedule optimization (Dijkstra demo)\n5) Bed queries (Segment Tree)\n6) Emergency queue\n7) Exit\n";
        cout << "Choose option: ";
        int opt; 
        if (!(cin>>opt)) break;
        if (opt==1){
            Patient p; 
            cout<<"Enter id name age history:\n"; 
            cin>>p.id>>p.name>>p.age; 
            cin.ignore(); 
            getline(cin,p.history);
            patientDB[p.id]=p; 
            patientTrie.insert(p.name);
            cout<<"Patient added: "<<p.id<<" "<<p.name<<"\n";
        } 
        else if (opt==2){
            cout<<"Search prefix: "; 
            string pref; 
            cin>>pref;
            auto res = patientTrie.autocomplete(pref, 10);
            cout<<"Patient matches: \n"; 
            for (auto &s: res) cout<<" - "<<s<<"\n";
            auto dres = doctorTrie.autocomplete(pref, 10);
            if (!dres.empty()){ 
                cout<<"Doctor matches: \n"; 
                for (auto &s: dres) cout<<" - "<<s<<"\n"; 
            }
        } 
        else if (opt==3){
            cout<<"Enter patient id to view: "; 
            string id; 
            cin>>id;
            if (patientDB.count(id)){
                Patient &p = patientDB[id];
                lru.put(id, p.name);
                cout<<"Patient: "<<p.id<<" "<<p.name<<" Age:"<<p.age<<" History:"<<p.history<<"\n";
                lru.display();
            } 
            else cout<<"Not found\n";
        } 
        else if (opt==4){
            cout<<"Dijkstra demo: enter source node (0..5): "; 
            int s; 
            cin>>s;
            auto dist = g.dijkstra(s);
            cout<<"Distances from "<<s<<" :\n";
            for (int i=0;i<(int)dist.size();++i){
                if (dist[i] > (1LL<<50)) cout<<i<<": INF\n"; 
                else cout<<i<<":"<<dist[i]<<"\n";
            }
            cout<<"(Interpret nodes as rooms/time-slot ids in a real model)\n";
        } 
        else if (opt==5){
            cout<<"Beds total: "<<beds.size()<<" Free in [0,9]: "<<seg.query(0,beds.size()-1)<<"\n";
            cout<<"1) occupy bed  2) free bed  3) range query  4) show all\nChoose: "; 
            int t;
            cin>>t;
            if (t==1){ 
                int idx; 
                cin>>idx; 
                if (idx>=0 && idx<(int)beds.size()){ 
                    beds[idx]=0; 
                    seg.update(idx,0); 
                    cout<<"Occupied "<<idx<<"\n";
                } 
            }
            else if (t==2){ 
                int idx; 
                cin>>idx; 
                if (idx>=0 && idx<(int)beds.size()){ 
                    beds[idx]=1; 
                    seg.update(idx,1); 
                    cout<<"Freed "<<idx<<"\n";
                } 
            }
            else if (t==3){ 
                int l,r; 
                cin>>l>>r; 
                cout<<"Free beds in ["<<l<<","<<r<<"] = "<<seg.query(l,r)<<"\n"; 
            }
            else if (t==4){ 
                cout<<"Beds: "; 
                for (int i=0;i<(int)beds.size();++i) cout<<beds[i]; 
                cout<<"\n"; 
            }
        } 
        else if (opt==6){
            cout<<"Emergency: 1)add 2)pop 3)peek 4)size\nChoose: "; 
            int c; 
            cin>>c;
            if (c==1){ 
                string name; 
                int sev; 
                cout<<"name severity(1..5): "; 
                cin>>name>>sev; 
                emq.push({name,sev,++global_ts}); 
                cout<<"Added\n"; 
            }
            else if (c==2){ 
                if (emq.empty()) cout<<"Empty\n"; 
                else { 
                    auto p=emq.top(); 
                    emq.pop(); 
                    cout<<"Handling: "<<p.name<<" severity="<<p.severity<<"\n"; 
                } 
            }
            else if (c==3){ 
                if (emq.empty()) cout<<"Empty\n"; 
                else { 
                    auto p=emq.top(); 
                    cout<<"Next: "<<p.name<<" severity="<<p.severity<<"\n"; 
                } 
            }
            else if (c==4) cout<<"Queue size: "<<emq.size()<<"\n";
        } 
        else if (opt==7) { 
            cout<<"Exiting...\n"; 
            break; 
        }
        else cout<<"Invalid\n";
    }
    return 0;
}
