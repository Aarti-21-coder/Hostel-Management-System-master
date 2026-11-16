/**
 * REST API Client for Hostel Management System
 */
const API = {
    baseUrl: '', // Same-origin relative path

    async request(endpoint, options = {}) {
        const url = `${this.baseUrl}${endpoint}`;
        const headers = {
            'Content-Type': 'application/json',
            ...(options.headers || {})
        };

        const token = localStorage.getItem('hostel_token');
        if (token) {
            headers['Authorization'] = `Bearer ${token}`;
        }

        try {
            const res = await fetch(url, { ...options, headers });
            const data = await res.json();
            if (!res.ok) {
                throw new Error(data.error || 'Server error occurred');
            }
            return data;
        } catch (err) {
            console.error(`API Error on ${endpoint}:`, err);
            throw err;
        }
    },

    // Auth
    login: (username, password) => API.request('/api/auth/login', {
        method: 'POST',
        body: JSON.stringify({ username, password })
    }),
    registerUser: (userData) => API.request('/api/auth/register', {
        method: 'POST',
        body: JSON.stringify(userData)
    }),

    // Stats
    getStats: () => API.request('/api/stats'),

    // Students (DFD 2.0)
    registerStudent: (studentData) => API.request('/api/students/register', {
        method: 'POST',
        body: JSON.stringify(studentData)
    }),
    getWaitingLists: () => API.request('/api/waiting-list'),
    getStudents: (page = 1, limit = 50, status = 'ALL', search = '') =>
        API.request(`/api/students?page=${page}&limit=${limit}&status=${encodeURIComponent(status)}&search=${encodeURIComponent(search)}`),
    cancelRegistration: (studentId) => API.request('/api/students/cancel', {
        method: 'POST',
        body: JSON.stringify({ studentId })
    }),

    // Allocation (DFD 3.0)
    allocateNext: () => API.request('/api/rooms/allocate-next', { method: 'POST' }),
    allocateManual: (studentId, roomId) => API.request('/api/rooms/allocate-manual', {
        method: 'POST',
        body: JSON.stringify({ studentId, roomId })
    }),
    vacateRoom: (studentId) => API.request('/api/rooms/vacate', {
        method: 'POST',
        body: JSON.stringify({ studentId })
    }),
    transferRoom: (studentId, newRoomId) => API.request('/api/rooms/transfer', {
        method: 'POST',
        body: JSON.stringify({ studentId, newRoomId })
    }),
    getAllocated: () => API.request('/api/allocated'),

    // Rooms & Grid
    getRooms: (block = '', floor = 0) =>
        API.request(`/api/rooms?block=${encodeURIComponent(block)}&floor=${floor}`),

    // Reports (DFD 4.0)
    getReports: (field = 'cgpa', ascending = false, algo = 'quicksort') =>
        API.request(`/api/reports?field=${field}&ascending=${ascending}&algo=${algo}`),

    // Undo / Redo
    undo: () => API.request('/api/actions/undo', { method: 'POST' }),
    redo: () => API.request('/api/actions/redo', { method: 'POST' }),

    // Campus Route (Dijkstra)
    getRoute: (from, to) =>
        API.request(`/api/campus/route?from=${encodeURIComponent(from)}&to=${encodeURIComponent(to)}`),

    // Search Autocomplete (Trie)
    autocomplete: (q) =>
        API.request(`/api/search/autocomplete?q=${encodeURIComponent(q)}`),

    // Dataset Generator & Binary Persistence
    generateStudents: (count = 1000) => API.request('/api/dataset/generate', {
        method: 'POST',
        body: JSON.stringify({ count })
    }),
    saveState: () => API.request('/api/dataset/save', { method: 'POST' }),
    loadState: () => API.request('/api/dataset/load', { method: 'POST' }),

    // Live DSA Benchmark
    runBenchmark: () => API.request('/api/benchmark/run', { method: 'POST' })
};
