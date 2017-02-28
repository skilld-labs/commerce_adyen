<?php

namespace Commerce\Adyen\Payment;

/**
 * Capture request.
 */
class Capture extends Modification {

  /**
   * {@inheritdoc}
   */
  public function __construct($order) {
    parent::__construct($order, COMMERCE_ADYEN_PAYMENT_REMOTE_STATUS_AUTHORISED, self::CAPTURE);
  }

  /**
   * {@inheritdoc}
   */
  public function isAvailable() {
    return $this->transaction->isAuthorised() && !empty($this->transaction->getRemoteId());
  }

}
