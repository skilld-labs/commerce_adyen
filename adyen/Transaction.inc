<?php
/**
 * @file
 * Commerce Adyen Payment Transaction.
 */

namespace Commerce\Adyen\Payment;

/**
 * Class Transaction.
 *
 * @package Commerce\Adyen\Payment
 */
class Transaction {

  /**
   * Is transaction has been created?
   *
   * @var bool
   */
  private $isNew = TRUE;
  /**
   * Commerce payment transaction.
   *
   * @var \stdClass
   */
  private $transaction;
  /**
   * Instance of a payment method, used for transaction.
   *
   * @var array
   */
  private $paymentMethod = [];

  /**
   * Transaction constructor.
   *
   * @param \stdClass|int|string $order
   *   Commerce order object or order ID.
   * @param string $remote_status
   *   Will be used as condition for loading existing transaction. Will
   *   not be used as a value of a new transaction.
   */
  public function __construct($order, $remote_status = '') {
    try {
      $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
      $conditions = [
        'order_id' => $order_wrapper->order_id->value(),
        'instance_id' => COMMERCE_ADYEN_PAYMENT_METHOD_INSTANCE,
        'payment_method' => COMMERCE_ADYEN_PAYMENT_METHOD,
      ];

      if (!empty($remote_status)) {
        $conditions['remote_status'] = $remote_status;
      }

      $transactions = commerce_payment_transaction_load_multiple([], $conditions);

      $this->isNew = empty($transactions);
      $this->transaction = $this->isNew ? commerce_payment_transaction_new() : reset($transactions);
      // Do not allow to change values below.
      $this->transaction->uid = $order_wrapper->uid->value();
      $this->transaction->order_id = $conditions['order_id'];
      $this->transaction->amount = (int) $order_wrapper->commerce_order_total->amount->value();
      $this->transaction->currency_code = $order_wrapper->commerce_order_total->currency_code->value();
    }
    catch (\Exception $e) {
      // Fallback. An order cannot be loaded. Assume that transaction is new.
      // In this case it cannot be saved since "order_id" property is required.
      $this->transaction = new \stdClass();

      watchdog(COMMERCE_ADYEN_PAYMENT_METHOD, 'Transaction cannot exist without the order! Input argument: <pre>@argument</pre>.', [
        '@argument' => print_r($order, TRUE),
      ], WATCHDOG_ERROR);
    }

    // Properties below are provided to make this object working
    // even when order cannot be loaded.
    if ($this->isNew) {
      $this->transaction->status = '';
      $this->transaction->remote_id = '';
      $this->transaction->remote_status = '';
      $this->transaction->message = '';
      $this->transaction->message_variables = [];
      $this->transaction->payload = [];
    }

    $this->transaction->instance_id = COMMERCE_ADYEN_PAYMENT_METHOD_INSTANCE;
    $this->transaction->payment_method = COMMERCE_ADYEN_PAYMENT_METHOD;
  }

  /**
   * Finalize a transaction. Must be used only after successful capturing.
   */
  public function finalize() {
    $this->setStatus(COMMERCE_PAYMENT_STATUS_SUCCESS);
    $this->setRemoteStatus(COMMERCE_ADYEN_PAYMENT_REMOTE_STATUS_CAPTURED);
    $this->setMessage('Payment has been captured and completed.');
  }

  /**
   * Returns instance of a payment method of transaction.
   *
   * @return array
   *   Payment method instance.
   */
  public function getPaymentMethod() {
    if (empty($this->paymentMethod)) {
      $this->paymentMethod = commerce_payment_method_instance_load($this->transaction->instance_id);
    }

    return $this->paymentMethod;
  }

  /**
   * Check if payment was only authorized and waiting for capturing.
   *
   * @return bool
   *   A state of check.
   */
  public function isAuthorized() {
    return
      COMMERCE_ADYEN_PAYMENT_STATUS_AUTHORISED === $this->transaction->status &&
      COMMERCE_ADYEN_PAYMENT_REMOTE_STATUS_AUTHORISED === $this->transaction->remote_status;
  }

  /**
   * Status: new or existing transaction.
   *
   * @return bool
   *   A state of checking.
   */
  public function isNew() {
    return $this->isNew;
  }

  /**
   * Returns amount of a transaction.
   *
   * @return int
   *   Transaction amount.
   */
  public function getAmount() {
    return $this->transaction->amount;
  }

  /**
   * Returns currency code of a transaction.
   *
   * @return string
   *   Currency code.
   */
  public function getCurrency() {
    return $this->transaction->currency_code;
  }

  /**
   * Set status of payment transaction.
   *
   * @param string $status
   *   Transaction status.
   */
  public function setStatus($status) {
    $statuses = commerce_payment_transaction_statuses();

    if (isset($statuses[$status])) {
      $this->transaction->status = $status;
    }
    else {
      watchdog(COMMERCE_ADYEN_PAYMENT_METHOD, 'The "@status" is invalid. It must be one of: @statuses.', [
        '@status' => $status,
        '@statuses' => implode(', ', array_keys($statuses)),
      ], WATCHDOG_ERROR);
    }
  }

  /**
   * Get status of payment transaction.
   *
   * @return string
   *   Transaction status.
   */
  public function getStatus() {
    return $this->transaction->status;
  }

  /**
   * Set remote transaction ID.
   *
   * @param string $remote_id
   *   Remote transaction ID.
   */
  public function setRemoteId($remote_id) {
    $this->transaction->remote_id = $remote_id;
  }

  /**
   * Get remote transaction ID.
   *
   * @return string
   *   Remote transaction ID.
   */
  public function getRemoteId() {
    return $this->transaction->remote_id;
  }

  /**
   * Set remote status of a transaction.
   *
   * @param string $remote_status
   *   Remote status of a transaction.
   */
  public function setRemoteStatus($remote_status) {
    $this->transaction->remote_status = $remote_status;
  }

  /**
   * Get remote status of a transaction.
   *
   * @return string
   *   Remote status of a transaction.
   */
  public function getRemoteStatus() {
    return $this->transaction->remote_status;
  }

  /**
   * Set message for a transaction.
   *
   * @param string $message
   *   Transaction message with placeholders for replacing. Do not
   *   process the string with "t()".
   * @param array $variables
   *   Values for placeholders in message.
   *
   * @see format_string()
   */
  public function setMessage($message, array $variables = []) {
    $this->transaction->message = $message;
    $this->transaction->message_variables = $variables;
  }

  /**
   * Get message for a transaction.
   *
   * @return string
   *   Transaction message.
   */
  public function getMessage() {
    return format_string($this->transaction->message, $this->transaction->message_variables);
  }

  /**
   * Set transaction payload.
   *
   * @param mixed $payload
   *   Transaction payload.
   */
  public function setPayload($payload) {
    $this->transaction->payload[REQUEST_TIME] = $payload;
  }

  /**
   * Save transaction in a database.
   */
  public function save() {
    foreach (['order_id', 'status', 'remote_id', 'remote_status'] as $property) {
      if (empty($this->transaction->{$property})) {
        watchdog(COMMERCE_ADYEN_PAYMENT_METHOD, 'Transaction cannot be saved. The "@property" property is not set.', [
          '@property' => $property,
        ], WATCHDOG_ERROR);

        return;
      }
    }

    commerce_payment_transaction_save($this->transaction);
  }

}
