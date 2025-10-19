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
