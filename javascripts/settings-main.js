var $invertColors = $('#invert_colors');
var $disconnectVibrate = $('#disconnect_vibrate');
var $updateFrequency = $('#update_frequency');
var $temperatureFormat = $('#temperature_format');
var $useGps = $('#use_gps');
var $zipCode = $('#zip_code');

$(document).ready(function() {
  $('#submit_button').click(function() {
    var return_to = getQueryParam('return_to', 'pebblejs://close#');
    document.location = return_to + encodeURIComponent(JSON.stringify(getAndStoreConfigData()));
  });

  $useGps.click(function() {
    $zipCode[0].disabled = $(this)[0].checked;
  });
});
loadSettings();

function loadSettings() {
  if (localStorage.invertColors) { $invertColors[0].checked = localStorage.invertColors === 'true'; }
  if (localStorage.disconnectVibrate) { $disconnectVibrate[0].checked = localStorage.disconnectVibrate === 'true'; }
  if (localStorage.updateFrequency) {
    var optionElements = $updateFrequency.children('option');
    for (var i = 0; i < optionElements.length; i++) {
      if (optionElements[i].value === localStorage.updateFrequency) { optionElements[i].selected = true; }
    }
  }
  if (localStorage.temperatureFormat) {
    var optionElements = $temperatureFormat.children('option');
    for (var i = 0; i < optionElements.length; i++) {
      if (optionElements[i].value === localStorage.temperatureFormat) { optionElements[i].selected = true; }
    }
  }
  if (localStorage.useGps) {
    $useGps[0].checked = localStorage.useGps === 'true';
    $zipCode[0].disabled = $useGps[0].checked;
  }
  if (localStorage.zipCode) { $zipCode[0].value = localStorage.zipCode; }
}

function getAndStoreConfigData() {
  var options = {
    invertColors: $invertColors[0].checked,
    disconnectVibrate: $disconnectVibrate[0].checked,
    updateFrequency: $updateFrequency[0].value,
    temperatureFormat: $temperatureFormat[0].value,
    useGps: $useGps[0].checked,
    zipCode: $zipCode[0].value
  };

  localStorage.invertColors = options.invertColors;
  localStorage.disconnectVibrate = options.disconnectVibrate;
  localStorage.updateFrequency = options.updateFrequency;
  localStorage.temperatureFormat = options.temperatureFormat;
  localStorage.useGps = options.useGps;
  localStorage.zipCode = options.zipCode;

  console.log('Got options: ' + JSON.stringify(options));
  return options;
}

function getQueryParam(variable, defaultValue) {
  var query = location.search.substring(1);
  var vars = query.split('&');
  for (var i = 0; i < vars.length; i++) {
    var pair = vars[i].split('=');
    if (pair[0] === variable) {
      return decodeURIComponent(pair[1]);
    }
  }
  return defaultValue || false;
}
