<?php
//here we go

if(!isset($_SESSION)) session_start();

/* Includes */
include_once('include/functions.php');

$con = include_once('include/connection.php');


$_MENU = array(
	"Train trips" => array("Trips", "Logs")
);


if(count($_MENU) > 0) {
	if(isset($_GET['manage']) && isset($_MENU[$_GET['manage']])) $MANAGE = $_GET['manage'];
	else {
		foreach($_MENU as $key => $value) {
			$MANAGE = $key;
			break;
		}
	}

	if(isset($_GET['page']) && in_array($_GET['page'], $_MENU[$MANAGE])) $PAGE = $_GET['page'];
	else $PAGE = $_MENU[$MANAGE][0];
}
//if not, and no applications are available for this user, set page to false
else {
	$PAGE = false;
	$MANAGE = false;
}

			
?>

<!DOCTYPE HTML>
<html>
<head>
	<title>CAS Manager</title>
	<link type="text/css" rel="stylesheet" href="fa/css/font-awesome.css" />
	<link type="text/css" rel="stylesheet" href="css/reset.css" />
	<link type="text/css" rel="stylesheet" href="css/layout.css?&v=1" />
	<script src="scripts/jquery.js" type="text/javascript"></script>
	<script src="scripts/main.js" type="text/javascript"></script>
	
	<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
</head>
<body>

<div id="wrapper">
	<div id="navigator">
		<?php
			for($i = 0; $i < count($_MENU[$MANAGE]); $i++) {
				$active = '';
				if($_MENU[$MANAGE][$i] == $PAGE) $active = ' active';
				echo '<a class="navi'.$active.'" id="navi_'.strtolower(str_replace(" ", "_", $_MENU[$MANAGE][$i])).'" href="index.php?&manage='.$MANAGE.'&page='.$_MENU[$MANAGE][$i].'">'.$_MENU[$MANAGE][$i].'</a>';
			}
		?>


	</div>
	<div id="editor">
		<div class="content">
			<?php
				/*******************************************************************
				 *
				 *		Main program logic is contained in these three files:
				 *
				 *******************************************************************/
				
				
				contextBoard();
				messageBoard();
				echo '<script src="plugins/train/main.js"></script>';
				include 'plugins/train/functions.php';
				include 'plugins/train/main.php';
					
			?>

		</div>
		<div id="popups"></div>
	</div>
</div>
	
	

	
</body>
</html>