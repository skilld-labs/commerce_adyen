<?php
/**
 * @file
 * Shopper interaction types.
 */

namespace Commerce\Adyen\Payment;

/**
 * Class ShopperInteraction.
 *
 * @package Commerce\Adyen\Payment
 */
abstract class ShopperInteraction {

  // @see https://docs.adyen.com/developers/api-manual#paymentrequests

  /**
   * Point-of-sale transactions where the shopper is physically present
   * to make a payment using a secure payment terminal.
   */
  const POS = 'POS';
  /**
   * Mail-order and telephone-order transactions where the shopper
   * is in contact with the merchant via email or telephone.
   */
  const MOTO = 'Moto';
  /**
   * Card on file and/or subscription transactions, where the card
   * holder is known to the merchant (returning customer). If the
   * shopper is present  (online), you can supply also the CSC to
   * improve authorisation (one-click payment).
   */
  const CONTAUTH = 'ContAuth';
  /**
   * Online transactions where the card holder is present (online). For
   * better authorisation rates we recommend sending the card security
   * code (CSC) along with the request.
   */
  const ECOMMERCE = 'Ecommerce';

}
