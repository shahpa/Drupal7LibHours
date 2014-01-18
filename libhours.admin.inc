<?php
/**
 * @file
 * Administration functions for the lib hours module.
 *
 * @author Parth Shah <parth.shah.edu>
 * @copyright 2011 University of Houston Libraries (http://info.lib.uh.edu)
 */
 
 /**
  * Display locations and operations.
  */
function libhours_admin()  {
  
  $rs = db_select('libhours_locations', 'l')
        ->fields('l', array('lid', 'name'))
        ->condition('parent', 0, '=')
        ->orderBy('name', 'ASC')
        ->execute();
   
  $header = array('Location Name', 'Operation');
  $rows = array();
	
  foreach($rs as $record)  {
    $rows[] = array($record->name, implode(' | ', array(
		l('Edit Hours', 'admin/content/libhours/location/hours/' . $record->lid),
		l('Edit Location', 'admin/content/libhours/location/edit/' . $record->lid),
		l('Delete Location', 'admin/content/libhours/location/delete/' . $record->lid)
		)));
    // display any sub-locations to this location
    $srs = db_select('libhours_locations', 'l')
           ->fields('l', array('lid', 'name'))
           ->condition('parent', $record->lid, '=')
           ->orderBy('name', 'ASC')
           ->execute();
          
    foreach($srs as $subrecord)  {
      $rows[] = array('&nbsp;&nbsp; - ' . $subrecord->name, implode(' | ', array(
		l('Edit Hours', 'admin/content/libhours/location/hours/' . $subrecord->lid),
		l('Edit Location', 'admin/content/libhours/location/edit/' . $subrecord->lid),
		l('Delete Location', 'admin/content/libhours/location/delete/' . $subrecord->lid)
		)));
    }
  }
  //display semesters
  $semester_header = array('semester name', 'operation');
  $semester_rows = array();
  $semester = db_select('libhours_semesters', 's')
    ->fields('s', array('name', 'sid'))
    ->orderby('name', 'ASC')
    ->execute();
  foreach($semester as $record)  {
    $semester_rows[] = array($record->name, implode(' | ', array(
		l('Edit Semester', 'admin/content/libhours/semester/edit/' . $record->sid),
		l('Delete Semester', 'admin/content/libhours/semester/delete/' . $record->sid)
		)));
  }
  //display exception type
  $exception_type_header = array('Exception Type Name', 'operation');
  $exception_type_rows = array();
  $exception_type = db_select('libhours_type', 'l')
    ->fields('l', array('name', 'tid'))
    ->orderBy('name', 'ASC')
    ->execute();
  foreach($exception_type as $record)  {
    $exception_type_rows[] = array($record->name, implode(' | ', array(
		l('Edit Exception Type', 'admin/content/libhours/exception_type/edit/' . $record->tid),
		l('Delete Exception Type', 'admin/content/libhours/exception_type/delete/' . $record->tid)
		)));
  }
  $output = theme('table', array('header' => $header, 'rows' => $rows)) . '<p>' . l('Add Location', 'admin/content/libhours/location/add') . '</p>' . '</br>' . '<p>' . theme('table', array('header' => $semester_header, 'rows' => $semester_rows)) . '<p>' . l('Add Semester', 'admin/content/libhours/semester/add') . '</p>' . '</br>' . '<p>' . theme('table', array('header' => $exception_type_header, 'rows' => $exception_type_rows)) . '<p>' . l('Add New Exception Type', 'admin/content/libhours/exception_type/add') . '</p>';

  return $output;
}

/*
 * Callback for creating or editing a semester
 *
 *@see libhours_admin_semester_form()
 */

function libhours_admin_semester_edit($sid = 0)  {
  if($sid)  {
    $semester = db_select('libhours_semesters', 's')
    ->fields('s', array('name', 'sid'))
    ->condition('sid', $sid, '=')
    ->execute()
    ->fetchAssoc();
    drupal_set_title('Edit Semester');
    $output_render = drupal_get_form('libhours_admin_semester_form', $semester);
    $output = drupal_render($output_render);
  }
  else  {
    $output = drupal_get_form('libhours_admin_semester_form');
  }
  return $output;
}

/* Creating a form for editing or creating a semester
 *
 * @see libhours_admin_semester_form_submit()
 */

function libhours_admin_semester_form($output, &$form_state, $semester = array('sid' => 0, 'name' => ''))  {
  
  $form['name'] = array(
	'#type' => 'textfield',
	'#title' => t('Semester Name'),
	'#default_value' => $semester['name'],
	'#required' => true,
	'#max_size' => 100
  );
  if ($semester['sid'])  {
    $form['sid'] = array(
		'#type' => 'value',
		'#value' => $semester['sid']
	);
		
    $form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save Semester')
	);
  }
  else  {
    $form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Add Semester')
    	);
  }
	
  return $form;
}

/**
 * Save Semester
 */
function libhours_admin_semester_form_submit($output, &$form_state)  {
  $sid = isset($form_state['values']['sid'])?$form_state['values']['sid']:0;
  if ($sid)  {
    db_update('libhours_semesters')
      ->fields(array('name' => $form_state['values']['name']))
      ->condition('sid', $sid, '=')
      ->execute();
  }
  else  {
    $id = db_insert('libhours_semesters')
      ->fields(array(
        'name' => $form_state['values']['name'],
        ))
      ->execute();
  }
	
  $form_state['redirect'] = 'admin/content/libhours';
}

/**
 * confirm deletion of a Semester
 *
 * @see libhours_admin_semester_delete_confirm_submit()
 */
