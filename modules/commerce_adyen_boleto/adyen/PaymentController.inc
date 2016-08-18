<?php
/**
 * @file
 * Boleto payment type.
 */

namespace Commerce\Adyen\Payment\Boleto;

use Commerce\Adyen\Payment\OpenInvoice;
use Commerce\Adyen\Payment\Composition\Address;

/**
 * Class PaymentController.
 *
 * @package Commerce\Adyen\Payment\Boleto
 */
class PaymentController extends OpenInvoice\PaymentController {

  /**
   * {@inheritdoc}
   */
  public function configForm() {
    return [];
  }

  /**
   * {@inheritdoc}
   */
  protected function build() {
    $order = $this->getOrder();
    $billing = $order->commerce_customer_billing;

    $this->set('shopper.lastName', $billing->commerce_customer_address->last_name->value());
    $this->set('shopper.firstName', $billing->commerce_customer_address->first_name->value());

    if (isset($billing->commerce_adyen_tax_number)) {
      $this->set('shopper.socialSecurityNumber', $billing->commerce_adyen_tax_number->value());
    }

    $this->addAddress(new Address(Address::BILLING), $order->commerce_customer_billing);
  }

}
