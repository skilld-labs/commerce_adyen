<?php
/**
 * @file
 * Authorisation Signature.
 */

namespace Commerce\Adyen\Payment\Authorization;

use Adyen\Util\Util;

/**
 * Class Signature.
 *
 * @package Commerce\Adyen\Payment\Authorization
 */
abstract class Signature implements \Iterator {

  /**
   * @var array
   */
  protected $data = [];
  /**
   * @var \stdClass
   */
  protected $order;
  /**
   * @var array
   */
  protected $paymentMethod = [];

  /**
   * Signature constructor.
   *
   * @param \stdClass $order
   *   Commerce order.
   * @param array $payment_method
   *   Payment method information.
   */
  public function __construct(\stdClass $order, array $payment_method) {
    $this->order = $order;
    $this->paymentMethod = $payment_method;
  }

  /**
   * Returns calculated SHA256 signature.
   *
   * @return string
   *   Calculated SHA256 signature.
   */
  protected function getSignature() {
    // Re-save the data to don't touch original array.
    $data = $this->data;
    // The "merchantSig" never must not take part in signature calculation.
    unset($data['merchantSig']);

    return Util::calculateSha256Signature($this->paymentMethod['settings']['hmac'], $data);
  }

  /**
   * {@inheritdoc}
   */
  public function current() {
    return current($this->data);
  }

  /**
   * {@inheritdoc}
   */
  public function next() {
    next($this->data);
  }

  /**
   * {@inheritdoc}
   */
  public function key() {
    return key($this->data);
  }

  /**
   * {@inheritdoc}
   */
  public function valid() {
    return $this->key() !== NULL;
  }

  /**
   * {@inheritdoc}
   */
  public function rewind() {
    reset($this->data);
  }

}