function libhours_admin_semester_delete_confirm($output, &$form_state, $sid = 0)  {
  $semester = db_select('libhours_semesters', 'l')
    ->fields('l')
    ->condition('sid', $sid, '=')
    ->execute()
    ->fetchObject();
	
  $form['sid'] = array(
	'#type' => 'value',
	'#value' => $sid
	);
 
  $output = confirm_form($form, t('Are you sure you want to delete %semester?', array('%semester' => $semester->name)), 'admin/content/libhours');
 	
  return $output;
}

/**
 * do Semester deletion
 */
function libhours_admin_semester_delete_confirm_submit($output, &$form_state)  {
  if ($form_state['values']['confirm'])  {
    
    $num_deleted = db_delete('libhours_semesters')
      ->condition('sid', $form_state['values']['sid'])
      ->execute();
    $form_state['redirect'] = 'admin/content/libhours';
  }
  
}

/*
 * Callback for creating or editing an exception type
 *
 *@see libhours_admin_exception_type_form()
 */

function libhours_admin_exception_type_edit($tid = 0)  {
  if($tid)  {
    $exception_type = db_select('libhours_type', 's')
    ->fields('s', array('name', 'tid'))
    ->condition('tid', $tid, '=')
    ->execute()
    ->fetchAssoc();
    drupal_set_title('Edit Exception Type');
    $output_render = drupal_get_form('libhours_admin_exception_type_form', $exception_type);
    $output = drupal_render($output_render);
  }
  else  {
    $output = drupal_get_form('libhours_admin_exception_type_form');
  }
  return $output;
}

/* Creating a form for editing or creating an exception type
 *
 * @see libhours_admin_exception_type_form_submit()
 */

function libhours_admin_exception_type_form($output, &$form_state, $exception_type = array('tid' => 0, 'name' => ''))  {
  
  $form['name'] = array(
	'#type' => 'textfield',
	'#title' => t('Exception Name'),
	'#default_value' => $exception_type['name'],
	'#required' => true,
	'#max_size' => 100
  );
  if ($exception_type['tid'])  {
    $form['tid'] = array(
		'#type' => 'value',
		'#value' => $exception_type['tid']
	);
		
    $form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save Exception')
	);
  }
  else  {
    $form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Add Exception')
    	);
  }
	
  return $form;
}

/**
 * Save Exception type
 */
function libhours_admin_exception_type_form_submit($output, &$form_state)  {
  $tid = isset($form_state['values']['tid'])?$form_state['values']['tid']:0;
  if ($tid)  {
    db_update('libhours_type')
      ->fields(array('name' => $form_state['values']['name']))
      ->condition('tid', $tid, '=')
      ->execute();
  }
  else  {
    $id = db_insert('libhours_type')
      ->fields(array('name' => $form_state['values']['name'],
        ))
      ->execute();
  }
	
  $form_state['redirect'] = 'admin/content/libhours';
}

/**
 * confirm deletion of an exception type
 *
 * @see libhours_admin_exception_type_delete_confirm_submit()
 */
function libhours_admin_exception_type_delete_confirm($output, &$form_state, $tid = 0)  {
  $exception_type = db_select('libhours_type', 'l')
    ->fields('l')
    ->condition('tid', $tid, '=')
    ->execute()
    ->fetchObject();
	
  $form['tid'] = array(
	'#type' => 'value',
	'#value' => $tid
	);
 
  $output = confirm_form($form, t('Are you sure you want to delete %exception?', array('%exception' => $exception_type->name)), 'admin/content/libhours');
 	
  return $output;
}

/**
 * do exception type deletion
 */
function libhours_admin_exception_type_delete_confirm_submit($output, &$form_state)  {
  if ($form_state['values']['confirm'])  {
    
    $num_deleted = db_delete('libhours_type')
      ->condition('tid', $form_state['values']['tid'])
      ->execute();
    $form_state['redirect'] = 'admin/content/libhours';

  }
  
}

/**
 * Callback for creating or editing a location
 *
 * @see libhours_admin_location_form()
 */
function libhours_admin_location_edit($lid = 0)  {
  if ($lid)  {
    $location = db_select('libhours_locations', 'l')
                ->fields('l', array('lid', 'name', 'description', 'parent'))
                ->condition('lid', $lid, '=')
                ->execute()
                ->fetchAssoc();

    drupal_set_title('Edit Location');
    $output_render = drupal_get_form('libhours_admin_location_form', $location);
    $output = drupal_render($output_render);
  }
  else  {
    $output = drupal_get_form('libhours_admin_location_form');
  }
	
  return $output;
}

/**
 * Form for creating or editing a location
 *
 * @see libhours_admin_location_form_submit()
 */
function libhours_admin_location_form($output, &$form_state, $location = array('lid' => 0, 'name' => '', 'description' => '', 'parent' => 0))  {

  
  $form['name'] = array(
	'#type' => 'textfield',
	'#title' => t('Location Name'),
	'#default_value' => $location['name'],
	'#required' => true,
	'#max_size' => 100
  );

  $form['description'] = array(
	'#type' => 'textarea',
	'#title' => t('Description'),
	'#default_value' => $location['description'],
	'#max_size' => 500
  );

  $locs = array();
  $locs[0] = 'None';
  $rs = db_select('libhours_locations', 'l')
        ->fields('l', array('lid', 'name'))
        ->condition('parent', 0, '=')
        ->condition('lid', $location['lid'], '!=')
        ->execute();
  foreach($rs as $record)  {
    $locs[$record->lid] = $record->name;
  }

  $form['parent'] = array(
	'#type' => 'select',
	'#title' => 'Parent Location',
	'#description' => t('If this location is a sub-location please select the location it belongs to.'),
	'#options' => $locs,
	'#default_value' => $location['parent']
  );

  if ($location['lid'])  {
    $form['lid'] = array(
		'#type' => 'value',
		'#value' => $location['lid']
	);
		
    $form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save Location')
	);
  }
  else  {
    $form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Add Location')
    	);
  }
  return $form;
}

