<?php

namespace Commerce\Adyen\Payment\Boleto;

use Commerce\Adyen\Payment\Controller\Checkout;
use germanoricardi\helpers\BrazilianHelper;

/**
 * Boleto checkout controller.
 */
class CheckoutController extends Checkout {

  /**
   * {@inheritdoc}
   */
  public function checkoutForm() {
    $form = [];

    $form['social_number'] = [
      '#type' => 'textfield',
      '#title' => t('Social security number'),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function checkoutFormValidate(array $form, array &$values) {
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

    return TRUE;
  }

}
