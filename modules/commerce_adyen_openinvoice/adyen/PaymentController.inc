<?php
/**
 * @file
 * OpenInvoice payment controller.
 */

namespace Commerce\Adyen\Payment\OpenInvoice;

use Commerce\Adyen\Payment\Controller\Payment;
use Commerce\Adyen\Payment\Composition\Address;
use Commerce\Adyen\Payment\Composition\Shopper;

/**
 * Class PaymentController.
 *
 * @package Commerce\Adyen\Payment\OpenInvoice
 */
class PaymentController extends Payment {

  // OpenInvoice payment types supported by Adyen.
  const KLARNA = 'klarna';
  const AFTERPAY = 'afterpay';

  /**
   * {@inheritdoc}
   */
  public function configForm() {
    $form = [];

    $form['type'] = [
      '#type' => 'select',
      '#title' => t('Type'),
      '#default_value' => static::KLARNA,
      '#options' => [
        static::KLARNA => 'Klarna',
        static::AFTERPAY => 'AfterPay',
      ],
    ];

    return $form + parent::configForm();
  }

  /**
   * {@inheritdoc}
   */
  protected function build() {
    $order = $this->getOrder();

    $this->payment->setBrandCode($this->config['type']);
    $this->addShopperInformation(new Shopper(), $order->commerce_customer_billing);
    $this->addAddress(new Address(Address::BILLING), $order->commerce_customer_billing);
    $this->addAddress(new Address(Address::DELIVERY), $order->commerce_customer_shipping);
    // @todo Why?
    // @see https://github.com/Adyen/magento/commit/9c27b78f8d08972c126df57c3133b67dce4a645e
    $this->addLineItems($order->commerce_line_items, static::AFTERPAY === $this->config['type'] ? 'High' : 'None');
  }

}
