<?php
/**
 * @file
 * Base controller.
 */

namespace Commerce\Adyen\Payment\Controller;

/**
 * Class Controller.
 *
 * @package Commerce\Adyen\Payment\Controller
 */
abstract class Controller {

  /**
   * Entity wrapper of "commerce_order" entity.
   *
   * @var \EntityDrupalWrapper
   */
  private $order;
  /**
   * Configuration of payment type. Here be the values
   * from "configFrom" method of payment controller.
   *
   * @var array
   */
  protected $config = [];

  /**
   * OpenInvoice constructor.
   *
   * @param array $config
   *   Payment type configuration.
   */
  public function __construct(array $config) {
    $this->config = $config;
  }

  /**
   * Order can be set everywhere.
   *
   * @param \EntityDrupalWrapper|\stdClass|string|int $order
   *   Entity wrapper of "commerce_order" entity.
   */
  public function setOrder($order) {
    if ($order instanceof \EntityMetadataWrapper) {
      $this->order = $order;
    }
    else {
      $this->order = entity_metadata_wrapper('commerce_order', $order);
    }
  }

  /**
   * Order can be obtained only by child class.
   *
   * @return \EntityDrupalWrapper
   *   Entity wrapper of "commerce_order" entity.
   */
  protected function getOrder() {
    return $this->order;
  }

}
