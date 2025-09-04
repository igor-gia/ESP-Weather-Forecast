#pragma once
#include <pgmspace.h>

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html style='font-family: sans-serif;'>
<head>
<meta name='viewport' content='width=device-width, initial-scale=1.0'>
<style>
  body { max-width:600px; margin:auto; padding:10px; background:#f5f5f5; }
  h2 { text-align:center; color:#333; }
  form { background:#fff; padding:20px; border-radius:8px; box-shadow:0 0 10px rgba(0,0,0,0.1); }

  .section { margin-bottom:20px; padding:15px; background:#eef; border-radius:6px; }
  .section-title { text-align:center; font-weight:bold; font-size:18px; margin-bottom:10px; color:#555; }

  .row {
    display:flex;
    flex-wrap:wrap;
    align-items:center;
    margin-bottom:8px;
  }
  label {
    min-width:100px;
    margin-right:8px;
  }
  input[type="text"], input[type="password"], input[type="number"], input[type="time"]{
    padding:6px;
    border:1px solid #ccc;
    border-radius:4px;
    box-sizing:border-box;
  }

  .input-wide { width:100%; }
  .input-latlon { width:30%; }
  .input-time   { width:18%; }
  .input-interval { width:18%; }

  input[type='submit'] { 
    width:100%; 
    padding:10px; 
    background:#4CAF50; 
    color:white; 
    border:none; 
    border-radius:4px; 
    font-size:16px; 
    cursor:pointer; 
  }
  input[type='submit']:hover { background:#45a049; }

</style>
</head>
<body>
  <h2>ESP Weather Forecast Setup</h2>
  <form action="/save" method="POST" autocomplete="off">

    <!-- Wi-Fi -->
    <div class="section">
      <div class="section-title">Wi-Fi Settings</div>
      <div class="row">
        <label for="ssid">SSID:</label>
        <input type="text" id="ssid" name="ssid" class="input-wide" value="%SSID%">
      </div>
      <div class="row">
        <label for="password">Password:</label>
        <input type="password" id="password" name="password" class="input-wide" value="%PASSWORD%">
      </div>
    </div>

    <!-- Time -->
    <div class="section">
      <div class="section-title">Time Settings</div>
      <div class="row">
        <label for="tzString">Time Zone:</label>
        <input type="text" id="tzString" name="tzString" class="input-wide" value="%TZ%">
      </div>
      <div class="row">
        <label for="ntpServer">NTP Server:</label>
        <input type="text" id="ntpServer" name="ntpServer" class="input-wide" value="%NTP%">
      </div>
    </div>

    <!-- Weather -->
    <div class="section">
      <div class="section-title">Weather Settings</div>
      <div class="row">
        <label for="latitude">Latitude:</label>
        <input type="text" id="latitude" name="latitude" class="input-latlon" value="%LAT%">
      </div>
      <div class="row">
        <label for="longitude">Longitude:</label>
        <input type="text" id="longitude" name="longitude" class="input-latlon" value="%LON%">
      </div>
      <div class="row">
        <label for="intervalWeather">Update Interval (min):</label>
        <input type="number" id="intervalWeather" name="intervalWeather" min="10" class="input-interval" value="%INTERVAL%">
      </div>
    </div>

    <!-- Night mode -->
    <div class="section">
      <div class="section-title">Display Sleep Mode</div>
        <div class="row">
          <label for="nightMode">
            <input type="checkbox" id="nightModeEnabled" name="nightModeEnabled" value="1" %SLEEPEN%> Enable Sleep Mode
          </label>
        </div>
      <div class="row">
        <label for="nightStart">Screen Off Time</label>
        <input type="time" id="nightStart" name="nightStart" class="input-time" value="%NSTART%" step="60" %DISABLED%>
      </div>
      <div class="row">
        <label for="nightEnd">Screen On Time</label>
        <input type="time" id="nightEnd" name="nightEnd" class="input-time" value="%NEND%" step="60" %DISABLED%>
      </div>
    </div>

    <!-- Submit -->
    <input type="submit" value="Save & Reboot">
  </form>

<script>
document.addEventListener('DOMContentLoaded', function() {
    const nightCheckbox = document.getElementById('nightModeEnabled');
    const nightStart = document.getElementById('nightStart');
    const nightEnd   = document.getElementById('nightEnd');

    function updateFields() {
        const disabled = !nightCheckbox.checked;
        nightStart.disabled = disabled;
        nightEnd.disabled   = disabled;
    }

    // Инициализация при загрузке
    updateFields();

    // Обработчик клика по чекбоксу
    nightCheckbox.addEventListener('change', updateFields);
});
</script>

</body>
</html>
)rawliteral";
