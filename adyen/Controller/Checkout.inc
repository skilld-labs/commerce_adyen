<?php
/**
 * @file
 * Base checkout controller.
 */

namespace Commerce\Adyen\Payment\Controller;

/**
 * Class Checkout.
 *
 * @package Commerce\Adyen\Payment\Controller
 */
abstract class Checkout extends Controller {

  /**
   * Field definitions to add to "billing" customer profile.
   *
   * The following fields are supported out of the box:
   * - gender
   * - birth_date
   * - phone_number
   * - social_number
   *
   * @return array[]
   *   Field definitions.
   *
   * @see \Commerce\Adyen\Payment\OpenInvoice\CheckoutController::checkoutFields()
   * @see \Commerce\Adyen\Payment\Controller\Payment::addShopperInformation()
   */
  abstract public function checkoutFields();

  /**
   * Build checkout form to allow customers fill additional data.
   *
   * @param \EntityDrupalWrapper $billing
   *   Commerce customer profile.
   *
   * @return array[]
   *   Form items.
   */
  abstract public function checkoutForm(\EntityDrupalWrapper $billing);

  /**
   * Validate and process user input of checkout form.
   *
   * @param array[] $form
   *   Form items.
   * @param array $values
   *   Submitted values.
   * @param \EntityDrupalWrapper $billing
   *   Commerce customer profile.
   *
   * @return bool
   *   A state of validation. In case of FALSE, customer will
   *   not be redirected to payment gateway. Here you're able
   *   to use "form_error()" function to indicate what's going on.
   */
  public function checkoutFormValidate(array $form, array &$values, \EntityDrupalWrapper $billing) {
    return TRUE;
  }

}
