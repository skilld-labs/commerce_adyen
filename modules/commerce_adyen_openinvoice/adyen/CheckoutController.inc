<?php

namespace Commerce\Adyen\Payment\OpenInvoice;

use Commerce\Adyen\Payment\Controller\Checkout;

/**
 * OpenInvoice checkout controller.
 */
class CheckoutController extends Checkout {

  /**
   * {@inheritdoc}
   */
  public function checkoutForm() {
    $form = [];

    $form['gender'] = [
      '#type' => 'radios',
      '#title' => t('Gender'),
      '#options' => [
        'MALE' => t('Male'),
        'FEMALE' => t('Female'),
      ],
    ];

    $form['phone_number'] = [
      '#type' => 'textfield',
      '#title' => t('Phone number'),
    ];

    $form['birth_date'] = [
      '#type' => 'date_select',
      '#title' => t('Date of birth'),
      '#date_format' => 'd/m/Y',
      // By OpenInvoice specification you must be older than 18 year.
      '#date_year_range' => '-100:-18',
    ];

    $form['social_number'] = [
      '#type' => 'textfield',
      '#title' => t('Social security number'),
    ];

    return $form;
  }

}
