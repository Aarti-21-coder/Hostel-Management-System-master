# Top 10 Technical Viva Questions and In-Depth Answers

This guide prepares students and engineers for rigorous viva voce examinations and technical defense of the **Hostel Management System**.

---

### Q1: Why did you choose a Binary Max-Heap for the Priority Waiting List instead of keeping a sorted array/vector or linked list?
**Answer:**
- In a sorted array/vector, finding the maximum element is $O(1)$, but inserting a new student requires $O(N)$ due to shifting elements.
- In an unsorted array, insertion is $O(1)$, but finding and extracting the highest priority student requires an $O(N)$ linear scan.
- A **Binary Max-Heap** provides an optimal balance:
  - Extracting the highest priority student (`pop`): $O(\log N)$.
  - Inserting a new student (`push`): $O(\log N)$.
  - Inspecting the top candidate (`top`): $O(1)$.
- In our system, high-priority students (senior year, high CGPA, long travel distance, special category) are inserted dynamically and must be dispatched quickly. The binary heap ensures guaranteed logarithmic scalability even with tens of thousands of applicants.

---

### Q2: How does the Circular Queue prevent the queue drift / false-overflow problem inherent in linear array queues?
**Answer:**
- In a naive linear array queue, dequeuing elements advances the `front` pointer forward. Once `rear` reaches the end of the array, no further elements can be enqueued even if earlier slots were freed by previous dequeues, causing false overflow.
- A **Circular Queue** solves this by treating the storage buffer as a ring using modulo arithmetic:
  $$\text{rear} = (\text{rear} + 1) \pmod{\text{capacity}}$$
  $$\text{front} = (\text{front} + 1) \pmod{\text{capacity}}$$
- This achieves $O(1)$ constant time enqueue and dequeue operations without shifting any elements, maintaining optimal space utilization within a fixed memory footprint. In our system, it powers the Normal Waiting List and the SOS emergency alert queue.

---

### Q3: What is the time complexity of vacating a room, and how is $O(1)$ removal achieved in the Allocated Students DB?
**Answer:**
- In a singly linked list or vector, deleting an arbitrary student requires traversing to find the node and shifting following elements ($O(N)$ time).
- In our system, the **Allocated Students DB** is engineered as a **Doubly Linked List** (`DoublyLinkedList<T>`).
- When a student is allocated, a node reference is stored or easily retrieved. Because each node maintains both `prev` and `next` pointers, unlinking a node can be performed in **strict $O(1)$ constant time**:
  ```cpp
  node->prev->next = node->next;
  node->next->prev = node->prev;
  delete node;
  ```
- Combined with $O(1)$ Hash Table updates on the room and student status records, the entire vacation process executes in $O(1)$ time.

---

### Q4: How is immediate automatic reallocation handled upon room vacation without causing data inconsistencies?
**Answer:**
- When an admin invokes `vacateStudent(studentId)`, the following atomic sequence executes within a single transaction:
  1. Student status is updated to `VACATED` and `allocatedRoomId` is cleared.
  2. The room's `occupiedCount` is decremented by 1, and its status resets to `AVAILABLE`.
  3. The student's node is removed from the `DoublyLinkedList` in $O(1)$.
  4. The **Auto-Allocation Trigger** executes immediately:
     - It checks the `BinaryHeap` (Priority Queue) first. If a waiting student matches the room's gender and eligibility, they are popped and allocated on the spot!
     - If the priority queue is empty or has no matching student, it checks the `CircularQueue` (Normal FIFO).
     - The newly allocated student's status changes to `ALLOCATED`, and a new node is appended to the `DoublyLinkedList`.
  5. The entire state transition is recorded as a `VacateCommand` in the `undoStack`, ensuring that an Undo operation will seamlessly revert both the vacation and the triggered auto-allocation.

---

### Q5: How does your custom Hash Table handle collisions, and what guarantees the average $O(1)$ lookup complexity?
**Answer:**
- Our `HashTable<K, V>` utilizes **Separate Chaining** with linked bucket nodes.
- To prevent clustered hash values, it uses the 64-bit **FNV-1a hash algorithm** combined with **prime-sized bucket counts** (e.g., 11, 23, 53, 97, 193...).
- The average lookup time remains $O(1)$ because the table monitors the **load factor**:
  $$\alpha = \frac{\text{size}}{\text{bucketCount}}$$
- Whenever $\alpha \ge 0.75$, an automatic **dynamic rehash** is triggered to the next prime capacity, re-distributing all existing entries across the larger bucket array. This bounds the average chain length to less than 1, ensuring near-instant $O(1)$ access.