/**
 * Save Location
 */
function libhours_admin_location_form_submit($output, &$form_state)  {
  $lid = isset($form_state['values']['lid'])?$form_state['values']['lid']:0;
  	
  $data = array($form_state['values']['name'], $form_state['values']['description'], $form_state['values']['parent'], $lid);
	
  if ($lid)  {
    
    db_update('libhours_locations')
      ->fields(array('name' => $form_state['values']['name'], 'description' => $form_state['values']['description'], 'parent' => $form_state['values']['parent']))
      ->condition('lid', $lid, '=')
      ->execute();
  }
  else  {
    // Adding default hours
    // New location has default hours from 7am to 5pm Monday to Friday.
    // Default Semester will be assigned automatically.
    // Date range will be from current date to 01/07/2030.

    $id = db_insert('libhours_locations')
      ->fields(array(
        'name' => $form_state['values']['name'],
        'description' => $form_state['values']['description'],
        'parent' => $form_state['values']['parent']
      ))
      ->execute();

    $id_select = db_select('libhours_locations', 'l')
      ->fields('l', array('lid'))
      ->condition('name', $form_state['values']['name'], '=')
      ->execute()
      ->fetchAssoc();

    $id_period = db_insert('libhours_periods')
      ->fields(array(
        'lid' => $id_select,
        'sid' => '1',
        'from_date' => strtotime("now"),
        'to_date' => strtotime('01/07/2030'),
      ))
      ->execute();
  
    $id_select = db_select('libhours_periods', 'p')
      ->fields('p', array('pid'))
      ->condition('lid', $id_select)
      ->execute()
      ->fetchAssoc();

    $id_hours = db_insert('libhours_hours')
      ->fields(array(
        'pid' => $id_select,
        'dow' => '0111110',
        'open' => date('Hi', strtotime('07:00:00')),
        'close' => date('Hi', strtotime('17:00:00')),
      ))
      ->execute();
      
  }
	
  $form_state['redirect'] = 'admin/content/libhours';
}

/**
 * confirm deletion of a location
 *
 * @see libhours_admin_location_delete_confirm_submit()
 */
function libhours_admin_location_delete_confirm($output, &$form_state, $lid = 0)  {
  
  $location = db_select('libhours_locations', 'l')
    ->fields('l')
    ->condition('lid', $lid, '=')
    ->execute()
    ->fetchObject();
	
  $form['lid'] = array(
	'#type' => 'value',
	'#value' => $lid
	);
 
  $output = confirm_form($form, t('Are you sure you want to delete %location?', array('%location' => $location->name)), 'admin/content/libhours');
 	
  return $output;
}

/**
 * do location deletion
 */
function libhours_admin_location_delete_confirm_submit($output, &$form_state)  {
  if ($form_state['values']['confirm'])  {
    /* all sub-locations to the deleted location will be made into parent locations */

    $rs = db_select('libhours_locations', 'l')
      ->fields('l', array('lid'))
      ->condition('parent', $form_state['values']['lid'])
      ->execute();
      

    foreach($rs as $record)  {
      $update = db_update('libhours_locations', 'l')
        ->fields(array(
          'patent' => 0,
          ))
        ->condition('lid', $record->lid, '=')
        ->execute();
    }

    $num_deleted = db_delete('libhours_locations')
      ->condition('lid', $form_state['values']['lid'], '=')
      ->execute();
    $num_delected2 = db_delete('libhours_exceptions')
      ->condition('lid', $form_state['values']['lid'])
      ->execute();

    $form_state['redirect'] = 'admin/content/libhours';
  }
}

/**
 * display location hours summery
 */
function libhours_admin_location_hours($lid = 0)  {
  $output = null;
  $location = db_select('libhours_locations', 'l')
    ->fields('l')
    ->condition('lid', $lid, '=')
    ->execute();
    
  foreach($location as $record)  {
    drupal_set_title('Location Information: ' . t($record->name));
  }
		
  $output .= '<h2>Periods</h2>';
	
  $header = array('Semester', 'Date Rage', 'Operations');
  $rows = array();
  $rs = db_query("SELECT p.*, s.name FROM {libhours_periods} p INNER JOIN {libhours_semesters} s ON s.sid=p.sid WHERE p.lid=:lid ORDER BY p.from_date ASC", array(':lid' => $lid));
  foreach($rs as $record)  {
    $from_date = date("n/j/Y", $record->from_date);
    $to_date = date("n/j/Y", $record->to_date);
		
    $rows[] = array($record->name, $from_date . ' - ' . $to_date, implode(' | ', array(
		l('View Hours', 'admin/content/libhours/period/hours/' . $record->pid),
		l('Edit Date Range', 'admin/content/libhours/period/edit/' . $lid . '/' . $record->pid),
		l('Delete Period', 'admin/content/libhours/period/delete/' . $record->pid)
		))); 
  }
  
  $output .= theme('table', array('header' => $header, 'rows' => $rows)) . '<p>' . l('Add Period', 'admin/content/libhours/period/add/' . $lid) . '</p>';
	
  $output .= '<br /><br /><h2>Exceptions</h2>';
  $header = array('Label', 'Type', 'Date Rage', 'Hours', 'Operations');
  $rows = array();
  $rs = db_query("SELECT e.*, t.name FROM {libhours_exceptions} e INNER JOIN {libhours_type} t ON t.tid=e.tid WHERE e.lid=:lid ORDER BY e.from_date", array(':lid' => $lid));
  foreach($rs as $record)  {
    $hdisplay = _libhours_displayhours($record->open, $record->close);
		
    $from_date = date("n/j/Y", $record->from_date);
    $to_date = date("n/j/Y", $record->to_date);
		
    $rows[] = array($record->label, $record->name, $from_date . ' - ' . $to_date, $hdisplay, implode(' | ', array(
		l('Edit', 'admin/content/libhours/exceptions/edit/' . $lid . '/' . $record->eid),
		l('Delete', 'admin/content/libhours/exceptions/delete/' . $record->eid)
		)));
  }
	
  $output .= theme('table', array('header' => $header, 'rows' => $rows)) . '<p>' . l('Add Exception', 'admin/content/libhours/exceptions/add/' . $lid) . '</p>';
	
	
  return $output;
}

