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
   * Configuration of payment type. Here be the values
   * from "configFrom" method of payment controller.
   *
   * @see \Commerce\Adyen\Payment\Controller\Payment::configForm()
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

}
