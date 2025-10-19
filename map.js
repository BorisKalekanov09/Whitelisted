function initMap() {
  // Location (latitude, longitude)
  const location = { lat: 48.1486, lng: 17.1077 }; // Example: Bratislava

  // Create the map
  const map = new google.maps.Map(document.getElementById("map"), {
    zoom: 12,
    center: location,
  });

  // Optional: add a marker
  const marker = new google.maps.Marker({
    position: location,
    map: map,
  });
}
// Start and end coordinates
const start = L.latLng(42.6977, 23.3219); // Sofia center
const end = L.latLng(42.6915, 23.4060);   // Airport

// Generate 3 slightly different routes
const startOffsets = [
    [0, 0],          // Route 1: original
    [0.0002, -0.0002], // Route 2: slightly north-west
    [-0.0002, 0.0002]  // Route 3: slightly south-east
];

const endOffsets = [
    [0, 0],
    [0.0001, 0.0001],
    [-0.0001, -0.0001]
];
const routes = [
    {color: 'red', waypoints: [[42.6977, 23.3219], [42.7000, 23.3400], [42.6915, 23.4060]]}, // Route 1
    {color: 'blue', waypoints: [[42.6977, 23.3219], [42.7100, 23.3600], [42.6915, 23.4060]]}, // Route 2
    {color: 'green', waypoints: [[42.6977, 23.3219], [42.6800, 23.3300], [42.6915, 23.4060]]} // Route 3
];


// Draw all 3 routes
const map = L.map('map').setView([42.6977, 23.3219], 12);
L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', { attribution: '© OpenStreetMap contributors' }).addTo(map);

for (let i = 0; i < 3; i++) {
    getRouteControl(i).addTo(map);
}
