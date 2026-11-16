#include <iostream>
#include <cassert>
#include <string>

#include "dsa/AVLTree.hpp"
#include "dsa/BPlusTree.hpp"
#include "dsa/SegmentTree.hpp"
#include "dsa/FenwickTree.hpp"
#include "dsa/IntervalTree.hpp"
#include "dsa/DisjointSet.hpp"
#include "dsa/BloomFilter.hpp"
#include "dsa/LRUCache.hpp"
#include "dsa/Graph.hpp"
#include "dsa/Trie.hpp"
#include "dsa/LinkedList.hpp"
#include "dsa/StringSearch.hpp"

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "\n[ADVANCED DSA TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)

#define TEST_CASE(name) \
    std::cout << "Testing: " << name << " ... " << std::flush

#define TEST_PASS() \
    std::cout << "[PASSED]" << std::endl

void testAVLTree() {
    TEST_CASE("AVLTree (Insert, Balancing, Erase, Range Query)");
    dsa::AVLTree<float, std::string> tree;

    // Insert student CGPAs
    tree.insert(8.5f, "Alice");
    tree.insert(9.2f, "Bob");
    tree.insert(7.1f, "Charlie");
    tree.insert(6.4f, "David");
    tree.insert(9.8f, "Eva");
    tree.insert(8.0f, "Frank");

    TEST_ASSERT(tree.size() == 6, "Tree size must be 6");
    TEST_ASSERT(tree.contains(9.2f), "Must contain Bob (9.2)");
    TEST_ASSERT(*tree.find(9.2f) == "Bob", "Find Bob");

    // Range Query: CGPA between 8.0 and 9.5
    auto inRange = tree.findRange(8.0f, 9.5f);
    TEST_ASSERT(inRange.size() == 3, "Range [8.0, 9.5] should match Frank(8.0), Alice(8.5), Bob(9.2)");

    // Erase
    bool erased = tree.erase(7.1f);
    TEST_ASSERT(erased, "Charlie erased");
    TEST_ASSERT(!tree.contains(7.1f), "Must not contain Charlie anymore");
    TEST_ASSERT(tree.size() == 5, "Size decremented to 5");

    TEST_PASS();
}

void testBPlusTree() {
    TEST_CASE("BPlusTree (Multi-way Indexing & Leaf Range Scan)");
    dsa::BPlusTree<int, std::string, 4> bpt;

    for (int i = 1; i <= 20; ++i) {
        bpt.insert(i * 10, "Val_" + std::to_string(i * 10));
    }

    TEST_ASSERT(bpt.size() == 20, "B+ Tree size must be 20");
    std::string* v = bpt.find(50);
    TEST_ASSERT(v != nullptr && *v == "Val_50", "Find key 50");

    // Range Query from 40 to 90
    auto rangeRes = bpt.findRange(40, 90);
    TEST_ASSERT(rangeRes.size() == 6, "Range [40, 90] should contain 40, 50, 60, 70, 80, 90");
    TEST_ASSERT(rangeRes[0].first == 40, "First in range is 40");
    TEST_ASSERT(rangeRes[5].first == 90, "Last in range is 90");

    TEST_PASS();
}

void testSegmentTree() {
    TEST_CASE("SegmentTree (Dynamic Fee Range Sum & Point Update)");
    float fees[] = {5000.0f, 6500.0f, 4500.0f, 9000.0f, 7000.0f};
    dsa::SegmentTree<float> seg(fees, 5);

    // Range sum of student 1 to 3: 6500 + 4500 + 9000 = 20000
    float sum1_3 = seg.query(1, 3);
    TEST_ASSERT(sum1_3 == 20000.0f, "Range sum [1, 3] must be 20000");

    // Point update: student 2 pays fee -> becomes 0
    seg.update(2, 0.0f);
    float sumAfter = seg.query(1, 3);
    TEST_ASSERT(sumAfter == 15500.0f, "Range sum [1, 3] after update must be 15500");

    TEST_PASS();
}

