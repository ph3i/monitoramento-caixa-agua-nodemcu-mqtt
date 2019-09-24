(function() {
	window.Main = {};

	Main.Page = (function() {

		var mosq = null;

		function Page() {

			var _this = this;
			mosq = new Mosquitto();

			$('#connect-button').click(function() {
				return _this.connect();
			});
			$('#disconnect-button').click(function() {
				return _this.disconnect();
			});
			$('#subscribe-button').click(function() {
				return _this.subscribe();
			});
			$('#unsubscribe-button').click(function() {
				return _this.unsubscribe();
			});
			$('#liga-output').click(function() {
				var payload = "L";  
				var TopicPublish = $('#pub-topic-text')[0].value;				
				mosq.publish(TopicPublish, payload, 0);
			});
			$('#desliga-output').click(function() {
				var payload = "D";  
				var TopicPublish = $('#pub-topic-text')[0].value;				
				mosq.publish(TopicPublish, payload, 0);
			});

			mosq.onconnect = function(rc){
				var p = document.createElement("p");
				var topic = $('#pub-subscribe-text')[0].value;
				p.innerHTML = "Conectado ao Broker!";
				$("#debug").append(p);
				mosq.subscribe(topic, 0);
			};
			mosq.ondisconnect = function(rc){
				var p = document.createElement("p");
				var url = "ws://iot.eclipse.org/ws";
				p.innerHTML = "A conexão com o broker foi perdida!";
				$("#debug").append(p);				
				mosq.connect(url);
			};
			mosq.onmessage = function(topic, payload, qos){
				var p = document.createElement("p");
				var f = document.createElement("f");
				var acao = payload[0];
				
				switch(acao) {
					case "C":
						p.innerHTML = "<h4><i class='ti-paint-bucket text-info'></i> Cheio!</h4>";
						f.innerHTML = "<img src='http://localhost/mqtt/assets/img/bucket-sensor-01.png'>";
					break;
					case "M":
						p.innerHTML = "<h4><i class='ti-paint-bucket text-info'></i> Médio!</h4>";
						f.innerHTML = "<img src='http://localhost/mqtt/assets/img/bucket-sensor-02.png'>";
					break;
					case "B":
						p.innerHTML = "<h4><i class='ti-paint-bucket text-info'></i> Baixo!</h4>";
						f.innerHTML = "<img src='http://localhost/mqtt/assets/img/bucket-sensor-03.png'>";
					break;
					case "V":
						p.innerHTML = "<h4><i class='ti-paint-bucket text-danger'></i> Vazio!</h4>";
						f.innerHTML = "<img src='http://localhost/mqtt/assets/img/bucket-sensor-04.png'>";
					break;
				}

				$("#status_io").html(p);
				$("#funil").html(f);
			};
		}
		Page.prototype.connect = function(){
			var url = "ws://iot.eclipse.org/ws";
			mosq.connect(url);
		};
		Page.prototype.disconnect = function(){
			mosq.disconnect();
		};
		Page.prototype.subscribe = function(){
			var topic = $('#sub-topic-text')[0].value;
			mosq.subscribe(topic, 0);
		};
		Page.prototype.unsubscribe = function(){
			var topic = $('#sub-topic-text')[0].value;
			mosq.unsubscribe(topic);
		};
		
		return Page;
	})();

	$(function(){
		return Main.controller = new Main.Page;
	});

}).call(this);

