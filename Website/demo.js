var adamData=[[Date.now(),0],[Date.now()+100,0]];
var nilsData=[[Date.now(),0],[Date.now()+100,0]];
var bör=[[Date.now(),0],[Date.now()+100,0]] 

function startConnect() { 
    updateScroll();

    host = "maqiatto.com";
    port = 8883; //hämtar värden från html dokumentet

    clientID = "clientID-" + parseInt(Math.random() * 100); //Skapar ett ID för vår client 

    client = new Paho.MQTT.Client(host, Number(port), clientID); //skapar en variabel så vi slipper skriva så mycket

    client.onConnectionLost = onConnectionLost;
    client.onMessageArrived = onMessageArrived; //Om vi tappar anslutning eller det kommer meddelanden så anropas funktioner
    client.connect({userName: "nils.borg@abbindustrigymnasium.se", password : "dodoärbra",
        onSuccess: onConnect,
        onFailure: onOff, //anropar olika funktioner beroende på hur det går när vi ansluter
    });
}

function onConnect() {
    document.getElementById("messages").innerHTML += '<span>Connected to ' + host + ' at ' + port +  '</span><br/>';
    topic = "nils.borg@abbindustrigymnasium.se/";
    client.subscribe(topic+"samrob");
    client.subscribe(topic+"samrob_Nils");
    client.subscribe(topic+"samrob_Adam");
    message= new Paho.MQTT.Message("It do the connect!");
    message.destinationName=topic+"samrob";
    client.send(message); //skickar ett meddelande 
}

function onConnectionLost(responseObject) {
    document.getElementById("messages").innerHTML += '<span>ERROR: Connection lost</span><br/>'; //när vi tappar anslutningen säger den det
    if (responseObject.errorCode !== 0) {
        document.getElementById("messages").innerHTML += '<span>ERROR: ' + responseObject.errorMessage + '</span><br/>'; //visar felet
    }
}

function onMessageArrived(message) {
    if (message.destinationName == topic+"samrob_Nils") {
        nilsData.push([Date.now(),message.payloadString]);
        bör.push([Date.now(), bör[bör.length-1][1]]);
        if (nilsData.length>50){
          nilsData.shift();
        }
        if (bör.length>50){
          bör.shift();
        }
        document.getElementById("nils").value=message.payloadString;
        updateChartNils();
    }
    else if (message.destinationName == topic+"samrob_Adam") {
        adamData.push([Date.now(), message.payloadString]);
        bör.push([Date.now(), bör[bör.length-1][1]]);
        if (adamData.length>50){
          adamData.shift();
        }
        if (bör.length>50){
          bör.shift();
        }
        document.getElementById("adam").value=message.payloadString;
        updateChartAdam();
    } 
    document.getElementById("messages").innerHTML += '<span>Topic: ' + message.destinationName + ' | ' + message.payloadString + '</span><br/>'; //visar meddelandet som kommer in i html
    updateScroll();
;}

function startDisconnect() { //funktionen kallas när vi trycker på disconnect
    client.disconnect(); //clienten disconnectar
    document.getElementById("messages").innerHTML += '<span>Disconnected</span><br/>'; //den säger att den disconnectas
    updateScroll(); 
    
}

function updateScroll() {
    var element = document.getElementById("messages");
    element.scrollTop = element.scrollHeight; //scrollar till meddelandet
}

function onOff(){
    message= new Paho.MQTT.Message("change");
    message.destinationName=topic+"samrob_onoff";
    client.send(message);
    document.getElementById("messages").innerHTML += '<span>Car state changed!</span><br/>';

} 

function slide() {
    var slider = document.getElementById("myRange");
    var speed=document.getElementById("speed");
    speed.value=slider.value;
}

function typer() {
    var slider = document.getElementById("myRange");
    var speed = document.getElementById("speed");
    slider.value=speed.value;

}

function send() {
    var slider = document.getElementById("myRange");
    document.getElementById("bör").value=slider.value;
    bör.push([Date.now(), slider.value]);
    if (bör.length>50){
      bör.shift();
    }
    updateChartAdam();
    updateChartNils();
    message= new Paho.MQTT.Message(slider.value);
    message.destinationName=topic+"samrob_speed";
    client.send(message);
    document.getElementById("messages").innerHTML += '<span>Set new speed at ' + slider.value +  ' cm/s</span><br/>'; 
}

function drawCharts(){
    var options = {
        chart: {
          type: 'line',
          height: 275,
        },
        dataLabels: {
          enabled: false
        },
        series: [
            {
            name: 'Speed',
            data: nilsData
            },
          {
            name: 'Setpoint',
            data: bör
          },
        ],

        xaxis: {
          type: 'datetime',
        },
        stroke: {
            curve: 'smooth',
        },
        yaxis: {
            min: 0,
            max: 60,
        },
        tooltip: {
          x: {
            format: 'mm:ss:ff'
          }
        },
        legend: {
            show: false,
        }
  
    }
    var chart = new ApexCharts(
        document.querySelector("#chart2"),
        options
    );
  
    chart.render();

    options = {
        chart: {
          type: 'line',
          height: 275,
        },
        dataLabels: {
          enabled: false
        },
        series: [{
            name: 'Speed',
            data: adamData  
          },
          {
            name: 'Setpoint',
            data: bör
          },
        ],

        xaxis: {
          type: 'datetime',
        },
        stroke: {
            curve: 'smooth',
        },
        legend: {
            show: false
        },
        yaxis: {
            min: 0,
            max: 60,
        },
        tooltip: {
          x: {
            format: 'mm:ss:ff'
          }
        },
  
    }
    chart = new ApexCharts(
        document.querySelector("#chart1"),
        options
    );
  
    chart.render();
}

function updateChartNils(){
    var options = {
        chart: {
          type: 'line',
          height: 275,
        },
        dataLabels: {
          enabled: false
        },
        series: [{
            name: 'Speed',
            data: nilsData
          },
          {
            name: 'Setpoint',
            data: bör
          },
        ],

        xaxis: {
          title: 'Time',
          type: 'datetime',
        },
        stroke: {
            curve: 'smooth',
        },
        yaxis: {
            min: 0,
            max: 60,
        },
        tooltip: {
          x: {
            format: 'mm:ss:ff'
          }
        },
        legend: {
            show: false
        },
    }
    var chart = new ApexCharts(
        document.querySelector("#chart2"),
        options
    );

    chart.updateSeries([{
        data: nilsData
    },
    {
        data: bör
}],false);
    chart.render();
}

function updateChartAdam() {
    var options = {
        chart: {
          type: 'line',
          height: 275,
        },
        dataLabels: {
          enabled: false
        },
        series: [{
            name: 'Speed',
            data: adamData
          },
          {
            name: 'Setpoint',
            data: bör
          },
        ],

        xaxis: {
          title: 'Time',
          type: 'datetime',
        },
        yaxis: {
            min: 0,
            max: 60,
        },
        stroke: {
            curve: 'smooth',
        },
        tooltip: {
          x: {
            format: 'mm:ss:ff'
          }
        },
        legend: {
            show: false
        },
  
    }
    var chart = new ApexCharts(
        document.querySelector("#chart1"),
        options
    );

    chart.updateSeries([{
        data: adamData
    },
    {
        data: bör
}],false);
    chart.render();
}