void testFenwickTree() {
    TEST_CASE("FenwickTree (Prefix Sum & Point Update)");
    dsa::FenwickTree<int> fen(10);

    fen.update(1, 10);
    fen.update(2, 20);
    fen.update(3, 30);
    fen.update(4, 40);

    TEST_ASSERT(fen.query(3) == 60, "Prefix sum [1, 3] must be 60");
    TEST_ASSERT(fen.rangeQuery(2, 4) == 90, "Range sum [2, 4] must be 90");

    TEST_PASS();
}

void testIntervalTree() {
    TEST_CASE("IntervalTree (Booking & Leave Conflict Detection)");
    dsa::IntervalTree<int> itree;

    itree.insert(10, 20, "Booking_A");
    itree.insert(25, 35, "Booking_B");
    itree.insert(40, 50, "Booking_C");

    // Overlapping query: [15, 22] overlaps with Booking_A
    dsa::Interval<int> q1(15, 22);
    TEST_ASSERT(itree.hasOverlap(q1), "[15, 22] must detect overlap with [10, 20]");

    // Non-overlapping query: [21, 24] does not overlap
    dsa::Interval<int> q2(21, 24);
    TEST_ASSERT(!itree.hasOverlap(q2), "[21, 24] should have no conflict");

    TEST_PASS();
}

void testDisjointSet() {
    TEST_CASE("DisjointSet (Union-Find Roommate Clustering)");
    dsa::DisjointSet dsu(6); // 6 students: 0 to 5

    // Cluster 0, 1, 2 together
    dsu.unite(0, 1);
    dsu.unite(1, 2);

    // Cluster 3, 4 together
    dsu.unite(3, 4);

    TEST_ASSERT(dsu.connected(0, 2), "0 and 2 must be connected");
    TEST_ASSERT(!dsu.connected(0, 3), "0 and 3 must not be connected");
    TEST_ASSERT(dsu.countSets() == 3, "Total clusters: {0,1,2}, {3,4}, {5}");

    TEST_PASS();
}

void testBloomFilter() {
    TEST_CASE("BloomFilter (Duplicate Email/Phone Fast Check)");
    dsa::BloomFilter bf(10000, 5);

    bf.insert("aarav@example.com");
    bf.insert("+919876543210");

    // Zero false negatives guaranteed:
    TEST_ASSERT(bf.contains("aarav@example.com"), "Must contain aarav@example.com");
    TEST_ASSERT(bf.contains("+919876543210"), "Must contain phone number");
    TEST_ASSERT(!bf.contains("stranger@unknown.com"), "Must not contain stranger email");

    TEST_PASS();
}

void testLRUCache() {
    TEST_CASE("LRUCache (O(1) Get/Put & Eviction)");
    dsa::LRUCache<std::string, std::string> cache(3);

    cache.put("STU1", "Profile 1");
    cache.put("STU2", "Profile 2");
    cache.put("STU3", "Profile 3");

    // Access STU1 to make it most recently used
    TEST_ASSERT(cache.get("STU1") != nullptr, "STU1 must exist");

    // Insert STU4 -> should evict STU2 (the least recently used!)
    cache.put("STU4", "Profile 4");

    TEST_ASSERT(cache.get("STU2") == nullptr, "STU2 must have been evicted");
    TEST_ASSERT(cache.get("STU1") != nullptr, "STU1 was preserved");
    TEST_ASSERT(cache.get("STU3") != nullptr, "STU3 was preserved");
    TEST_ASSERT(cache.get("STU4") != nullptr, "STU4 exists");

    TEST_PASS();
}

