<?php

/**
 * @file
 * Commerce Adyen OpenInvoice API.
 */

/**
 * Change default values of the address.
 *
 * @param \Commerce\Adyen\Payment\Composition\Address $address
 *   Address for OpenInvoice Adyen payment.
 * @param \EntityDrupalWrapper $order
 *   Metadata wrapper for "commerce_order" entity.
 * @param \EntityDrupalWrapper $profile
 *   Commerce customer profile.
 *
 * @see \Commerce\Adyen\Payment\OpenInvoice\PaymentController::addAddress()
 */
function hook_commerce_adyen_shopper_address_alter(\Commerce\Adyen\Payment\Composition\Address $address, \EntityDrupalWrapper $order, \EntityDrupalWrapper $profile) {
  if ('Dnipropetrovsk' === $address->getCity()) {
    $address->setCity('Dnipro');
  }
}

/**
 * Change default values of shopper information.
 *
 * @param \Commerce\Adyen\Payment\Composition\Shopper $shopper
 *   Shopper information for OpenInvoice Adyen payment.
 * @param \EntityDrupalWrapper $order
 *   Metadata wrapper for "commerce_order" entity.
 * @param \EntityDrupalWrapper $billing
 *   Commerce customer profile.
 *
 * @see \Commerce\Adyen\Payment\OpenInvoice\PaymentController::addShopperInformation()
 */
function hook_commerce_adyen_shopper_information_alter(\Commerce\Adyen\Payment\Composition\Shopper $shopper, \EntityDrupalWrapper $order, \EntityDrupalWrapper $billing) {
  $shopper->setFirstName('Sergii');
}
