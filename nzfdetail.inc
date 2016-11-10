<?php

function nzfdetail_page() {
  // initialize civicrm
  civicrm_initialize();

  $html = '<main class="content">';

  // get query string parameters
  $queryString = drupal_get_query_parameters();

  // check if we have filters in the query string
  $filterLevel = '';
  if (array_key_exists('filterLevel', $queryString)) {
    $filterLevel = urldecode($queryString['filterLevel']);
  }

  $filterPreviousLevel = '';
  if (array_key_exists('filterPreviousLevel', $queryString)) {
    $filterPreviousLevel = urldecode($queryString['filterPreviousLevel']);
  }
  else {
    $filterPreviousLevel = $filterLevel;
  }

  $filterPreviousItem = '';
  if (array_key_exists('filterItem', $queryString)) {
    $filterItem = urldecode($queryString['filterItem']);
  }

  $filterPreviousItem = '';
  if (array_key_exists('filterPreviousItem', $queryString)) {
    $filterPreviousItem = urldecode($queryString['filterPreviousItem']);
  }
  else {
    $filterPreviousItem = $filterItem;
  }

  // get the data
  $data = nzdetail_getData($filterLevel, $filterItem, $filterPreviousLevel, $filterPreviousItem);

  // show back link
  $html .= '<p>' . $data['previousURL'] . '</p>';

  // show title if data is filtered
  if ($filterLevel && $filterItem) {
    $html .= "<h2>$filterLevel: $filterItem</h2>";
  }
  else {
    $html .= '<h2>Alle provincies</h2>';
  }

  $html .= '<table class="nzfdetail">';
  $html .= '<tr>';
  $html .= "<th>{$data['columnHeader']}</th>";
  $html .= "<th>Aantal</th>";
  $html .= '</tr>';

  while ($data['dao']->fetch()) {
    $html .= '<tr>';

    // check if we need to add a drill down url
    if ($data['nextLevel']) {
      $url = 'nationaal-zorgfonds-detail?filterLevel=' . $data['nextLevel'] . '&filterItem=' . urlencode($data['dao']->item);

      if ($filterPreviousLevel) {
        $url .= '&filterPreviousLevel=' . urlencode($filterPreviousLevel) . '&filterPreviousItem=' . urlencode($filterPreviousItem);
      }
      $html .= "<td class=\"nzfitem\"><a href=\"$url\">{$data['dao']->item}</a></td>";
    }
    else {
      // no drill down on item
      $html .= "<td class=\"nzfitem\">{$data['dao']->item}</td>";
    }

    $html .= "<td class=\"nzfvalue\">{$data['dao']->aantal}</td>";
    $html .= '</tr>';
  }

  $html .= '</table>';

  $html .= '</main>';

  return $html;
}

/**
 * returns array with column header and $dao
 */
function nzdetail_getData($filterLevel = '', $filterItem = '', $filterPreviousLevel = '', $filterPreviousItem = '') {
  // group id of NFZ - aanmeldingen
  $NZF_groupID = 2658;

  // prepare return value array
  $retval = array(
    'columnHeader' => '',
    'nextLevel' => '',
    'dao' => '',
  );

  $sqlParams = array(
    1 => array($NZF_groupID, 'Integer'),
  );

  switch ($filterLevel) {
    case 'Provincie':
      // show all cities in the specified province
      $retval['columnHeader'] = 'Gemeente';
      $retval['previousURL'] = '<a href="nationaal-zorgfonds-detail">&lt terug naar alle provincies</a>';
      $retval['nextLevel'] = 'Gemeente';

      $sql = "
        SELECT
           ag.`gemeente_24`  item
          , COUNT(gc.id) aantal
        FROM
          civicrm_group g
        INNER JOIN
          civicrm_group_contact gc ON gc.`group_id` = g.id
        INNER JOIN
          civicrm_contact c ON gc.contact_id = c.id
        INNER JOIN
          civicrm_value_geostelsel gs ON gs.`entity_id` = c.`id`
        INNER JOIN
          civicrm_contact prov ON prov.id = gs.`provincie`          
        INNER JOIN
          civicrm_address a ON a.`contact_id` = c.id AND a.is_primary = 1
        INNER JOIN
          `civicrm_value_adresgegevens_12` ag ON ag.`entity_id` = a.`id`
        WHERE
          g.id = %1
          AND c.is_deleted = 0
          AND gc.status = 'Added'
          AND prov.`display_name` = %2
        GROUP BY
          ag.`gemeente_24`
        ORDER BY 1
      ";

      $sqlParams[2] = array('SP-provincie ' . $filterItem, 'String');
      break;
    case 'Gemeente':
      // show all neighborhoors in the specified city
      $retval['columnHeader'] = 'Buurt';
      $retval['previousURL'] = '<a href="nationaal-zorgfonds-detail?filterLevel=' . urlencode($filterPreviousLevel) . '&filterItem=' . urlencode($filterPreviousItem) . '"">&lt terug naar ' . $filterPreviousLevel . ' ' . $filterPreviousItem . '</a>';
      $retval['nextLevel'] = '';

      $sql = "
        SELECT
           ag.`buurt_25`  item
          , COUNT(gc.id) aantal
        FROM
          civicrm_group g
        INNER JOIN
          civicrm_group_contact gc ON gc.`group_id` = g.id
        INNER JOIN
          civicrm_contact c ON gc.contact_id = c.id
        INNER JOIN
          civicrm_value_geostelsel gs ON gs.`entity_id` = c.`id`
        INNER JOIN
          civicrm_address a ON a.`contact_id` = c.id AND a.is_primary = 1
        INNER JOIN
          `civicrm_value_adresgegevens_12` ag ON ag.`entity_id` = a.`id`
        WHERE
          g.id = %1
          AND c.is_deleted = 0
          AND gc.status = 'Added'
          AND ag.`gemeente_24` = %2
        GROUP BY
          ag.`buurt_25`
        ORDER BY 1
      ";

      $sqlParams[2] = array($filterItem, 'String');
      break;
    default:
      // show all provinces
      $retval['columnHeader'] = 'Provincie';
      $retval['previousURL'] = '<a href="nationaal-zorgfonds-overzicht">&lt terug naar overzicht</a>';
      $retval['nextLevel'] = 'Provincie';

      $sql = "
        SELECT
          REPLACE(prov.`display_name`, 'SP-provincie ', '') item
          , COUNT(gc.id) aantal
        FROM
          civicrm_group g
        INNER JOIN
          civicrm_group_contact gc ON gc.`group_id` = g.id
        INNER JOIN
          civicrm_contact c ON gc.contact_id = c.id
        INNER JOIN
          civicrm_value_geostelsel gs ON gs.`entity_id` = c.`id`
        INNER JOIN
          civicrm_contact prov ON prov.id = gs.`provincie`
        WHERE
          g.id = 1503
          AND c.is_deleted = 0
          AND gc.status = 'Added'
        GROUP BY
          prov.`display_name`
        ORDER BY 1
      ";
  }

  $retval['dao'] = CRM_Core_DAO::executeQuery($sql, $sqlParams);


  return $retval;
}