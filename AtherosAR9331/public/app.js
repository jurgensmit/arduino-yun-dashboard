(function () {
    'use strict';

    var dashboardApp = angular.module('dashboardApp', []);

    dashboardApp.controller('dashboardController', ['$scope', 'websocket',
        function ($scope, websocket) {
            var vm = this;

            vm.tiles = {
                temperature: {
                    order: 1, title: 'Temperature', unit: '°C'
                },
                distance: {
                    order: 2, title: 'Distance', unit: 'cm'
                },
                angle: {
                    order: 3, title: 'Angle'
                },
                light: {
                    order: 4, title: 'Light'
                },
                redLed: {
                    order: 5, title: 'Red Led', class: 'red'
                },
                yellowLed: {
                    order: 6, title: 'Yellow Led', class: 'yellow'
                },
                greenLed: {
                    order: 7, title: 'Green Led', class: 'green'
                }
            };

            websocket.on(function (data) {
                console.log('controller received data: ' + data);
                vm.tiles.temperature.value = data.temperature;
                vm.tiles.distance.value = data.distance;
                vm.tiles.angle.value = data.angle;
                vm.tiles.light.value = data.light;
                vm.tiles.redLed.value = data.redLedState;
                vm.tiles.yellowLed.value = data.yellowLedState;
                vm.tiles.greenLed.value = data.greenLedState;
            });

            vm.toggleLed = function (color) {
                if (color) {
                    websocket.sendRequest('toggle_led:' + color);
                }
            };
        }
    ]);

    dashboardApp.factory('websocket', ['$q', '$rootScope', '$location',
        function ($q, $rootScope, $location) {
            var Service = {};

            // Create our websocket object with the address to the websocket
            var websocketUrl = 'ws://' + $location.host() + ':' + $location.port() + '/ws';
            var ws = new WebSocket(websocketUrl);

            ws.onopen = function () {
                console.log('WebSocket opened');
            };

            ws.onclose = function () {
                console.log('WebSocket closed');
            };

            Service.on = function (callback) {
                ws.onmessage = function (message) {
                    console.log('Received data from websocket: ', message);
                    var data = angular.fromJson(message.data);
                    $rootScope.$apply(function () {
                        callback(data);
                    });
                };
            };

            Service.sendRequest = function (request) {
                console.log('Sending request', request);
                ws.send(request);
            };

            return Service;
        }
    ]);

    dashboardApp.filter('orderObjectBy', function () {
        return function (items, field, reverse) {
            var filtered = [];
            angular.forEach(items, function (item) {
                filtered.push(item);
            });
            filtered.sort(function (a, b) {
                return (a[field] > b[field] ? 1 : -1);
            });
            if (reverse) {
                filtered.reverse();
            }
            return filtered;
        };
    });

    dashboardApp.directive('dashboardTile', function () {
        return {
            restrict: 'E',
            template:
            '<div class="tile {{tile.class}}" ng-click="toggleLed({ color: tile.class })">' +
            '<h3 class="title">{{tile.title}}</h3>' +
            '<p class="value">&nbsp;{{tile.value}}{{tile.unit}}</p></div>',
            replace: true,
            scope: {
                tile: '=',
                toggleLed: '&'
            }
        };
    });
} ());
