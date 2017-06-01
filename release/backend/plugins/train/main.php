<style>
canvas {
	-moz-user-select: none;
	-webkit-user-select: none;
	-ms-user-select: none;
}
#block_left {
    float: left;
    width: 49%;
}
#block_right {
    float: right;
    width: 49%;
}

.fullscreen {
	width: 100%;
    position: fixed;
    height: 100%;
    top: 0;
    left: 0;
    background: rgba(255, 255, 255, 1.0);
    z-index: 100001;
	cursor: zoom-out !important;
}

#ui-datepicker-div { 
	background: white; 
	padding: 10px;
	border-radius: 5px;
	border: 1px solid #c3c3c3;
	
}
.ui-datepicker-title {
	margin-top: 20px;
    margin-bottom: 20px;
    font-size: 150%;
}
.ui-corner-all {
	cursor: pointer;
}
.hidden {
	display: none;
}
@media screen and (max-width: 700px) {
#block_right, #block_left {
    float: none;
    width: 100%;
}
}

</style>
	
<script src="plugins/train/chart.js/Chart.bundle.js"></script>
<script src="plugins/train/chart.js/utils.js"></script>
<script src="http://code.jquery.com/ui/1.10.3/jquery-ui.js"/></script>
<script src="plugins/train/datepicker/jquery-ui.multidatespicker.js"/></script>
<link rel="stylesheet" type="text/css" href="plugins/train/datepicker/jquery-ui.multidatespicker.css">


<script>
	function checkComparisonMode(obj) {
		if($(obj).val() == "range") {
			$(".range").each(function() {
				if($(this).hasClass('hidden')) $(this).toggleClass('hidden');
			});
			$(".multiple").each(function() {
				if(!$(this).hasClass('hidden')) $(this).toggleClass('hidden');
			});
		}
		else {
			$(".range").each(function() {
				if(!$(this).hasClass('hidden')) $(this).toggleClass('hidden');
			});
			$(".multiple").each(function() {
				if($(this).hasClass('hidden')) $(this).toggleClass('hidden');
			});
		}
	}

