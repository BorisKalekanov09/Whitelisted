// Map setup
const map = L.map('map').setView([42.6977, 23.3219], 12);
L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: '© OpenStreetMap contributors'
}).addTo(map);

// Routes data
const routes = [
    {color: 'red', waypoints: [[42.6977, 23.3219], [42.7000, 23.3400], [42.6915, 23.4060]], name: 'Route 1'},
    {color: 'blue', waypoints: [[42.6977, 23.3219], [42.7100, 23.3600], [42.6915, 23.4060]], name: 'Route 2'},
    {color: 'green', waypoints: [[42.6977, 23.3219], [42.6800, 23.3300], [42.6915, 23.4060]], name: 'Route 3'}
];

let currentSectors = [];
const numSectors = 8;

// Placeholder data for sectors
function getSectorInfo(routeIndex, sectorIndex) {
    return {
        temperature: (20 + sectorIndex) + " °C",
        vibrations: (Math.random()*5).toFixed(1) + " /10",
        holes: Math.floor(Math.random()*3) + " detected",
        recommendedSpeed: (50 + sectorIndex*5) + " km/h"
    };
}

// Draw sectors for selected route
function showRouteSectors(routeIndex) {
    // Remove old sectors
    currentSectors.forEach(s => map.removeLayer(s));
    currentSectors = [];

    const routeData = routes[routeIndex];
    const control = L.Routing.control({
        waypoints: routeData.waypoints.map(p => L.latLng(p[0], p[1])),
        createMarker: () => null,
        addWaypoints: false,
        draggableWaypoints: false,
        routeWhileDragging: false,
        fitSelectedRoutes: true
    }).addTo(map);

    control.on('routesfound', function(e){
        const route = e.routes[0];
        const coords = route.coordinates;
        const sectorLength = Math.floor(coords.length / numSectors);

        for(let i=0; i<numSectors; i++){
            const startIdx = i * sectorLength;
            const endIdx = (i === numSectors-1) ? coords.length : (i+1)*sectorLength;
            const sectorCoords = coords.slice(startIdx, endIdx);

            const sector = L.polyline(sectorCoords, { color: routeData.color, weight: 6, opacity: 0.8 }).addTo(map);

            // Clickable sector opens a Leaflet popup
            sector.on('click', (e) => {
                const info = getSectorInfo(routeIndex, i);
                const popupContent = `
                    <div style="font-size:14px;">
                        <strong>${routeData.name} - Sector ${i+1}</strong><br>
                        Temperature: ${info.temperature}<br>
                        Vibrations: ${info.vibrations}<br>
                        Holes: ${info.holes}<br>
                        Recommended Speed: ${info.recommendedSpeed}
                    </div>
                `;
                L.popup()
                    .setLatLng(e.latlng)  // show popup where clicked
                    .setContent(popupContent)
                    .openOn(map);

                // Highlight selected sector
                sector.setStyle({ color: 'orange', weight: 7 });
                currentSectors.forEach((s, idx) => {
                    if(idx !== i) s.setStyle({ color: routeData.color, weight: 6, opacity: 0.8 });
                });
            });

            currentSectors.push(sector);
        }

        // Remove full route line drawn by routing machine
        if(control._line) map.removeLayer(control._line);
    });
}

// Dropdown logic
const select = document.getElementById('routeSelect');
select.addEventListener('change', function(){
    const routeIndex = parseInt(this.value);
    showRouteSectors(routeIndex);
});
