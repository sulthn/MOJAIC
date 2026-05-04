import {
    chart
} from "./graph.js";

let delay = 5 * 1000;
let max_t = 100 * 1000;
let elapsed = 0;

export let temperature = [];

window.addEventListener('load', function () {
    var websocket = new WebSocket(`ws://${window.location.hostname}/ws`);
    websocket.onopen = function (event) {
        console.log('Connection established');
    }
    websocket.onclose = function (event) {
        console.log('Connection died');
    }
    websocket.onerror = function (error) {
        console.log('error');
    };
    websocket.onmessage = function (event) {
        document.getElementById('state').innerHTML = event.data;
        
        temperature.push(
            {
                time: (elapsed - delay) / 1000, 
                temp: Number((+event.data).toFixed(1))
            }
        );
        /*if (temperature.length > temperature_max) {
            temperature.shift();
        }*/
        
        console.log(event.data, (+event.data).toFixed(1), temperature[Math.max(temperature.length - 1, 0)]);

        document.getElementById("graph").replaceChildren(chart(delay));
        document.getElementById("dtable").innerHTML += "<tr><td>" + ((elapsed - delay) / 1000) + "</td><td>" + (+event.data).toFixed(1) + "</td></tr>";
    };

    let intervalid;

    function run() {
        elapsed += delay;
        websocket.send('toggle');

        if (elapsed > max_t) {
            clearInterval(intervalid);
        }
    }

    document.getElementById("graph").append(chart());

    document.getElementById('run-btn').addEventListener('click', function () {
        temperature = [];
        elapsed = 0;
        document.getElementById('dtable').innerHTML = "<tr><th>time /s</th><th>temperature /oC</th></tr>";


        intervalid = setInterval(run, delay);

        console.log("starting ", max_t);

        // second 0
        run();

        /*setTimeout(() => {
            clearInterval(intervalid);
        }, max_t);*/
    });

    document.getElementById('max2').oninput = function() {
        document.getElementById('max_b').innerHTML = 'record for ' + this.value + ' seconds';
        max_t = +this.value * 1000;
    }

    document.getElementById('poll').oninput = function () {
        document.getElementById('poll_b').innerHTML = 'record every ' + this.value + ' seconds';
        delay = +this.value * 1000;
    }
});