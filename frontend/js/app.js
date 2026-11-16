/**
 * Application Controller & Event Wiring
 */

function showToast(message, type = 'info') {
    const container = document.getElementById('toastContainer');
    if (!container) return;

    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;
    const icon = type === 'success' ? 'check-circle' : (type === 'error' ? 'exclamation-circle' : 'info-circle');
    toast.innerHTML = `<i class="fas fa-${icon}"></i> <span>${message}</span>`;

    container.appendChild(toast);
    setTimeout(() => {
        toast.style.opacity = '0';
        toast.style.transform = 'translateX(100%)';
        setTimeout(() => toast.remove(), 300);
    }, 3500);
}

const App = {
    currentTab: 'dashboard',
    selectedBlock: 'Block A',
    selectedFloor: 1,

    init() {
        this.bindEvents();
        this.checkAuth();
        this.loadDashboard();
        this.setupLivePriorityCalculator();
    },

    checkAuth() {
        const username = localStorage.getItem('hostel_user') || 'Admin';
        const role = localStorage.getItem('hostel_role') || 'Admin';
        const userEl = document.getElementById('currentUserDisplay');
        if (userEl) userEl.innerText = username;
        const roleEl = document.getElementById('currentRoleDisplay');
        if (roleEl) roleEl.innerText = role;
    },

    switchTab(tabId) {
        document.querySelectorAll('.page-container').forEach(p => p.classList.remove('active'));
        document.querySelectorAll('.nav-item').forEach(n => n.classList.remove('active'));

        const target = document.getElementById(`page-${tabId}`);
        const navTarget = document.getElementById(`nav-${tabId}`);
        if (target) target.classList.add('active');
        if (navTarget) navTarget.classList.add('active');

        this.currentTab = tabId;

        if (tabId === 'dashboard') this.loadDashboard();
        else if (tabId === 'waiting') this.loadWaitingLists();
        else if (tabId === 'rooms') this.loadRooms();
        else if (tabId === 'allocated') this.loadAllocated();
        else if (tabId === 'students') this.loadStudents();
        else if (tabId === 'reports') this.loadReports();
        else if (tabId === 'lab') DSALab.runBenchmarks();
    },

    bindEvents() {
        // Navigation clicks
        document.querySelectorAll('.nav-item a').forEach(a => {
            a.addEventListener('click', (e) => {
                e.preventDefault();
                const tab = a.getAttribute('data-tab');
                if (tab) this.switchTab(tab);
            });
        });

        // Theme toggle
        const btnTheme = document.getElementById('btnToggleTheme');
        if (btnTheme) {
            btnTheme.addEventListener('click', () => {
                const current = document.body.getAttribute('data-theme') || 'dark';
                const next = current === 'dark' ? 'light' : 'dark';
                document.body.setAttribute('data-theme', next);
                btnTheme.innerHTML = next === 'dark' ? '<i class="fas fa-moon"></i>' : '<i class="fas fa-sun"></i>';
            });
        }

        // Global search input with Trie autocomplete
        const searchInput = document.getElementById('globalSearchInput');
        const searchDropdown = document.getElementById('searchDropdown');
        if (searchInput && searchDropdown) {
            searchInput.addEventListener('input', async (e) => {
                const q = e.target.value.trim();
                if (q.length < 2) {
                    searchDropdown.style.display = 'none';
                    return;
                }
                try {
                    const data = await API.autocomplete(q);
                    if (data.results && data.results.length > 0) {
                        searchDropdown.innerHTML = data.results.map(r => `
                            <div class="search-result-item" onclick="App.inspectStudent('${r.payload}')">
                                <span><strong>${r.word}</strong></span>
                                <span class="badge badge-waiting">${r.payload}</span>
                            </div>
                        `).join('');
                        searchDropdown.style.display = 'block';
                    } else {
                        searchDropdown.style.display = 'none';
                    }
                } catch (err) {
                    searchDropdown.style.display = 'none';
                }
            });

            document.addEventListener('click', (e) => {
                if (!searchInput.contains(e.target) && !searchDropdown.contains(e.target)) {
                    searchDropdown.style.display = 'none';
                }
            });
        }

        // Undo & Redo buttons
        const btnUndo = document.getElementById('btnTopUndo');
        if (btnUndo) {
            btnUndo.addEventListener('click', async () => {
                try {
                    const res = await API.undo();
                    showToast(res.message, 'success');
                    this.refreshCurrent();
                } catch (err) {
                    showToast(err.message, 'error');
                }
            });
        }

        const btnRedo = document.getElementById('btnTopRedo');
        if (btnRedo) {
            btnRedo.addEventListener('click', async () => {
                try {
                    const res = await API.redo();
                    showToast(res.message, 'success');
                    this.refreshCurrent();
                } catch (err) {
                    showToast(err.message, 'error');
                }
            });
        }

        // Student Registration Form
        const regForm = document.getElementById('formRegisterStudent');
        if (regForm) {
            regForm.addEventListener('submit', async (e) => {
                e.preventDefault();
                const formData = {
                    id: document.getElementById('regId').value.trim(),
                    name: document.getElementById('regName').value.trim(),
                    age: parseInt(document.getElementById('regAge').value),
                    gender: document.getElementById('regGender').value,
                    course: document.getElementById('regCourse').value,
                    year: parseInt(document.getElementById('regYear').value),
                    cgpa: parseFloat(document.getElementById('regCgpa').value),
                    distanceKm: parseFloat(document.getElementById('regDistance').value || 0),
                    isSpecialCategory: document.getElementById('regSpecial').checked,
                    phone: document.getElementById('regPhone').value.trim(),
                    email: document.getElementById('regEmail').value.trim(),
                    city: document.getElementById('regCity').value.trim()
                };

                try {
                    const res = await API.registerStudent(formData);
                    showToast(`${res.name} registered! Priority: ${res.priorityScore.toFixed(2)} (${res.queueType})`, 'success');
                    regForm.reset();
                    this.setupLivePriorityCalculator();
                    this.switchTab('waiting');
                } catch (err) {
                    showToast(err.message, 'error');
                }
            });
        }

        // Allocate Next button
        const btnAllocNext = document.getElementById('btnAllocateNext');
        if (btnAllocNext) {
            btnAllocNext.addEventListener('click', async () => {
                try {
                    const res = await API.allocateNext();
                    showToast(`Allocated ${res.studentName} to ${res.roomId} (${res.block})!`, 'success');
                    this.loadDashboard();
                    if (this.currentTab === 'waiting') this.loadWaitingLists();
                    else if (this.currentTab === 'allocated') this.loadAllocated();
                } catch (err) {
                    showToast(err.message, 'error');
                }
            });
        }

        // Campus Route Finder
        const btnRoute = document.getElementById('btnFindRoute');
        if (btnRoute) {
            btnRoute.addEventListener('click', async () => {
                const from = document.getElementById('routeFrom').value;
                const to = document.getElementById('routeTo').value;
                try {
                    const res = await API.getRoute(from, to);
                    const container = document.getElementById('routeResult');
                    if (container) {
                        container.innerHTML = `
                            <div class="glass-card" style="margin-top: 16px; border-left: 4px solid var(--accent-primary);">
                                <h3 style="margin-bottom: 8px;">Dijkstra Shortest Path Result</h3>
                                <p style="color: var(--text-secondary); margin-bottom: 12px;">
                                    Total Distance: <strong>${res.totalDistanceMeters} meters</strong>
                                </p>
                                <div style="display: flex; flex-wrap: wrap; align-items: center; gap: 8px;">
                                    ${res.path.map((node, i) => `
                                        <span class="badge badge-allocated">${node}</span>
                                        ${i < res.path.length - 1 ? '<i class="fas fa-arrow-right" style="color: var(--text-muted); font-size: 11px;"></i>' : ''}
                                    `).join('')}
                                </div>
                            </div>
                        `;
                    }
                } catch (err) {
                    showToast(err.message, 'error');
                }
            });
        }
    },

    setupLivePriorityCalculator() {
        const yearInput = document.getElementById('regYear');
        const cgpaInput = document.getElementById('regCgpa');
        const distInput = document.getElementById('regDistance');
        const specInput = document.getElementById('regSpecial');
        const previewEl = document.getElementById('priorityPreviewScore');
        const queueBadge = document.getElementById('priorityQueueRouting');

        const calculate = () => {
            if (!yearInput || !cgpaInput || !previewEl) return;
            const year = parseInt(yearInput.value || 1);
            const cgpa = parseFloat(cgpaInput.value || 0);
            const dist = parseFloat(distInput ? distInput.value || 0 : 0);
            const isSpec = specInput ? specInput.checked : false;

            const score = (year * 25.0) + (cgpa * 5.0) + (dist * 0.02) + (isSpec ? 15.0 : 0.0);
            previewEl.innerText = score.toFixed(2);

            if (queueBadge) {
                if (score >= 50.0) {
                    queueBadge.className = 'badge badge-waiting';
                    queueBadge.innerHTML = '<i class="fas fa-crown"></i> Priority Queue (Binary Max-Heap)';
                } else {
                    queueBadge.className = 'badge badge-vacated';
                    queueBadge.innerHTML = '<i class="fas fa-clock"></i> Normal Queue (Circular FIFO)';
                }
            }
        };

        [yearInput, cgpaInput, distInput, specInput].forEach(el => {
            if (el) el.addEventListener('input', calculate);
        });
        calculate();
    },

    refreshCurrent() {
        this.loadDashboard();
        if (this.currentTab === 'waiting') this.loadWaitingLists();
        else if (this.currentTab === 'allocated') this.loadAllocated();
        else if (this.currentTab === 'rooms') this.loadRooms();
        else if (this.currentTab === 'students') this.loadStudents();
    },

    async loadDashboard() {
        try {
            const stats = await API.getStats();
            document.getElementById('statTotalStudents').innerText = stats.totalStudents;
            document.getElementById('statWaitingCount').innerText = stats.waitingCount;
            document.getElementById('statAllocatedCount').innerText = stats.allocatedCount;
            document.getElementById('statOccupancyPercent').innerText = stats.occupancyPercent.toFixed(1) + '%';
            document.getElementById('navWaitingBadge').innerText = stats.waitingCount;

            Charts.initOccupancyChart('occupancyChartCanvas', stats.occupiedBeds, stats.availableBeds);
            Charts.initStatusChart('statusChartCanvas', stats.waitingCount, stats.allocatedCount, stats.vacatedCount, stats.cancelledCount);
        } catch (err) {
            console.error('Failed to load dashboard:', err);
        }
    },

    async loadWaitingLists() {
        try {
            const data = await API.getWaitingLists();
            const prioContainer = document.getElementById('priorityQueueTableBody');
            const normContainer = document.getElementById('normalQueueTableBody');

            if (prioContainer) {
                prioContainer.innerHTML = data.priorityQueue.map((s, i) => `
                    <tr>
                        <td><span class="badge badge-waiting">#${i + 1}</span></td>
                        <td><strong>${s.id}</strong></td>
                        <td>${s.name}</td>
                        <td>${s.course} (Yr ${s.year})</td>
                        <td>${s.cgpa.toFixed(1)}</td>
                        <td><strong>${s.priorityScore.toFixed(2)}</strong></td>
                        <td>${s.gender}</td>
                        <td><button class="btn-primary" style="padding: 4px 10px; font-size: 11px;" onclick="App.allocateNext()">Allocate</button></td>
                    </tr>
                `).join('') || '<tr><td colspan="8" style="text-align: center; color: var(--text-muted);">Priority queue is empty</td></tr>';
            }

            if (normContainer) {
                normContainer.innerHTML = data.normalQueue.map((s, i) => `
                    <tr>
                        <td><span class="badge badge-vacated">#${i + 1}</span></td>
                        <td><strong>${s.id}</strong></td>
                        <td>${s.name}</td>
                        <td>${s.course} (Yr ${s.year})</td>
                        <td>${s.cgpa.toFixed(1)}</td>
                        <td>${s.priorityScore.toFixed(2)}</td>
                        <td>${s.gender}</td>
                        <td><button class="btn-secondary" style="padding: 4px 10px; font-size: 11px;" onclick="App.allocateNext()">Allocate</button></td>
                    </tr>
                `).join('') || '<tr><td colspan="8" style="text-align: center; color: var(--text-muted);">Normal queue is empty</td></tr>';
            }
        } catch (err) {
            showToast('Failed to load waiting lists: ' + err.message, 'error');
        }
    },

    async loadRooms() {
        const block = this.selectedBlock;
        const floor = this.selectedFloor;
        try {
            const data = await API.getRooms(block, floor);
            const grid = document.getElementById('roomGridContainer');
            if (grid) {
                grid.innerHTML = data.rooms.map(r => {
                    const statusClass = r.status === 'FULL' ? 'status-full' : (r.status === 'MAINTENANCE' ? 'status-maintenance' : '');
                    const dots = [];
                    for (let i = 0; i < r.capacity; ++i) {
                        dots.push(`<div class="bed-dot ${i < r.occupiedCount ? 'occupied' : ''}"></div>`);
                    }
                    return `
                        <div class="room-card ${statusClass}" onclick="App.openRoomModal('${r.id}')">
                            <div class="room-id">${r.id}</div>
                            <div class="room-meta">${r.roomType} • ${r.isAC ? 'AC' : 'Non-AC'}</div>
                            <div class="room-meta">Gender: ${r.allowedGender}</div>
                            <div style="font-size: 12px; font-weight: 600; margin-top: 4px;">
                                ${r.occupiedCount}/${r.capacity} Beds
                            </div>
                            <div class="bed-dots">${dots.join('')}</div>
                        </div>
                    `;
                }).join('') || '<div style="color: var(--text-muted);">No rooms found for this block/floor.</div>';
            }
        } catch (err) {
            showToast('Failed to load rooms: ' + err.message, 'error');
        }
    },

    setBlock(block) {
        this.selectedBlock = block;
        document.querySelectorAll('.block-tab-btn').forEach(b => {
            b.classList.toggle('active', b.getAttribute('data-block') === block);
        });
        this.loadRooms();
    },

    setFloor(floor) {
        this.selectedFloor = floor;
        document.querySelectorAll('.floor-tab-btn').forEach(b => {
            b.classList.toggle('active', parseInt(b.getAttribute('data-floor')) === floor);
        });
        this.loadRooms();
    },

    async loadAllocated() {
        try {
            const data = await API.getAllocated();
            const body = document.getElementById('allocatedTableBody');
            if (body) {
                body.innerHTML = data.allocatedStudents.map(rec => `
                    <tr>
                        <td><strong>${rec.studentId}</strong></td>
                        <td>${rec.studentName}</td>
                        <td><span class="badge badge-allocated">${rec.roomId}</span></td>
                        <td>Bed #${rec.bedIndex}</td>
                        <td>${rec.allocatedDate}</td>
                        <td>
                            <button class="btn-secondary" style="padding: 4px 10px; font-size: 11px; margin-right: 6px;"
                                onclick="App.openTransferModal('${rec.studentId}')">
                                Transfer
                            </button>
                            <button class="btn-secondary" style="padding: 4px 10px; font-size: 11px; color: var(--danger);"
                                onclick="App.vacateStudent('${rec.studentId}')">
                                Vacate
                            </button>
                        </td>
                    </tr>
                `).join('') || '<tr><td colspan="6" style="text-align: center; color: var(--text-muted);">No students currently allocated</td></tr>';
            }
        } catch (err) {
            showToast('Failed to load allocated list: ' + err.message, 'error');
        }
    },

    async loadStudents(page = 1) {
        const status = document.getElementById('filterStudentStatus') ? document.getElementById('filterStudentStatus').value : 'ALL';
        const search = document.getElementById('filterStudentSearch') ? document.getElementById('filterStudentSearch').value.trim() : '';

        try {
            const data = await API.getStudents(page, 50, status, search);
            const body = document.getElementById('studentsMasterTableBody');
            if (body) {
                body.innerHTML = data.students.map(s => {
                    const badgeClass = `badge-${s.status.toLowerCase()}`;
                    return `
                        <tr>
                            <td><strong>${s.id}</strong></td>
                            <td>${s.name}</td>
                            <td>${s.course}</td>
                            <td>${s.cgpa.toFixed(1)}</td>
                            <td>${s.priorityScore.toFixed(2)}</td>
                            <td><span class="badge ${badgeClass}">${s.status}</span></td>
                            <td>${s.allocatedRoomId || '—'}</td>
                            <td>
                                ${s.status === 'WAITING' ? `
                                    <button class="btn-secondary" style="padding: 4px 8px; font-size: 11px; color: var(--danger);"
                                        onclick="App.cancelStudent('${s.id}')">Cancel</button>
                                ` : ''}
                            </td>
                        </tr>
                    `;
                }).join('') || '<tr><td colspan="8" style="text-align: center; color: var(--text-muted);">No matching students found</td></tr>';
            }

            const pageIndicator = document.getElementById('studentPageIndicator');
            if (pageIndicator) pageIndicator.innerText = `Page ${data.page} of ${data.totalPages || 1} (${data.total} total)`;
        } catch (err) {
            showToast('Failed to load students: ' + err.message, 'error');
        }
    },

    async loadReports() {
        const field = document.getElementById('reportSortField').value;
        const algo = document.getElementById('reportAlgorithm').value;
        const asc = document.getElementById('reportSortOrder').value === 'asc';

        try {
            const data = await API.getReports(field, asc, algo);
            document.getElementById('reportElapsedUs').innerText = data.elapsedMicroseconds.toFixed(2) + ' μs';
            document.getElementById('reportComparisons').innerText = data.comparisons;
            document.getElementById('reportSwaps').innerText = data.swaps;

            const body = document.getElementById('reportsTableBody');
            if (body) {
                body.innerHTML = data.students.map((s, i) => `
                    <tr>
                        <td>#${i + 1}</td>
                        <td><strong>${s.id}</strong></td>
                        <td>${s.name}</td>
                        <td>${s.course}</td>
                        <td>${s.cgpa.toFixed(1)}</td>
                        <td>${s.priorityScore.toFixed(2)}</td>
                        <td><span class="badge badge-${s.status.toLowerCase()}">${s.status}</span></td>
                        <td>${s.allocatedRoomId || '—'}</td>
                    </tr>
                `).join('');
            }
        } catch (err) {
            showToast('Failed to generate report: ' + err.message, 'error');
        }
    },

    async vacateStudent(studentId) {
        if (!confirm(`Are you sure student ${studentId} is vacating? This will immediately auto-allocate the freed bed to the next waiting student!`)) {
            return;
        }
        try {
            const res = await API.vacateRoom(studentId);
            showToast(res.message, 'success');
            this.refreshCurrent();
        } catch (err) {
            showToast(err.message, 'error');
        }
    },

    async cancelStudent(studentId) {
        if (!confirm(`Are you sure you want to cancel registration for ${studentId}?`)) {
            return;
        }
        try {
            const res = await API.cancelRegistration(studentId);
            showToast(res.message, 'success');
            this.refreshCurrent();
        } catch (err) {
            showToast(err.message, 'error');
        }
    },

    openTransferModal(studentId) {
        const targetRoom = prompt(`Enter new destination Room ID to transfer student ${studentId} (e.g. A-102):`);
        if (targetRoom) {
            API.transferRoom(studentId, targetRoom.trim())
                .then(res => {
                    showToast(res.message, 'success');
                    this.refreshCurrent();
                })
                .catch(err => showToast(err.message, 'error'));
        }
    },

    inspectStudent(studentId) {
        this.switchTab('students');
        const searchInput = document.getElementById('filterStudentSearch');
        if (searchInput) {
            searchInput.value = studentId;
            this.loadStudents(1);
        }
    },

    async generateDataset(count) {
        try {
            showToast(`Generating ${count} synthetic student records...`, 'info');
            const res = await API.generateStudents(count);
            showToast(`Generated ${res.generated} students in ${res.elapsedMs.toFixed(2)} ms! Total: ${res.totalStudents}`, 'success');
            this.refreshCurrent();
        } catch (err) {
            showToast(err.message, 'error');
        }
    },

    async saveState() {
        try {
            const res = await API.saveState();
            showToast(res.message, 'success');
        } catch (err) {
            showToast(err.message, 'error');
        }
    },

    async loadState() {
        try {
            const res = await API.loadState();
            showToast(`State restored in ${res.elapsedMs.toFixed(2)} ms! Total: ${res.totalStudents} students`, 'success');
            this.refreshCurrent();
        } catch (err) {
            showToast(err.message, 'error');
        }
    }
};

window.addEventListener('DOMContentLoaded', () => {
    App.init();
});
