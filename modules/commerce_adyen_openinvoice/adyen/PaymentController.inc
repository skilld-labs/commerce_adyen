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
  protected static function subTypes() {
    return [
      static::KLARNA => 'Klarna',
      static::AFTERPAY => 'AfterPay',
    ];
  }

  /**
   * {@inheritdoc}
   */
  protected function build() {
    $order = $this->getOrder();
    $shopper = new Shopper();

    if (!empty($this->config['type'])) {
      $this->payment->setBrandCode($this->config['type']);
      // Auto agreement with terms and conditions. Will be used for DE and AT.
      // Note, that this will work only on "SKIP_DETAILS" endpoint.
      // @see https://github.com/Adyen/adyen-magento2/blob/46c6758b3fc472a52a0bf5a7dd2fbf184bfcd970/Block/Redirect/Redirect.php#L276
      $this->set($this->config['type'] . '.acceptPrivacyPolicy', 'true');
    }

    // The billing and delivery address must match. Do not offer an alternative,
    // different delivery address for Klarna shoppers from your end, since this
    // would cause a billing/delivery address mismatch, and payments would fail.
    // This is a restriction on Klarna's side.
    // @see https://docs.adyen.com/developers/open-invoice-manual
    $this->addAddress(new Address(Address::BILLING), $order->commerce_customer_billing);
    $this->addAddress(new Address(Address::DELIVERY), $order->commerce_customer_billing);
    $this->addLineItems($order->commerce_line_items);
    $this->addShopperInformation($shopper, $order->commerce_customer_billing);

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
    // @see https://github.com/Adyen/adyen-magento2/blob/46c6758b3fc472a52a0bf5a7dd2fbf184bfcd970/Block/Redirect/Redirect.php#L459
    $vat_category = empty($this->config['type']) || static::AFTERPAY === $this->config['type'] ? 'None' : 'High';
    $item = 0;

    /* @var \EntityMetadataWrapper $label */
    foreach ($line_items as $line_item) {
      $line_item_total = $line_item->commerce_total->value();
      $line_item_type = $line_item->type->value();
      $currency_code = $line_item->commerce_unit_price->currency_code->value();
      $vat_amount = commerce_adyen_amount(commerce_vat_total_amount($line_item_total['data']['components'], TRUE, $currency_code), $currency_code);
      $label = isset($line_item->commerce_product) ? $line_item->commerce_product->title : $line_item->line_item_label;
      $item++;

      foreach ([
        'itemAmount' => commerce_adyen_amount($line_item->commerce_unit_price->amount->value(), $currency_code),
        'description' => "{$label->value()} [$line_item_type]",
        'currencyCode' => $currency_code,
        'numberOfItems' => (int) $line_item->quantity->value(),
        // Can be one these values: "High", "Low", "None".
        'vatCategory' => $vat_category,
        // Value must be represented in minor units. E.g. "3 Euro" is "300".
        'itemVatAmount' => $vat_amount,
        // Value must be represented in minor units. E.g. "7%" is "700".
        'itemVatPercentage' => $vat_amount > 0 ? ($line_item_total['amount'] * 100 / $vat_amount) : 0,
      ] as $field => $value) {
        $this->set("openinvoicedata.line$item.$field", $value);
      }
    }

    $this->set('openinvoicedata.numberOfLines', $item);
  }

}
