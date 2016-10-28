<?php

function nzfoverzicht_page() {
  $NZF_groupID = 2658;

  civicrm_initialize();

  $html = '<p>Aantal aanmeldingen voor de Nationaal ZorgFond campagne</p>';

  // 1. count the total number of supporters
  $sql = "
    SELECT
      COUNT(gc.id)
    FROM
      civicrm_group g
    INNER JOIN
      civicrm_group_contact gc ON gc.`group_id` = g.id
    INNER JOIN
      civicrm_value_geostelsel gs ON gs.`entity_id` = gc.`contact_id`
    INNER JOIN
      civicrm_contact afd ON gs.`afdeling` = afd.`id`
    WHERE
      g.id = %1
  ";
  $sqlParams = array(
    1 => array($NZF_groupID, 'Integer'),
  );
  $totalNumber = CRM_Core_DAO::singleValueQuery($sql, $sqlParams);

  $html .= '<h2>Landelijk</h2>';
  $html .= "<ul><li>$totalNumber</li></ul>";

  // 2. count the number of supporters per local branch
  $sql = "
    SELECT
      afd.`display_name` as afdeling
      , COUNT(gc.id) as aantal
    FROM
      civicrm_group g
    INNER JOIN
      civicrm_group_contact gc ON gc.`group_id` = g.id
    INNER JOIN
      civicrm_value_geostelsel gs ON gs.`entity_id` = gc.`contact_id`
    INNER JOIN
      civicrm_contact afd ON gs.`afdeling` = afd.`id`
    WHERE
      g.id = %1
    GROUP BY
      afd.`display_name`
    ORDER BY
      afd.`sort_name`
  ";
  $sqlParams = array(
    1 => array($NZF_groupID, 'Integer'),
  );

  $html .= '<h2>Afdeling</h2>';
  $html .= '<ul>';

  $dao = CRM_Core_DAO::executeQuery($sql, $sqlParams);
  while($dao->fetch()) {

    $html .= "<li>{$doa->afdeling}: {$dao->aantal}</li>";
  }

  $html .= '</ul>';

  return $html;
}
