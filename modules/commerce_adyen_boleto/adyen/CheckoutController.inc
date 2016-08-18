<?php
/**
 * @file
 * OpenInvoice checkout controller.
 */

namespace Commerce\Adyen\Payment\Boleto;

use Commerce\Adyen\Payment\Controller\Checkout;
use germanoricardi\helpers\BrazilianHelper;

/**
 * Class CheckoutController.
 *
 * @package Commerce\Adyen\Payment\Boleto
 */
class CheckoutController extends Checkout {

  /**
   * {@inheritdoc}
   */
  public function checkoutFields() {
    $fields = [];

    $fields['tax_number'] = [
      'label' => t('VAT Number'),
      'type' => 'text',
      'cardinality' => 1,
      'translatable' => FALSE,
    ];

    return $fields;
  }

  function checkoutForm(\EntityDrupalWrapper $billing) {
    $form = [];

    $form['tax_number'] = [
      '#type' => 'textfield',
      '#title' => t('VAT Number'),
      '#default_value' => isset($billing->commerce_adyen_tax_number) ? $billing->commerce_adyen_tax_number->value() : '',
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function checkoutFormValidate(array $form, array &$values, \EntityDrupalWrapper $billing) {
    if (!empty($values['social_security_number'])) {
      $length = strlen(preg_replace('/[^0-9]/', '', $values['social_security_number']));
      $helper = new BrazilianHelper();

      if (!in_array($length, [11, 14])) {
        form_error($form['social_security_number'], t('Please fill valid CPF/CNPJ (11 or 14).'));

        return FALSE;
      }

      $type = 11 === $length ? 'Cpf' : 'Cnpj';

      if (NULL === $helper->{"as$type"}($values['social_security_number'])) {
        form_error($form['social_security_number'], t('@type number you have entered is invalid.', ['@type' => strtoupper($type)]));

        return FALSE;
      }
    }

    return parent::checkoutFormValidate($form, $values, $billing);
  }
}
