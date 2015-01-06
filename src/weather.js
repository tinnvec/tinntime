var xhrRequest = function(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function capitalizeEachWord(str) {
  return str.replace(/\w\S*/g, function(txt) {
    return txt.charAt(0).toUpperCase() + txt.substr(1).toLowerCase();
  });
}

function locationSuccess(pos) {
  // Construct URL
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=" + pos.coords.latitude + "&lon=" + pos.coords.longitude;
  
  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', function(responseText) {
    // responseText contains a JSON object with weather info
    var json = JSON.parse(responseText);
    var configuration;
    if (window.localStorage.getItem("tinntime_config") !== null) {
      configuration = JSON.parse(window.localStorage.tinntime_config);
    }
    // Temperature in Kelvin requires adjustment
    var temperature;
    if(configuration.temp == "c") {
      temperature = Math.round(json.main.temp - 273.15);
    } else {
      temperature = Math.round(json.main.temp * 9/5 - 459.675);
    }
    console.log("Temperature is " + temperature);
    
    // Conditions Icon string
    var conditions_icon = json.weather[0].icon;
    console.log("Conditions Icon: " + conditions_icon);
    
    // Conditions
    var conditions = capitalizeEachWord(json.weather[0].description);
    console.log("Conditions are " + conditions);
    
    var location = json.name;
    console.log("Location is " + location);
    
    // Assemble dictionary using our keys
    var dictionary = {
      "KEY_TEMPERATURE": temperature,
      "KEY_CONDITIONS_ICON": conditions_icon,
      "KEY_CONDITIONS": conditions,
      "KEY_LOCATION": location
    };
    
    // Send to Pebble
    Pebble.sendAppMessage(dictionary, function(e) {
      console.log("Weather info sent to Pebble successfully!");
    }, function(e) {
      console.log("Error sending weather info to Pebble!");
    });
  });
}

function locationError(err) {
  console.log("Error requesting location!");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    { timeout: 15000, maximumAge: 60000 }
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', function(e) {
  console.log("PebbleKit JS ready!");
  // Get the initial weather
  getWeather();
});

// Listen for when AppMessage is received
Pebble.addEventListener('appmessage', function(e) {
  console.log("AppMessage received!");
  getWeather();
});

// Listen for configuration event
Pebble.addEventListener('showConfiguration', function(e) {
  //console.log("Configuration window launching...");
  var configuration;
  var baseURL = 'http://dev.tinnvec.com/tinntime_config/?';
  if (window.localStorage.getItem("tinntime_config") !== null) {
    configuration = JSON.parse(window.localStorage.tinntime_config);
  }
  var params = Object.keys(configuration).map(function(k) {
    return encodeURIComponent(k) + '=' + encodeURIComponent(configuration[k]);
  }).join('&');
  console.log("URL: " + baseURL + params);
  // Show config page
  Pebble.openURL(baseURL + params);
});

// Listen for configuration end event
Pebble.addEventListener('webviewclosed', function(e) {
  var configuration = JSON.parse(decodeURIComponent(e.response));
  window.localStorage.tinntime_config = JSON.stringify(configuration);
  console.log("Configuration window returned: " + JSON.stringify(configuration));
  
  var dictionary = {
    "KEY_INVERT": configuration.invert,
  };
  
  //Send to Pebble, persist there
  Pebble.sendAppMessage(dictionary, function(e) {
    console.log("Settings data sent successfully!");
    getWeather();
  }, function(e) {
    console.log("Settings feedback failed!");
  });
});