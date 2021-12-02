var player;
var player2;
var rebufferStartArray1;
var rebufferEndArray1;
var rebufferStartArray2;
var rebufferEndArray2;
var isStarted = false;

function onStartResetButtonClicked() {
    if (!isStarted) {// not started yet
	document.getElementById("start_demo").innerText = "Reset";
	isStarted = true;
	playVideo();
    } else { // started
	document.getElementById("start_demo").innerText = "Start";
	isStarted = false;
	resetAll();
    }
}

function showMenu() {
    // var demotype = parseInt(document.getElementById("select_demotype").value);
    // if (demotype == 1) {//offline demo
    // 	document.getElementById("select_trace").disabled = false;
    // } else {
    // 	document.getElementById("select_trace").disabled = true;
    // }

    var demotype = parseInt(document.getElementById("select_demotype").value);
    if (demotype == 1) {//offline demo
	document.getElementById("select_trace").style.display = "inline";
    } else {
	document.getElementById("select_trace").style.display = "none";
    }
}

function resetAll() {
    // alert("reset players, player1="+player+", player2="+player2);
    player.reset();
    player2.reset();
	// alert("reset players, player1="+player+", player2="+player2);
    var vid1 = document.getElementById("ours");
    var vid2 = document.getElementById("others");
    vid1.load();
    vid2.load();
    vid1.currentTime = 0;
    vid2.currentTime = 0;
    // alert("reset players, player1="+player+", player2="+player2);
    // delete player;
    // delete player2;

        // enable the menu
    document.getElementById("select_algo1").disabled = false;
    document.getElementById("select_algo2").disabled = false;
    document.getElementById("select_trace").disabled = false;
    document.getElementById("start_demo").disabled = false;
    document.getElementById("select_demotype").disabled = false;
    document.getElementById("select_video").disabled = false;
    resetPlaybackStats();
    clearAllTimeouts();
}

