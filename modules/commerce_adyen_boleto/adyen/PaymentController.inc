<?php

namespace Commerce\Adyen\Payment\Boleto;

use Commerce\Adyen\Payment\Controller\Payment;
use Commerce\Adyen\Payment\Composition\Address;
use Commerce\Adyen\Payment\Composition\Shopper;

/**
 * Boleto payment controller.
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
  public static function subTypes() {
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
    $order = $this->payment->getOrder();
    // @see https://github.com/Adyen/adyen-magento2/blob/46c6758b3fc472a52a0bf5a7dd2fbf184bfcd970/Helper/Data.php#L651-L671
    // $this->payment->setBrandCode('boletobancario_<TYPE>');
    $this->addShopperInformation(new Shopper(), $order->commerce_customer_billing);
    $this->addAddress(new Address(Address::BILLING), $order->commerce_customer_billing);
  }

}
