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
   * Instance of a payment method, used for transaction.
   *
   * @var array
   */
  private $paymentMethod = [];

  /**
   * Transaction constructor.
   *
   * @param \stdClass $order
   *   Commerce order.
   * @param string $remote_status
   *   Will be used as condition for loading existing transaction. Will
   *   not be used as a value of a new transaction.
   */
  public function __construct(\stdClass $order, $remote_status = '') {
    $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
    $conditions = [
      'order_id' => $order->order_id,
      'instance_id' => COMMERCE_ADYEN_PAYMENT_METHOD_INSTANCE,
      'payment_method' => COMMERCE_ADYEN_PAYMENT_METHOD,
    ];

    if (!empty($remote_status)) {
      $conditions['remote_status'] = $remote_status;
    }

    $transactions = commerce_payment_transaction_load_multiple([], $conditions);

    $this->isNew = empty($transactions);
    $this->transaction = $this->isNew ? commerce_payment_transaction_new() : reset($transactions);

    $this->transaction->uid = $order->uid;
    $this->transaction->order_id = $order->order_id;
    $this->transaction->instance_id = COMMERCE_ADYEN_PAYMENT_METHOD_INSTANCE;
    $this->transaction->payment_method = COMMERCE_ADYEN_PAYMENT_METHOD;
    $this->transaction->amount = (int) $order_wrapper->commerce_order_total->amount->value();
    $this->transaction->currency_code = $order_wrapper->commerce_order_total->currency_code->value();
    $this->transaction->status = COMMERCE_PAYMENT_STATUS_SUCCESS;
    $this->transaction->remote_id = '';
    $this->transaction->remote_status = '';
    $this->transaction->message = '';
    $this->transaction->message_variables = [];
    $this->transaction->payload = [];
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
    if (NULL === $this->paymentMethod) {
      $this->paymentMethod = commerce_payment_method_instance_load($this->transaction->instance_id);
    }

    return $this->paymentMethod;
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

    if (!isset($statuses[$status])) {
      throw new \InvalidArgumentException(t('The "@status" is invalid. It must be one of: @statuses.', [
        '@status' => $status,
        '@statuses' => implode(', ', array_keys($statuses)),
      ]));
    }

    $this->transaction->status = $status;
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
    foreach (['status', 'remote_id', 'remote_status'] as $property) {
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
