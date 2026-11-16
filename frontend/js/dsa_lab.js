/**
 * DSA Lab & Live Benchmark Suite
 */
const DSALab = {
    sortChart: null,
    searchChart: null,

    async runBenchmarks() {
        const btn = document.getElementById('btnRunBenchmark');
        if (btn) {
            btn.disabled = true;
            btn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Running Empirical Tests...';
        }

        try {
            const data = await API.runBenchmark();
            this.renderSortChart(data.sortingBenchmarks);
            this.renderSearchChart(data.searchBenchmarks);
            showToast('DSA Benchmark successfully executed on live backend!', 'success');
        } catch (err) {
            showToast('Benchmark failed: ' + err.message, 'error');
        } finally {
            if (btn) {
                btn.disabled = false;
                btn.innerHTML = '<i class="fas fa-play"></i> Run Live Benchmarks';
            }
        }
    },

    renderSortChart(results) {
        const ctx = document.getElementById('sortBenchmarkChart');
        if (!ctx) return;

        const labels = results.map(r => `N = ${r.n}`);
        const qData = results.map(r => r.quickSortUs);
        const mData = results.map(r => r.mergeSortUs);
        const hData = results.map(r => r.heapSortUs);

        if (this.sortChart) this.sortChart.destroy();

        this.sortChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels,
                datasets: [
                    { label: 'QuickSort (Median-of-3) [μs]', data: qData, borderColor: '#6366f1', tension: 0.3, fill: false },
                    { label: 'MergeSort (Stable) [μs]', data: mData, borderColor: '#10b981', tension: 0.3, fill: false },
                    { label: 'HeapSort (In-Place) [μs]', data: hData, borderColor: '#f59e0b', tension: 0.3, fill: false }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: { labels: { color: '#94a3b8' } }
                },
                scales: {
                    x: { ticks: { color: '#94a3b8' } },
                    y: { title: { display: true, text: 'Time (Microseconds)', color: '#94a3b8' }, ticks: { color: '#94a3b8' } }
                }
            }
        });
    },

    renderSearchChart(results) {
        const ctx = document.getElementById('searchBenchmarkChart');
        if (!ctx) return;

        const labels = results.map(r => `N = ${r.n}`);
        const linData = results.map(r => r.linearSearchUs);
        const htData = results.map(r => r.hashTableUs);
        const avlData = results.map(r => r.avlTreeUs);

        if (this.searchChart) this.searchChart.destroy();

        this.searchChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels,
                datasets: [
                    { label: 'Linear Search O(N) [μs]', data: linData, borderColor: '#ef4444', tension: 0.3, fill: false },
                    { label: 'AVL Tree O(log N) [μs]', data: avlData, borderColor: '#06b6d4', tension: 0.3, fill: false },
                    { label: 'HashTable O(1) [μs]', data: htData, borderColor: '#10b981', tension: 0.3, fill: false }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: { labels: { color: '#94a3b8' } }
                },
                scales: {
                    x: { ticks: { color: '#94a3b8' } },
                    y: { title: { display: true, text: 'Time (Microseconds)', color: '#94a3b8' }, ticks: { color: '#94a3b8' } }
                }
            }
        });
    }
};
