<?php

class SwagNodesMigration extends Migration {
  public function __construct(array $arguments) {
    parent::__construct();
    $type = isset($arguments['type']) ? $arguments['type'] : NULL;
    $new_type = isset($arguments['new type']) ? $arguments['new type'] : $type;
    $this->description = t('Migrate swagonline.net nodes');
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'nid' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'D5 Unique User ID',
          'alias' => 'n',
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $query = $this->node_query($type);
    $this->set_highwater_field();
    $this->set_source($query);
    $this->destination = new MigrateDestinationNode($new_type);
    $this->node_field_mapping();
  }

  public function set_highwater_field() {
    $this->highwaterField = array(
      'name' => 'changed',
      'alias' => 'n',
    );
  }

  public function node_query($type) {
    $query = Database::getConnection('swagonline')
      ->select('node', 'n')
      ->fields('n', array('nid', 'vid', 'title', 'uid', 'status', 'created', 'changed', 'comment', 'promote', 'moderate', 'sticky'))
      ->condition('n.type', $type);
    $query->join('node_revisions', 'nr', 'nr.vid = n.vid');
    $query->fields('nr', array('body', 'teaser', 'format'));
    return $query;
  }

  public function set_source($query) {
    $this->source = new MigrateSourceSQL($query);
    $this->source->setMapJoinable(FALSE);
  }

  public function node_field_mapping() {
    $body_arguments = MigrateTextFieldHandler::arguments(array('source_field' => 'teaser'), array('source_field' => 'format'), NULL);
    // Make the mappings
    $this->addFieldMapping('nid', 'nid');
    $this->addFieldMapping('vid', 'vid');
    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('uid', 'uid');
    $this->addFieldMapping('status', 'status');
    $this->addFieldMapping('created', 'created');
    $this->addFieldMapping('changed', 'changed');
    $this->addFieldMapping('comment', 'comment');
    $this->addFieldMapping('promote', 'promote');
    $this->addFieldMapping('moderate', 'moderate');
    $this->addFieldMapping('sticky', 'sticky');
    $this->addFieldMapping('body', 'body')->arguments($body_arguments);
    $this->addFieldMapping('language')->defaultValue('und');
    $this->addFieldMapping('is_new')->defaultValue(TRUE);
  }

  public function prepareRow($current_row) {
    $formats = array(
      '1' => 'filtered_html',
      '2' => 'php_code',
      '3' => 'full_html',
    );
    $current_row->format = isset($formats[$current_row->format]) ? $formats[$current_row->format] : 'plain_text';
  }
}

class SwagNodesPagesMigration extends SwagNodesMigration {
  public function __construct() {
    parent::__construct(array('type' => 'page'));
  }
}

class SwagNodesNewsMigration extends SwagNodesMigration {
  public function __construct() {
    parent::__construct(array('type' => 'story', 'new type' => 'news'));
  }
}

class SwagNodesImageMigration extends SwagNodesMigration {
  public function __construct() {
    parent::__construct(array('type' => 'image'));
  }

  public function set_source($query) {
    $source_fields = array(
      'images' => t('The image associated with this node.'),
      'terms' => t('Gallery terms.'),
    );
    $this->source = new MigrateSourceSQL($query, $source_fields);
    $this->source->setMapJoinable(FALSE);
  }

  public function node_field_mapping() {
    parent::node_field_mapping();
    $associated_file_arguments = MigrateFileFieldHandler::arguments(NULL, 'file_copy', FILE_EXISTS_REPLACE);
    $this->addFieldMapping('field_image', 'images')->arguments($associated_file_arguments);
    $this->addFieldMapping('field_tag_galleries', 'terms')->separator(',');
  }

  public function prepareRow($current_row) {
    parent::prepareRow($current_row);
    $node = node_load($current_row->nid);

    $images = array();
    if (!$node) {
      $query = Database::getConnection('swagonline')
        ->select('files', 'f')
        ->fields('f', array('fid', 'filepath'))
        ->condition('f.filename', '_original')
        ->condition('f.nid', $current_row->nid);
      $results = $query->execute();
      foreach ($results as $result) {
        $current_image_data = array(
          'path' => 'http://www.swagonline.net/image/view/'. $current_row->nid,
        );
        $images[] = drupal_json_encode($current_image_data);
      }
    }
    $current_row->images = $images;
    $current_row->terms = migrate_swag_retrieve_tags($current_row->nid);
    return TRUE;
  }
}

class SwagNodesBlogMigration extends SwagNodesMigration {
  public function __construct() {
    parent::__construct(array('type' => 'blog'));
  }

  public function set_source($query) {
    $source_fields = array(
      'images' => t('The images associated with this node.'),
      'terms' => t('Blog terms.'),
    );
    $this->source = new MigrateSourceSQL($query, $source_fields);
    $this->source->setMapJoinable(FALSE);
  }

  public function node_field_mapping() {
    parent::node_field_mapping();
    $associated_file_arguments = MigrateFileFieldHandler::arguments(NULL, 'file_copy', FILE_EXISTS_REPLACE);
    $this->addFieldMapping('field_blog_images', 'images')->arguments($associated_file_arguments);
    $this->addFieldMapping('field_blog_content', 'terms')->separator(',');
  }

  public function prepareRow($current_row) {
    parent::prepareRow($current_row);
    $node = node_load($current_row->nid);

    $images = array();
    if (!$node) {
      $query = Database::getConnection('swagonline')
        ->select('files', 'f')
        ->fields('f', array('fid', 'filepath'));
      $query->join('content_field_wip_images', 'cfwi', 'cfwi.field_wip_images_fid = f.fid');
      $query->fields('cfwi', array('field_wip_images_title', 'field_wip_images_alt'))
        ->condition('cfwi.vid', $current_row->vid);
      $results = $query->execute();
      foreach ($results as $result) {
        $current_image_data = array(
          'path' => 'http://www.swagonline.net/' . $result->filepath,
          'alt' => $result->field_wip_images_alt,
          'title' => $result->field_wip_images_title,
        );
        $images[] = drupal_json_encode($current_image_data);
      }
    }
    $current_row->images = $images;
    $current_row->terms = migrate_swag_retrieve_tags($current_row->nid);
    return TRUE;
  }
}

class SwagNodesForumPostsMigration extends SwagNodesMigration {
  public function __construct() {
    parent::__construct(array('type' => 'forum'));
  }

  public function set_source($query) {
    $source_fields = array(
      'terms' => t('Forum.'),
    );
    $this->source = new MigrateSourceSQL($query, $source_fields);
    $this->source->setMapJoinable(FALSE);
  }

  public function node_field_mapping() {
    parent::node_field_mapping();
    $this->addFieldMapping('taxonomy_forums', 'terms')->separator(',');
  }

  public function prepareRow($current_row) {
    $current_row->terms = migrate_swag_retrieve_tags($current_row->nid);
  }
}
