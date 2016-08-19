<?php
/**
 * @file
 * Boleto checkout controller.
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

    $fields['social_number'] = [
      'type' => 'text',
      'label' => t('Social security number'),
      'cardinality' => 1,
      'translatable' => FALSE,
    ];

    return $fields;
  }

  /**
   * {@inheritdoc}
   */
  public function checkoutForm(\EntityDrupalWrapper $billing) {
    $fields = $this->checkoutFields();
    $form = [];

    $form['social_number'] = [
      '#type' => 'textfield',
    ];

    foreach ($fields as $field => $specification) {
      $field_name = COMMERCE_ADYEN_PAYMENT_METHOD . '_' . $field;

      $form[$field] += [
        '#title' => $fields[$field]['label'],
        '#required' => !empty($fields[$field]['required']),
        '#default_value' => isset($billing->{$field_name}) ? $billing->{$field_name}->value() : NULL,
      ];
    }

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function checkoutFormValidate(array $form, array &$values, \EntityDrupalWrapper $billing) {
    if (!empty($values['social_number'])) {
      $helper = new BrazilianHelper();

      switch (strlen(preg_replace('/[^0-9]/', '', $values['social_number']))) {
        // @see https://en.wikipedia.org/wiki/Cadastro_de_Pessoas_F%C3%ADsicas
        case 11:
          $result = $helper->asCpf($values['social_number']);
          $type = 'CPF';
          break;

        // @see https://en.wikipedia.org/wiki/CNPJ
        case 14:
          $result = $helper->asCnpj($values['social_number']);
          $type = 'CNPJ';
          break;

        default:
          form_error($form['social_number'], t('Please fill valid CPF/CNPJ (11 or 14 characters long).'));

          return FALSE;
      }

      if (NULL === $result) {
        form_error($form['social_number'], t('@type number you have entered is invalid.', [
          '@type' => $type,
        ]));

        return FALSE;
      }
    }

    return parent::checkoutFormValidate($form, $values, $billing);
  }
}
