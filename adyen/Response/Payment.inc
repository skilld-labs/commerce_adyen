<?php
/**
 * @file
 * Adyen payment response.
 */

namespace Commerce\Adyen\Response;

use Commerce\Adyen\Exception;
use Commerce\Adyen\ResponseInterface;
use Commerce\Adyen\SignatureInterface;

/**
 * Class Payment.
 *
 * @package Commerce\Adyen\Response
 */
class Payment implements ResponseInterface, SignatureInterface {

  /**
   * Input data.
   *
   * @var \stdClass
   */
  private $data;
  /**
   * Commerce order.
   *
   * @var \stdClass
   */
  private $order;

  /**
   * {@inheritdoc}
   */
  public static function type() {
    return 'payment';
  }

  /**
   * {@inheritdoc}
   */
  public function __toString() {
    return '';
  }

  /**
   * {@inheritdoc}
   */
  public static function signatureFields() {
    return [
      'authResult',
      'pspReference',
      'merchantReference',
      'skinCode',
      'paymentMethod',
      'shopperLocale',
      'merchantReturnData',
      'additionalData',
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function __construct(array $data) {
    if (empty($data)) {
      throw new Exception(t('Empty response from Adyen has been received.'));
    }

    $this->data = (object) $data;
    $this->data->authResult = strtolower($this->data->authResult);

    $payment_method = commerce_payment_method_instance_load(COMMERCE_ADYEN_PAYMENT_METHOD_INSTANCE);
    // Create signature from a raw data.
    $payment_signature = commerce_adyen_sha_signature($data, $payment_method['settings']['hmac'], $this);

    if ($payment_signature !== $this->data->merchantSig) {
      throw new Exception(t('Received Adyen response with invalid HMAC signature.'));
    }

    // Merchant reference MUST be the "order_number" property.
    // @see commerce_adyen_redirect_form()
    $this->order = commerce_order_load_by_number($this->data->merchantReference);

    if (empty($this->order->order_id)) {
      throw new Exception(t('Received Adyen response with invalid merchantReference.'));
    }

    switch ($this->data->authResult) {
      case 'authorised':
        $transaction = $this->createTransaction();
        $transaction->status = COMMERCE_PAYMENT_STATUS_PENDING;
        $transaction->message = t('Payment has been successfully authorized.');
        break;

      case 'pending':
        $transaction = $this->createTransaction();
        $transaction->status = COMMERCE_PAYMENT_STATUS_PENDING;
        $transaction->message = t('Payment is pending because final status information is unavailable or shopper needs to take further actions to complete it.');
        break;

      case 'error':
      case 'refused':
      case 'cancelled':
        commerce_payment_redirect_pane_previous_page($this->order);
        break;

      default:
        throw new Exception(t('Received Adyen response with invalid authResult.'));
    }

    if (isset($transaction)) {
      commerce_payment_transaction_save($transaction);
      commerce_payment_redirect_pane_next_page($this->order);
    }

    drupal_goto(commerce_checkout_order_uri($this->order));
  }

  /**
   * Create Commerce payment transaction.
   *
   * @return \stdClass
   *   Payment transaction.
   */
  private function createTransaction() {
    $order_wrapper = entity_metadata_wrapper('commerce_order', $this->order);
    $transactions = commerce_payment_transaction_load_multiple([], [
      'order_id' => $this->order->order_id,
      'instance_id' => COMMERCE_ADYEN_PAYMENT_METHOD_INSTANCE,
      'payment_method' => COMMERCE_ADYEN_PAYMENT_METHOD,
    ]);

    $transaction = empty($transactions) ?
      commerce_payment_transaction_new(COMMERCE_ADYEN_PAYMENT_METHOD, $this->order->order_id) :
      reset($transactions);

    $transaction->uid = $this->order->uid;
    $transaction->amount = $order_wrapper->commerce_order_total->amount->value();
    $transaction->currency_code = $order_wrapper->commerce_order_total->currency_code->value();
    $transaction->remote_id = $this->data->pspReference;
    $transaction->remote_status = $this->data->authResult;
    $transaction->instance_id = COMMERCE_ADYEN_PAYMENT_METHOD_INSTANCE;
    $transaction->payload[REQUEST_TIME] = $this->data;

    return $transaction;
  }

}
