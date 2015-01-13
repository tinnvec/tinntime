function capitalizeEachWord(str) {
  return str.replace(/\w\S*/g, function(txt) {
    return txt.charAt(0).toUpperCase() + txt.substr(1).toLowerCase();
  });
}

function locationSuccess(pos) {
  // http://api.openweathermap.org/data/2.5/weather?q=53913,USA
  // console.log('lat= ' + pos.coords.latitude + ' lon= ' + pos.coords.longitude);
  var configuration, url;
  var req = new XMLHttpRequest();
  
  if (window.localStorage.getItem("tinntime_config") !== null) {
    configuration = JSON.parse(window.localStorage.tinntime_config);
  }
  
  if (configuration.gps === "no") {
    url = "http://api.openweathermap.org/data/2.5/weather?q=" + configuration.zipcode + ",USA";
  } else {
    url = "http://api.openweathermap.org/data/2.5/weather?lat=" + pos.coords.latitude + "&lon=" + pos.coords.longitude;
  }
  
  req.open('GET', url, true);
  req.onload = function(e) {
    if (req.readyState == 4 && req.status == 200) {
      if(req.status == 200) {
        var configuration;
        var response = JSON.parse(req.responseText);
        var temperature = response.main.temp;
        var icon = response.weather[0].icon;
        var location = response.name;
        var conditions = capitalizeEachWord(response.weather[0].description);
        
        if (window.localStorage.getItem("tinntime_config") !== null) {
          configuration = JSON.parse(window.localStorage.tinntime_config);
        }
        
        // Temperature in Kelvin requires adjustment
        if(configuration.temp == "c") {
          temperature = Math.round(temperature - 273.15);
        } else {
          temperature = Math.round(temperature * 9/5 - 459.675);
        }
        
        console.log("Temperature is " + temperature);
        console.log("Conditions Icon: " + icon);
        console.log("Location is " + location);
        console.log("Conditions are " + conditions);
        
        var dictionary = {
          "KEY_TEMPERATURE": temperature,
          "KEY_CONDITIONS_ICON": icon,
          "KEY_CONDITIONS": conditions,
          "KEY_LOCATION": location
        };
        
        Pebble.sendAppMessage(dictionary, function(e) {
          console.log("Weather info sent to Pebble successfully!");
        }, function(e) {
          console.log("Error sending weather info to Pebble!");
        });
      } else { console.log('Error'); }
    }
  };
  req.send(null);
}

function locationError(err) {
  console.log('location error (' + err.code + '): ' + err.message);
}

function getWeather() {
  var configuration;
  
  if (window.localStorage.getItem("tinntime_config") !== null) {
    configuration = JSON.parse(window.localStorage.tinntime_config);
  }
  
  if (configuration.gps === "no") {
    locationSuccess(null);
  } else {
    var locationOptions = {
      enableHighAccuracy: true, 
      maximumAge: 10000, 
      timeout: 10000
    };
    navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
  }
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', function(e) {
  if (window.localStorage.getItem("tinntime_config") === null) {
    var configuration = {
      "invert": "yes",
      "temp": "f",
      "gps": "yes",
      "zipcode": "000000",
      "update": "30",
      "vibe": "yes"
    };
    window.localStorage.tinntime_config = JSON.stringify(configuration);
  }
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
  var configuration, params;
  var baseURL = 'https://www.googledrive.com/host/0B2tKMa3V6dkkN1Z4aGJSYU8xekU/?';
  if (window.localStorage.getItem("tinntime_config") !== null) {
    configuration = JSON.parse(window.localStorage.tinntime_config);
    params = Object.keys(configuration).map(function(k) {
      return encodeURIComponent(k) + '=' + encodeURIComponent(configuration[k]);
    }).join('&');
  }
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
    "KEY_UPDATE": configuration.update,
    "KEY_VIBRATE": configuration.vibe
  };
  
  //Send to Pebble, persist there
  Pebble.sendAppMessage(dictionary, function(e) {
    console.log("Settings data sent successfully!");
    getWeather();
  }, function(e) {
    console.log("Settings feedback failed!");
  });
});