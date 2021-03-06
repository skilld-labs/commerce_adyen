<?php

namespace Commerce\Adyen\Payment\Authorisation;

use Adyen\Util\Util;
use Commerce\Adyen\Payment\Facade;

/**
 * Authorisation signature.
 */
abstract class Signature implements \Iterator {

  use Facade;

  /**
   * Payment data.
   *
   * @var array
   */
  protected $data = [];

  /**
   * Signature constructor.
   *
   * @param \stdClass $order
   *   Commerce order.
   * @param array $payment_method
   *   Payment method information.
   */
  public function __construct(\stdClass $order, array $payment_method) {
    $this->setOrder($order);
    $this->setPaymentMethod($payment_method);
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

    return Util::calculateSha256Signature($this->getPaymentMethod()['settings']['hmac'], $data);
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
