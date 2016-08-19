<?php
/**
 * @file
 * Boleto payment controller.
 */

namespace Commerce\Adyen\Payment\Boleto;

use Commerce\Adyen\Payment\Controller\Payment;
use Commerce\Adyen\Payment\Composition\Address;
use Commerce\Adyen\Payment\Composition\Shopper;

/**
 * Class PaymentController.
 *
 * @package Commerce\Adyen\Payment\Boleto
 */
class PaymentController extends Payment {

  /**
   * {@inheritdoc}
   */
  protected function build() {
    $order = $this->getOrder();

    $this->addShopperInformation(new Shopper(), $order->commerce_customer_billing);
    $this->addAddress(new Address(Address::BILLING), $order->commerce_customer_billing);
  }

}