/**
 * callback to create or edit a period
 *
 * @see libhours_admin_period_form()
 */
function libhours_admin_period_edit($lid = 0, $pid = 0)  {
  if ($pid)  {
    drupal_set_title('Edit period');

    $period = db_select('libhours_periods', 'p')
      ->fields('p')
      ->condition('pid', $pid, '=')
      ->execute()
      ->fetchAssoc();
    $output = drupal_get_form('libhours_admin_period_form', $period);
  }
  else  {
    $period = array('pid' => 0, 'lid' => $lid, 'sid' => 0, 'from_date' => time(), 'to_date' => time());
    $output = drupal_get_form('libhours_admin_period_form', $period);
  }
	
  return $output;
}

/**
 * form for creating or editing periods
 *
 * @see libhours_admin_period_form_submit()
 */
function libhours_admin_period_form($output, &$form_state, $period)  { 
  $form['lid'] = array(
	'#type' => 'value',
	'#value' => $period['lid']
	);
	
  $semesters = array();
  $rs = db_select('libhours_semesters', 's')
    ->fields('s')
    ->orderBy('name')
    ->execute();
    

  
  foreach($rs as $record)  {
    $semesters[$record->sid] = $record->name;
  }
  	
  $form['sid'] = array(
	'#type' => 'select',
	'#title' => 'Period Semester',
	'#options' => $semesters,
	'#default_value' => $period['sid'],
	'#required' => true
	);
	
  $form['daterange'] = array(
	'#type' => 'fieldset',
	'#title' => 'Date Rage',
	'#collapsible' => false
	);
	
  $form['daterange']['from_date'] = array(
	'#type'	=> 'date',
	'#title' => t('From'),
	);
  	
  $form['daterange']['to_date'] = array(
	'#type' => 'date',
	'#title' => t('To'),
	);
	
  if ($period['pid'])  {
    $form['pid'] = array(
		'#type' => 'value',
		'#value' => $period['pid']
		);
	
    $form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save')
		);
  }
  else  {
    $form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Add')
		);
	
  }
  return $form;
}

/**
 * Validate Period date range
 */
function libhours_admin_period_form_validate($output, &$form_state)  {
  $form_state['values']['from_date'] = mktime(0, 0, 0, $form_state['values']['from_date']['month'], $form_state['values']['from_date']['day'], $form_state['values']['from_date']['year']);
  
  $form_state['values']['to_date'] = mktime(23, 59, 59, $form_state['values']['to_date']['month'], $form_state['values']['to_date']['day'], $form_state['values']['to_date']['year']);
	
  if ($form_state['values']['to_date'] < $form_state['values']['from_date'])  {
    form_set_error('to_date', t('To date is invalid and happens before the from date.'));
  }
}

/**
 * Save Periods
 */
function libhours_admin_period_form_submit($output, &$form_state)  {

  
  $pid = isset($form_state['values']['pid'])?$form_state['values']['pid']:0;
		
  $data = array($form_state['values']['lid'], $form_state['values']['sid'], $form_state['values']['from_date'], $form_state['values']['to_date'], $pid);
  if ($pid)  {
    $num_updated = db_update('libhours_periods')
      ->fields(array(
        'lid' => $form_state['values']['lid'],
        'sid' => $form_state['values']['sid'],
        'from_date' => $form_state['values']['from_date'],
        'to_date' => $form_state['values']['to_date']
      ))
      ->condition('pid', $pid, '=')
      ->execute();
    
  }
  else  {
    $num_insert = db_insert('libhours_periods')
      ->fields(array(
        'lid' => $form_state['values']['lid'],
        'sid' => $form_state['values']['sid'],
        'from_date' => $form_state['values']['from_date'],
        'to_date' => $form_state['values']['to_date'],
      ))
      ->execute();
  }
	
  $form_state['redirect'] = 'admin/content/libhours/location/hours/' . $form_state['values']['lid'];
}

/**
 * confirm deletion of a period
 *
 * @see libhours_admin_period_delete_confirm_submit()
 */
