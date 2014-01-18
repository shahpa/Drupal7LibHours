<?php
/**
 * @file
 * API function callbacks for libhours module
 *
 * @author Parth Shah <parth.shah@ucdenver.edu>
 * 
 */
 
/**
 * Pre processes the REST API call
 */
function libhours_api()  {
  $format   = isset($_GET['format'])?$_GET['format']:'';
  $callback = isset($_GET['callback'])?$_GET['callback']:'';
  $action   = isset($_GET['action'])?$_GET['action']:'';
  $library  = isset($_GET['library'])?$_GET['library']:'';
	
  libhours_api_process($format, $action, $library, $callback);
}

/**
 * Process the API call
 *
 * @param $format The return format to use
 * @param $action The action being done to return the results
 * @param $library The unique identification id for the library location
 * @param $callback The callback function used in JSONP
 */
function libhours_api_process($format = '', $action = '', $library = 0, $callback = '')  {
  global $conf;
  // turn off drupal cache
  $conf['cache'] = false;
	
  $format = strtolower($format);

  if (empty($action))  {
    libhours_api_error($format, "Missing 'action'.");
    return;
  }
	
  switch ($action)  {

    case 'locations':
      $rs = db_select('libhours_locations', 'l')
        ->fields('l')
        ->condition('parent', '0', '=')
        ->orderBy('name', 'ASC')
        ->execute();
      foreach($rs as $record)  {
        $locations[] = array('id' => $record->lid, 'location' => $record->name);
        $srs = db_select('libhours_locations', 'l')
          ->fields('l')
          ->condition('parent', $record->lid, '=')
          ->orderBy('name', 'ASC')
          ->execute();
        foreach($srs as $Subrecord)  {
          $locations[] = array('id' => $Subrecord->lid, 'location' => ' -- ' . $Subrecord->name);
        }
      }	
      $return = array('status' => 'ok', 'locations' => $locations);
      break;
		
    case 'hours':
      if (empty($library))  {
        libhours_api_error($format, "Missing 'library' location id");
	return;
      }
			
      $location = db_query("SELECT name FROM {libhours_locations} WHERE lid=%d", $library)->fetchField();
			
      $today = time();
      $period = db_query("SELECT p.pid, s.name, p.from_date, p.to_date FROM {libhours_periods} p 
	INNER JOIN {libhours_semesters} s ON p.sid = s.sid
	WHERE %d >= from_date AND %d <= to_date AND lid=%d ORDER BY from_date DESC LIMIT 1", $today, $today, $library)->fetchObject();
	
      $stdhours = _libhours_api_gethours($library, $period->pid);
      $exhours = _libhours_api_getexceptions($library, $period->pid, $period->from_date, $period->to_date);
		
      $return = array('status' => 'ok', 'location' => $location, 'period' => $period->name, 'hours' => $stdhours, 'exceptions' => $exhours);
      break;
	
    case 'open':

    case 'opennow':
      $locations = _libhours_api_getopennow();
			
      $return = array('status' => 'ok', 'locations' => $locations);
      break;
	
    default:
      libhours_api_error($format, 'action not supported');
      break;
  }
	
  $output = '';
  switch ($format)  {
 
    case 'json':
      $output = json_encode($return);
      $output = (empty($callback)?$output:($callback . '(' . $output . ')'));
      drupal_set_header('Content-Type: application/json');
      break;

    default:
      libhours_api_error('html', 'Invalid format provided. Only JSON is supported.');
      return;
  }
	
  print $output;
}

/**
 * Process any error messages
 *
 * @param $format The format to return the error
 * @param $errormsg The error message
 */
function libhours_api_error($format = 'html', $errormsg = 'Unknown error occurred.')  {
  switch ($format)  {

    case 'json':
      $return['status'] = $errormsg;
      $output = json_encode($return);
      drupal_set_header('Content-Type: application/json');
      break;

    default:
      $output = '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">'
		. '<html><head><title>Error</title></head><body><strong>Error: </strong> ' . $errormsg . '</body></html>';
      break;
  }

  print $output;
  die();
}

/**
 * Returns the standard hours for a location
 *
 * @param $lid location unique identifier
 * @param $pid period unique identifier
 */
function _libhours_api_gethours($lid = 0, $pid = 0)  {
  $rh = db_select('libhours_hours', 'l')
    ->fields('l')
    ->condition('pid', $pid, '=')
    ->orderBy('dow', 'DESC')
    ->execute();
  foreach($rh as $record)  {
    for ($i=0;$i<strlen($rh->dow);$i++)  {
      if ($rh->dow[$i] == '1')  {
	$hours[$i] = array('open' => $rh->open, 'close' => $rh->close);
      }
    }

  }
 
  $return = array();
  for ($i=0;$i<count($hours);$i++)  {
    $previdx = ($i == 0)?(count($hours) - 1):($i - 1);
    $nextidx = (($i + 1) == count($hours))?0:($i + 1);
	
    $return[] = array('dayofweek' => _libhours_dow($i), 'hour' => _libhours_displayhours($hours[$i]['open'], $hours[$i]['close'], $hours[$previdx], $hours[$nextidx]));
  }
	
  return $return;
}
 
/**
 * Gets the exceptions
 * 
 * @param $lid location unique identifier
 * @param $pid period unique identifier
 * @param $from_date int period from date
 * @param $to_date int period to date
 */
function _libhours_api_getexceptions($lid = 0, $pid = 0, $from_date = 0, $to_date = 0)  {
  $today = time();
  $hours = array();
  $ers = db_select('libhours_exceptions', 'e')
    ->fields('e')
    ->condition('lid', $lid, '=')
    ->condition('from_date', $from_date, '<=')
    ->condition('to_date', $to_date, '<=')
    ->orderBy('from_date', 'ASC')
    ->execute();
  foreach($ers as $record)  {
    if (date("n/d/Y", $ers->from_date) == date("n/d/Y", $ers->to_date))  {
      $date = date("n/d l", $ers->from_date);
    }
    else  {
      $date = date("n/d", $ers->from_date) . ' - ' . date("n/d", $ers->to_date);
    }
		
    $hours[] = array('date' => $date, 'hour' => _libhours_displayhours($ers->open, $ers->close));
  }
	
  return $hours;
}

/**
 * Get list of locations currently open
 */
function _libhours_api_getopennow()  {
  $today = time();
  $dow  = date("w", $today);
  $time = date("Hi", $today);


  $query = "SELECT l.lid AS id, l.name AS location, l.parent, h.open, h.close FROM {libhours_locations} l 
	INNER JOIN {libhours_periods} p ON l.lid=p.lid
	INNER JOIN {libhours_hours} h ON p.pid=h.pid
	WHERE SUBSTRING(h.dow, :dow, 1) = '1' AND :today >= p.from_date AND :today <= p.to_date";

  /* get the regular hours */
  $rs = db_query($query . " AND parent=0 ORDER BY location", array(':dow' => ($dow+1), ':today' => $today, ':today' => $today));
  foreach($rs as $record)  {
    $checktime = ($rs->close < $rs->open)?($rs->close + 2400):$rs->close;
    if ($time >= $rs->open && $time < $checktime)  {
      $return[$rs->id] = array('id' => $rs->id, 'location' => $rs->location, 'hours' => _libhours_displayhours($rs->open, $rs->close));
    }
    $crs = db_query($query . " AND parent=:rs ORDER BY location", array(':dow' => ($dow+1), ':today' => $today, '$today' => $today, ':rs' => $rs->id));
    foreach($crs as $Subrecord)  {
      $checktime = ($Subrecord->close < $Subrecord->open)?($Subrecord->close + 2400):$Subrecord->close;
      if ($time >= $Subrecord->open && $time < $checktime)  {
	$return[$Subrecord->id] = array('id' => $Subrecord->id, 'location' => ' -- ' . $Subrecord->location, 'hours' => _libhours_displayhours($Subrecord->open, $Subrecord->close));
      }
    }
  }

  /* find any exceptions for today */
  $rs = db_query("SELECT l.lid AS id, l.name AS location, e.open, e.close FROM {libhours_locations} l
		INNER JOIN {libhours_exceptions} e ON e.lid=l.lid
		WHERE :today >= e.from_date AND :today <= e.to_date", array(':today' => $today, ':today' => $today));
  /* going to only removed locations that might be currently closed because of a exception, or see if a new location is open */
  foreach($rs as $record)  {
    $checktime = ($rs->close < $rs->open)?($rs->close + 2400):$rs->close;
    /* we have a new location that is open because of a exception */
    if ($time >= $rs->open && $time < $checktime && !isset($return[$rs->id]))  {
      $return[$rs->id] = array('id' => $rs->id, 'location' => $rs->location, 'hours' => _libhours_displayTodaysHours($rs->open, $rs->close));
    }
    elseif ($rs->open == 0 && $rs->close == 0 && isset($return[$rs->id]))  {
      /* the location is closed */
      unset($return[$rs->id]);
    }
    elseif ($rs->open == 0 && $rs->close == 2359 && !isset($return[$rs->id]))  {
      /* the location is open 24hrs */
      $return[$rs->id] = array('id' => $rs->id, 'location' => $rs->location, 'hours' => _libhours_displayTodaysHours($rs->open, $rs->close));
    }
    elseif  ($time <= $rs->open || $time >= $checktime)  {
      /* the location is closed */
      unset($return[$rs->id]);
    }
  }

  foreach($return as $index => $location)  {
    $returns[] = $location;
  }

  return $returns;
}
