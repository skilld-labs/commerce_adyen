<?php
/**
 * @file
 * Base payment controller.
 */

namespace Commerce\Adyen\Payment\Controller;

use Commerce\Adyen\Payment\Authorisation\Request;

/**
 * Class Payment.
 *
 * @package Commerce\Adyen\Payment\Controller
 */
abstract class Payment extends Controller {

  /**
   * An array with data of particular object.
   *
   * @var array
   */
  private $data = [];
  /**
   * An instance of payment request.
   *
   * @var Request
   */
  protected $payment;

  /**
   * Build the data.
   */
  abstract protected function build();

  /**
   * Set an instance of payment request.
   *
   * @param Request $payment
   *   An instance of payment request.
   */
  public function setPayment(Request $payment) {
    $this->payment = $payment;
  }

  /**
   * Data can be set only by child class.
   *
   * @param string $key
   *   The name of property.
   * @param string|int $value
   *   Value for a key.
   */
  protected function set($key, $value) {
    $this->data[$key] = $value;
  }

  /**
   * Data can be obtained everywhere.
   *
   * @return array
   *   Data that were set.
   */
  public function getData() {
    if (empty($this->data)) {
      $this->build();
    }

    return $this->data;
  }

  /**
   * Create configuration for the payment type.
   *
   * @return array[]
   *   Form items.
   */
  public function configForm() {
    return [];
  }

}