function libhours_admin_period_delete_confirm($output, &$form_state, $pid = 0)  {
  
  $period = db_select('libhours_periods', 'p')
    ->fields('p')
    ->condition('pid', $pid, '=')
    ->execute()
    ->fetchAssoc();
  	
  $form['pid'] = array(
	'#type' => 'value',
	'#value' => $period->pid
	);
	
  $form['lid'] = array(
	'#type' => 'value',
	'#value' => $period->lid
	);
	
  $from_date = date("n/j/Y", $period->from_date);
  $to_date   = date("n/j/Y", $period->to_date);
	
  $output = confirm_form($form, t('Are you sure you want to delete this period from %from_date to %to_date?', array('%from_date' => $from_date, '%to_date' => $to_date)), 'admin/content/libhours/location/hours/' . $period->lid);

  return $output;
}

/**
 * do the deletion of period
 */
function libhours_admin_period_delete_confirm_submit($output, &$form_state)  {
  if ($form_state['values']['confirm'])  {
    db_query("DELETE FROM {libhours_periods} WHERE pid=:form_state", array(':form_state' => $form_state['values']['pid']));
    db_query("DELETE FROM {libhours_hours} WHERE pid=:form_state", array(':form_state' => $form_state['values']['pid']));
	
    $form_state['redirect'] = 'admin/content/libhours/location/hours/' . $form_state['values']['lid'];
  }
}

/**
 * display period hours
 */
function libhours_admin_period_hours($pid = 0)  {
  if (!$pid) 
    drupal_goto('admin/content/libhours');
  $result = db_query("SELECT p.*, s.name FROM {libhours_periods} p INNER JOIN {libhours_semesters} s ON p.sid=s.sid WHERE p.pid=:pid", array(':pid' => $pid));
  $period = $result->fetchObject();
  $location_name = db_select('libhours_locations', 'l')
    ->fields('l', array('name'))
    ->condition('lid', $period->lid, '=')
    ->execute()
    ->fetchObject();
  $output = '<h2>Location: ' . $location_name->name . '</h2>';
  $output .= '<br />';

  drupal_set_title($period->name . ' Hours: ' . date("n/j/Y", $period->from_date) . ' - ' . date("n/j/Y", $period->to_date));	
  $header = array('Day of Week', 'Hours', 'Operation');
  $rows = array();
  $dow = array();	
  $row = db_select('libhours_hours', 'l')
    ->fields('l')
    ->condition('pid', $pid, '=')
    ->orderBy('dow', 'DESC')
    ->execute();

  foreach($row as $record)  {
    $dow = _libhours_dowstring($record->dow);
    $hdisplay = _libhours_displayhours($record->open, $record->close);
		
    $rows[] = array($dow, $hdisplay, implode(' | ', array(
		l('Edit', 'admin/content/libhours/hours/edit/' . $record->hid),
		l('Delete', 'admin/content/libhours/hours/delete/' . $record->hid)
		)));
  }
  	
  $output .= theme('table', array('header' => $header, 'rows' => $rows));
  $output_render = drupal_get_form('libhours_admin_hours_form', $pid);
  $output .= drupal_render($output_render);
  return $output;
}

/**
 * callback to create or edit period hours
 *
 * @see libhours_admin_hours_form()
 */
function libhours_admin_hours_edit($hid = 0)  {
  $output = array();
  if ($hid)  {
    $hours = db_select('libhours_hours', 'l')
      ->fields('l')
      ->condition('hid', $hid, '=')
      ->execute()
      ->fetchAssoc();
    $output = drupal_get_form('libhours_admin_hours_form', $hours['pid'], $hours);
  }
  else  {
    $output = drupal_get_form('libhours_admin_hours_form');
  }
	
  return $output;
}

/**
 * Form to create or edit period hours
 *
 * @see libhours_admin_hours_form_validate()
 * @see libhours_admin_hours_form_submit()
 */
function libhours_admin_hours_form($output, &$form_state, $pid = 0, $hours = array('hid' => 0, 'pid' => 0, 'dow' => '0000000', 'open' => 0, 'close' => 0))  {
 
  if ($hours['hid'])  {
    $form['general_hours'] = array(
		'#type' => 'fieldset',
		'#title' => 'Edit Hours'
		);
  }
  else  {
    $form['general_hours'] = array(
		'#type' => 'fieldset',
		'#title' => 'Add Hours'
		);
  }
	
  $dow = array();
  $dow_values = array();
  for ($i=0;$i<7;$i++)  {
    $dow[$i+1] = _libhours_dow($i);
    if ($hours['dow'][$i] == '1') 
      $dow_values[] = $i+1;
  }
  $format = array();
  $format = 'g:ia';
  $default = '00:00 a';
  $form['general_hours']['dayofweek'] = array(
	'#type' => 'checkboxes',
	'#options' => $dow,
	'#default_value' => $dow_values,
	'#prefix' => '<div class="libhours-dow-checkboxes">',
	'#suffix' => '</div>'
	);
	
  
  $open = _libhours_parsetime($hours['open']);
  
  $open_arry = _libhours_splittime($open);
	
  $close = _libhours_parsetime($hours['close']);
  $close_arry = _libhours_splittime($close);
	
  $form['general_hours']['open'] = array(
	'#type' => 'date_select',
	'#title' => t('Open'),
	'#description' => t('Time the location opens. To close a location set Open and Close to 12:00am. To open 24hrs set Open to 12:00am and Close to 11:59pm'),
	'#required' => true,
        '#default_value' => $default,
	'#date_format' => $format,
        //'#date_increment' => 5
	);
  	
  $form['general_hours']['close'] = array(
	'#type' => 'date_select',
	'#title' => t('Close'),
	'#description' => t('Time the location closes. To close a location set Open and Close to 12:00am. To open 24hrs set Open to 12:00am and Close to 11:59pm'),
	'#required' => true,
	'#default_value' => $default,
	'#date_format' => $format,
	);
	
  if ($pid)  {
    $form['pid'] = array(
		'#type' => 'value',
		'#value' => $pid
	);
  }
	
  if($hours['hid'])  {
    $form['hid'] = array(
		'#type' => 'value',
		'#value' => $hours['hid']
		);
  $form['general_hours']['submit'] = array(
		'#type' => 'submit',
		'#value' => 'Save Hours'
		);
  $form['general_hours']['cancel'] = array(
		'#type' => 'button',
		'#value' => 'Cancel',
		'#executes_submit_callback' => true,
		'#submit' => array('libhours_admin_hours_form_cancel')
		);
  }
  else  {
    $form['general_hours']['submit'] = array(
		'#type' => 'submit',
		'#value' => 'Add Hours'
		);
  }
  $lid = db_select('libhours_periods', 'p')
      ->fields('p', array('lid'))
      ->condition('pid', $pid, '=')
      ->execute()
      ->fetchObject();
  $form['cancel'] = array(
      '#type' => 'markup',
      '#value' => l('Done', 'admin/content/libhours/location/hours/' . $lid->lid)
    );	
  return $form;
}

