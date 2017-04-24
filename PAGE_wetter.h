const char PAGE_Wetter[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="de">
<head>
  <title>Wetterstation</title>
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <!-- The above 3 meta tags *must* come first in the head; any other head content must come *after* these tags -->
  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css" integrity="sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u" crossorigin="anonymous">
  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap-theme.min.css" integrity="sha384-rHyoN1iRsVXV4nD0JutlnGaslCJuC7uwjduW9SVrLvRYooPp2bWYgmgJQIXwl/Sp" crossorigin="anonymous">
  <!--[if lt IE 9]>
 <script src="https://oss.maxcdn.com/html5shiv/3.7.3/html5shiv.min.js"></script>
  <script src="https://oss.maxcdn.com/respond/1.4.2/respond.min.js"></script>
<![endif]-->
</head>

<body>
<div class="container">
<div class="page-header"><h1>Wetterstation</h1></div>

<div id="error"></div>

<div class="panel panel-info"><div class="panel-heading"><h3 class="panel-title">Innen</h3></div><div class="panel-body">
<ul>
  <li>Temperatur: <span id="t_in"></span>&deg;C</li>
  <li>Relative Feuchtigkeit: <span id="h_in"></span>%</li>
  <li>Absolute Feuchtigkeit: <span id="f_in"></span> g/m³</li>
  <li>Taupunkt: <span id="dp_in"></span>&deg;C</li>
</ul>
</div></div>

<div class="panel panel-info"><div class="panel-heading"><h3 class="panel-title">Aussen</h3></div><div class="panel-body">
<ul>
<li>Temperatur: <span id="t_out"></span>&deg;C</li>
<li>Relative Feuchtigkeit: <span id="h_out"></span>%</li>
<li>Absolute Feuchtigkeit: <span id="f_out"></span> g/m³</li>
<li>Letzte Aktualisierung vor <span id="last_out"></span> s</li>
</ul>
</div></div>

<div class="panel panel-primary"><div class="panel-heading"><h3 class="panel-title">Absolute Feuchtigkeitsdifferenz Innen/Aussen</h3></div><div class="panel-body">
<ul>
<li><span id="f_diff"></span> g/m³</li>
</ul>
</div></div>

<!-- /container -->
<script src="https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js"></script>
<script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js" integrity="sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa" crossorigin="anonymous"></script>

<script src="https://unpkg.com/axios/dist/axios.min.js"></script>

<script>
   Number.prototype.round = function(decimals) { return Number((Math.round(this + "e" + decimals)  + "e-" + decimals)); }
   var fn = function(){
     axios.get('/data.json')
       .then(function (response) {
           document.getElementById('t_in').innerHTML = response.data.t_in.round(1);
           document.getElementById('h_in').innerHTML = response.data.h_in;
           document.getElementById('f_in').innerHTML = response.data.f_in.round(1);
           document.getElementById('dp_in').innerHTML = response.data.dp_in.round(1);
           document.getElementById('t_out').innerHTML = response.data.t_out.round(1);
           document.getElementById('h_out').innerHTML = response.data.h_out;
           document.getElementById('f_out').innerHTML = response.data.f_out.round(1);
           document.getElementById('last_out').innerHTML = response.data.last_out;
           document.getElementById('f_diff').innerHTML = response.data.f_diff.round(1);
           document.getElementById('error').innerHTML = '';
        })
       .catch(function (err) {
         document.getElementById('error').innerHTML = '<div class="alert alert-danger" role="alert">' + err.message + '</div>';
       });
       };
       fn();
       var interval = setInterval(fn, 5000);
 </script>

</body></html>
)=====";