void testGraph() {
    TEST_CASE("Graph (Campus Layout & Dijkstra Shortest Path)");
    dsa::Graph campus;

    int bA = campus.addNode("Block A", "Hostel Block");
    int bB = campus.addNode("Block B", "Hostel Block");
    int mess = campus.addNode("Dining Hall / Mess", "Mess");
    int gate = campus.addNode("Main Gate", "Gate");
    int lib = campus.addNode("Central Library", "Amenity");

    campus.addEdge(bA, bB, 80.0f, "Corridor Walk");
    campus.addEdge(bB, mess, 120.0f, "Avenue Path");
    campus.addEdge(bA, mess, 250.0f, "Direct Outer Road");
    campus.addEdge(mess, lib, 90.0f, "Green Park Path");
    campus.addEdge(lib, gate, 150.0f, "Campus Boulevard");

    // Shortest path from Block A to Dining Hall: Block A -> Block B -> Mess = 80 + 120 = 200m
    auto route = campus.shortestPath(bA, mess);
    TEST_ASSERT(route.reachable, "Route must be reachable");
    TEST_ASSERT(route.totalDistance == 200.0f, "Shortest path must be 200m via Block B");
    TEST_ASSERT(route.pathNodeNames.size() == 3, "Path must have 3 nodes: Block A -> Block B -> Mess");

    TEST_PASS();
}

void testTrie() {
    TEST_CASE("Trie (Autocomplete & Levenshtein Typo-Tolerance)");
    dsa::Trie trie;

    trie.insert("Aarav", "STU101");
    trie.insert("Aaradhya", "STU102");
    trie.insert("Aaryan", "STU103");
    trie.insert("Bob", "STU104");

    // Autocomplete with prefix "aar"
    auto autoRes = trie.autocomplete("aar");
    TEST_ASSERT(autoRes.size() == 3, "Autocomplete for 'aar' should match 3 students");

    // Typo-tolerant search for "Arav" (missing an 'a', edit distance 1 from 'aarav')
    auto fuzzyRes = trie.searchWithTypoTolerance("Arav", 1);
    TEST_ASSERT(!fuzzyRes.empty(), "Should find 'Aarav' despite typo");

    TEST_PASS();
}

void testLinkedList() {
    TEST_CASE("LinkedList (Singly-Linked Event Log)");
    dsa::LinkedList<std::string> log;

    log.push_back("Event 1: Gate Open");
    log.push_back("Event 2: Student Entry");
    log.push_back("Event 3: Gate Close");

    TEST_ASSERT(log.size() == 3, "Log size must be 3");

    log.pop_front();
    TEST_ASSERT(log.size() == 2, "Log size decremented to 2");

    TEST_PASS();
}

void testStringSearch() {
    TEST_CASE("StringSearch (KMP & Rabin-Karp)");
    std::string text = "HOSTEL_STUDENT_ALLOCATION_SYSTEM_HOSTEL";
    std::string pattern = "ALLOCATION";

    int kmpIdx = dsa::StringSearch::kmpFind(text, pattern);
    int rkIdx = dsa::StringSearch::rabinKarpFind(text, pattern);

    TEST_ASSERT(kmpIdx == 15, "KMP must find match at index 15");
    TEST_ASSERT(rkIdx == 15, "Rabin-Karp must find match at index 15");

    TEST_PASS();
}

int main() {
    std::cout << "\n==============================================" << std::endl;
    std::cout << "HOSTEL MANAGEMENT SYSTEM - ADVANCED DSA TESTS" << std::endl;
    std::cout << "==============================================\n" << std::endl;

    testAVLTree();
    testBPlusTree();
    testSegmentTree();
    testFenwickTree();
    testIntervalTree();
    testDisjointSet();
    testBloomFilter();
    testLRUCache();
    testGraph();
    testTrie();
    testLinkedList();
    testStringSearch();

    std::cout << "\n==============================================" << std::endl;
    std::cout << "ALL 12 ADVANCED DSA TEST SUITES PASSED (100%)" << std::endl;
    std::cout << "==============================================\n" << std::endl;

    return 0;
}