/**
 * cancels editing the from and returns to the period
 */ 
function libhours_admin_hours_form_cancel($output, &$form_state)  {
  $hid = (isset($form_state['values']['hid']))?$form_state['values']['hid']:0;

  if ($hid)
    drupal_goto('admin/content/libhours/period/hours/' . $form_state['values']['pid']);
  else  {
    $lid = db_select('libhours_periods', 'p')
      ->fields('p', array('lid'))
      ->condition('pid', $form_state['values']['pid'])
      ->execute()
      ->fetchObject();

    drupal_goto('admin/content/libhours/location/hours/' . $lid);
  }
}

/**
 * Hours from validation
 *
 * @see libhours_admin_hours_form()
 */
function libhours_admin_hours_form_validate($form, &$form_state)  {
  /* check to make sure these hours are attached to a period. they should be but incase
     of coder error.
  */
  if (!isset($form_state['values']['pid']))  {
    form_set_error('', t('Internal error occurred. Unable to find pid identifier.'));
  }
	
  $hid = (isset($form_state['values']['hid'])?$form_state['values']['hid']:0);
	
  /* sort out the dow binary */
  $dow = '';
  for ($i=1;$i<=7;$i++)  {
    $dow .= ($form_state['values']['dayofweek'][$i] != 0)?'1':'0';
  }
  if ($dow == '0000000')  {
    form_set_error('dayofweek', t('At least one day must be selected'));
  }
  $form_state['values']['dow'] = $dow;
	
  $open = date('Hi', strtotime($form_state['values']['open']));
  $close = date('Hi', strtotime($form_state['values']['close']));
  $form_state['values']['open'] = $open;
  $form_state['values']['close'] = $close;
  	
  /* ok lets look at the other hours in this period and make sure we don't conflict with anything */
  
  $rs = db_query("SELECT * FROM {libhours_hours} WHERE pid = :pid AND hid != :hid", array(':pid' => $form_state['values']['pid'], ':hid' => $hid));
  foreach($rs as $record)  {
    $error_hours = _libhours_dowstring($record->dow) . ' / ' . _libhours_parsetime($record->open) . ' - ' . _libhours_parsetime($record->close);
    for ($i=0;$i<strlen($record->dow);$i++)  {
      if ($record->dow[$i] == '1' && $dow[$i] == '1')  {
	$close_check = ($record->close < $record->open)?$record->close+2400:$record->close;
	$error = false;
				
	/* time to check the hours for any conflicts */
	if ($record->open == 0 && $record->close == 0)  { 
          /* location closed */
	  $error = true;
	}
	elseif ($record->open == 0 && $record->close == 2359)  { 
          /* location open 24hrs */
	  $error = true;
	}
	elseif ($open > $record->open || $close > $record->open)  {
	  if ($open < $close_check || $close < $close_check)  {
	    $error = true;
	  }
	}
	elseif ($record->close < $record->open)  {
	  if ($open < $record->close || $close < $record->close)  {
	    $error = true;
	  }
	}
				
	if ($error) 
          form_set_error('', t('Sorry, this time conflicts with %hours.', array('%hours' => $error_hours)));
				
      }
    }
  }
}

/**
 * save the hours
 */
function libhours_admin_hours_form_submit($output, &$form_state)  {
  $hid = (isset($form_state['values']['hid'])?$form_state['values']['hid']:0);
	
  $data = array($form_state['values']['pid'], $form_state['values']['dow'], $form_state['values']['open'], $form_state['values']['close'], $hid);
	
  if($hid)  {
    $num_update = db_update('libhours_hours')
      ->fields(array(
      'pid' => $form_state['values']['pid'],
      'dow' => $form_state['values']['dow'],
      'open' => $form_state['values']['open'],
      'close' => $form_state['values']['close'],
      ))
      ->condition('hid', $hid, '=')
      ->execute();
  }
  else  {
    $nid = db_insert('libhours_hours')
      ->fields(array(
      'pid' => $form_state['values']['pid'],
      'dow' => $form_state['values']['dow'],
      'open' => $form_state['values']['open'],
      'close' => $form_state['values']['close'],
      ))
      ->execute();
  }
	
  $form_state['redirect'] = 'admin/content/libhours/period/hours/' . $form_state['values']['pid'];
}

