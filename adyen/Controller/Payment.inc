<?php
/**
 * @file
 * Base payment controller.
 */

namespace Commerce\Adyen\Payment\Controller;

use Commerce\Adyen\Payment\Authorisation\Request;
use Commerce\Adyen\Payment\Composition\Address;
use Commerce\Adyen\Payment\Composition\Shopper;

/**
 * Class Payment.
 *
 * @package Commerce\Adyen\Payment\Controller
 */
abstract class Payment extends Controller {

  /**
   * An array with data of particular object.
   *
   * @var array
   */
  private $data = [];
  /**
   * An instance of payment request.
   *
   * @var Request
   */
  protected $payment;

  /**
   * Build the data.
   */
  abstract protected function build();

  /**
   * Set an instance of payment request.
   *
   * @param Request $payment
   *   An instance of payment request.
   */
  public function setPayment(Request $payment) {
    $this->payment = $payment;
  }

  /**
   * Data can be set only by child class.
   *
   * @param string $key
   *   The name of property.
   * @param string|int $value
   *   Value for a key.
   */
  protected function set($key, $value) {
    $this->data[$key] = $value;
  }

  /**
   * Data can be obtained everywhere.
   *
   * @return array
   *   Data that were set.
   */
  public function getData() {
    if (empty($this->data)) {
      $this->build();
    }

    return $this->data;
  }

  /**
   * Create configuration for the payment type.
   *
   * @return array[]
   *   Form items.
   */
  public function configForm() {
    return [];
  }

  /**
   * Add line items.
   *
   * @param \EntityListWrapper $line_items
   *   Line items list.
   * @param string $vat_category
   *   Can be one these values: "High", "Low", "None".
   */
  protected function addLineItems(\EntityListWrapper $line_items, $vat_category) {
    $product_line_item_types = commerce_product_line_item_types();
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
      $birth = $billing->commerce_adyen_birth_date->value();

      $shopper->setDateOfBirthYear(date('Y', $birth));
      $shopper->setDateOfBirthMonth(date('m', $birth));
      $shopper->setDateOfBirthDayOfMonth(date('d', $birth));
    }

    if (isset($billing->commerce_adyen_social_number)) {
      $shopper->setSocialSecurityNumber($billing->commerce_adyen_social_number->value());
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
    $this->set('shopper.socialSecurityNumber', $shopper->getSocialSecurityNumber());
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
   * @param int|string $state
   *   Possible values:
   *   - 0: Fields are visible and modifiable.
   *   - 1: Fields are visible but unmodifiable.
   *   - 2: Fields are not visible and unmodifiable.
   */
  protected function addAddress(Address $address, \EntityDrupalWrapper $profile, $state) {
    $type = $address->getType() . 'Address';

    if (is_numeric($state) && $state > 0) {
      $this->set($type . 'Type', $state);
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
