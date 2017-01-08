<?php

namespace Commerce\Adyen\Payment\Controller;

use Commerce\Adyen\Payment\Authorisation\Request;
use Commerce\Adyen\Payment\Composition\Address;
use Commerce\Adyen\Payment\Composition\Shopper;

/**
 * Base payment controller.
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
   * Values form the form of checkout controller.
   *
   * @var array
   *
   * @see Checkout::checkoutForm()
   */
  protected $checkoutValues = [];

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
   * Set values form the form of checkout controller.
   *
   * @param array $checkout_values
   *   Values form the form of checkout controller.
   */
  public function setCheckoutValues(array $checkout_values) {
    $this->checkoutValues = $checkout_values;
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
   * List of payment subtypes.
   *
   * @return string[]
   *   An associative array where key - is a machine name
   *   of subtype and value - human-readable label.
   */
  public static function subTypes() {
    return [];
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
    $state = $this->payment->getPaymentMethod()['settings']['state'];
    $order = $this->payment->getOrder();

    if (!empty($state)) {
      $this->set('shopperType', $state);
    }

    if (isset($address['first_name'])) {
      $shopper->setFirstName($address['first_name']);
    }

    if (isset($address['last_name'])) {
      $shopper->setLastName($address['last_name']);
    }

    if (!empty($this->checkoutValues['gender'])) {
      $shopper->setGender($this->checkoutValues['gender']);
    }

    if (!empty($this->checkoutValues['phone_number'])) {
      $shopper->setTelephoneNumber($this->checkoutValues['phone_number']);
    }

    if (!empty($this->checkoutValues['social_number'])) {
      $shopper->setTelephoneNumber($this->checkoutValues['social_number']);
    }

    if (!empty($this->checkoutValues['birth_date'])) {
      $birth_date = strtotime($this->checkoutValues['birth_date']);

      $shopper->setDateOfBirthYear(date('Y', $birth_date));
      $shopper->setDateOfBirthMonth(date('m', $birth_date));
      $shopper->setDateOfBirthDayOfMonth(date('d', $birth_date));
    }

    // Allow other modules alter the address.
    drupal_alter('commerce_adyen_shopper_information', $shopper, $billing, $this->checkoutValues, $order);

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
   *   Shopper address.
   * @param \EntityDrupalWrapper $profile
   *   Commerce customer profile.
   */
  protected function addAddress(Address $address, \EntityDrupalWrapper $profile) {
    $state = $this->payment->getPaymentMethod()['settings']['state'];
    $order = $this->payment->getOrder();
    $type = $address->getType() . 'Address';

    if (!empty($state)) {
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
      $address->setHouseNumberOrName($profile_address['premise']);
    }

    // Allow other modules alter the address.
    drupal_alter('commerce_adyen_shopper_address', $address, $profile, $this->checkoutValues, $order);

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
   *   Shopper address.
   */
  protected function validateAddress(Address $address) {
    // @todo Add validation.
  }

}
