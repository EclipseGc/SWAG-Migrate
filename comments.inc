<?php

class SwagCommentsMigration extends Migration {
  public function __construct(array $arguments) {
    //$bundles = entity_get_info('comment');
    //drupal_set_message('<pre>' . var_export($bundles['bundles'], TRUE) . '</pre>');
    parent::__construct();
    $type = isset($arguments['node type']) ? $arguments['node type'] : NULL;
    $comment_type = isset($arguments['comment type']) ? $arguments['comment type'] : NULL;
    $this->description = t('Migrate swagonline.net comments');
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'cid' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'D5 Unique comment ID',
          'alias' => 'c',
        )
      ),
      MigrateDestinationComment::getKeySchema()
    );

    $query = Database::getConnection('swagonline')
      ->select('comments', 'c')
      ->fields('c', array('cid', 'pid', 'nid', 'uid', 'subject', 'comment', 'hostname', 'timestamp', 'score', 'status', 'format', 'thread', 'users', 'name', 'mail', 'homepage'));
    $query->join('node', 'n', 'c.nid = n.nid');
    $query->condition('n.type', $type);
    $query->orderBy('c.cid');

    $this->source = new MigrateSourceSQL($query);
    $this->source->setMapJoinable(FALSE);
    $this->destination = new MigrateDestinationComment($comment_type);
    
    $this->addSimpleMappings(array('cid', 'pid', 'nid', 'uid', 'subject', 'hostname', 'thread', 'name', 'mail', 'homepage'));
    $this->addFieldMapping('status')->defaultValue(TRUE);
    $this->addFieldMapping('language')->defaultValue('und');
    $body_arguments = MigrateTextFieldHandler::arguments(NULL, array('source_field' => 'format'), NULL);
    $this->addFieldMapping('comment_body', 'comment')->arguments($body_arguments);
    $this->addFieldMapping('created', 'timestamp');
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

class SwagBlogCommentsMigration extends SwagCommentsMigration {
  public function __construct() {
    parent::__construct(array('node type' => 'blog', 'comment type' => 'comment_node_blog'));
  }
}

class SwagForumCommentsMigration extends SwagCommentsMigration {
  public function __construct() {
    parent::__construct(array('node type' => 'forum', 'comment type' => 'comment_node_forum'));
  }
}

class SwagImageCommentsMigration extends SwagCommentsMigration {
  public function __construct() {
    parent::__construct(array('node type' => 'image', 'comment type' => 'comment_node_image'));
  }
}

class SwagNewsCommentsMigration extends SwagCommentsMigration {
  public function __construct() {
    parent::__construct(array('node type' => 'story', 'comment type' => 'comment_node_news'));
  }
}
