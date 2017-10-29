angular.module('app.controllers')

.controller('editPatternCtrl',  // The following is the constructor function for this page's controller. See https://docs.angularjs.org/guide/controller
// You can include any angular dependencies as parameters for this function
// TIP: Access Route Parameters for your page via $stateParams.parameterName
function ($scope, $state, $stateParams, $ionicPopup, $compile) {
  window.$scope = $scope;
  window.$stateParams = $stateParams;
  $scope.pattern = $stateParams.set.patterns[$stateParams.patIndex];
  $scope.currentColor = {red: 0, green: 0, blue:0};
  $scope.deleting = false;
  $scope.patternArguments = [];
  $scope.argumentValues = [];
  $scope.appControlled = false;
  $scope.palleteColors = ["FFFFFF", "FF0000", "00FF00", "0000FF", "FFFF00", "FF00FF", "00FFFFF", "000000", "FFA500"];
  $scope.newPalleteColor = {};
  $scope.currentPalleteColor = "FFFFFF";
  $scope.painting = false;
  $scope.pixelColors = [];

  //get list of patterns from database
  var req = new XMLHttpRequest();
  req.open("GET", "https://api.mlab.com/api/1/databases/desklamp/collections/patterns?apiKey=XXXXXX", true);
  req.setRequestHeader("Content-type", "application/json");
  req.withCredentials = true;
  req.onload = function() {
    var response = req.responseText;
    console.log(response);
    var patternData = JSON.parse(response);
    for(var i = 0; i < patternData.length; i++) {
      if(patternData[i].code==$scope.pattern.code) {
        if(patternData[i].arguments.length>0) {
          if(patternData[i].arguments[0].html!="App Controlled") {
            console.log(patternData[i].arguments[0].html);
            $scope.patternArguments = patternData[i].arguments;
            $scope.argumentValues = $scope.parseOut($scope.pattern.arguments,",");
            console.log($scope.argumentValues);
          } else {
            $scope.patternArguments = [];
            $scope.appControlled = true;
          }
        }

      }
    }
    $scope.$digest();
    //special patterns (currently only "paint" pattern) that require special code on the app side
    if($scope.appControlled) {
      var item = document.getElementById("appControlled");
      $scope.appControlledItem(item, $scope.pattern.arguments);
    } else {
      //otherwise display all attributes of a pattern
      for(var i = 0; i < $scope.patternArguments.length; i++) {
        var element = document.getElementById("arg "+i);
        for(var property in $scope.patternArguments[i].html) {
            element.setAttribute(property, $scope.patternArguments[i].html[property]);
        }

        var parentItem = element.parentNode;
        parentItem.setAttribute("class", $scope.argClass(i));
      }
    }
  }
  req.send();

  $scope.appControlledItem = function(item, args) {
    var patCode = $scope.pattern.code;
    //paint pattern code
    if(patCode=="PT") {
      var palette = document.createElement("div");
      palette.setAttribute("id", "pal");
      palette.setAttribute("style", "height:35px");
      item.appendChild(palette);
      $scope.palleteFromCode(args.substring(30));
      $scope.currentPalleteColor = $scope.palleteColors[0];
      for(var i = 0; i < 9; i++) {
        var col = document.createElement("div");
        col.setAttribute("style", "background-color: #{{palleteColors["+i+"]}}; height:35px; float:right; width:"+parseInt((item.offsetWidth-34)/9)+"px");
        col.setAttribute("on-hold", "shoot('hold'); changePaletteColor("+i+"); $event.stopPropagation()");
        col.setAttribute("ng-click", "setPaintColor("+i+")");
        col.setAttribute("id", "pal "+i);
        $compile(col)($scope);
        palette.appendChild(col);
      }
      var leds = args.substring(0,30);
      $scope.pixelColors = $scope.pixels(leds);
      for(var i = 0; i < 60; i++) {
        var pixel = document.createElement("div");
        pixel.setAttribute("style", "background-color: #{{palleteColors[pixelColors["+i+"]]}}; height:3px");
        pixel.setAttribute("onmousedown", "$scope.startPainting()");
        pixel.setAttribute("onmouseup", "$scope.endPainting()");
        pixel.setAttribute("onmousemove", "$scope.draw("+i+")");
        pixel.setAttribute("touchstart", "$scope.startPainting()");
        pixel.setAttribute("touchend", "$scope.endPainting()");
        pixel.setAttribute("touchmove", "$scope.draw("+i+")");
        $compile(pixel)($scope);
        item.appendChild(pixel);
      }
    }
  }

//paint related functions
  $scope.draw = function(i) {
    console.log("mouse");
    if($scope.painting) {
      $scope.pixelColors[i] = $scope.currentPalleteColor;
      $scope.$digest();
    }
  }
  $scope.setPaintColor = function(i) {
    $scope.currentPalleteColor = $scope.palleteColors[i];
  }

  $scope.startPainting = function() {
    console.log("painting");
    $scope.painting = true;
  }

  $scope.endPainting = function() {
    $scope.painting = false;
  }

  $scope.shoot = function(string) {
    console.log(string);
  }

  $scope.changePaletteColor = function(index) {
    console.log("change pallete color");
    $scope.newPalleteColor.red = parseInt($scope.palleteColors[index].substring(0,2),16)/4;
    $scope.newPalleteColor.green = parseInt($scope.palleteColors[index].substring(2,4),16)/4;
    $scope.newPalleteColor.blue = parseInt($scope.palleteColors[index].substring(4,6),16)/4;
    var changeColorPopup = $ionicPopup.show({
      template: "<ion-item class = 'range range-positive'>Red<input type = 'range' min = '0' max = '63' ng-model='newPalleteColor.red' step = '1'></ion-item><ion-item class = 'range range-positive'>Green<input type = 'range' min = '0' max = '63' ng-model='newPalleteColor.green' step = '1'></ion-item><ion-item class = 'range range-positive'>Blue<input type = 'range' min = '0' max = '63' ng-model='newPalleteColor.blue' step = '1'></ion-item><ion-item style = 'background-color: #{{hexColor(newPalleteColor.red, newPalleteColor.green, newPalleteColor.blue, 4)}}'></ion-item>",
      title: "Change Color",
      scope: $scope,
      buttons: [
        {
          text: 'Cancel'
        },
        {
          text: '<b>Save</b>',
          type: 'button-positive',
          onTap: function(e) {
            $scope.palleteColors[index] = $scope.hexColor($scope.newPalleteColor.red, $scope.newPalleteColor.green, $scope.newPalleteColor.blue,4);
          }
        }
      ]
    });
  }

//app controlled item - get argument to send to microcontroller
  $scope.parseAppArgs = function() {
    var patCode = $scope.pattern.code;
    if(patCode=="PT") {
      var palleteString = $scope.codeFromPallete($scope.palleteColors);
      var pixelString = $scope.codeFromPixels();
      return pixelString + palleteString;
    }
  }

//paint related functions
  $scope.codeFromPixels = function() {
    var code = "";
    for(var i = 0; i < 30; i++) {
      //var col1 = $scope.palleteColors.indexOf($scope.pixelColors[i*2]);
      //var col2 = $scope.palleteColors.indexOf($scope.pixelColors[i*2+1]);
      var col1 = $scope.pixelColors[i*2];
      var col2 = $scope.pixelColors[i*2+1];
      var charToAdd = String.fromCharCode(col1*9+col2+32);
      code += charToAdd;
    }
    return code;
  }

  $scope.pixels = function(leds) {
    var arr = [];
    for(var i = 0; i < 30; i++) {
      var pairNum = leds.charCodeAt(i)-32;
      var col1 = parseInt(pairNum/9);
      var col2 = pairNum % 9;
      arr.push(col1);
      arr.push(col2);
      //arr.push($scope.palleteColors[col1]);
      //arr.push($scope.palleteColors[col2]);
    }
    return arr;
  }

  $scope.fillArray = function(item, num) {
    var arr = [];
    for(var i = 0; i < num; i++) {
      arr[i] = item;
    }
    return arr;
  }

  $scope.palleteFromCode = function(code) {
    //console.log(code);
    for(var i = 0; i < 9; i++) {
      var colorString = code.substring(i*3, (i+1)*3);
      var fullColorString = "";
      for(var c = 0; c < 3; c++) {
        var rgb = (colorString.charCodeAt(c)-32)*4;
        fullColorString += $scope.twoDigits(rgb.toString(16));
      }
      $scope.palleteColors[i] = fullColorString;
    }
    return $scope.palleteColors;
  }

//make a number two digits long
  $scope.twoDigits = function(str) {
    if(str.length==1) {
      return "0"+str;
    } else {
      return str;
    }
  }

  $scope.codeFromPallete = function(pal) {
    var colorString = "";
    for(var i = 0; i<9; i++) {
      var currentColor = pal[i];
      console.log(currentColor);
      for(var c = 0; c < 3; c++) {
        var rgb = parseInt(currentColor.substring(c*2, (c+1)*2),16)/4;
        colorString += String.fromCharCode(rgb+32);
      }
    }
    return colorString;

  }

//stylize elements based on type of input
  $scope.argClass = function(index) {
    var patArg = $scope.patternArguments[index];
    var argType = patArg.html.type;
    if(argType=="range") {
      return "range range-positive";
    } else if(argType=="text") {
      return "";
    } else if(argType=="radio") {
      return "item item-radio";
    } else if(argType=="checkbox") {
      return "item item-toggle";
    } else if(argType=="number") {
      return "";
    }
    return "";
  }

//parse a list of arguments
  $scope.parseOut = function(argList, delimiter) {
    console.log("In parse out");
    var parsed = [];
    while(argList.indexOf(delimiter)>=0) {
      parsed.push(argList.substring(0,argList.indexOf(delimiter)));
      argList = argList.substring(argList.indexOf(delimiter)+1);
    }
    parsed.push(argList);
    return parsed;
  }

//change a color -  popup
  $scope.colorChange = function(index) {
    $scope.currentColor.red = parseInt($scope.pattern.colors[index].substring(0,2),16);
    $scope.currentColor.green = parseInt($scope.pattern.colors[index].substring(2,4),16);
    $scope.currentColor.blue = parseInt($scope.pattern.colors[index].substring(4,6),16);
    var changeColorPopup = $ionicPopup.show({
      template: "<ion-item class = 'range range-positive'>Red<input type = 'range' min = '0' max = '255' ng-model='currentColor.red' step = '1'></ion-item><ion-item class = 'range range-positive'>Green<input type = 'range' min = '0' max = '255' ng-model='currentColor.green' step = '1'></ion-item><ion-item class = 'range range-positive'>Blue<input type = 'range' min = '0' max = '255' ng-model='currentColor.blue' step = '1'></ion-item><ion-item style = 'background-color: #{{hexColor(currentColor.red, currentColor.green, currentColor.blue, 1)}}'></ion-item>",
      title: "Change Color",
      scope: $scope,
      buttons: [
        {
          text: 'Cancel'
        },
        {
          text: '<b>Save</b>',
          type: 'button-positive',
          onTap: function(e) {
            $scope.pattern.colors[index] = $scope.hexColor($scope.currentColor.red, $scope.currentColor.green, $scope.currentColor.blue, 1);
          }
        }
      ]
    });
  }


//add a color popup
  $scope.addColor = function() {
    $scope.currentColor = {red: 128, green: 128, blue: 128};
    var addColorPopup = $ionicPopup.show({
      template: "<ion-item class = 'range range-positive'>Red<input type = 'range' min = '0' max = '255' ng-model='currentColor.red' step = '1'></ion-item><ion-item class = 'range range-positive'>Green<input type = 'range' min = '0' max = '255' ng-model='currentColor.green' step = '1'></ion-item><ion-item class = 'range range-positive'>Blue<input type = 'range' min = '0' max = '255' ng-model='currentColor.blue' step = '1'></ion-item><ion-item style = 'background-color: #{{hexColor(currentColor.red, currentColor.green, currentColor.blue,1)}}'></ion-item>",
      title: "Add Color",
      scope: $scope,
      buttons: [
        {
          text: 'Cancel'
        },
        {
          text: "<b>Save<b>",
          type: "button-positive",
          onTap: function(e) {
            $scope.pattern.colors.push($scope.hexColor($scope.currentColor.red, $scope.currentColor.green, $scope.currentColor.blue,1));
          }
        }
      ]
    });
  }

//get hex value of a color (mult is a factor to scale rgb values by)
  $scope.hexColor = function(red, green, blue, mult) {
    var redString = parseInt(red*mult).toString(16).toUpperCase();
    if(redString.length<2) {
      redString = "0" + redString;
    }
    var greenString = parseInt(green*mult).toString(16).toUpperCase();
    if(greenString.length<2) {
      greenString = "0" + greenString;
    }
    var blueString = parseInt(blue*mult).toString(16).toUpperCase();
    if(blueString.length<2) {
      blueString = "0" + blueString;
    }
    return redString + greenString + blueString;
  }

  $scope.currentlyDeleting = function() {
    return $scope.deleting;
  }

  $scope.toggleDeleting= function() {
    $scope.deleting = !$scope.deleting;
  }

  $scope.deleteColor = function(index) {
    $scope.pattern.colors.splice(index,1);
  }

  $scope.reorderColors = function(from, to) {
    var item = $scope.pattern.colors[from];
    $scope.pattern.colors.splice(from, 1);
    $scope.pattern.colors.splice(to, 0, item);
  }

//turn arguments into delimited string
  $scope.parseIn = function(args) {
    var parseString = "";
    if(args.length>0) {
      for(var i = 0; i <args.length-1; i++) {
        parseString += args[i] + ",";
      }
      parseString += args[args.length-1];
    }

    return parseString;
  }
  $scope.doneEditing = function() {
    if($scope.appControlled) {
      $scope.pattern.arguments = $scope.parseAppArgs();
    } else {
      $scope.pattern.arguments = $scope.parseIn($scope.argumentValues);
    }
    var params = angular.toJson($stateParams.set);
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
    console.log($stateParams.set);
    $state.go("tabsController.editPatternSet", {set: $stateParams.set})
  }

  $scope.sendBrightness = function() {
    $scope.passData("BRT"+$scope.parameters.brightness);
  }

  $scope.sendVolume = function() {
    $scope.passData("VOL"+$scope.parameters.volume);
  }

})
