//Codigo pagina WEB

#ifndef servidor_web_h
#define servidor_web_h

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>

<body>
  <h2>Servidor WEB</h2>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Fecha:</span>
    <span id="hora">HORA</span>
  </p>
  <form action="/actualizar" method="get" id="formReloj">
    <input type="text" name="year" placeholder="Ano (AAAA)" id="ano"><br>
    <input type="text" name="mounth" placeholder="Mes (MM)" id="mes"><br>
    <input type="text" name="day" placeholder="Dia (DD)" id="dia"><br>
    <input type="text" name="hour" placeholder="Hora (HH)" id="hora"><br>
    <input type="text" name="minutes" placeholder="Minutos (MM)" id="min"><br>
    <input type="text" name="seconds" placeholder="Segundos (SS)" id="seg"><br>
    <a href=\"/on\"><button type='submit'>Actualizar</button></a>
  </form>
      </p>
      <br>
  <span class="dht-labels">Tiempo escucha:</span>
  <span id="tiempoescucha">ESCUCHA</span>
  <form action="/tiempo_escucha" id="t_escucha">
    <input type="text" name="tiempo_escucha" placeholder="Tiempo escucha (segundos)" id="escucha">
    <a href=\"/on\"><button type='submit'>Guardar</button></a>
  </form>

  <span class="dht-labels">Tiempo dormido:</span>
  <span id="tiempodormido">DORMIDO</span>
  <form action="/tiempo_dormido" id="t_dormido">
    <input type="text" name="tiempo_dormido" placeholder="Tiempo dormido (segundos)" id="dormido">
    <a href=\"/on\"><button type='submit'>Guardar</button></a>
  </form>

  <span class="dht-labels">Canal 1:</span>
  <span id="estacion1">ESTACION1</span>
  <form action="/canal1" id="estacion_1">
    <input type="text" name="canal1" placeholder="Canal 1" id="canal_1">
    <a href=\"/on\"><button type='submit'>Guardar</button></a>
  </form>

  <span class="dht-labels">Canal 2:</span>
  <span id="estacion2">ESTACION2</span>
  <form action="/canal2" id="estacion_2">
    <input type="text" name="canal2" placeholder="Canal 2" id="canal_2">
    <a href=\"/on\"><button type='submit'>Guardar</button></a>
  </form>

  <span class="dht-labels">Canal 3:</span>
  <span id="estacion3">ESTACION3</span>
  <form action="/canal3" id="estacion_3">
    <input type="text" name="canal3" placeholder="Canal 3" id="canal_3">
    <a href=\"/on\"><button type='submit'>Guardar</button></a>
  </form>
  <br>
  <br>
  <a href='/files'><button>Archivos</button></a>
</body>

<script type="text/javascript">
   document.addEventListener("DOMContentLoaded", function() {
  document.getElementById("formReloj").addEventListener('submit', validarFormReloj); 
});
   document.addEventListener("DOMContentLoaded", function() {
  document.getElementById("t_dormido").addEventListener('submit', validarDormido); 
});
   document.addEventListener("DOMContentLoaded", function() {
  document.getElementById("t_escucha").addEventListener('submit', validarEscucha); 
});
   document.addEventListener("DOMContentLoaded", function() {
  document.getElementById("estacion_1").addEventListener('submit', validarCanalA); 
});
   document.addEventListener("DOMContentLoaded", function() {
  document.getElementById("estacion_2").addEventListener('submit', validarCanalB); 
});
   document.addEventListener("DOMContentLoaded", function() {
  document.getElementById("estacion_3").addEventListener('submit', validarCanalC); 
});

function validarFormReloj(evento) {
  evento.preventDefault();
  var ano = document.getElementById('ano').value;
  var mes = document.getElementById('mes').value;
  var dia = document.getElementById('dia').value;
  var hora = document.getElementById('hora').value;
  var min = document.getElementById('min').value;
  var seg = document.getElementById('seg').value;
  if ((ano=="")||(mes=="")||(dia=="")||(hora=="")||(min=="")||(seg=="")) {
    alert('Hay un campo vacio');
    return false;
  };
  this.submit();
};
function validarDormido(evento) {
  evento.preventDefault();
  var tdormido = document.getElementById('dormido').value;
  if (tdormido==""){
    alert('El campo esta vacio');
    return false;
  };
  if ((tdormido<5)||(tdormido>15)) {
    alert('El tiempo dormido debe estar entre 5 min y 15 min');
    return false;
  };
  this.submit();
};
function validarEscucha(evento) {
  evento.preventDefault();
  var tescucha = document.getElementById('escucha').value;
  if (tescucha==""){
    alert('El campo esta vacio');
    return false;
  };
  if ((tescucha<60)) {
    alert('El tiempo de escucha debe de ser mayor a 60 segundos');
    return false;
  };
  this.submit();
};
function validarCanalA(evento) {
  evento.preventDefault();
  var can1 = document.getElementById('canal_1').value;
  var can2 = document.getElementById("estacion2").innerHTML;
  var can3 = document.getElementById("estacion3").innerHTML;
  if (can1==""){
    alert('El campo esta vacio');
    return false;
  };
  if ((can1<0)||(can1>7)){
    alert('El canal debe estar entre 0 y 7');
    return false;
  };
  if ((can1==can2)){
    alert('Este canal es igual al canal 2');
    return false;
  };
  if ((can1==can3)){
    alert('Este canal es igual al canal 3');
    return false;
  };
  this.submit();
};
function validarCanalB(evento) {
  evento.preventDefault();
  var can2 = document.getElementById('canal_2').value;
  var can1 = document.getElementById("estacion1").innerHTML;
  var can3 = document.getElementById("estacion3").innerHTML;
  if (can2==""){
    alert('El campo esta vacio');
    return false;
  };
  if ((can2<0)||(canal2>7)) {
    alert('El canal debe estar entre 0 y 7');
    return false;
  };
  if ((can1==can2)){
    alert('Este canal es igual al canal 1');
    return false;
  };
  if ((can2==can3)){
    alert('Este canal es igual al canal 3');
    return false;
  };
  this.submit();
};
function validarCanalC(evento) {
  evento.preventDefault();
  var can3 = document.getElementById('canal_3').value;
  var can2 = document.getElementById("estacion2").innerHTML;
  var can1 = document.getElementById("estacion1").innerHTML;
  if (can3==""){
    alert('El campo esta vacio');
    return false;
  };
  if ((can3<0)||(can3>7)) {
    alert('El canal debe estar entre 0 y 7');
    return false;
  };
  if ((can3==can2)){
    alert('Este canal es igual al canal 2');
    return false;
  };
  if ((can1==can3)){
    alert('Este canal es igual al canal 1');
    return false;
  };
  this.submit();
};
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("tiempoescucha").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/escucha", true);
  xhttp.send();
}, 1000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("tiempodormido").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/dormido", true);
  xhttp.send();
}, 1000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("estacion1").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/estacion1", true);
  xhttp.send();
}, 1000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("estacion2").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/estacion2", true);
  xhttp.send();
}, 1000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("estacion3").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/estacion3", true);
  xhttp.send();
}, 1000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("hora").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/hora", true);
  xhttp.send();
}, 1000 ) ;

</script>
</html>)rawliteral";

#endif 