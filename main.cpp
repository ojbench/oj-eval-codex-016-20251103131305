// Problem 016 - File Storage BPT (Persistent KV store)
// Implementation: In-memory map persisted to a binary file between runs.
// Keys: string (<=64 bytes); Values: int; No duplicate values per key.

#include <bits/stdc++.h>
using namespace std;

namespace kvstore {

static constexpr uint32_t MAGIC = 0x4B565354u; // 'KVST'
static constexpr uint32_t VERSION = 1u;

struct Header {
    uint32_t magic = MAGIC;
    uint32_t version = VERSION;
    uint32_t reserved = 0u;
    uint64_t count = 0u; // total number of pairs
};

// Read whole file content into structure: map<string, vector<int>> (sorted unique)
static void load(const string &path, unordered_map<string, vector<int>> &store) {
    store.clear();
    ifstream fin(path, ios::binary);
    if (!fin.good()) return; // no file yet

    Header h{};
    fin.read(reinterpret_cast<char*>(&h), sizeof(h));
    if (!fin || h.magic != MAGIC || h.version != VERSION) {
        return; // treat as empty if invalid
    }
    for (uint64_t i = 0; i < h.count; ++i) {
        uint16_t klen = 0;
        fin.read(reinterpret_cast<char*>(&klen), sizeof(klen));
        if (!fin) break;
        string key;
        key.resize(klen);
        if (klen) fin.read(&key[0], klen);
        int32_t val = 0;
        fin.read(reinterpret_cast<char*>(&val), sizeof(val));
        if (!fin) break;
        auto &vec = store[key];
        // maintain sorted unique insert
        auto it = lower_bound(vec.begin(), vec.end(), val);
        if (it == vec.end() || *it != val) vec.insert(it, val);
    }
}

static void save(const string &path, const unordered_map<string, vector<int>> &store) {
    // Count total pairs
    uint64_t total = 0;
    for (const auto &kv : store) total += kv.second.size();

    ofstream fout(path, ios::binary | ios::trunc);
    Header h{};
    h.count = total;
    fout.write(reinterpret_cast<const char*>(&h), sizeof(h));
    if (!fout) return;

    for (const auto &kv : store) {
        const string &key = kv.first;
        const vector<int> &vals = kv.second;
        uint16_t klen = static_cast<uint16_t>(min<size_t>(key.size(), 65535));
        for (int v : vals) {
            fout.write(reinterpret_cast<const char*>(&klen), sizeof(klen));
            if (klen) fout.write(key.data(), klen);
            int32_t val = v;
            fout.write(reinterpret_cast<const char*>(&val), sizeof(val));
        }
    }
}

} // namespace kvstore

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    const string dbfile = "kv_store.bin";
    unordered_map<string, vector<int>> store;
    store.reserve(400000);
    kvstore::load(dbfile, store);

    int n;
    if (!(cin >> n)) return 0;
    string cmd;
    string key;
    key.reserve(64);
    for (int i = 0; i < n; ++i) {
        cin >> cmd;
        if (cmd[0] == 'i') { // insert
            int v; cin >> key >> v;
            auto &vec = store[key];
            auto it = lower_bound(vec.begin(), vec.end(), v);
            if (it == vec.end() || *it != v) vec.insert(it, v);
        } else if (cmd[0] == 'd') { // delete
            int v; cin >> key >> v;
            auto itK = store.find(key);
            if (itK != store.end()) {
                auto &vec = itK->second;
                auto it = lower_bound(vec.begin(), vec.end(), v);
                if (it != vec.end() && *it == v) vec.erase(it);
                if (vec.empty()) store.erase(itK);
            }
        } else { // find
            cin >> key;
            auto itK = store.find(key);
            if (itK == store.end() || itK->second.empty()) {
                cout << "null\n";
            } else {
                const auto &vec = itK->second;
                for (size_t j = 0; j < vec.size(); ++j) {
                    if (j) cout << ' ';
                    cout << vec[j];
                }
                cout << '\n';
            }
        }
    }

    kvstore::save(dbfile, store);
    return 0;
}

