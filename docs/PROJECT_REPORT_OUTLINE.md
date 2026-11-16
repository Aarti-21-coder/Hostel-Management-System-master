# Formal Project Report Outline: Hostel Management System (DSA-Focused)

## Title Page
- **Project Title**: Design and Implementation of an Enterprise Hostel Management System Utilizing Custom High-Performance Data Structures and Algorithms in C++17
- **Course**: Advanced Data Structures & System Design
- **Authors**: Senior Engineering Team

---

## 1. Executive Summary & Problem Statement
- **1.1 Background**: Traditional hostel management systems suffer from high latency, database bottlenecks, and lack of granular algorithmic optimization when scheduling rooms, balancing priorities, and handling thousands of dynamic registrations.
- **1.2 Problem Statement**: Building an end-to-end hostel controller in pure modern C++17 without external database engines, relying exclusively on custom from-scratch data structures, and delivering sub-millisecond transactions for up to 100,000 students.
- **1.3 Key Innovations**:
  - Zero third-party container dependencies (all data structures engineered from scratch).
  - Dual-tier waiting queue (Binary Max-Heap + Circular FIFO Ring Buffer).
  - Automatic zero-downtime bed reallocation on room vacation.
  - PBKDF2-HMAC-SHA256 cryptographic authentication with progressive brute-force lockout.
  - Multi-threaded REST API server with interactive glassmorphic web dashboard and real-time DSA benchmark suite.

---

## 2. System Architecture & Data Flow Design (DFD)
- **2.1 Context Level 0 DFD**: Interaction boundary between Hostel Administrators, Students, Wardens, and Security Guards.
- **2.2 Level 1 Process Decomposition**:
  - Process 1.0: Central Controller & Request Dispatcher.
  - Process 2.0: Student Registration & Composite Priority Score Calculation.
  - Process 3.0: Room Allocation & Availability Manager.
  - Process 4.0: View Reports, Statistical Analytics & Slicing.
  - Process 5.0: Vacate & Instant Auto-Reallocation Flow.
  - Process 6.0: Transactional Undo/Redo Engine.
- **2.3 Level 2 Detailed Flowcharts**:
  - Registration pipeline with duplicate detection (Bloom Filter + Hash Table).
  - Strict Priority-First room assignment state machine.
  - Cross-structure atomic consistency during transfers and cancellations.

---

## 3. Data Structures & Algorithms Engineered from Scratch
Detailed theoretical and implementation analysis for all 17 custom data structures:
- **3.1 Dynamic Array (`Vector<T>`)**: Amortized $O(1)$ dynamic growth factor, memory layout, iterator protocol.
- **3.2 Hash Table (`HashTable<K, V>`)**: Separate chaining with prime modulo table sizes, FNV-1a hashing, dynamic load-factor monitoring ($\alpha \ge 0.75$), and rehashing.
- **3.3 Binary Heap (`BinaryHeap<T, Compare>`)**: Array-based binary tree with sift-up and sift-down; Min-Heap for complaints SLA and Max-Heap for priority waiting lists.
- **3.4 Circular Queue (`CircularQueue<T>`)**: Ring buffer using modulo arithmetic, front/rear pointers, bounded capacity enforcement.
- **3.5 Stack (`Stack<T>`)**: LIFO transactional stack for the Command Pattern (Undo/Redo).
- **3.6 Doubly Linked List (`DoublyLinkedList<T>`)**: Bidirectional node references for $O(1)$ node-level deletions in active allocation stores.
- **3.7 Balanced AVL Tree (`AVLTree<K, V>`)**: Height balancing, 4 rotation primitives (LL, RR, LR, RL), $O(\log N + k)$ range queries.
- **3.8 B+ Tree (`BPlusTree<K, V>`)**: High-fanout order-M balanced tree with sequential leaf pointers for disk-friendly storage.
- **3.9 Segment Tree (`SegmentTree<T>`)**: Recursive balanced tree over array intervals for dynamic fee range queries ($O(\log N)$) and updates ($O(\log N)$).
- **3.10 Fenwick Tree (`FenwickTree<T>`)**: Binary Indexed Tree using low-bit masking for space-optimal prefix sums.
- **3.11 Interval Tree (`IntervalTree<T>`)**: Augmented BST tracking subtree maximum endpoints for room booking conflict detection.
- **3.12 Disjoint Set Union (`DisjointSet`)**: Union-Find with path compression and union by rank for roommate affinity clustering.
- **3.13 Bloom Filter (`BloomFilter`)**: Bit-array with double-hashing (FNV-1a + Murmur3) for zero false-negative duplicate detection.
- **3.14 LRU Cache (`LRUCache<K, V>`)**: Hash table coupled with Doubly Linked List for $O(1)$ cache recency eviction.
- **3.15 Graph (`Graph`)**: Adjacency list representation with Dijkstra's algorithm for shortest campus navigation.
- **3.16 Prefix Tree (`Trie`)**: 128-ary ASCII tree for instant autocomplete and Levenshtein typo-tolerant fuzzy matching.
- **3.17 Exact String Matching (`StringSearch`)**: Knuth-Morris-Pratt (LPS array) and Rabin-Karp rolling hash algorithms.

---

## 4. Security & Cryptographic Subsystem
- **4.1 Password Hashing**: PBKDF2-HMAC-SHA256 with 10,000 iterations and unique 16-byte random salts per user.
- **4.2 Role-Based Access Control (RBAC)**: Distinct permission matrices for Admin, Warden, Student, and Guard.
- **4.3 Brute-Force Countermeasures**: Automatic account lockout for 15 minutes after 5 consecutive failed login attempts.
- **4.4 Session Management**: Cryptographically secure 24-byte hex tokens with expiration validation.

---

## 5. Storage & Persistence Architecture
- **5.1 Zero Database Overhead**: Completely standalone binary file storage.
- **5.2 Fast Binary Serializer (`BinaryStorage`)**: Direct binary stream dumping (`data/hostel_state.bin`) with magic header validation.
- **5.3 Synthetic Big Dataset Generator (`DataGenerator`)**: Procedural generation of 1K, 10K, and 100K realistic students and 2000+ rooms.

---

## 6. Web Interface & UI/UX Design
- **6.1 Modern Glassmorphism System**: CSS backdrop-filter blur, CSS custom properties for instant dark/light switching.
- **6.2 Interactive Floor Map**: Color-coded room status grid per block and floor with real-time bed visualization.
- **6.3 Chart.js Visualizations**: Live bed occupancy doughnut, status distribution bars, and DSA benchmark curves.

---

## 7. Verification, Testing & Empirical Results
- **7.1 Unit Testing Suite**: Automated tests for duplicate IDs, full queues, full rooms, cancellations, transfers, auto-reallocations, and undo/redo stacks.
- **7.2 Empirical Benchmark Analysis**:
  - Linear Search vs Hash Table vs AVL Tree execution times across $N = 100$ to $N = 2500$.
  - QuickSort vs MergeSort vs HeapSort comparison and swap metrics.

---

## 8. Conclusion & Future Scope
- **8.1 Summary**: Achievement of all functional requirements and performance objectives.
- **8.2 Future Scope**: WebSocket-based live room occupancy broadcasting, biometric fingerprint integration.
