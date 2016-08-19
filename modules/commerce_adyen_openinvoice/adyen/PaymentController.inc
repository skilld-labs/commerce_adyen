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
    $shopper = new Shopper();

    $this->payment->setBrandCode($this->config['type']);
    $this->addShopperInformation($shopper, $order->commerce_customer_billing);
    $this->addAddress(new Address(Address::BILLING), $order->commerce_customer_billing);
    $this->addAddress(new Address(Address::DELIVERY), $order->commerce_customer_shipping);
    $this->addLineItems($order->commerce_line_items);

    $this->set('openinvoicedata.refundDescription', t('Refund to @first_name @last_name', [
      '@first_name' => $shopper->getFirstName(),
      '@last_name' => $shopper->getLastName(),
    ]));
  }

  /**
   * Add line items.
   *
   * @param \EntityListWrapper $line_items
   *   Line items list.
   */
  protected function addLineItems(\EntityListWrapper $line_items) {
    $product_line_item_types = commerce_product_line_item_types();
    // @todo Why?
    // @see https://github.com/Adyen/magento/commit/9c27b78f8d08972c126df57c3133b67dce4a645e
    $vat_category = static::AFTERPAY === $this->config['type'] ? 'High' : 'None';
    $item = 0;

    foreach ($line_items as $line_item) {
      $line_item_total = $line_item->commerce_total->value();
      $line_item_type = $line_item->type->value();
      $currency_code = $line_item->commerce_unit_price->currency_code->value();
      $vat_amount = commerce_adyen_amount(commerce_vat_total_amount($line_item_total['data']['components'], TRUE, $currency_code), $currency_code);

      if (in_array($line_item_type, $product_line_item_types)) {
        $name = $line_item->commerce_product->title->value();
      }
      else {
        $name = $line_item->line_item_label->value();
      }

      $item++;
      $this->set("openinvoicedata.line.$item.itemAmount", commerce_adyen_amount($line_item->commerce_unit_price->amount->value(), $currency_code));
      $this->set("openinvoicedata.line.$item.description", "$name [$line_item_type]");
      $this->set("openinvoicedata.line.$item.currencyCode", $currency_code);
      $this->set("openinvoicedata.line.$item.numberOfItems", (int) $line_item->quantity->value());
      // Can be one these values: "High", "Low", "None".
      $this->set("openinvoicedata.line.$item.vatCategory", $vat_category);
      // Value must be represented in minor units. E.g. "3 Euro" is "300".
      $this->set("openinvoicedata.line.$item.itemVatAmount", $vat_amount);
      // Value must be represented in minor units. E.g. "7%" is "700".
      $this->set("openinvoicedata.line.$item.itemVatPercentage", $vat_amount > 0 ? ($line_item_total['amount'] * 100 / $vat_amount) : 0);
    }

    $this->set('openinvoicedata.numberOfLines', $item);
  }

}
