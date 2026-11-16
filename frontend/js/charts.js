/**
 * Chart.js Visualizations for Hostel Management System
 */
const Charts = {
    occupancyChart: null,
    statusChart: null,

    initOccupancyChart(canvasId, occupied, available) {
        const ctx = document.getElementById(canvasId);
        if (!ctx) return;

        if (this.occupancyChart) {
            this.occupancyChart.destroy();
        }

        this.occupancyChart = new Chart(ctx, {
            type: 'doughnut',
            data: {
                labels: ['Occupied Beds', 'Available Beds'],
                datasets: [{
                    data: [occupied, available],
                    backgroundColor: ['#ef4444', '#10b981'],
                    borderColor: 'transparent',
                    hoverOffset: 4
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: {
                        position: 'bottom',
                        labels: { color: '#94a3b8', font: { family: 'Inter', size: 12 } }
                    }
                },
                cutout: '72%'
            }
        });
    },

    initStatusChart(canvasId, waiting, allocated, vacated, cancelled) {
        const ctx = document.getElementById(canvasId);
        if (!ctx) return;

        if (this.statusChart) {
            this.statusChart.destroy();
        }

        this.statusChart = new Chart(ctx, {
            type: 'bar',
            data: {
                labels: ['Waiting', 'Allocated', 'Vacated', 'Cancelled'],
                datasets: [{
                    label: 'Students Count',
                    data: [waiting, allocated, vacated, cancelled],
                    backgroundColor: ['#f59e0b', '#10b981', '#06b6d4', '#ef4444'],
                    borderRadius: 6
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: { display: false }
                },
                scales: {
                    x: {
                        grid: { display: false },
                        ticks: { color: '#94a3b8' }
                    },
                    y: {
                        grid: { color: 'rgba(255, 255, 255, 0.05)' },
                        ticks: { color: '#94a3b8', precision: 0 }
                    }
                }
            }
        });
    }
};
