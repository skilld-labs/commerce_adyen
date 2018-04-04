<?php

namespace Drupal\commerce_adyen\PluginForm\Adyen;

use Drupal\commerce_payment\PluginForm\PaymentMethodAddForm as BasePaymentMethodAddForm;
use Drupal\Core\Form\FormStateInterface;

class AdyenPaymentMethodAddForm extends BasePaymentMethodAddForm {

  /**
   * {@inheritdoc}
   */
  protected function buildCreditCardForm(array $element, FormStateInterface $form_state) {
    $element = parent::buildCreditCardForm($element, $form_state);
    // Default to a known valid test credit card number.
    $element['number']['#default_value'] = '4111111111111111';

    return $element;
  }

}
