<?php
/**
 * @file
 * Notification Interface.
 */

namespace Commerce\Adyen\Payment;

/**
 * Interface NotificationInterface.
 *
 * @package Commerce\Adyen\Payment
 */
interface NotificationInterface {

  // @see https://github.com/Adyen/php/blob/master/3.Notifications/httppost/notification-server.php

  const AUTHORISATION = 'authorisation';
  const CANCELLATION = 'cancellation';
  const REFUND = 'refund';
  const CANCEL_OR_REFUND = 'cancel_or_refund';
  const CAPTURE = 'CAPTURE';
  const REFUNDED_REVERSED = 'refunded_reversed';
  const CAPTURE_FAILED = 'capture_failed';
  const REQUEST_FOR_INFORMATION = 'request_for_information';
  const NOTIFICATION_OF_CHARGE_BACK = 'notification_of_chargeback';
  const CHARGE_BACK = 'chargeback';
  const CHARGE_BACK_REVERSED = 'chargeback_reversed';
  const REPORT_AVAILABLE = 'report_available';

}
