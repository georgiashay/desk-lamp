angular.module('app.controllers')

.controller('patternSetsCtrl', function ($scope, $stateParams, $state, $http, $ionicPopup) {


window.$scope = $scope;
$scope.deleting = false;
$scope.loaded = false;
$scope.loadedDefault = false;
$scope.sets = [];
$scope.setToAdd = {name: ""};
$scope.editSet = {name: ""};

//get pattern sets and default sets created
$scope.reloadData = function() {
  var req = new XMLHttpRequest();
  req.open("GET", "https://api.mlab.com/api/1/databases/desklamp/collections/pattern_sets?apiKey=XXXXXX", true);
  req.withCredentials = true;
  req.onload = function() {
    var response = req.responseText;
    var statust = req.statusText;
    $scope.sets = JSON.parse(response);
    $scope.loaded = true;
    $scope.$digest();
  }
  req.send();
  var creq = new XMLHttpRequest();
  creq.open("GET", "https://api.mlab.com/api/1/databases/desklamp/collections/default_sets?apiKey=XXXXXX", true);
  creq.withCredentials = true;
  creq.onload = function() {
    var response = creq.responseText;
    console.log(response);
    $scope.defaultSets = JSON.parse(response);
    $scope.loadedDefault = true;
  }
  creq.send();
}

$scope.reloadData();

//for debugging purposes
window.$scope = $scope;


//highlight a pattern set's text
$scope.highlight = function(set) {
  if($scope.loadedDefault) {
    if(set._id.$oid==$scope.defaultSets[0].setID) {
      return "#387ef5";
    } else {
      return "black";
    }
  } else {
    return "black";
  }

}

$scope.sendSet = function(index) {
  $scope.passData("PAU");
}

$scope.deleteSet = function(index) {
  var deletePopup = $ionicPopup.confirm({
    title: "Delete Pattern Set",
    template: "Are you sure you want to delete " + $scope.sets[index].name + "? Action cannot be undone."
  });

  deletePopup.then(function(res) {
    if(res) {
      console.log("Deleting #"+index);
      var xhr = new XMLHttpRequest();
      var deleteID = $scope.sets[index]._id.$oid;
      console.log("ID: "+deleteID);
      xhr.open("DELETE", "https://api.mlab.com/api/1/databases/desklamp/collections/pattern_sets/"+deleteID+"?apiKey=XXXXXX");
      xhr.setRequestHeader("Access-Control-Allow-Methods","GET, POST, OPTIONS, PUT, DELETE");
      xhr.withCredentials = true;
      xhr.send();
      xhr.onload = function() {
        console.log(xhr.responseText);
      }
      $scope.sets.splice(index, 1);
    }
  });
}

$scope.passData = function(index, commandList) {
  if(index < commandList.length) {
    var req = new XMLHttpRequest();
    req.open("POST", "https://api.particle.io/v1/devices/2d005d001051363036373538/passdata", true);
    req.setRequestHeader("Authorization", "Bearer da17b4f4c5ce108b68da85be9d5cdc21165b688b");
    req.setRequestHeader("Content-type", "application/json");
    var params =  JSON.stringify({arg: commandList[index]});
    req.send(params);
    req.onload = function() {
      console.log(req.responseText);
      $scope.passData(index+1, commandList);
    }
  }


}

$scope.getCommands = function(set) {
  var commands = [];
  commands.push("FRZ");
  commands.push("CLR");
  commands.push("DUR"+set.delay);
  for(var i = 0; i < set.patterns.length; i++) {
    var patNum = $scope.digits(i,3);
    commands.push("PAT"+patNum+set.patterns[i].code);
    commands.push("SPD"+patNum+set.patterns[i].speed);
    if(set.patterns[i].arguments!="") {
      commands.push("ARG"+patNum+set.patterns[i].arguments);
    }
    for(var c = 0; c < set.patterns[i].colors.length; c++) {
      commands.push("COL"+patNum + $scope.digits(c,3)+set.patterns[i].colors[c]);
    }
  }
  commands.push("RES");
  return commands;
}

$scope.digits = function(num, digits) {
  var s = String(num);
  while(s.length < digits) {
    s = "0" + s;
  }
  return s;
}

$scope.sendSet = function(index) {
  var currentSet = $scope.sets[index];
  var commands = $scope.getCommands(currentSet);
  $scope.passData(0, commands);
  $scope.defaultSets[0].setID = $scope.sets[index]._id.$oid;
  $scope.defaultSets[0].codes = commands;
  var req = new XMLHttpRequest();
  var params = angular.toJson($scope.defaultSets[0]);
  req.open("POST", "https://api.mlab.com/api/1/databases/desklamp/collections/default_sets?apiKey=XXXXXX", true);
  req.setRequestHeader("Content-type", "application/json");
  req.withCredentials = true;
  req.onload = function() {
    console.log(req.responseText);
  }
  req.send(params);
}

//if a pattern set is clicked on, go to the edit pattern set page
$scope.clickOn = function(thisSet) {
    $state.go("tabsController.editPatternSet", {set: thisSet});
}

$scope.addSet = function() {
  var addPopup = $ionicPopup.show({
    template: "<ion-item><ion-label>Name: </ion-label><input type = 'text' ng-model='setToAdd.name'></input></ion-item>",
    title: "Add Set",
    scope: $scope,
    buttons: [
      {
        text: 'Cancel'
      },
      {
        text: '<b>Save</b>',
        type: 'button-positive',
        onTap: function(e) {
          var req = new XMLHttpRequest();
          req.open("POST", "https://api.mlab.com/api/1/databases/desklamp/collections/pattern_sets?apiKey=XXXXXX", true);
          req.setRequestHeader("Content-type", "application/json");
          req.withCredentials = true;
          var newSet = {
            "name": $scope.setToAdd.name,
            "delay": 5000,
            "audio_mode": "NONE",
            "patterns": []
          }
          var params = angular.toJson(newSet);
          req.send(params);
          req.onload = function() {
          }
          $scope.reloadData();
        }
      }
    ]
  });
}

$scope.editName = function(index) {
  $scope.editSet.name = $scope.sets[index].name;
  var editPopup = $ionicPopup.show({
    template: "<input type = 'text' ng-model='editSet.name'></input>",
    title: "Change Set Name",
    scope: $scope,
    buttons: [
      {
        text: 'Cancel'
      },
      {
        text: '<b>Save</b',
        type: 'button-positive',
        onTap: function(e) {
          $scope.sets[index].name = $scope.editSet.name;
          var req = new XMLHttpRequest();
          var params = angular.toJson($scope.sets[index]);
          req.open("POST", "https://api.mlab.com/api/1/databases/desklamp/collections/pattern_sets?apiKey=XXXXX", true);
          req.setRequestHeader("Content-type", "application/json");
          req.withCredentials = true;
          req.send(params);
        }
      }
    ]
  })
}

});
