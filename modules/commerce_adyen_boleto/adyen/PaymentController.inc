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

  // Boleto payment types supported by Adyen.
  const HSBC = 'hsbc';
  const ITAU = 'itau';
  const BRADESCO = 'bradesco';
  const SANTANDER = 'santander';
  const BANCODOBRASIL = 'bancodobrasil';

  /**
   * {@inheritdoc}
   */
  protected static function subTypes() {
    return [
      static::HSBC => 'HSBC',
      static::ITAU => 'ITAU',
      static::BRADESCO => 'Bradesco',
      static::SANTANDER => 'Santander',
      static::BANCODOBRASIL => 'Banco do Brasil',
    ];
  }

  /**
   * {@inheritdoc}
   */
  protected function build() {
    $order = $this->getOrder();

    if (!empty($this->config['type'])) {
      $this->payment->setBrandCode($this->config['type']);
    }

    $this->addShopperInformation(new Shopper(), $order->commerce_customer_billing);
    $this->addAddress(new Address(Address::BILLING), $order->commerce_customer_billing);
  }

}
