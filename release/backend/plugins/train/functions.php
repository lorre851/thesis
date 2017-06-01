<?php
	/*
	#define ERR_OK 0
	#define ERR_CAMNODE_LOST 1
	#define ERR_CAMSERVER_LOST 2
	#define ERR_SYNC 3
	#define ERR_MASTER 4
	#define ERR_TERMINATE 5
	#define ERR_NO_SERVER 6
	#define ERR_RESULTS_LOST 7
	#define ERR_CAMNODE 9
	#define ERR_CAMNODE_CRIT 10
	#define ERR_END 20
	*/
	$CPP_ERRORS = array(
		0 => "Trip started",
		1 => "Camnode lost",
		2 => "Camserver lost",
		3 => "Sync failure",
		4 => "Master failure",
		5 => "Force terminated",
		6 => "No camservers found",
		7 => "Results lost after terminate",
		8 => "State mismatch",
		9 => "Issue with camNode",
		10 => "Critical issue with camNode",
		20 => "Trip ended"
	);
	
	function parse_error($error) {
		
		global $CPP_ERRORS;
		$error = intval($error);
		if($error == 0 || $error == 20) $level = 0;
		else if($error == 1 || $error == 9) $level = 1;
		else $level = 2;
		
		
		if(!isset($CPP_ERRORS[$error])) return false;
		else return array(
			"error" => $CPP_ERRORS[$error],
			"level" => $level
		
		);
	}
	
	function get_level_icon($level) {
		$level = intval($level);
		if($level == 0) return '<i class="fa fa-info-circle" style="color: blue;"></i>';
		else if($level == 1) return '<i class="fa fa-exclamation-triangle" style="color: orange;"></i>';
		else if($level == 2) return '<i class="fa fa-times-circle" style="color: red;"></i>';
		else return false;
		
	}
	
	
	function draw_graph($datapoints) {
		if(!is_array($datapoints)) {
			echo '<p>Cannot draw graph, invalid data!</p>';
			return false;
		}
		
		$smallest_x = 0;
		$biggest_x = 0;
		$smallest_y = 0;
		$biggest_y = 0;
		
		$i = 0;
		foreach($datapoints as $key => $value) {
			if(!is_numeric($key) || !is_numeric($value)) {
				echo '<p>Invalid pair: '.htmlentities($key).' - '.htmlentities($value).'</p>';
				return false;
			}
			
			
			if($i == 0) {
				$smallest_x = $biggest_x = $key;
				$smallest_y = $biggest_y = $value;
			}
			else {
				if($key < $smallest_x) $smallest_x = $key;
				else if($key > $biggest_x) $biggest_x = $key;
				
				if($value < $smallest_y) $smallest_y = $value;
				if($value > $biggest_y) $biggest_y = $value;
			}
			
			$i++;
		}
		
		$data_width = doubleval($biggest_x - $smallest_x);
		$data_height = doubleval($biggest_y - $smallest_y);
		
		echo '<div style="width: 100%; padding: 20px;">';
		echo '<div style="width: 100%; height: 300px; border: 1px solid black; position: relative;">';
		
		foreach($datapoints as $x => $y) {
			echo '<div style="position: absolute; margin-left: '.($x / $data_width) * 100 .'%; margin-top: '. (100 - (($y / $data_height) * 100)) .'%; width: 5px; height: 5px; background: red;"></div>';
		}
		
		echo '</div>';	
		echo '</div>';
		
	}
	
	function contextMenuMain() {
		$data = '<div class="contextmenu">';
		$data .= '<i style="margin-left: 5px;" class="fa fa-caret-right"></i>';
		$data .= '<a href="#" onclick="$(\'.compare\').fadeIn()" class="contextlink"><i class="fa fa-plus"></i> Compare trips</a> ';
		$data .= '<a href="#" onclick="$(\'.search\').fadeIn()" class="contextlink"><i class="fa fa-search"></i> Filter trajects</a> ';
		$data .= '</div>';
		setContextMenu($data); 
	}
	
	function contextMenuLogs() {
		$data = '<div class="contextmenu">';
		$data .= '<i style="margin-left: 5px;" class="fa fa-caret-right"></i>';
		$data .= '<a href="#" onclick="$(\'.search\').fadeIn()" class="contextlink"><i class="fa fa-search"></i> Filter logs</a> ';
		$data .= '</div>';
		setContextMenu($data); 
	}

	function get_trip_info($trip_id) {
		global $con;
		global $timestamp_thresh;
		
		$fetch = mysqli_query($con, "SELECT * FROM `railreports` WHERE TRIPID='$trip_id' ORDER BY TIMESTAMP DESC;");
		//iterate over errors
		$has_ended = false;
		$last_timestamp = -1;
		$has_errors = false;
		$has_terminated = false;
		$has_died = false;
		$status = "";
		$IC = "";
		
		while($data = mysqli_fetch_array($fetch)) {
			$last_timestamp = intval($data['TIMESTAMP']);
			$err = intval($data['ERROR']);
			if($err == 20) $has_ended = true;
			else if($err == 5 || $err == 7) $has_terminated = true;
			else if($err == 0) {
				$IC = $data['TARGET'];
			}
			else $has_errors = true;
		}
		
		/*
		//check if there was an exit status of success
		if(!$has_ended) {
			if($has_terminated) $status = "$ICON_RED Aborted";
			else {
				if(($last_timestamp + $timestamp_thresh) < time()) $status = "$ICON_RED Assumed dead";
				else if($has_errors) $status = "$ICON_ORANGE In progress with errors";
				else $status = "$ICON_BLUE In progress";
			}
		}
		else {
			if($has_errors) $status = "$ICON_ORANGE Completed with errors";
			else $status = "$ICON_GREEN Completed";
		}
		*/
		
		if(!$has_ended && !$has_terminated && ($last_timestamp + $timestamp_thresh) < time()) $has_died = true;
		//$output .= "<tr><td>$row[TRIPID]</td><td>$IC</td><td>$status</td><td><a href=\"".guessForm()."&open=$row[TRIPID]\"><button>Open</button></a></td></tr>";
		
		return array(
			'trip_id' => $trip_id,
			'ic' => $IC,
			'has_ended' => $has_ended,
			'has_errors' => $has_errors,
			'has_terminated' => $has_terminated,
			'has_died' => $has_died,
			'last_timestamp' => $last_timestamp
		);
	}
	
	function draw_chart($keyvalues, $J_TITLE = 'Chart') {
		$J_LABELS = '[';
		$J_DATAPOINTS = '[';
		
		$count = 0;
		
		foreach($keyvalues as $key => $value) {
			if($count == 0) {
				$J_LABELS .= '"'.$key.'"';
				$J_DATAPOINTS .= $value;
			}
			else {
				$J_LABELS .= ', "'.$key.'"';
				$J_DATAPOINTS .= ', '.$value;
			}
			$count++;
		}
		$J_LABELS .= ']';
		$J_DATAPOINTS .= ']';
		
		
		echo '<script>';
		echo "var color = Chart.helpers.color;
		var barChartData = {
			labels: $J_LABELS, 
			datasets: [{
				label: 'Trip $J_TITLE',
				backgroundColor: color(window.chartColors.red).alpha(0.5).rgbString(),
				borderColor: window.chartColors.red,
				borderWidth: 1,
				data: $J_DATAPOINTS
			}]

		};";


		echo 'window.onload = function() {
			var ctx = document.getElementById("canvas").getContext("2d");
			window.myBar = new Chart(ctx, {
				type: \'bar\',
				data: barChartData,
				options: {
					responsive: true,
					legend: {
						position: \'top\',
					},
					title: {
						display: true,
						text: \'Overview\'
					},
					scales: {
						yAxes: [{
							ticks: {
								beginAtZero: true
							}
						}]
					}
				}
			});

		};';
		
		echo '</script>';
	}

	function draw_points($LABELS, $DATAPOINTS, $DATASETS, $DOM_ELEMENT = "canvas") {
		$J_LABELS = '[';
		$J_DATAPOINTS = '';
		$J_DATASETS = '';
		
		$r = rand(0, 255);
		$g = rand(0, 255);
		$b = rand(0, 255);
		for($i = 0; $i < count($LABELS); $i++) {
			if($i == 0) $J_LABELS .= '"'.$LABELS[$i].'"';
			else $J_LABELS .= ', "'.$LABELS[$i].'"';
		}
		$J_LABELS .= ']';
		
		
		for($i = 0; $i < count($DATAPOINTS); $i++) {
			$r = rand(0, 255);
			$g = rand(0, 255);
			$b = rand(0, 255);
			$point = '{
				label: "'.$DATASETS[$i].'",
				backgroundColor: \'rgba('.$r.', '.$g.', '.$b.', 1.0)\',
				borderColor: \'rgba('.$r.', '.$g.', '.$b.', 1.0)\',
				data: [';
					
			for($j = 0; $j < count($DATAPOINTS[$i]); $j++) {
				if($j == 0) $point .= $DATAPOINTS[$i][$j];
				else $point .= ', ' . $DATAPOINTS[$i][$j];
			}
			$point .= '],
				fill: false,
			}';
			
			if($i == 0) $J_DATASETS .= $point;
			else $J_DATASETS .= ', '.$point;
			
		}
		
		
		echo '<script>';
		
		echo "var config = {
            type: 'line',
            data: {
                labels: $J_LABELS,
                datasets: [ $J_DATASETS ]
            },
            options: {
                responsive: true,
                title:{
                    display:true,
                    text:'Comparison'
                },
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                hover: {
                    mode: 'nearest',
                    intersect: true
                },
                scales: {
                    xAxes: [{
                        display: true,
                        scaleLabel: {
                            display: true,
                            labelString: 'Month'
                        }
                    }],
                    yAxes: [{
                        display: true,
                        scaleLabel: {
                            display: true,
                            labelString: 'Value'
                        }
                    }]
                }
            }
        };

        window.addEventListener(\"load\", function() {
            var ctx = document.getElementById(\"$DOM_ELEMENT\").getContext(\"2d\");
			new Chart(ctx, config);
        }, false)";
		
		echo '</script>';
	}
	
	
	function draw_deviations($LABELS, $DATAPOINTS, $DOM_ELEMENT = "canvas") {
		$J_LABELS = '[';
		$J_DATAPOINTS = '';
		$J_DATASETS = '';
		
		$r = rand(0, 255);
		$g = rand(0, 255);
		$b = rand(0, 255);
		for($i = 0; $i < count($LABELS); $i++) {
			if($i == 0) $J_LABELS .= '"'.$LABELS[$i].'"';
			else $J_LABELS .= ', "'.$LABELS[$i].'"';
		}
		$J_LABELS .= ']';
		
		
		$averages = array();
		$deviations = array();
		
		//first index specifies the dataset
		//second index specifies the label value
		//todo: fix this, it won't work for shite.
		for($i = 0; $i < count($DATAPOINTS[0]); $i++) {

			//calculate mean (average)
			$avg = 0.0;
			for($j = 0; $j < count($DATAPOINTS); $j++) $avg += $DATAPOINTS[$j][$i];
			$avg = $avg / (1.0 * count($DATAPOINTS[0]));

			
			//calculate standard deviation
			$deviation = 0.0;
			for($j = 0; $j < count($DATAPOINTS); $j++) $deviation += ($DATAPOINTS[$j][$i] - $avg) * ($DATAPOINTS[$j][$i] - $avg);
			$deviation = sqrt($deviation / (1.0 * count($DATAPOINTS[0])));
		
			array_push($averages, $avg);
			array_push($deviations, $deviation);
		}
		
		
		
		

		$avg = '{
			label: "average",
			backgroundColor: \'rgba(0, 0, 255, 1.0)\',
			borderColor: \'rgba(0, 0, 255, 1.0)\',
			data: [';
			
		$pos_stdev = '{
			label: "pos stddev",
			backgroundColor: \'rgba(255, 0, 0, 0.6)\',
			borderColor: \'rgba(255, 0, 0, 1.0)\',
			data: [';
			
		$neg_stdev = '{
			label: "neg stddev",
			backgroundColor: \'rgba(255, 0, 0, 0.6)\',
			borderColor: \'rgba(255, 0, 0, 1.0)\',
			data: [';
				
		for($i = 0; $i < count($averages); $i++) {
			if($i == 0) {
				$avg .= $averages[$i];
				$pos_stdev .= ($averages[$i] + $deviations[$i]);
				$neg_stdev .= ($averages[$i] - $deviations[$i]);
			}
			else {
				$avg .=  ', ' .$averages[$i];
				$pos_stdev .=  ', ' .($averages[$i] + $deviations[$i]);
				$neg_stdev .=  ', ' .($averages[$i] - $deviations[$i]);

			}
		}
		
		
		$avg .= '],
			fill: false,
		}';
		
		$pos_stdev .= '],
			fill: false,
		}';
		
		$neg_stdev .= '],
			fill: false,
		}';

			
		
		
		
		
		echo '<script>';
		
		echo "var configdev = {
            type: 'line',
            data: {
                labels: $J_LABELS,
                datasets: [ $avg, $pos_stdev, $neg_stdev ]
            },
            options: {
                responsive: true,
                title:{
                    display:true,
                    text:'Average and deviations'
                },
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                hover: {
                    mode: 'nearest',
                    intersect: true
                },
                scales: {
                    xAxes: [{
                        display: true,
                        scaleLabel: {
                            display: true,
                            labelString: 'Month'
                        }
                    }],
                    yAxes: [{
                        display: true,
                        scaleLabel: {
                            display: true,
                            labelString: 'Value'
                        }
                    }]
                }
            }
        };
		window.addEventListener(\"load\", function() {
            var ctx2 = document.getElementById(\"$DOM_ELEMENT\").getContext(\"2d\");
            new Chart(ctx2, configdev);
        }, false);";
		
		echo '</script>';
	}

?>