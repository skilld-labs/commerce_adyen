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

    $form['state'] = [
      '#type' => 'select',
      '#title' => t('State'),
      '#default_value' => 0,
      '#options' => [
        t('Fields are visible and modifiable'),
        t('Fields are visible but unmodifiable'),
        t('Fields are not visible and unmodifiable'),
      ],
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  protected function build() {
    $order = $this->getOrder();

    $this->set('brandCode', $this->config['type']);
    $this->addShopperInformation(new Shopper(), $order->commerce_customer_billing);
    $this->addAddress(new Address(Address::BILLING), $order->commerce_customer_billing);
    $this->addAddress(new Address(Address::DELIVERY), $order->commerce_customer_shipping);
    $this->addLineItems($order->commerce_line_items);
  }

  /**
   * Add line items.
   *
   * @param \EntityListWrapper $line_items
   *   Line items list.
   */
  protected function addLineItems(\EntityListWrapper $line_items) {
    $product_line_item_types = commerce_product_line_item_types();
    $vat_category = 'None';
    $item = 0;

    // @todo Why?
    // @see https://github.com/Adyen/magento/commit/9c27b78f8d08972c126df57c3133b67dce4a645e
    if (static::AFTERPAY === $this->config['type']) {
      $vat_category = 'High';
    }

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

  /**
   * Add shopper information.
   *
   * @param Shopper $shopper
   *   Shopper information.
   * @param \EntityDrupalWrapper $billing
   *   Commerce customer profile.
   */
  protected function addShopperInformation(Shopper $shopper, \EntityDrupalWrapper $billing) {
    $address = $billing->commerce_customer_address->value();

    if (!empty($this->config['state'])) {
      $this->set('shopperType', $this->config['state']);
    }

    // Add shopper gender if field exists.
    if (isset($billing->commerce_adyen_gender)) {
      $shopper->setGender($billing->commerce_adyen_gender->value());
    }

    // Add shopper phone number if field exists.
    if (isset($billing->commerce_adyen_phone_number)) {
      $shopper->setTelephoneNumber($billing->commerce_adyen_phone_number->value());
    }

    // Add shopper birth date if field exists.
    if (isset($billing->commerce_adyen_birth_date)) {
      $birth = strtotime($billing->commerce_adyen_birth_date->value());

      $shopper->setDateOfBirthYear(date('Y', $birth));
      $shopper->setDateOfBirthMonth(date('m', $birth));
      $shopper->setDateOfBirthDayOfMonth(date('d', $birth));
    }

    if (isset($address['first_name'])) {
      $shopper->setFirstName($address['first_name']);
    }

    if (isset($address['last_name'])) {
      $shopper->setLastName($address['last_name']);
    }

    if (isset($address['name_line'])) {
      $this->set('openinvoicedata.refundDescription', t('Refund to @customer', [
        '@customer' => $address['name_line'],
      ]));
    }

    // Allow other modules alter the address.
    drupal_alter('commerce_adyen_shopper_information', $shopper, $this->getOrder(), $billing);

    $this->validateShopperInformation($shopper);
    $this->set('shopper.infix', $shopper->getInfix());
    $this->set('shopper.gender', $shopper->getGender());
    $this->set('shopper.lastName', $shopper->getLastName());
    $this->set('shopper.firstName', $shopper->getFirstName());
    $this->set('shopper.telephoneNumber', $shopper->getTelephoneNumber());
    $this->set('shopper.dateOfBirthYear', $shopper->getDateOfBirthYear());
    $this->set('shopper.dateOfBirthMonth', $shopper->getDateOfBirthMonth());
    $this->set('shopper.dateOfBirthDayOfMonth', $shopper->getDateOfBirthDayOfMonth());
  }

  /**
   * Validate shopper information.
   *
   * @param Shopper $shopper
   *   Shopper information.
   */
  protected function validateShopperInformation(Shopper $shopper) {
    // @todo Add validation.
  }

  /**
   * Add address.
   *
   * @param Address $address
   *   OpenInvoice address.
   * @param \EntityDrupalWrapper $profile
   *   Commerce customer profile.
   */
  protected function addAddress(Address $address, \EntityDrupalWrapper $profile) {
    $type = $address->getType() . 'Address';

    if (!empty($this->config['state'])) {
      $this->set($type . 'Type', $this->config['state']);
    }

    // Prefill data from address from customer profile.
    $profile_address = $profile->commerce_customer_address->value();

    if (isset($profile_address['locality'])) {
      $address->setCity($profile_address['locality']);
    }

    if (isset($profile_address['thoroughfare'])) {
      $address->setStreet($profile_address['thoroughfare']);
    }

    if (isset($profile_address['country'])) {
      $address->setCountry($profile_address['country']);
    }

    if (isset($profile_address['postal_code'])) {
      $address->setPostalCode($profile_address['postal_code']);
    }

    if (isset($profile_address['administrative_area'])) {
      $address->setStateOrProvince($profile_address['administrative_area']);
    }

    if (isset($profile_address['premise'])) {
      $address->setStateOrProvince($profile_address['premise']);
    }

    // Allow other modules alter the address.
    drupal_alter('commerce_adyen_shopper_address', $address, $this->getOrder(), $profile);

    $this->validateAddress($address);
    $this->set("$type.city", $address->getCity());
    $this->set("$type.street", $address->getStreet());
    $this->set("$type.country", $address->getCountry());
    $this->set("$type.postalCode", $address->getPostalCode());
    $this->set("$type.stateOrProvince", $address->getStateOrProvince());
    $this->set("$type.houseNumberOrName", $address->getHouseNumberOrName());
  }

  /**
   * Validate address.
   *
   * @param Address $address
   *   OpenInvoice address.
   */
  protected function validateAddress(Address $address) {
    // @todo Add validation.
  }

}
