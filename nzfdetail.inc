<?php

/**
 TIJDELIJK ALAIN
 *
 *
 *
 * http://localhost/~alain/sp_upgrade_46/nationaal-zorgfonds-detail?filterLevel=Provincie&filterItem=Utrecht
 *
 * TODO
 * url op item
 * terug link
 */

function nzfdetail_page() {
  // initialize civicrm
  civicrm_initialize();

  $html = '<main class="content">';

  // get query string parameters
  $queryString = drupal_get_query_parameters();

  // check if we have filters in the query string
  $filterLevel = '';
  if (array_key_exists('filterLevel', $queryString)) {
    $filterLevel = $queryString['filterLevel'];
  }

  $filterItem = '';
  if (array_key_exists('filterItem', $queryString)) {
    $filterItem = $queryString['filterItem'];
  }

  $html .= '<p><a href="nationaal-zorgfonds-overzicht">&lt; terug</a></p>';

  $data = nzdetail_getData($filterLevel, $filterItem);

  $html .= '<table>';
  $html .= '<tr>';
  $html .= "<th>{$data['columnHeader']}</th>";
  $html .= "<th>Aantal</th>";
  $html .= '</tr>';


  while($data['dao']->fetch()) {
    $html .= '<tr>';
    $html .= "<td>{$data['dao']->item}</td>";
    $html .= "<td>{$data['dao']->aantal}</td>";
    $html .= '</tr>';
  }

  $html .= '</table>';

  $html .= '</main>';

  return $html;
}

/**
 * @param string $filterLevel
 * @param string $filterItem
 *
 * @Return array with column header and $dao
 */
function nzdetail_getData($filterLevel = '', $filterItem = '') {
  // group id of NFZ - aanmeldingen
  $NZF_groupID = 1503; /* ORIG = 2658; */

  // prepare return value array
  $retval = array(
    'columnHeader' => '',
    'dao' => '',
  );

  $sqlParams = array(
    1 => array($NZF_groupID, 'Integer'),
  );

  switch ($filterLevel) {
    case 'Provincie':
      // show all cities in the specified province
      $retval['columnHeader'] = 'Gemeente';
      $retval['higherLevel'] = 'ttt';

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
      break;
    default:
      // show all provinces
      $retval['columnHeader'] = 'Provincie';

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