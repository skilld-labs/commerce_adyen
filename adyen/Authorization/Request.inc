<?php
/**
 * @file
 * Payment authorization request.
 */

namespace Commerce\Adyen\Payment\Authorization;

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
    $this->setShopperEmail($this->order->mail);
    $this->setShopperReference($order_wrapper->user->name->value());
    $this->setShopperLocale($this->paymentMethod['settings']['shopper_locale']);
    $this->setMerchantReturnData($this->order->data['payment_redirect_key']);
  }

  /**
   * {@inheritdoc}
   */
  protected function signatureFields() {
    return [
      'resURL',
      'skinCode',
      'currencyCode',
      'shopperEmail',
      'shopperLocale',
      'paymentAmount',
      'merchantAccount',
      'sessionValidity',
      'merchantReference',
      'merchantReturnData',
    ];
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
   * Returns amount of a payment.
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
   * Returns currency code.
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
   * Returns merchant reference.
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
   * Returns skin code.
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
   * Returns merchant account.
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
   * Returns session validity.
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
   * Returns shopper email.
   *
   * @return string
   *   Shopper email.
   */
  public function getShopperEmail() {
    return $this->data['shopperEmail'];
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
   * Returns return data.
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
   * Returns shopper locale.
   *
   * @return string
   *   Shopper locale.
   */
  public function getShopperLocale() {
    return $this->data['shopperLocale'];
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
