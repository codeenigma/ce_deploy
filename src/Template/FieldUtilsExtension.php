<?php

namespace Drupal\ce_deploy\Template;

use Drupal\Core\Render\Element;

/**
 * Class FieldUtilsExtension
 */
class FieldUtilsExtension extends \Twig_Extension {

  /**
   * {@inheritdoc}
   */
  public function getFunctions() {
    return [
      new \Twig_SimpleFunction('field_children', [$this, 'getFieldChildren']),
      new \Twig_SimpleFunction('property', [$this, 'getProperty']),
    ];
  }

  /**
   * This serves to get the actual children of a given field (Render array).
   * Useful as well to get an empty array for an empty field.
   *
   * @return array
   */
  public function getFieldChildren($fieldArray) {
    $childrenKeys = Element::children($fieldArray);
    $result = array_intersect_key($fieldArray, $childrenKeys);
    return $result;
  }

  /**
   * @param $renderArray
   * @param $propertyName
   */
  public function getProperty($renderArray, $propertyName) {
    return $renderArray[$propertyName] ?? null;
  }

}
