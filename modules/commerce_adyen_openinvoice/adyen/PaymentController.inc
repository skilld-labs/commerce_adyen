<?php

namespace Commerce\Adyen\Payment\OpenInvoice;

use Commerce\Adyen\Payment\Controller\Payment;
use Commerce\Adyen\Payment\Composition\Address;
use Commerce\Adyen\Payment\Composition\Shopper;

/**
 * OpenInvoice payment controller.
 */
class PaymentController extends Payment {

  // OpenInvoice payment types supported by Adyen.
  const KLARNA = 'klarna';
  const AFTERPAY = 'afterpay';

  /**
   * {@inheritdoc}
   */
  public static function subTypes() {
    return [
      static::KLARNA => 'Klarna',
      static::AFTERPAY => 'AfterPay',
    ];
  }

  /**
   * {@inheritdoc}
   */
  protected function build() {
    $order = $this->payment->getOrder();
    $shopper = new Shopper();
    // Auto agreement with terms and conditions. Will be used for DE and AT.
    // Note, that this will work only on "SKIP_DETAILS" endpoint.
    // @see https://github.com/Adyen/adyen-magento2/blob/46c6758b3fc472a52a0bf5a7dd2fbf184bfcd970/Block/Redirect/Redirect.php#L276
    $this->set(static::KLARNA . '.acceptPrivacyPolicy', 'true');
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
    $item = 0;

    /* @var \EntityMetadataWrapper $label */
    foreach ($line_items as $line_item) {
      $line_item_total = $line_item->commerce_total->value();
      $line_item_type = $line_item->type->value();
      $currency_code = $line_item->commerce_unit_price->currency_code->value();
      $vat_amount = commerce_adyen_amount(commerce_vat_total_amount($line_item_total['data']['components'], TRUE, $currency_code), $currency_code);
      $label = isset($line_item->commerce_product) ? $line_item->commerce_product->title : $line_item->line_item_label;
      $item++;
      $percentage = ($line_item_total['amount'] / ($line_item_total['amount'] - $vat_amount) - 1) * 100;

      foreach ([
        // Referring to Adyen documentation we must send item price having VAT
        // amount excluded.
        // @link https://docs.adyen.com/developers/api-reference/payments-api/paymentrequest/paymentrequest-additionaldata/open-invoice-fields#openinvoicedataline
        'itemAmount' => commerce_adyen_amount($line_item->commerce_unit_price->amount->value(), $currency_code) - $vat_amount,
        'description' => "{$label->value()} [$line_item_type]",
        'currencyCode' => $currency_code,
        'numberOfItems' => (int) $line_item->quantity->value(),
        // Can be one these values: "High", "Low", "None".
        // @see https://github.com/Adyen/adyen-magento2/blob/46c6758b3fc472a52a0bf5a7dd2fbf184bfcd970/Block/Redirect/Redirect.php#L459
        'vatCategory' => 'None',
        // Value must be represented in minor units. E.g. "3 Euro" is "300".
        'itemVatAmount' => $vat_amount,
        // Value must be represented in minor units. E.g. "7%" is "700".
        'itemVatPercentage' => commerce_round(COMMERCE_ROUND_HALF_UP, $percentage) * 100,
      ] as $field => $value) {
        $this->set("openinvoicedata.line$item.$field", $value);
      }
    }

    $this->set('openinvoicedata.numberOfLines', $item);
  }

}
