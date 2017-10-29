angular.module('app.controllers')

.controller('editPatternSetCtrl', // The following is the constructor function for this page's controller. See https://docs.angularjs.org/guide/controller
// You can include any angular dependencies as parameters for this function
// TIP: Access Route Parameters for your page via $stateParams.parameterName
function ($scope, $stateParams, $state, $ionicPopup) {
  window.$scope = $scope;
  window.$stateParams = $stateParams;
  $scope.editSet = $stateParams.set;
  $scope.patternTypes = [];
  $scope.selectPattern = {pat:{}}
  $scope.things = [1,2,3,4,5];
  $scope.reordering = false;

  console.log($scope.editSet);

  var req = new XMLHttpRequest();
  req.open("GET", "https://api.mlab.com/api/1/databases/desklamp/collections/patterns?apiKey=XXXXXX", true);
  req.withCredentials = true;
  console.log("sending request");
  req.onload = function() {
    var patResponse = req.responseText
    console.log(patResponse);
    $scope.patternTypes = JSON.parse(patResponse);
  }
  req.send();

  $scope.setup = function() {
    $scope.editSet = $stateParams.set;

  }

  $scope.reorderPatterns = function(from, to) {
    console.log("from: "+from+", to: "+to);
    console.log("reordering");
    var toAdd = $scope.editSet.patterns[from];
    console.log("taking "+toAdd.name);
    console.log("removing "+$scope.editSet.patterns.splice(from,1)[0].name+" from "+from);
    $scope.editSet.patterns.splice(to, 0, toAdd);
    console.log("adding it to "+to);

  }

//go to edit pattern page if you click on a pattern
  $scope.clickOn = function(index) {
    $state.go("tabsController.editPattern", {set: $scope.editSet, patIndex: index});
  }

  $scope.addPattern = function() {
    var addPopup = $ionicPopup.show({
      template: "<select ng-options='pat as pat.name for pat in patternTypes' ng-model='selectPattern.pat'></select>",
      title: "Add Pattern",
      scope: $scope,
      buttons: [
        { text: "Cancel"},
        {
          text: "<b>Save</b>",
          type: "button-positive",
          onTap: function(e) {
            var newPat = {
              name: $scope.selectPattern.pat.name,
              code: $scope.selectPattern.pat.code,
              colors: [],
              brightness: 128,
              volume: 128,
              speed: 200,
              arguments: ""
            }
            $scope.editSet.patterns.push(newPat);
          }
        }
      ]
    })
  }

//make a number n digits long
  $scope.digits = function(num, digits) {
    var s = String(num);
    while(s.length < digits) {
      s = "0" + s;
    }
    return s;
  }

  $scope.doneEditing = function() {
    console.log($scope.editSet.delay);
    var id = $scope.editSet._id;
    var params = angular.toJson($scope.editSet);
    console.log(params);
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "https://api.mlab.com/api/1/databases/desklamp/collections/pattern_sets?apiKey=XXXXXX", true);
    xhr.setRequestHeader("Content-type", "application/json");
    xhr.withCredentials = true;
    xhr.onload = function() {
      var response = xhr.responseText;
      console.log("Response from CORS request: "+response);
    }
    xhr.send(params);

    $state.go("tabsController.patternSets");
  }

  $scope.deletePat = function(index) {
    var deletePopup = $ionicPopup.confirm({
      title: 'Delete Pattern',
      template: "Are you sure you want to delete " + $scope.editSet.patterns[index].name + "? Action cannot be undone."
    });

    deletePopup.then(function(res) {
      if(res) {
        $scope.editSet.patterns.splice(index,1);
      }
    });
  }
})