/**
 * confirm deletion of hours
 *
 * @see libhours_admin_hours_delete_confirm_submit()
 */
function libhours_admin_hours_delete_confirm($output, &$form_state, $hid = 0)  {
  $hours = db_select('libhours_hours', 'l')
    ->fields('l')
    ->condition('hid', $hid, '=')
    ->execute()
    ->fetchObject();
  $form['hid'] = array(
	'#type' => 'value',
	'#value' => $hid
	);
	
  $form['pid'] = array(
	'#type' => 'value',
	'#value' => $hours->pid
	);
	
  $hdisplay = _libhours_dowstring($hours->dow) . ' / ' . _libhours_displayhours($hours->open, $hours->close);
	
  $output = confirm_form($form, t('Are you sure you want to delete the hours for %hours?', array('%hours' => $hdisplay)), 'admin/content/libhours/period/hours/' . $hours->pid);

  return $output;
}

/**
 * do the deletion of hours
 */
function libhours_admin_hours_delete_confirm_submit($output, &$form_state)  { 
  if($form_state['values']['confirm'])  {
    db_query("DELETE FROM {libhours_hours} WHERE hid=:hid", array(':hid' => $form_state['values']['hid']));
		
    $form_state['redirect'] = 'admin/content/libhours/period/hours/' . $form_state['values']['pid'];
  }
}

/**
 * Callback to create or edit exceptions
 */
function libhours_admin_exceptions_edit($lid = 0, $eid = 0)  {
  if ($eid)  {
    $exception = db_select('libhours_exceptions','e' )
      ->fields('e')
      ->condition('eid', $eid, '=')
      ->execute()
      ->fetchAssoc();
    $output = drupal_get_form('libhours_admin_exceptions_form', $lid, $exception);
  }
  else  {
    $output = drupal_get_form('libhours_admin_exceptions_form', $lid);
  }
	
  return $output;
}

/**
 * form for creating or editing exceptions
 *
 * @see libhours_admin_exceptions_form_validate()
 * @see libhours_admin_exceptions_form_submit()
 */
function libhours_admin_exceptions_form($output, &$form_state, $lid = 0, $exception = array('eid' => 0, 'lid' => 0, 'tid' => 0, 'label' => '', 'from_date' => 0, 'to_date' => 0, 'open' => 0, 'close' => 0))  {
  $form['lid'] = array(
		'#type' => 'value',
		'#value' => $lid
	);

  $rs = db_select('libhours_type', 'l')
    ->fields('l')
    ->orderBy('name', 'ASC')
    ->execute();
  $types = array();
  $format = 'H:i a';
  $default = '12:00 a';
  foreach($rs as $record)  {
    $types[$record->tid] = $record->name;
  }	

  $form['tid'] = array(
	'#type' => 'select',
	'#title' => 'Type',
	'#options' => $types,
	'#default_value' => $exception['tid']
	);
	
  $form['label'] = array(
	'#type' => 'textfield',
	'#title' => 'Label',
	'#max_size' => 500,
	'#required' => true,
	'#default_value' => $exception['label']
	);
	
  $form['daterange'] = array(
	'#type' => 'fieldset',
	'#title' => 'Date Range',
	'#collapsable' => false
	);
	
  if ($exception['from_date'] == 0) 
    $exception['from_date'] = time();
  if($exception['to_date'] == 0) 
    $exception['to_date'] = time();
		
  $form['daterange']['from_date'] = array(
	'#type'	=> 'date',
	'#title' => t('From'),
	'#default_value' => array('year' => date('Y', $exception['from_date']), 'month' => date('n', $exception['from_date']), 'day' => date('j', $exception['from_date']))
	);
	
  $form['daterange']['to_date'] = array(
	'#type' => 'date',
	'#title' => t('To'),
	'#default_value' => array('year' => date('Y', $exception['to_date']), 'month' => date('n', $exception['to_date']), 'day' => date('j', $exception['to_date']))
	);
	
  $open = _libhours_parsetime($exception['open']);
  $open_arry = _libhours_splittime($open);
	
  $close = _libhours_parsetime($exception['close']);
  $close_arry = _libhours_splittime($close);
	
  $form['general_hours'] = array(
	'#type' => 'fieldset',
	'#title' => 'Hours',
	'#collapsable' => false
	);

  
  $form['general_hours']['open'] = array(
	'#type' => 'date_select',
	'#title' => t('Open'),
	'#description' => t('Time the location opens. For closed location set Open and Close to 12:00am. To open 24hrs set Open to 12:00am and Close to 11:59pm'),
	'#required' => true,
	'#default_value' => $default,
	'#date_format' => $format,
        '#date_increment' => 10
	);
	
  $form['general_hours']['close'] = array(
	'#type' => 'date_select',
	'#title' => t('Close'),
	'#description' => t('Time the location closes. For closed location set Open and Close to 12:00am. To open 24hrs set Open to 12:00am and Close to 11:59pm'),
	'#required' => true,
	'#default_value' => $default,
	'#date_format' => $format,
        '#date_increment' => 10
	);

  if($exception['eid'])  {
    $form['eid'] = array(
		'#type' => 'value',
		'#value' => $exception['eid']
		);
    $form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save Exception')
		);	
  }
  else  {
    $form['submit'] = array(
	'#type' => 'submit',
	'#value' => t('Add Exception')
	);
  }

  return $form;
}

/**
 * validate exception from
 *
 * @see libhours_admin_exceptions_form_submit()
 */
