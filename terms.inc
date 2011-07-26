<?php

class SwagTermMigration extends Migration {
  public function __construct(array $arguments) {
    $vid = isset($arguments['vid']) ? $arguments['vid'] : NULL;
    $vocabulary_name = isset($arguments['vocabulary_name']) ? $arguments['vocabulary_name'] : NULL;
    $description = isset($arguments['description']) ? $arguments['description'] : NULL;
    parent::__construct();
    $this->description = $description;
    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'tid' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'alias' => 'td',
        )
      ),
        MigrateDestinationTerm::getKeySchema()
      );

    $query = Database::getConnection('swagonline')
      ->select('term_data', 'td')
      ->fields('td', array('tid', 'name', 'description', 'weight'))
      ->condition('vid', $vid);
    $query->join('term_hierarchy', 'th', 'th.tid = td.tid');
    $query->fields('th', array('parent'))
      // This sort assures that parents are saved before children.
      ->orderBy('parent', 'ASC');
    $this->source = new MigrateSourceSQL($query);
    $this->source->setMapJoinable(FALSE);
    $this->destination = new MigrateDestinationTerm($vocabulary_name);

    // Mapped fields
    $this->addFieldMapping('tid', 'tid');
    $this->addFieldMapping('name', 'name');
    $this->addFieldMapping('description', 'description');
    $this->addFieldMapping('parent', 'parent');
    $this->addFieldMapping('weight', 'weight');
    //$this->addFieldMapping('is_new')->defaultValue(TRUE);
  }

  public function prepareRow($current_row) {
    return TRUE;
  }
}

class SwagForumsMigration extends SwagTermMigration {
  public function __construct() {
    $arguments = array('vid' => '4', 'vocabulary_name' => 'forum', 'description' => t('Migrate Forum Terms'));
    parent::__construct($arguments);
  }
}

class SwagGalleriesMigration extends SwagTermMigration {
  public function __construct() {
    $arguments = array('vid' => '1', 'vocabulary_name' => 'image_galleries', 'description' => t('Migrate Image Gallery Terms'));
    parent::__construct($arguments);
  }
}

class SwagBlogContentMigration extends SwagTermMigration {
  public function __construct() {
    $arguments = array('vid' => '5', 'vocabulary_name' => 'blog_content', 'description' => t('Migrate Blog Content Terms'));
    parent::__construct($arguments);
  }
}
