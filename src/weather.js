/*jslint browser: true*/
var baseUrl = 'http://api.openweathermap.org/data/2.5/weather?';
var ow_api = '944cae404e1e7852dd18f81da45928fc';

function capitalizeEachWord(str) {
  return str.replace(/\w\S*/g, function(txt) {
    return txt.charAt(0).toUpperCase() + txt.substr(1).toLowerCase();
  });
}

function checkWeather() {
  var gps = window.localStorage.getItem('useGps');
  var zip = window.localStorage.getItem('zipCode');

  if (gps == 'false' && zip !== '00000' && zip !== '') {
    fetchWeather(baseUrl + 'q=' + zip + ',USA');
  } else {
    window.navigator.geolocation.getCurrentPosition(
      function(pos) {
        fetchWeather(baseUrl + 'lat=' + pos.coords.latitude + '&lon=' + pos.coords.longitude + '&cnt=1');
      },
      function(err) {
        console.warn('location error (' + err.code + '): ' + err.message);
        Pebble.sendAppMessage({
          "KEY_TEMPERATURE": 0,
          "KEY_CONDITIONS_ICON": '',
          "KEY_CONDITIONS": '',
          "KEY_LOCATION": 'Location Error'
        });
      },
      { maximumAge: 1800000, timeout: 60000 }
    );
  }
}

function fetchWeather(url) {
  var req = new XMLHttpRequest();
  req.open('GET', url + '&appid=' + ow_api, true);
  req.onload = function () {
    if (req.readyState === 4) {
      if (req.status === 200) {
        console.log(req.responseText);
        var response = JSON.parse(req.responseText);
        var temperature = response.main.temp;
        var icon = response.weather[0].icon;
        var city = response.name;
        var conditions = capitalizeEachWord(response.weather[0].description);
        
        // Temperature in Kelvin requires adjustment
        if(window.localStorage.getItem('temperatureFormat') === "C") {
          temperature = Math.round(temperature - 273.15);
        } else {
          temperature = Math.round(temperature * 9/5 - 459.675);
        }
        
        Pebble.sendAppMessage({
          "KEY_TEMPERATURE": temperature,
          "KEY_CONDITIONS_ICON": icon,
          "KEY_CONDITIONS": conditions,
          "KEY_LOCATION": city
        });
      } else {
        console.log('Error');
        Pebble.sendAppMessage({
          "KEY_TEMPERATURE": 0,
          "KEY_CONDITIONS_ICON": '',
          "KEY_CONDITIONS": 'Weather Error',
          "KEY_LOCATION": ''
        });
      }
    }
  };
  req.send(null);
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', function(e) {
  // console.log(e.ready);
  console.log(e.type);
  checkWeather();
});

// Listen for when AppMessage is received
Pebble.addEventListener('appmessage', function (e) {
  console.log(e.type);
  checkWeather();
});

// Listen for configuration event
Pebble.addEventListener('showConfiguration', function (e) {
  var url = 'http://tinnvec.github.io/tinntime/settings.html?v=2';
  for (var i = 0; i < window.localStorage.length; i++) {
    var key = window.localStorage.key(i);
		var val = window.localStorage.getItem(key);
		if (val !== null) {
			url += "&" + encodeURIComponent(key) + "=" + encodeURIComponent(val);
		}
  }
  console.log(url);
  Pebble.openURL(url);
});

// Listen for configuration end event
Pebble.addEventListener('webviewclosed', function (e) {
  window.localStorage.clear();
  if (e.response) {
    var values = JSON.parse(decodeURIComponent(e.response));
    console.log("Configuration window returned: " + JSON.stringify(values));
    for (var key in values) {
      window.localStorage.setItem(key, values[key]);
    }
    Pebble.sendAppMessage({
      "KEY_INVERT": values.invertColors.toString(),
      "KEY_UPDATE": parseInt(values.updateFrequency),
      "KEY_VIBRATE": values.disconnectVibrate.toString()
    }, function (e) {
      console.log("Settings data sent successfully!");
      checkWeather();
    }, function (e) {
      console.log("Settings feedback failed!" + e.error.message);
    });
  }
});