---

### Q6: Why did you implement both a Hash Table and an AVL Tree / B+ Tree in the same system?
**Answer:**
- **Hash Table**: Excels at exact key lookups (e.g., find student by ID or session token) in $O(1)$ average time. However, hash tables cannot perform range queries or maintain sorted order.
- **AVL Tree**: A height-balanced binary search tree where the difference between left and right subtree heights is at most 1. It enables **ordered range queries** in $O(\log N + k)$ time (e.g., "Find all students with CGPA between 8.5 and 9.5").
- **B+ Tree**: A high-fanout balanced search tree where all data resides in leaves, and all leaves are linked sequentially. It is optimal for large-dataset disk or block-memory indexing because sequential range scans traverse the leaf linked list in $O(k)$ time without jumping back up parent nodes.

---

### Q7: How does your Trie implement typo-tolerant search and what is its algorithmic complexity?
**Answer:**
- Our `Trie` stores words in a 128-ary ASCII prefix tree.
- For exact matching and autocomplete, it traverses character nodes in $O(L)$ time, where $L$ is query length.
- For **typo-tolerant search**, it implements a **Levenshtein Automaton** combined with dynamic programming:
  - While traversing Trie nodes, it maintains a DP row representing edit distances (insertions, deletions, substitutions) between the current Trie path and the user query.
  - If the minimum value in the DP row exceeds the maximum allowable distance ($D$, typically 1 or 2), the entire subtree is pruned immediately (branch-and-bound optimization).
  - This avoids evaluating the full string dictionary and finds fuzzy matches in $O(\Sigma^D \cdot L)$ time.

---

### Q8: How does the Disjoint Set Union (Union-Find) achieve near-constant $\alpha(N)$ time complexity for roommate clustering?
**Answer:**
- The `DisjointSet` structure manages student affinity groups and roommate preference clusters using two crucial optimizations:
  1. **Path Compression**: During `find(x)`, every visited node is reparented directly to the root of the tree, flattening the tree structure for subsequent queries.
  2. **Union by Rank**: During `unite(x, y)`, the root of the shallower tree is attached under the root of the deeper tree, preventing trees from growing tall.
- Together, these optimizations guarantee that any sequence of $M$ operations on $N$ elements executes in $O(M \cdot \alpha(N))$ time, where $\alpha(N)$ is the **Inverse Ackermann function**. Since $\alpha(N) < 5$ for all practical universe sizes, operations run in effective $O(1)$ amortized time.

---

### Q9: Explain how PBKDF2-HMAC-SHA256 provides superior security over plain salted SHA-256.
**Answer:**
- Plain salted SHA-256 executes a single iteration of the SHA-256 compression function. Modern GPUs and ASICs can compute billions of SHA-256 hashes per second, making brute-force and dictionary attacks feasible if a database is breached.
- **PBKDF2-HMAC-SHA256** implements **Key Stretching**:
  - It applies the pseudo-random function (HMAC-SHA256) repeatedly over **10,000 iterations** combined with a unique 16-byte cryptographically secure random salt.
  - This increases the computational cost of testing a candidate password by 10,000-fold, completely rendering GPU rainbow tables and high-speed dictionary attacks computationally intractable while remaining negligible (under 5 milliseconds) for legitimate user logins.

---

### Q10: How does the Undo/Redo mechanism work under the Command Pattern across disparate data structures?
**Answer:**
- The Undo/Redo engine uses the **Command Pattern** with two custom `Stack<std::shared_ptr<ActionCommand>>` containers (`undoStack_` and `redoStack_`).
- Every mutating administrative operation is encapsulated in a concrete command object (`AllocateCommand`, `VacateCommand`, `TransferCommand`, `RegisterCommand`):
  - When an action is executed, the command stores the previous state (e.g., student ID, old room ID, allocation date, and whether the student was drawn from the priority or normal queue).
  - The command is pushed onto `undoStack_`, and `redoStack_` is cleared.
- When `undo()` is called:
  - The top command is popped from `undoStack_`.
  - Its `undo(*this)` method is executed, which atomically rolls back changes across the Vector, Hash Table, Queues, Doubly Linked List, and Room status stores.
  - The command is then pushed onto `redoStack_`.
- When `redo()` is called:
  - The top command is popped from `redoStack_`, its `redo(*this)` is executed, and it is pushed back onto `undoStack_`.
- This ensures 100% data integrity and state synchrony across all memory stores.
