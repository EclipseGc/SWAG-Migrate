<?php

class SwagUserMigration extends Migration {
  public function __construct() {
    parent::__construct();
    $this->description = t('Migrate swagonline.net users');
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'uid' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'D5 Unique User ID',
          'alias' => 'u',
        )
      ),
      MigrateDestinationUser::getKeySchema()
    );

    $query = Database::getConnection('swagonline')
      ->select('users', 'u')
      ->fields('u', array('uid', 'name', 'pass', 'mail', 'created', 'access', 'login', 'status', 'init'))
      ->condition('u.name', 'Eclipse', '!=')
      ->condition('u.uid', 0, '>');
    $this->source = new MigrateSourceSQL($query);
    $this->source->setMapJoinable(FALSE);
    $this->destination = new MigrateDestinationUser();

    // Make the mappings
    $this->addFieldMapping('uid', 'uid');
    $this->addFieldMapping('name', 'name')->dedupe('users', 'name');
    $this->addFieldMapping('pass', 'pass');
    $this->addFieldMapping('mail', 'mail')->dedupe('users', 'mail');
    $this->addFieldMapping('signature')->defaultValue('');
    $this->addFieldMapping('signature_format')->defaultValue('filtered_html');
    $this->addFieldMapping('created', 'created');
    $this->addFieldMapping('access', 'access');
    $this->addFieldMapping('login', 'login');
    $this->addFieldMapping('status', 'status');
    $this->addFieldMapping('init', 'init');
    $this->addFieldMapping('timezone')->defaultValue(NULL);
    $this->addFieldMapping('roles', 'roles');
    $this->addFieldMapping('is_new')->defaultValue(TRUE);
  }

  public function prepareRow($current_row) {
    $source_id = $current_row->uid;
    $query = Database::getConnection('swagonline')
      ->select('users_roles', 'ur')
      ->fields('ur', array('uid', 'rid', ))
      ->condition('ur.uid', $source_id, '=');
    $results = $query->execute();
    $roles = array('2' => '2');
    $role_map = array(
      '2' => '2',
      '3' => '3',
      '4' => '5',
      '5' => '6',
      '7' => '4',
    );
    foreach ($results as $row) {
      if (isset($role_map[$row->rid])) {
        $roles[$role_map[$row->rid]] = $role_map[$row->rid];
      }
    }
    $current_row->roles = $roles;
    return TRUE;
  }
}