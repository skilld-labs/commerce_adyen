<?php
/**
 * @file
 * Payment authorization request.
 */

namespace Commerce\Adyen\Payment\Authorization;

use Commerce\Adyen\Payment;

/**
 * Class Request.
 *
 * @package Commerce\Adyen\Payment\Authorization
 */
class Request extends Signature {

  /**
   * Request constructor.
   *
   * @param \stdClass $order
   *   Commerce order.
   * @param array $payment_method
   *   Payment method information.
   */
  public function __construct(\stdClass $order, array $payment_method) {
    if (empty($payment_method['settings'])) {
      throw new \UnexpectedValueException(t('You are not configured Adyen payment gateway!'));
    }

    parent::__construct($order, $payment_method);

    $order_wrapper = entity_metadata_wrapper('commerce_order', $this->order);
    $order_total = $order_wrapper->commerce_order_total->value();

    // Payment fields.
    // @link https://docs.adyen.com/developers/hpp-manual#hpppaymentfields
    $this->setMerchantAccount($this->paymentMethod['settings']['merchant_account']);
    $this->setMerchantReference($this->order->order_number);
    $this->setSkinCode($this->paymentMethod['settings']['skin_code']);
    $this->setPaymentAmount(commerce_adyen_amount($order_total['amount'], $order_total['currency_code']));
    $this->setCurrencyCode($order_total['currency_code']);
    $this->setSessionValidity(strtotime('+ 2 hour'));
    $this->setShopperIp(ip_address());
    $this->setShopperEmail($this->order->mail);
    // The "Anonymous" value will be here in case when order will
    // have "uid" equal to "0".
    $this->setShopperReference($order_wrapper->owner->name->value());
    $this->setShopperLocale($this->paymentMethod['settings']['shopper_locale']);
    $this->setShopperInteraction(Payment\ShopperInteraction::ECOMMERCE);
    $this->setMerchantReturnData($this->order->data['payment_redirect_key']);
  }

  /**
   * Set amount of a payment.
   *
   * @param int $payment_amount
   *   Payment amount. Specified in minor units.
   */
  public function setPaymentAmount($payment_amount) {
    $this->data['paymentAmount'] = $payment_amount;
  }

  /**
   * Get amount of a payment.
   *
   * @return string
   *   Amount of a payment.
   */
  public function getPaymentAmount() {
    return $this->data['paymentAmount'];
  }

  /**
   * Set currency code.
   *
   * @param string $currency_code
   *   Currency code.
   */
  public function setCurrencyCode($currency_code) {
    $this->data['currencyCode'] = $currency_code;
  }

  /**
   * Get currency code.
   *
   * @return string
   *   Currency code.
   */
  public function getCurrencyCode() {
    return $this->data['currencyCode'];
  }

  /**
   * Set merchant reference.
   *
   * @param string $merchant_reference
   *   Merchant reference.
   *
   * @example
   * $payment->setMerchantReference('DE-LW-2013');
   */
  public function setMerchantReference($merchant_reference) {
    $this->data['merchantReference'] = $merchant_reference;
  }

  /**
   * Get merchant reference.
   *
   * @return string
   *   Merchant reference.
   */
  public function getMerchantReference() {
    return $this->data['merchantReference'];
  }

  /**
   * Set skin code.
   *
   * @param string $skin_code
   *   Skin code that should be used for the payment.
   */
  public function setSkinCode($skin_code) {
    $this->data['skinCode'] = $skin_code;
  }

  /**
   * Get skin code.
   *
   * @return string
   *   Skin code.
   */
  public function getSkinCode() {
    return $this->data['skinCode'];
  }

  /**
   * Set merchant account.
   *
   * @param string $merchant_account
   *   The merchant account you want to process this payment with.
   */
  public function setMerchantAccount($merchant_account) {
    $this->data['merchantAccount'] = $merchant_account;
  }

  /**
   * Get merchant account.
   *
   * @return string
   *   Merchant account.
   */
  public function getMerchantAccount() {
    return $this->data['merchantAccount'];
  }

  /**
   * Set session validity.
   *
   * @param int $session_validity
   *   The final time by which a payment needs to have been made.
   */
  public function setSessionValidity($session_validity) {
    $this->data['sessionValidity'] = date(DATE_ATOM, $session_validity);
  }

  /**
   * Get session validity.
   *
   * @return string
   *   Session validity.
   */
  public function getSessionValidity() {
    return $this->data['sessionValidity'];
  }

