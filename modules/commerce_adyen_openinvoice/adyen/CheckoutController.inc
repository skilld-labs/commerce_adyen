<?php
/**
 * @file
 * OpenInvoice checkout controller.
 */

namespace Commerce\Adyen\Payment\OpenInvoice;

use Commerce\Adyen\Payment\Controller\Checkout;

/**
 * Class CheckoutController.
 *
 * @package Commerce\Adyen\Payment\OpenInvoice
 */
class CheckoutController extends Checkout {

  /**
   * {@inheritdoc}
   */
  public function checkoutFields() {
    $fields = [];

    $fields['gender'] = [
      'type' => 'list_text',
      'label' => t('Gender'),
      'widget' => [
        'type' => 'options_buttons',
      ],
      'settings' => [
        'allowed_values' => [
          'MALE' => t('Male'),
          'FEMALE' => t('Female'),
        ],
      ],
    ];

    $fields['phone_number'] = [
      'type' => 'text',
      'label' => t('Phone number'),
    ];

    $fields['birth_date'] = [
      'type' => 'datestamp',
      'label' => t('Date of birth'),
      'settings' => [
        // By OpenInvoice specification you must be older than 18 year.
        'year_range' => '-100:-18',
        'granularity' => drupal_map_assoc(['year', 'month', 'day']),
      ],
      'widget' => [
        'input_format' => 'd/m/Y',
      ],
    ];

    $fields['social_number'] = [
      'type' => 'text',
      'label' => t('Social security number'),
    ];

    foreach ($fields as $name => $info) {
      $fields[$name] += [
        'required' => TRUE,
        'cardinality' => 1,
        'translatable' => FALSE,
      ];
    }

    return $fields;
  }

  /**
   * {@inheritdoc}
   */
  public function checkoutForm(\EntityDrupalWrapper $billing) {
    $fields = $this->checkoutFields();
    $form = [];

    $form['gender'] = [
      '#type' => 'radios',
      '#options' => $fields['gender']['settings']['allowed_values'],
    ];

    $form['phone_number'] = [
      '#type' => 'textfield',
    ];

    $form['birth_date'] = static::element('date_select', [
      '#process' => [[static::class, 'processTimestamp']],
      '#date_format' => $fields['birth_date']['widget']['input_format'],
      '#date_year_range' => $fields['birth_date']['settings']['year_range'],
    ]);

    $form['social_number'] = [
      '#type' => 'textfield',
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public static function processTimestamp(array $element) {
    // Since entity metadata does not allow us to have dates even with "date"
    // type for Drupal field definition then we will use timestamp and convert
    // it into the date here.
    $element['#default_value'] = date(DATE_FORMAT_DATE, $element['#default_value']);

    return $element;
  }

  /**
   * {@inheritdoc}
   */
  public function checkoutFormValidate(array $form, array &$values, \EntityDrupalWrapper $billing) {
    // Entity metadata accept only UNIX timestamps for "date" type.
    // @see entity_property_verify_data_type()
    $values['birth_date'] = strtotime($values['birth_date']);

    return TRUE;
  }

  /**
   * Fully build renderable element.
   *
   * @param string $type
   *   Type of element.
   * @param array $data
   *   Custom properties of element definition.
   *
   * @return array
   *   Renderable element definition.
   */
  private static function element($type, array $data = []) {
    $element_info = element_info($type);

    if (!empty($element_info)) {
      // Get all properties of an element of array type.
      $arrays = array_filter($data, 'is_array');
      // Override every property which is not an array.
      $element_info = array_merge($element_info, array_diff_key($data, $arrays));
      // Recursively merge every property of array type.
      $element_info = array_merge_recursive($arrays, $element_info);
      // Do not call the "element_info()" function during building the form.
      // @see form_builder()
      $element_info['#defaults_loaded'] = TRUE;
    }

    return $element_info;
  }

}
