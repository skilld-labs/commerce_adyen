<?php
/**
 * @file
 * Recurring payment types.
 */

namespace Commerce\Adyen\Payment;

/**
 * Class RecurringContract.
 *
 * @package Commerce\Adyen\Payment
 */
abstract class RecurringContract {

  // @see https://docs.adyen.com/developers/recurring-manual#creatingarecurringcontract

  /**
   * The shopper opts to store their card details for future use. The
   * shopper is present for the subsequent transaction, for cards the
   * security code (CVC/CVV) is required.
   */
  const ONECLICK = 'ONECLICK';
  /**
   * Payment details are stored for future use. For cards, the security
   * code (CVC/CVV) is not required for subsequent payments.
   */
  const RECURRING = 'RECURRING';
  /**
   * Payment details are stored for future use. This allows the use of
   * the stored payment details regardless of whether the shopper is on
   * your site or not.
   */
  const ONECLICK_RECURRING = 'ONECLICK,RECURRING';

}