  /**
   * Set shopper email.
   *
   * @param string $shopper_email
   *   The email address of a shopper.
   */
  public function setShopperEmail($shopper_email) {
    $this->data['shopperEmail'] = $shopper_email;
  }

  /**
   * Get shopper email.
   *
   * @return string
   *   Shopper email.
   */
  public function getShopperEmail() {
    return $this->data['shopperEmail'];
  }

  /**
   * Set shopper reference.
   *
   * @param string $shopper_reference
   *   Shopper reference.
   *
   * @example
   * $payment->setShopperReference('admin');
   */
  public function setShopperReference($shopper_reference) {
    $this->data['shopperReference'] = $shopper_reference;
  }

  /**
   * Get shopper reference.
   *
   * @return string
   *   Shopper reference.
   */
  public function getShopperReference() {
    return $this->data['shopperReference'];
  }

  /**
   * Set shopper IP address.
   *
   * @param string $shopper_ip
   *   Shopper IP address.
   */
  public function setShopperIp($shopper_ip) {
    $this->data['shopperIP'] = $shopper_ip;
  }

  /**
   * Get shopper IP address.
   *
   * @return string
   *   Shopper IP address.
   */
  public function getShopperIp() {
    return $this->data['shopperIP'];
  }

  /**
   * Set shopper interaction.
   *
   * @param string $shopper_interaction
   *   Shopper interaction.
   *
   * @see \Commerce\Adyen\Payment\ShopperInteraction
   */
  public function setShopperInteraction($shopper_interaction) {
    $this->data['shopperInteraction'] = $shopper_interaction;
  }

  /**
   * Get shopper interaction.
   *
   * @return string
   *   Shopper interaction.
   */
  public function getShopperInteraction() {
    return $this->data['shopperInteraction'];
  }

  /**
   * Set return data.
   *
   * @param string $merchant_return_data
   *   This data will be passed back as-is on the return URL when the shopper
   *   completes (or abandons) the payment and returns to your shop.
   */
  public function setMerchantReturnData($merchant_return_data) {
    $this->data['merchantReturnData'] = $merchant_return_data;
  }

  /**
   * Get return data.
   *
   * @return string
   *   Return data.
   */
  public function getMerchantReturnData() {
    return $this->data['merchantReturnData'];
  }

  /**
   * Set shopper locale.
   *
   * @param string $shopper_locale
   *   A combination of language code and country code to specify
   *   the language used in the session.
   */
  public function setShopperLocale($shopper_locale) {
    $this->data['shopperLocale'] = $shopper_locale;
  }

  /**
   * Get shopper locale.
   *
   * @return string
   *   Shopper locale.
   */
  public function getShopperLocale() {
    return $this->data['shopperLocale'];
  }

  /**
   * Set recurring contract type.
   *
   * @param string $recurring_contract
   *   Recurring contract type.
   *
   * @see \Commerce\Adyen\Payment\RecurringContract
   */
  public function setRecurringContract($recurring_contract) {
    // Shopper interaction must be set to "Ecomerce" for "ONECLICK"
    // contract type and to "ContAuth" for "RECURRING".
    // @see https://github.com/Adyen/php/blob/master/5.Recurring/soap/submit-recurring-payment.php
    if (Payment\RecurringContract::ONECLICK === $recurring_contract) {
      $this->setShopperInteraction(Payment\ShopperInteraction::ECOMMERCE);
    }
    else {
      $this->setShopperInteraction(Payment\ShopperInteraction::CONTAUTH);
    }

    $this->data['recurringContract'] = $recurring_contract;
    // @todo Do we need to add a second argument for this?
    $this->data['selectedRecurringDetailReference'] = 'LATEST';
  }

  /**
   * Get recurring contract type.
   *
   * @return string
   *   Recurring contract type.
   */
  public function getRecurringContract() {
    return $this->data['recurringContract'];
  }

  /**
   * Sign payment request.
   */
  public function signRequest() {
    // The default result landing page shoppers are redirected to when
    // they complete a payment on the HPP.
    $this->data['resURL'] = url('checkout/' . $this->order->order_id . '/payment/return/' . $this->order->data['payment_redirect_key'], ['absolute' => TRUE]);
    $this->data['merchantSig'] = $this->getSignature();
  }

  /**
   * Returns endpoint URL.
   *
   * @link https://docs.adyen.com/developers/hpp-manual#hppendpoints
   *
   * @return string
   *   Endpoint URL.
   */
  public function getEndpoint() {
    return sprintf('https://%s.adyen.com/hpp/pay.shtml', $this->paymentMethod['settings']['mode']);
  }

}
