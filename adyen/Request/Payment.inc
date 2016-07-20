<?php
/**
 * @file
 * Adyen payment request.
 */

namespace Commerce\Adyen\Request;

/**
 * Class Payment.
 *
 * @package Commerce\Adyen\Request
 */
class Payment implements \Iterator {

  /**
   * Data for payment request.
   *
   * @var string[]
   */
  private $data = [];

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
   * Set currency code.
   *
   * @param string $currency_code
   *   Currency code.
   */
  public function setCurrencyCode($currency_code) {
    $this->data['currencyCode'] = $currency_code;
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
   * Set skin code.
   *
   * @param string $skin_code
   *   Skin code that should be used for the payment.
   */
  public function setSkinCode($skin_code) {
    $this->data['skinCode'] = $skin_code;
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
   * Set session validity.
   *
   * @param int $session_validity
   *   The final time by which a payment needs to have been made.
   */
  public function setSessionValidity($session_validity) {
    $this->data['sessionValidity'] = date(DATE_ATOM, $session_validity);
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
   * Set response URL.
   *
   * @param string $res_url
   *   The default result landing page shoppers are redirected
   *   to when they complete a payment on the HPP.
   */
  public function setResUrl($res_url) {
    $this->data['resURL'] = $res_url;
  }

  /**
   * Sign payment request.
   *
   * @param string $hmac
   *   HMAC signature from Adyen backend.
   */
  public function signRequest($hmac) {
    $this->data['merchantSig'] = commerce_adyen_sha_signature($this->data, $hmac);
  }

  /**
   * {@inheritdoc}
   */
  public function current() {
    return current($this->data);
  }

  /**
   * {@inheritdoc}
   */
  public function next() {
    next($this->data);
  }

  /**
   * {@inheritdoc}
   */
  public function key() {
    return key($this->data);
  }

  /**
   * {@inheritdoc}
   */
  public function valid() {
    return $this->key() !== NULL;
  }

  /**
   * {@inheritdoc}
   */
  public function rewind() {
    reset($this->data);
  }

}
