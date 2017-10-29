angular.module('app.controllers')

.controller('settingsCtrl',  // The following is the constructor function for this page's controller. See https://docs.angularjs.org/guide/controller
// You can include any angular dependencies as parameters for this function
// TIP: Access Route Parameters for your page via $stateParams.parameterName
function ($scope, $stateParams) {
  window.$scope = $scope;
  $scope.parameters = {
    brightness: 128,
    volume:128,
    audioMode: "NONE",
    audioState: true,
    patternState: true
  }

  $scope.audioModes = [
    {
      "name": "No audio",
      "code": "NONE"
    },
    {
      "name": "Pattern audio only",
      "code": "CARD"
    },
    {
      "name": "Bluetooth audio only",
      "code": "BLUE"
    },
    {
      "name": "Dual audio",
      "code": "DUAL"
    }
  ];

//currently not working as intended
    $scope.getVariable = function(varName) {
      var req = new XMLHttpRequest();
      req.open("GET", "https://api.particle.io/v1/devices/XXXXXX/"+varName);
      req.setRequestHeader("Authorization", "Bearer XXXXXX");
      req.setRequestHeader("Content-type", "application/json");
      req.send();
      req.onload = function() {
        console.log(req.responseText);
        var result = JSON.parse(req.responseText).result;
        $scope.parameters[varName] = result;
        console.log(varName + ": "+result);
      }
    }

  $scope.getVariable("brightness");
  $scope.getVariable("volume");
  $scope.getVariable("audioMode");
  $scope.getVariable("patternState");


  $scope.reloadVariables = function() {
    $scope.getVariable("brightness");
    $scope.getVariable("volume");
    $scope.getVariable("audioMode");
    $scope.getVariable("patternState");
  }

  $scope.passData = function(dataString) {
      var req = new XMLHttpRequest();
      req.open("POST", "https://api.particle.io/v1/devices/XXXXXX/passdata", true);
      req.setRequestHeader("Authorization", "Bearer XXXXXX");
      req.setRequestHeader("Content-type", "application/json");
      var params =  JSON.stringify({arg: dataString});

      req.send(params);
      req.onload = function() {
        console.log(req.responseText);
      }
  }

  $scope.setEnabled = function() {
    if($scope.parameters.patState) {
      document.getElementById("settings-button11").removeAttribute("disabled");
      document.getElementById("settings-button13").removeAttribute("disabled");
    } else {
      document.getElementById("settings-button11").setAttribute("disabled", true);
      document.getElementById("settings-button13").setAttribute("disabled", true);
    }
    if($scope.parameters.audioMode!="BLUE") {
      document.getElementById("settings-button17").setAttribute("disabled", true);
      document.getElementById("settings-button19").setAttribute("disabled", true);
    } else {
      document.getElementById("settings-button17").removeAttribute("disabled");
      document.getElementById("settings-button19").removeAttribute("disabled");
    }
  }

  $scope.patPausePlay = function() {
    if($scope.parameters.patState) {
      $scope.patPause();
      document.getElementById("settings-button11").removeAttribute("disabled");
      document.getElementById("settings-button13").removeAttribute("disabled");
    } else {
      $scope.patPlay();
      document.getElementById("settings-button11").setAttribute("disabled", true);
      document.getElementById("settings-button13").setAttribute("disabled",true);
    }
    $scope.parameters.patState = !$scope.parameters.patState;
  }

  $scope.patPause = function() {
    $scope.passData("PAU");
  }

  $scope.patPrevious = function() {
    $scope.passData("PRE");
  }

  $scope.patNext = function() {
    $scope.passData("NXT");
  }

  $scope.patPlay = function() {
    $scope.passData("RES");
  }

  $scope.audPausePlay = function() {
    if($scope.parameters.audState) {
      $scope.audPause();
    } else {
      $scope.audPlay();
    }
    $scope.parameters.audState = !$scope.parameters.audState;
  }

  $scope.audPause = function() {
    $scope.passData("APA");
  }

  $scope.audPrevious = function() {
    $scope.passData("APR");
  }

  $scope.audNext = function() {
    $scope.passData("ANX");
  }

  $scope.audPlay = function() {
    $scope.passData("APL");
  }

  $scope.sendBrightness = function() {
    $scope.passData("BRT"+$scope.parameters.brightness);
  }

  $scope.sendVolume = function() {
    $scope.passData("VOL"+$scope.parameters.volume);
  }

  $scope.sendMode = function() {
    console.log("sending mode");
    $scope.passData("AUD"+$scope.parameters.audioMode);
  }


  $scope.patChoice = function() {
    if($scope.parameters.patState) {
      return "Pause";
    } else {
      return "Play";
    }
  }

  $scope.audChoice = function() {
    if($scope.parameters.audState) {
      return "Pause";
    } else {
      return "Play";
    }
  }

});