function libhours_admin_exceptions_form_validate($form, &$form_state)  {
  $eid = (isset($form_state['values']['eid']))?$form_state['values']['eid']:0;
	
  $form_state['values']['from_date'] = mktime(0, 0, 0, $form_state['values']['from_date']['month'], $form_state['values']['from_date']['day'], $form_state['values']['from_date']['year']);
  $form_state['values']['to_date'] = mktime(23, 59, 59, $form_state['values']['to_date']['month'], $form_state['values']['to_date']['day'], $form_state['values']['to_date']['year']);
	
  if ($form_state['values']['to_date'] < $form_state['values']['from_date'])  {
    form_set_error('to_date', t('Sorry, the dates are invalid. The To date falls before the From date.'));
  }
	
  $form_state['values']['open'] = date('Hi', strtotime($form_state['values']['open']));
  $form_state['values']['close'] = date('Hi', strtotime($form_state['values']['close']));
	
  $rs = db_query("SELECT * FROM {libhours_exceptions} WHERE lid=:lid AND eid != :eid", array(':lid' => $form_state['values']['lid'], ':eid' => $eid));
  foreach($rs as $record)  {
    /* check to see if the exception entered is during another exception date */
    if (($form_state['values']['from_date'] >= $record->from_date && $form_state['values']['from_date'] <= $record->to_date) || ($form_state['values']['to_date'] >= $record->from_date && $form_state['values']['to_date'] <= $record->to_date))  {
      /* they dates overlap so check to make sure the hours don't overlap */
      $close_check = ($record->close < $record->open)?$record->close+2400:$record->close;
      $error = false;
				
      /* time to check the hours for any conflicts */
      if ($record->open == 0 && $record->close == 0)  { 
        /* location closed */
        $error = true;
      }
      elseif ($record->open == 0 && $record->close == 2359)  { 
        /* location open 24hrs */
        $error = true;
      }
      elseif ($form_state['values']['open'] > $record->open || $form_state['values']['close'] > $record->open)  {
        if ($form_state['values']['open'] < $close_check || $form_state['values']['close'] < $close_check)  {
          $error = true;
        }
      }
      elseif ($record->close < $record->open)  {
        if($form_state['values']['open'] < $record->close || $form_state['values']['close'] < $record->close)  {
          $error = true;
        }
      }
			
      $excstr = $record->label . ' From: ' . date("n/j/Y", $record->from_date) . ' To: ' . date("n/j/Y", $record->to_date) . ' @ ' . _libhours_displayhours($record->open, $record->close); 
			
      if ($error) 
        form_set_error('', t('Sorry but the exception conflicts with other exception: %exc', array('%exc' => $excstr)));
    }	
  }
}

/**
 * do save of exceptions form
 */
function libhours_admin_exceptions_form_submit($output, &$form_state)  {
  $eid = (isset($form_state['values']['eid']))?$form_state['values']['eid']:0;
	
  $data = array($form_state['values']['lid'], $form_state['values']['tid'], $form_state['values']['label'], $form_state['values']['from_date'], $form_state['values']['to_date'], $form_state['values']['open'], $form_state['values']['close'], $eid);
	
  if ($eid)  {
    $num_updated = db_update('libhours_exceptions')
      ->fields(array(
        'lid' => $form_state['values']['lid'],
        'tid' => $form_state['values']['tid'],
        'label' => $form_state['values']['label'],
        'from_date' => $form_state['values']['from_date'],
        'to_date' => $form_state['values']['to_date'],
        'open' => $form_state['values']['open'],
        'close' => $form_state['values']['close']
        ))
      ->condition('eid', $eid, '=')
      ->execute();
  }
  else  {
    $id = db_insert('libhours_exceptions')
      ->fields(array(
        'lid' => $form_state['values']['lid'],
        'tid' => $form_state['values']['tid'],
        'label' => $form_state['values']['label'],
        'from_date' => $form_state['values']['from_date'],
        'to_date' => $form_state['values']['to_date'],
        'open' => $form_state['values']['open'],
        'close' => $form_state['values']['close']
      ))
      ->execute();
  }
	
  $form_state['redirect'] = 'admin/content/libhours/location/hours/' . $form_state['values']['lid'];
}

/**
 * confirm deletion of exception
 *
 * @see libhours_admin_exceptions_delete_confirm_submit()
 */
function libhours_admin_exceptions_delete_confirm($output, &$form_state, $eid = 0)  {
  $exception = db_select('libhours_exceptions', 'e')
    ->fields('e')
    ->condition('eid', $eid, '=')
    ->execute();
  foreach($exception as $record)  {	
    $form['eid'] = array(
      '#type' => 'value',
      '#value' => $eid
      );
	
    $form['lid'] = array(
      '#type' => 'value',
      '#value' => $record->lid
      );
	
    $excdisplay = $record->label . ' on ' . date("n/j/Y", $record->from_date) . ' - ' . date("n/j/Y", $record->to_date) . ' @ ' . _libhours_displayhours($record->open, $record->close);
  }
  $output = confirm_form($form, t('Are you sure you want to delete the exception %exc?', array('%exc' => $excdisplay)), 'admin/content/libhours/location/hours/' . $record->lid);

  return $output;
}

/**
 * do the deletion of hours
 */
function libhours_admin_exceptions_delete_confirm_submit($output, &$form_state)  {
  if ($form_state['values']['confirm'])  {
    db_query("DELETE FROM {libhours_exceptions} WHERE eid=:eid", array('eid' => $form_state['values']['eid']));
		
    $form_state['redirect'] = 'admin/content/libhours/location/hours/' . $form_state['values']['lid'];
  }
}