function playVideo() {

    // var rebuf1 = document.getElementById("isRebuf1");
    // alert(rebuf1+rebuf1.innerHTML);
    // rebuf1.innerHTML = "NO rebuffer!";

    var s1 = document.getElementById("select_algo1");
    var s2 = document.getElementById("select_algo2");
    var algo1 = parseInt(s1.value);
    var algo2 = parseInt(s2.value);
    var trace = parseInt(document.getElementById("select_trace").value);
    // alert(algo1 + ", " + algo2 + ", "+ trace);
    
    var result1 = getPlaybackResult(trace, algo1);
    var fixedBitrateArray1 = result1.bitrate;
    rebufferStartArray1 = result1.rebufferStart;
    rebufferEndArray1 = result1.rebufferEnd;

    var result2 = getPlaybackResult(trace, algo2);
    var fixedBitrateArray2 = result2.bitrate;
    rebufferStartArray2 = result2.rebufferStart;
    rebufferEndArray2 = result2.rebufferEnd;
    
    // alert(result1.bitrate);
    // alert(result1.rebufferStart);
    // alert(result1.rebufferEnd);

    // disable the menu
    document.getElementById("select_algo1").disabled = true;
    document.getElementById("select_algo2").disabled = true;
    document.getElementById("select_trace").disabled = true;
    // document.getElementById("start_demo").disabled = true;
    document.getElementById("select_demotype").disabled = true;
    document.getElementById("select_video").disabled = true;
    

    var videoid = parseInt(document.getElementById("select_video").value);
    var url;

    if (videoid == 1) {
	url = "../newvideo0/Manifest_updated.mpd";
	chunkDuration = 4.08;
    } else if (videoid == 2) {
	url = "../newvideo2/manifest_remote.mpd";
	chunkDuration = 4.003;
    } else if (videoid == 3) {
	url = "../newvideo1/manifest_remote.mpd";
	chunkDuration = 3.84;
    } else if (videoid == 4) {
        url = "../newvideo4/manifest_remote.mpd";
        chunkDuration = 5.56;
    }
    // start player
    //var url = "Manifest_updated.mpd";
    //var url = "newvideo1/manifest_local.mpd";
    //var url = "newvideo2/manifest_local.mpd";
    var context = new Dash.di.DashContext();
    player = new MediaPlayer(context);
    player.startup();
    player.setPlayerId(1);
    
    var context2 = new Dash.di.DashContext();
    player2 = new MediaPlayer(context2);
    player2.startup();
    player2.setPlayerId(2);
    
    //player.setAbrAlgorithm(algo1);
    var demotype = parseInt(document.getElementById("select_demotype").value);
    if (demotype == 1) {//offline

	// player.setAbrAlgorithm(0);
	player.setFixedBitrateArray(fixedBitrateArray1);
	// player2.setAbrAlgorithm(0);
	player2.setFixedBitrateArray(fixedBitrateArray2);
    } else { //online
	// player.setAbrAlgorithm(algo1);
	// player2.setAbrAlgorithm(algo2);
    }

    player.attachView(document.querySelector("#ours"));
    player.attachSource(url);
    
    //player2.setAbrAlgorithm(algo2);
    // player2.setAbrAlgorithm(5);
    // player2.setFixedBitrateArray(fixedBitrateArray2);
    player2.attachView(document.querySelector("#others"));
    player2.attachSource(url);

    // startBitrateVisual();
    
    // Set rebuffer events
    if (demotype == 1) { //offline demo
	var iRebuffer;
	var vid1 = document.getElementById("ours");
	//vid2.defaultPlaybackRate = 3.0;
	for (iRebuffer = 1; iRebuffer < rebufferStartArray1.length; iRebuffer++) { // ignoring startup time
    	    // setTimeout(function(){ vid1.pause(); document.getElementById("isRebuf1").innerHTML="Rebuffering";}, 1000*rebufferStartArray1[iRebuffer]);
    	    // setTimeout(function(){ vid1.play(); document.getElementById("isRebuf1").innerHTML="";}, 1000*rebufferEndArray1[iRebuffer]);
	    mySetTimeout(function(){ vid1.pause(); document.getElementById("isRebuf1").innerHTML="Rebuffering";}, 1000*rebufferStartArray1[iRebuffer]);
    	    mySetTimeout(function(){ vid1.play(); document.getElementById("isRebuf1").innerHTML="";}, 1000*rebufferEndArray1[iRebuffer]);
	}
	// for (iRebuffer = 0; iRebuffer < rebufferStartArray1.length; iRebuffer++) {
    	//     setTimeout(function(){ vid1.pause(); }, 10*iRebuffer);
    	//     setTimeout(function(){ vid1.play(); }, 10*iRebuffer+5);
	// }
	// var iRebuffer;
	var vid2 = document.getElementById("others");
	//vid2.defaultPlaybackRate = 3.0;
	for (iRebuffer = 1; iRebuffer < rebufferStartArray2.length; iRebuffer++) { // ignoring startup time
    	    // setTimeout(function(){ vid2.pause(); document.getElementById("isRebuf2").innerHTML="Rebuffering";}, 1000*rebufferStartArray2[iRebuffer]);
    	    // setTimeout(function(){ vid2.play(); document.getElementById("isRebuf2").innerHTML="";}, 1000*rebufferEndArray2[iRebuffer]);
	    mySetTimeout(function(){ vid2.pause(); document.getElementById("isRebuf2").innerHTML="Rebuffering";}, 1000*rebufferStartArray2[iRebuffer]);
    	    mySetTimeout(function(){ vid2.play(); document.getElementById("isRebuf2").innerHTML="";}, 1000*rebufferEndArray2[iRebuffer]);
	}
	startTime1 = Date.now();
	startTime2 = Date.now();
    }
} 

var pending = {};
function mySetTimeout(callback, delay) {
  var t;
  t = setTimeout(function() {delete pending[t];callback()}, delay)
  pending[t]=1;
}
function clearAllTimeouts() {
  for (var t in pending) if (pending.hasOwnProperty(t)) {
    clearTimeout(t);
    delete pending[t];
  }
}    

var startTime1;
var startTime2; 

