<?php

$RELEASE = '0.5';


function contextMenu() {
	$data = '<div class="contextmenu">';
	$data .= '<i style="margin-left: 5px;" class="fa fa-caret-right"></i>';
	$data .= '<a href="'.guessForm().'&sort=all" class="contextlink"><i class="fa fa-sort-amount-desc"></i> Oude berichten weergeven</a> ';
	$data .= '</div>';
	setContextMenu($data);
}

//messageboard for error / warning / success messages
function messageBoard() {
	echo '<div id="messageboard"></div>';
}

function contextBoard() {
	echo '<div id="contextboard"></div>';
}



/*
	--------------------- BASE POPUP FUNCTIONS ---------------------
*/

//draw success message (messageBoard has to be executed!)
function success($title, $msg) {
	echo '<script>
	$( document ).ready(function() {
	$("#messageboard").html($("#messageboard").html() + \'<div class="event" id="green"><strong>'.str_replace("'", "\'", $title).'</strong>: '.str_replace("'", "\'", $msg).'</div>\');
	});
	</script>';
}

//draw warning message (messageBoard has to be executed!)
function warning($title, $msg) {
	echo '<script>
	$( document ).ready(function() {
	$("#messageboard").html($("#messageboard").html() + \'<div class="event" id="highlight"><strong>'.str_replace("'", "\'", $title).'</strong>: '.str_replace("'", "\'", $msg).'</div>\');
	});
	</script>';
}

//draw error message (messageBoard has to be executed!)
function fail($title, $msg) {
	echo '<script>
	$( document ).ready(function() {
	$("#messageboard").html($("#messageboard").html() + \'<div class="event" id="red"><strong>'.str_replace("'", "\'", $title).'</strong>: '.str_replace("'", "\'", $msg).'</div>\');
	});
	</script>';
}

//draw notification message (messageBoard has to be executed!) 
function notif($title, $msg) {
	echo '<script>
	$( document ).ready(function() {
	$("#messageboard").html($("#messageboard").html() + \'<div class="event" id="notif"><strong>'.str_replace("'", "\'", $title).'</strong>: '.str_replace("'", "\'", $msg).'</div>\');
	});
	</script>';
}

function setContextMenu($data) {
	echo '<script>
	$( document ).ready(function() {
	$("#contextboard").html(\''.str_replace("'", "\'", $data).'\');
	});
	</script>';
}

//draw basic content block with title
function drawBlock($title, $content, $width="auto", $height="auto", $display=0) {
	$style="width:".$width.";height:".$height.";";
	if($display === "block") $style .= "display:inline-block;margin-right:20px;";
	echo '<div class="table" style="'.$style.'">';
		echo '<div class="thead">'.$title.'</div>
		<div class="tbody">'.$content.'</div></div>';
}

//draw content block in an overlay
function drawOverlay($title, $content, $classname, $display) {
	if($display === true) $display = '';
	else $display = ' style="display:none;"';
	if($classname == "") $close = "";
	else $close = '<a href="#" onclick="$(\'.'.$classname.'\').fadeOut()" style="float:right;">X</a>';
	echo '<div id="overlay" class="'.$classname.'"'.$display.'><div class="outer"><div class="middle"><div class="inner">
	<div id="dialog" class="newpage1" style="padding:0px;">
	<div class="thead">'.$title.$close.'</div>
	<div class="tbody">'.$content.'</div>
	</div></div></div></div></div>';
}

function guessForm() {	
	$return = 'index.php';
	if(count($_GET) > 0) {
		$return .= '?';
		foreach($_GET as $var => $val) {
			$return .= '&'.$var.'='.$val;
		}
	}
	else $return .= '?';
	return $return;
}


function bulk_escape($array) {
	global $con;
	$output = array();
	if(is_array($array)) {
		foreach($array as $key => $value) {
			$output[mysqli_real_escape_string($con, $key)] = mysqli_real_escape_string($con, $value);
		}
		return $output;
	}
	else return mysqli_real_escape_string($con, $array);
}

function generateRandomString($length = 10) {
    $characters = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
    $charactersLength = strlen($characters);
    $randomString = '';
    for ($i = 0; $i < $length; $i++) {
        $randomString .= $characters[rand(0, $charactersLength - 1)];
    }
    return $randomString;
}



?>