</script>
<?php
	$timestamp_thresh = 60 * 60 * 12;
	
	$ICON_GREEN = '<i class="fa fa-check-circle" style="color: green;"></i>';
	$ICON_ORANGE = '<i class="fa fa-exclamation-triangle" style="color: orange;"></i>';
	$ICON_RED = '<i class="fa fa-times-circle" style="color: red;"></i>';
	$ICON_BLUE = '<i class="fa fa-info-circle" style="color: blue;"></i>';
	
	if($MANAGE == "Train trips") {
		if($PAGE == "Trips") {
			if(isset($_GET['open'])) {
				//prepare javascript list / datapoints
				$J_DATA = array();
				$J_TITLE = "";
				
				//get traject info
				$qinfo = get_trip_info($_GET['open']);
				$status = '';
					
				if(!$qinfo['has_ended']) {
					if($qinfo['has_terminated']) $status = "$ICON_RED Aborted";
					else if($qinfo['has_died']) $status = "$ICON_RED Assumed dead";
					else if($qinfo['has_errors']) $status = "$ICON_ORANGE In progress with errors";
					else $status = "$ICON_BLUE In progress";
				}
				else {
					if($qinfo['has_errors']) $status = "$ICON_ORANGE Completed with errors";
					else $status = "$ICON_GREEN Completed";
				}
				
				$info = "<table>
				<tr><td>Traject ID</td><td>$qinfo[ic]</td></tr>
				<tr><td>Trip ID</td><td>$qinfo[trip_id]</td></tr>
				<tr><td>Last timestamp</td><td>".date('d/m/Y G:i:s', $qinfo['last_timestamp'])."</td></tr>
				<tr><td>Status</td><td>$status</td></tr>
				</table>";
				
				
				
				//get logs
				$logs = mysqli_query($con, "SELECT * FROM `railreports` WHERE TRIPID='$_GET[open]' ORDER BY TIMESTAMP ASC;");
				$logdata = '<table><thead><tr><td>Timestamp</td><td>Error</td><td>Source</td><td>Target</td></tr></thead>';
				while($row = mysqli_fetch_array($logs)) {
					$error = parse_error($row['ERROR']);
					$logdata .= "<tr><td>".date("d/m/Y G:i:s", $row['TIMESTAMP'])."</td><td>".get_level_icon($error['level'])." $error[error]</td><td>$row[SOURCE]</td><td>$row[TARGET]</td></tr>";
				}
				$logdata .= '</table>';
				
				//get passenger info
				
				$passengers = mysqli_query($con, "SELECT * FROM `railrecords` WHERE TRIPID='$_GET[open]' ORDER BY TIMESTAMP ASC;");
				$countdata = '<table><thead><tr><td>Timestamp</td><td>Stop</td><td># in</td><td># out</td><td>Net passengers</td><td>Detection ratio</td></tr></thead>';
				$previous = 0;
				$count = 0;
				while($row = mysqli_fetch_array($passengers)) {
					$interpolated = false;
					$negative = false;
					$ratio = floatval($row['GATHERED']) / floatval($row['TOTAL']);
					if($row['GATHERED'] != $row['TOTAL']) {
						$interpolated = true;
						
						if($ratio != 0) {
							$row['IN'] = intval($row['IN'] * (1.0/$ratio));
							$row['OUT'] = intval($row['OUT'] * (1.0/$ratio));
						}
					}
					$net = $previous + $row['IN'] - $row['OUT'];

					//check for negative people?
					
					$J_DATA[$row['STATION']] = $net;

					
					
					$previous = $net;
					if($previous < 0) {
						$previous = 0;
						$negative = false;
						$net .= " **";
					}
					else if($interpolated) $net .= " *";
					$countdata .= '<tr><td>'.date('d/m/Y G:i:s', $row['TIMESTAMP'])."</td><td>$row[STATION]</td><td>$row[IN]</td><td>$row[OUT]</td><td>$net</td><td>$ratio ($row[GATHERED] / $row[TOTAL])</td></tr>";
					$count++;
				}
				
				$countdata .= '</table><br/>';
				$countdata .= '<div id="container" style="width: 100%;cursor: zoom-in;" ondblclick="$(\'#container\').toggleClass(\'fullscreen\');">
					<canvas id="canvas"></canvas>
				</div>';
				
				echo '<div id="block_left">';
				drawBlock("Counting results", $countdata);
				echo '</div><div id="block_right">';
				drawBlock("Trip info", $info);
				drawBlock("Trip log", $logdata);
				echo '</div>';
				
				
				
				draw_chart($J_DATA, $qinfo['ic']);

				
				
				
			}
			else if(isset($_GET['dev'])) {
				/*
				$data = file(dirname(__FILE__)."/list.csv");
				
				for($i = 0; $i < count($data); $i++) {
					$entry = explode(';', $data[$i]);
					//echo $entry[1].'<br/>';
					$trajectory = file_get_contents("http://www.beluxtrains.net/indexnl.php?page=belgium-files/2017_1/2017-".$entry[1]);
					if($trajectory === false) echo 'ERROR';
					
					//echo htmlspecialchars($trajectory, ENT_IGNORE | ENT_COMPAT | ENT_HTML401);
					$delim_1 = '<span class="trip">Traject:</span><br />';
					$edit = strpos($trajectory, $delim_1);
					//echo $edit.'<br/>';
					
					$trajectory = substr($trajectory, $edit + strlen($delim_1));
					$edit = strpos($trajectory, '<br />');
					//echo $edit.'<br/>';
					$trajectory = substr($trajectory, 0, $edit);
					$trajectory = trim(preg_replace('/\s+/', ' ', $trajectory));
					
					//echo utf8_encode($trajectory);
					mysqli_query($con, "INSERT INTO `railtrips` VALUES('".$entry[0].$entry[1]."', '".$entry[2]."', '".$entry[3]."', '".$trajectory."');");
				}
				*/
				
				/*
				$trips = mysqli_query($con, "SELECT * FROM `railtrips`;");
				
				$base_date = strtotime('01/01/2017 0:00');
				$day = 60 * 60 * 24;
				$interval = 60 * 15;
				
				
				
				while(($row = mysqli_fetch_array($trips))) {
					for($days = 0; $days < 30; $days++) {
						//parse the traject 
						$stations = explode(" - ", $row['STOPS']);
						$tstart = $base_date + intval(1.0 * $day * (rand(0, 1000) / 1000.0)) + $day * $days;
						//generate start record
						$trip_id = rand(100000, 999999).rand(100000, 999999);
						mysqli_query($con, "INSERT INTO `railreports`(TRIPID, SOURCE, TARGET, ERROR, TIMESTAMP) VALUES('$trip_id', '123456789', '$row[TRAJECT]', 0, $tstart);");
						
						//remember net passengers
						$previous = 0;
						for($i = 0; $i < count($stations); $i++) {
							$in = intval(100.0 *  (rand(0, 1000) / 1000.0));
							$out = intval(100.0 * (rand(0, 1000) / 1000.0));
							$total = 40;
							if(rand(0, 1000) > 900) $gathered = intval(1.0 * $total * (rand(900, 1000) / 1000.0));
							else $gathered = $total;
							
							if($i == 0) {
								$out = 0;
								$previous = $in;
							}
							else if($i == count($stations) - 1) {
								$in = 0;
								$out = $previous;
							}
							else {
								$net = $previous + $in - $out;
								if($net < 0) {
									$a = $in;
									$in = $out;
									$out = $a;
									$net = $previous + $in - $out;
								}
								$previous = $net;
							}
							
							mysqli_query($con, "INSERT INTO `railrecords` (TRIPID, TRAJECTORY, TIMESTAMP, STATION, `IN`, `OUT`, GATHERED, TOTAL) VALUES('$trip_id', '$row[TRAJECT]', $tstart, '$stations[$i]', $in, $out, $gathered, $total);");
							$tstart += intval(1.0 * $interval * (rand(0, 1000) / 1000.0));
						}
						//generate end record
						mysqli_query($con, "INSERT INTO `railreports`(TRIPID, SOURCE, TARGET, ERROR, TIMESTAMP) VALUES('$trip_id', '123456789', '$row[TRAJECT]', 20, $tstart);");
					}
					
					
					
					
					
				}
				*/
				
			}
			else if(isset($_GET['compare'])) {
				$t_start = time();
				$t_max = 40;

				if($_GET['type'] == 'full') {
					//Get stops
					
					$stops = mysqli_query($con, "SELECT STOPS FROM `railtrips` WHERE TRAJECT='$_GET[traject]';");
					$data = mysqli_fetch_array($stops)['STOPS'];
					$ids = mysqli_query($con, "SELECT TRAJECT FROM `railtrips` WHERE STOPS='$data';");
					
					$count = 0;
					$clause = '';
					while($row = mysqli_fetch_array($ids)) {
						if($count == 0) $clause .= "TARGET = '$row[TRAJECT]'";
						else $clause .= "OR TARGET = '$row[TRAJECT]'";
						$count++;
					}
					$clause = '('.$clause.')';
				}
				else {
					$clause = "TARGET='$_GET[traject]'";
				}
				
				if($_GET['timemode'] == 'range') {
					$time1 = strtotime($_GET['dstart'].' '.$_GET['tstart']);
					$time2 = strtotime($_GET['dend'].' '.$_GET['tend']);
					$times = "TIMESTAMP < $time2 AND TIMESTAMP > $time1";
				}
				else {
					$dates = explode(',', $_GET['range']);
					$times = '(';
					
					for($i = 0; $i < count($dates); $i++) {
						$time1 = strtotime($dates[$i]." 0:00");
						$time2 = $time1 + 24*60*60;
						$add = "(TIMESTAMP < $time2 AND TIMESTAMP > $time1)";
						if($i > 0) $times .= ' OR '.$add;
						else $times .= $add;
					}
					$times .= ')';
				}
			
				//Get trips that have succesfully ended
				$trips = mysqli_query($con, "SELECT * FROM (SELECT TRIPID, TARGET, TIMESTAMP, MAX(ERROR) AS DONE FROM `railreports` WHERE $times AND $clause GROUP BY TRIPID ORDER BY TIMESTAMP ASC) r WHERE r.DONE = 20;");
				
				$J_DATASETS = array();
				$J_DATAPOINTS = array();
				
				$TRIP_IDS = array();
				
				//we have the trip_id's, count passengers
				$count = 0;
				$mismatch = 0;
				$J_LABELS = '';
				while($row = mysqli_fetch_array($trips)) {
					if(time() > ($t_start + $t_max)) {
						fail("Error", "timeout of 40 seconds reached, gathered $count results. Please narrow your search criteria and try again.");
						break;
					}
					if($count == 0) {
						//fetch labels
						$labels = mysqli_query($con, "SELECT STOPS FROM `railtrips` WHERE TRAJECT = '$row[TARGET]';");
						$parse = mysqli_fetch_array($labels);
						$J_LABELS = explode(" - ", $parse['STOPS']);
						
					}
					$fetch = mysqli_query($con, "SELECT * FROM `railrecords` WHERE TRIPID='$row[TRIPID]';");
					
					//get count data
					$previous = 0;
					$add = array();
					while($counters = mysqli_fetch_array($fetch)) {
						if(intval($counters['TOTAL']) === 0 || intval($counters['GATHERED']) === 0) $ratio = 0;
						else $ratio = floatval($counters['TOTAL']) / floatval($counters['GATHERED']);
						$counters["IN"] = intval($counters["IN"] * $ratio);
						$counters["OUT"] = intval($counters["OUT"] * $ratio);
						
						$net = $previous + $counters["IN"] - $counters["OUT"];
						if($net < 0) $net = 0;
						
						$previous = $net;
						
						
						array_push($add, $net);
						
					}
					
					//check for column size mismatch
					if(count($add) != count($J_LABELS)) $mismatch++;
					else array_push($J_DATAPOINTS, $add);
					
					//add dataset name
					array_push($J_DATASETS, $row['TARGET'].' '.date('d/m/Y G:i:s', $row['TIMESTAMP']));
					array_push($TRIP_IDS, $row['TRIPID']);
					$count++;
				}
				
				$content = '<p>Gathered info from '.$count.' trip(s).</p>
				<p>'.$mismatch.' trip(s) discarded because of traject info mismatch.</p>
				<div id="container" style="width: 100%;cursor: zoom-in;" ondblclick="$(\'#container\').toggleClass(\'fullscreen\');">
					<canvas id="canvas"></canvas>
				</div>';
				
				echo '<div id="block_left">';
				drawBlock("Comparison", $content);
				
				
				
				
				$dev = '<div id="container2" style="width: 100%;cursor: zoom-in;" ondblclick="$(\'#container2\').toggleClass(\'fullscreen\');">
					<canvas id="deviations"></canvas>
				</div>';
				drawBlock("Average and deviation", $dev);
				
				
				draw_points($J_LABELS, $J_DATAPOINTS, $J_DATASETS, "canvas");
				draw_deviations($J_LABELS, $J_DATAPOINTS, "deviations");
				
				
				echo '</div><div id="block_right">';
				
				$details = '<table><thead><tr><td>Description</td><td>Trip ID</td><td>Actions</td></tr></thead>';
				for($i = 0; $i < count($J_DATASETS); $i++) $details .= '<tr><td>'.$J_DATASETS[$i].'</td><td>'.$TRIP_IDS[$i].'</td><td><a href="index.php?&manage=P_train_Train%20trips&page=Trips&open='.$TRIP_IDS[$i].'" target="_blank"><button>View trip</button></a></tr>';
				$details .= '</table>';
				
				drawBlock("Trip details", $details);
				echo '</div>';
			}
			else {
				contextMenuMain();
				$results_per_page = 50;
				
				
				
				
				//get last 50 trips or all of them
				if(isset($_GET['view'])) {
					$page = intval($_GET['view']);
					if($page < 0) $page = 0;
					unset($_GET['view']);
				}
				else $page = 0;
				
				//make naviagtion buttons
				$button = '<a href="'.guessForm().'&view='. ($page+1) .'"><button>Previous '.$results_per_page.' records</button></a>';
				if($page > 0) $button .= ' <a href="'.guessForm().'&view='. ($page-1) .'"><button>Next '.$results_per_page.' records</button></a> <a href="'.guessForm().'"><button>Most recent</button></a>';
				$limit = " LIMIT ".($results_per_page * $page).", ".$results_per_page;
				
				
				//prepare query
				if(isset($_GET['search'])) {
					if($_GET['type'] == 'full') {
						//Get stops
						
						$stops = mysqli_query($con, "SELECT STOPS FROM `railtrips` WHERE TRAJECT='$_GET[traject]';");
						$data = mysqli_fetch_array($stops)['STOPS'];
						$ids = mysqli_query($con, "SELECT TRAJECT FROM `railtrips` WHERE STOPS='$data';");
						
						$count = 0;
						$clause = '';
						while($row = mysqli_fetch_array($ids)) {
							if($count == 0) $clause .= "TARGET = '$row[TRAJECT]'";
							else $clause .= "OR TARGET = '$row[TRAJECT]'";
							$count++;
						}
						$clause = '('.$clause.')';
					}
					else {
						$clause = "TARGET='$_GET[traject]'";
					}
					
					if($_GET['timemode'] == 'range') {
						$time1 = strtotime($_GET['dstart'].' '.$_GET['tstart']);
						$time2 = strtotime($_GET['dend'].' '.$_GET['tend']);
						$times = "TIMESTAMP < $time2 AND TIMESTAMP > $time1";
					}
					else {
						$dates = explode(',', $_GET['range']);
						$times = '(';
						
						for($i = 0; $i < count($dates); $i++) {
							$time1 = strtotime($dates[$i]." 0:00");
							$time2 = $time1 + 24*60*60;
							$add = "(TIMESTAMP < $time2 AND TIMESTAMP > $time1)";
							if($i > 0) $times .= ' OR '.$add;
							else $times .= $add;
						}
						$times .= ')';
					}
					

					
					
					$get = mysqli_query($con, "SELECT ID, TRIPID, TIMESTAMP FROM `railreports` WHERE $times AND $clause GROUP BY TRIPID ORDER BY TIMESTAMP DESC".$limit.";");
				}
				
				else $get = mysqli_query($con, "SELECT ID, TRIPID, TIMESTAMP FROM `railreports` GROUP BY TRIPID ORDER BY TIMESTAMP DESC".$limit.";");
				
				//make table
				$output = $button.'<br/><br/>';
				$output .= '<table><thead><tr><td>Timestamp</td><td>ID</td><td>Traject</td><td>Status</td><td>Actions</td></tr></thead>';
				
				//get trip id status
				while($row = mysqli_fetch_array($get)) {
					
					$info = get_trip_info($row['TRIPID']);
					$status = '';
					
					if(!$info['has_ended']) {
						if($info['has_terminated']) $status = "$ICON_RED Aborted";
						else if($info['has_died']) $status = "$ICON_RED Assumed dead";
						else if($info['has_errors']) $status = "$ICON_ORANGE In progress with errors";
						else $status = "$ICON_BLUE In progress";
					}
					else {
						if($info['has_errors']) $status = "$ICON_ORANGE Completed with errors";
						else $status = "$ICON_GREEN Completed";
					}
					
					$output .= "<tr><td>".date('d/m/Y G:i:s', $row['TIMESTAMP'])."</td><td>$row[TRIPID]</td><td>$info[ic]</td><td>$status</td><td><a href=\"".guessForm()."&open=$row[TRIPID]\"><button>Open</button></a></td></tr>";
				}
				
				$output .= "</table>";
				$output .= $button;
				drawBlock("Trip info", $output);
				
				
				/*
					Make comparison window
				*/
				$trajectpicker = '<select name="traject">';
				$trajects = mysqli_query($con, "SELECT DISTINCT TRAJECT FROM `railtrips`");
				while($row = mysqli_fetch_array($trajects)) $trajectpicker .= '<option>'.$row['TRAJECT'].'</option>';
				$trajectpicker .= '</select>';
				
				$compare = '<form method="GET" action="index.php">
				<input type="hidden" value="P_train_Train trips" name="manage"/>
				<table>
				
				<tr><td>Type</td><td><select name="type">
					<option value="id">Traject ID historical data</option>
					<option value="full">Full traject historical data</option>
				</select></td></tr>
				<tr><td>Traject ID</td><td>'.$trajectpicker.'</td></tr>
				<tr><td>Timespan</td><td><select name="timemode" id="timemode" onchange="checkComparisonMode(this)"><option value="range">Date range</option><option value="multiple">Select dates</option></select></td></tr>
				<tr class="range"><td>From</td><td><input type="date" name="dstart"> <input type="time" name="tstart"></td></tr>
				<tr class="range"><td>To</td><td><input type="date" name="dend"> <input type="time" name="tend"></td></tr>
				<tr class="multiple hidden"><td>Dates</td><td><input id="datePick" name="range" type="text"/></td></tr>
				<tr><td></td><td><button name="compare">Generate chart</button></td></tr>
				</table>
				</form>';

				drawOverlay("New comparison", $compare, 'compare', false);
				echo "<script>$('#datePick').multiDatesPicker();</script>";
				
				
				/*
					Make search window
				*/

				$search = '<form method="GET" action="index.php">
				<input type="hidden" value="P_train_Train trips" name="manage"/>
				<table>
				
				<tr><td>View results from</td><td><select name="type">
					<option value="id">Single traject ID</option>
					<option value="full">All equivalent trajects</option>
				</select></td></tr>
				<tr><td>Traject ID</td><td>'.$trajectpicker.'</td></tr>
				<tr><td>Timespan</td><td><select name="timemode" id="timemode" onchange="checkComparisonMode()"><option value="range">Date range</option><option value="multiple">Select dates</option></select></td></tr>
				<tr class="range"><td>From</td><td><input type="date" name="dstart"> <input type="time" name="tstart"></td></tr>
				<tr class="range"><td>To</td><td><input type="date" name="dend"> <input type="time" name="tend"></td></tr>
				
				<tr class="multiple hidden"><td>Dates</td><td><input id="datePick2" name="range" type="text"/></td></tr>
				<tr><td></td><td><button name="search">Enable filter</button></td></tr>
				</table>
				</form>';
				
				
				drawOverlay("Filter trips", $search, 'search', false);
				echo "<script>$('#datePick2').multiDatesPicker();</script>";
			}
			
		}
		else if($PAGE == "Logs") {
			contextMenuLogs();
				
			
			$results_per_page = 50;
			//get last 50 trips or all of them
			if(isset($_GET['view'])) {
				$page = intval($_GET['view']);
				if($page < 0) $page = 0;
				unset($_GET['view']);
			}
			else $page = 0;
			
			
			//make naviagtion buttons
			$button = '<a href="'.guessForm().'&view='. ($page+1) .'"><button>Previous '.$results_per_page.' records</button></a>';
			if($page > 0) $button .= ' <a href="'.guessForm().'&view='. ($page-1) .'"><button>Next '.$results_per_page.' records</button></a> <a href="'.guessForm().'"><button>Most recent</button></a>';
			$limit = " LIMIT ".($results_per_page * $page).", ".$results_per_page;
				
				
			//apply filter
			if(isset($_GET['search'])) {				
				if($_GET['timemode'] == 'range') {
					$time1 = strtotime($_GET['dstart'].' '.$_GET['tstart']);
					$time2 = strtotime($_GET['dend'].' '.$_GET['tend']);
					$times = "TIMESTAMP < $time2 AND TIMESTAMP > $time1";
				}
				else {
					$dates = explode(',', $_GET['range']);
					$times = '(';
					
					for($i = 0; $i < count($dates); $i++) {
						$time1 = strtotime($dates[$i]." 0:00");
						$time2 = $time1 + 24*60*60;
						$add = "(TIMESTAMP < $time2 AND TIMESTAMP > $time1)";
						if($i > 0) $times .= ' OR '.$add;
						else $times .= $add;
					}
					$times .= ')';
				}
				
				if($_GET['status'] == 'notice') {
					$errors = "(ERROR = 0 OR ERROR = 20)";
				}
				else if($_GET['status'] == 'warning') {
					$errors = "(ERROR = 1 OR ERROR = 9)";
				}
				else if($_GET['status'] == 'error') {
					$errors = "(ERROR = 2 OR ERROR = 3 OR ERROR = 4 OR ERROR = 5 OR ERROR = 6 OR ERROR = 7 OR ERROR = 8 OR ERROR = 10)";
				}
				else {
					$errors = "1=1";
				}
				
				$loglist = mysqli_query($con, "SELECT * FROM `railreports` WHERE $times AND $errors ORDER BY TIMESTAMP DESC $limit;");
			}
			
			else $loglist = mysqli_query($con, "SELECT * FROM `railreports` ORDER BY TIMESTAMP DESC $limit;");
			
			
			$table = $button.'<br/><br/>';
			$table .= '<table><thead><tr><td></td><td>Timestamp</td><td>Trip ID</td><td>Generated by</td><td>Caused by</td><td>Error type</td><td>Actions</td></tr></thead>';
			
			while($row = mysqli_fetch_array($loglist)) {
				$error = parse_error($row['ERROR']);
				$table.= '<tr><td>'.get_level_icon($error['level']).'</td><td>'.date('d/m/Y G:i:s', $row['TIMESTAMP'])."</td><td>$row[TRIPID]</td><td>$row[SOURCE]</td><td>$row[TARGET]</td><td>".$error['error']." ($row[ERROR])</td><td><a href=\"index.php?&manage=P_train_Train%20trips&page=Trips&open=$row[TRIPID]\" target=\"blank\"><button>Open trip</button></a></td></tr>";
				
			}
			
			$table .= '</table>';
			$table .= $button;
			
			drawBlock('Logs and reports', $table);
			
			$search = '<form method="GET" action="index.php">
			<input type="hidden" value="P_train_Train trips" name="manage"/>
			<input type="hidden" value="Logs" name="page"/>
			
			<table>
			<tr><td>Statuscode</td><td><select name="status">
				<option value="all">All</option>
				<option value="notice">Notices</option>
				<option value="warning">Warnings</option>
				<option value="error">Errors</option>
			</select></td></tr>
			
			<tr><td>Timespan</td><td><select name="timemode" id="timemode" onchange="checkComparisonMode(this)"><option value="range">Date range</option><option value="multiple">Select dates</option></select></td></tr>
			<tr class="range"><td>From</td><td><input type="date" name="dstart"> <input type="time" name="tstart"></td></tr>
			<tr class="range"><td>To</td><td><input type="date" name="dend"> <input type="time" name="tend"></td></tr>
			<tr class="multiple hidden"><td>Dates</td><td><input id="datePick" name="range" type="text"/></td></tr>
			<tr><td></td><td><button name="search">Enable filter</button></td></tr>
			</table>';
			
			drawOverlay('Filter logs', $search, 'search', false);
			echo "<script>$('#datePick').multiDatesPicker();</script>";
		}
	}

?>
