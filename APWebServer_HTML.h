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
  input[type="text"], input[type="password"], input[type="number"] {
    padding:6px;
    border:1px solid #ccc;
    border-radius:4px;
    box-sizing:border-box;
  }

  /* Широкие поля для Wi-Fi и Time Settings */
  .input-wide { width:100%; }

  /* Компактные поля для Weather Settings */
  .input-latlon { width:30%; }
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

    <!-- Submit -->
    <input type="submit" value="Save & Reboot">
  </form>
</body>
</html>
)rawliteral";
